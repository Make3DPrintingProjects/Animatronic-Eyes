#include <Adafruit_PWMServoDriver.h>
#include <Wire.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define RLRoffset 25       // Right Eye Left-Right Offset
#define RUDoffset 5        // Right Eye Up-Down Offset
#define RTLoffset 5        // Right Eye Top Lid Offset
#define RBLoffset 10        // Right Eye Bottom Lid Offset

#define LLRoffset -5       // Left Eye Left-Right Offset
#define LUDoffset 8        // Left Eye Up-Down Offset
#define LTLoffset 8        // Left Eye Top Lid Offset
#define LBLoffset 40        // Left Eye Bottom Lid Offset

//--------Receiver Pins--------
const uint8_t LR    = 2;   // Rx's Channel 1 / Aileron
const uint8_t UD    = 3;   // Rx's Channel 2 / Elevator
const uint8_t Lids  = 4;   // Rx's Channel 3 / Throttle
const uint8_t Blink = 5;   // Rx's Channel 5 / Gear

//--------Eye-Ball Pins--------

//Right Eye
uint8_t Right_LR = 1;      // Shortest servo lead on right eye
uint8_t Right_UD = 2;      // Second shortest servo lead on right eye
uint8_t Right_Btm_Lid = 3; // Second longest servo lead on right eye
uint8_t Right_Top_Lid = 4; // Longest servo lead on right eye

//Left Eye
uint8_t Left_LR = 5;       // Shortest servo lead on left eye
uint8_t Left_UD = 6;       // Second shortest servo lead on left eye
uint8_t Left_Btm_Lid = 7;  // Second longest servo lead on left eye
uint8_t Left_Top_Lid = 8;  // Longest servo lead on left eye


//----Variables-----
double Duration_Lids;      // Value of Rx's Throttle pulse, used for Eye-lid movement
double Duration_Lids_TR;
double Duration_Lids_BR;
double Duration_Lids_TL;
double Duration_Lids_BL;
double Duration_LR;        // Value of Rx's Aileron pulse, used for Left-Right movement
double Duration_UD;        // Value of Rx's Elevator pulse, used for Up-Down movement
double Duration_UD_R;
double Duration_UD_L;
double Duration_Blink;     // Value of Rx's Gear pulse, used to determine when to blink
int Current_Blink;         // Used to store current value of Duration_Blink
int Last_Blink;            // Used to store the value of Current_Blink the last time through cycle


void setup()
{
  // Sets the Arduino pins connected to your Rx as inputs
  pinMode(Lids, INPUT);
  pinMode(LR, INPUT);
  pinMode(UD, INPUT);
  pinMode(Blink, INPUT);

  pwm.begin();         // Starts the servo driver
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates, and that's what we are using

  // Gets and sets the current value of your Rx's Gear channel.  This makes sure your eyes don't
  // blink out of control on startup
  Duration_Blink = pulseIn(Blink, HIGH);
  Current_Blink = constrain(Duration_Blink, 1100, 1800);
  Last_Blink = Current_Blink;
}

void loop()
{
  Read_Signals();         // Gets the incoming signals
  ConstrainMapSignals();  // Maps the signals to the appropriate ranges for us to use
  Mixing();               // Mixes the Duration_Lids signal so your eyelids move with your eyes
  Blink_Check();          // Checks the Current_Blink value to see if you wnat your eyes to blink
  Set_Servos();           // Sets the servos based on the current inputs
}

void Read_Signals()
{
  Duration_Lids = pulseIn(Lids, HIGH);
  Duration_LR = pulseIn(LR, HIGH);
  Duration_UD = pulseIn(UD, HIGH);
  Duration_Blink = pulseIn(Blink, HIGH);
}

void ConstrainMapSignals()
{
  Duration_Lids = constrain(Duration_Lids, 1080, 1880);
  Duration_Lids_TR = map(Duration_Lids, 1080, 1880, 400, 320);
  Duration_Lids_BR = map(Duration_Lids, 1080, 1880, 320, 380);
  Duration_Lids_TL = map(Duration_Lids, 1080, 1880, 320, 400);
  Duration_Lids_BL = map(Duration_Lids, 1080, 1880, 380, 320);

  Duration_LR = constrain(Duration_LR, 1080, 1880);
  Duration_LR = map(Duration_LR, 1080, 1880, 290, 420);

  Duration_UD = constrain(Duration_UD, 1080, 1880);
  Duration_UD_R = map(Duration_UD, 1080, 1880, 430, 270);
  Duration_UD_L = map(Duration_UD, 1080, 1880, 270, 430);

  Current_Blink = constrain(Duration_Blink, 1100, 1800);
}

void Mixing()
{
  int i = Duration_UD_R - 350;
  i = map(i, 0, 80, 0, 40);
  constrain(i, 0, 40);

  Duration_Lids_TR = Duration_Lids_TR + i;
  Duration_Lids_BR = Duration_Lids_BR + i;

  int a = Duration_UD_L - 350;
  a = map(a, 0, 80, 0, 40);
  constrain(a, 0, 40);

  Duration_Lids_TL = Duration_Lids_TL + a;
  Duration_Lids_BL = Duration_Lids_BL + a;
}

void Blink_Check()
{
  if (Current_Blink != Last_Blink)
  {
    pwm.setPin(Right_Top_Lid, 400);
    pwm.setPin(Right_Btm_Lid, 320);

    pwm.setPin(Left_Top_Lid, 320);
    pwm.setPin(Left_Btm_Lid, 400);

    delay(100);

    pwm.setPin(Right_Top_Lid, (Duration_Lids_TR + RTLoffset));
    pwm.setPin(Right_Btm_Lid, (Duration_Lids_BR + RBLoffset));

    pwm.setPin(Left_Top_Lid, (Duration_Lids_TL + LTLoffset));
    pwm.setPin(Left_Btm_Lid, (Duration_Lids_BL + LBLoffset));
  }
  Last_Blink = Current_Blink;
}


void Set_Servos()
{
  // Right Eye
  pwm.setPin(Right_Top_Lid, (Duration_Lids_TR + RTLoffset));
  pwm.setPin(Right_Btm_Lid, (Duration_Lids_BR + RBLoffset));
  pwm.setPin(Right_LR, (Duration_LR + RLRoffset));
  pwm.setPin(Right_UD, (Duration_UD_R + RUDoffset));

  // Left Eye
  pwm.setPin(Left_Top_Lid, Duration_Lids_TL + LTLoffset);
  pwm.setPin(Left_Btm_Lid, Duration_Lids_BL + LBLoffset);
  pwm.setPin(Left_LR, (Duration_LR + LLRoffset));
  pwm.setPin(Left_UD, (Duration_UD_L + LUDoffset));
}

