// Declaration of Libraries
// LCD
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
bool refreshScreen = false;

// Button
#include "JobTimer.h"
#include "control.h"
// Encoder
#include <ClickEncoder.h>
// Timer 1 for encoder
#include <TimerOne.h>
//Save Function
#include <EEPROMex.h>

//Declaration of Variables
//Rotary Encoder Variables
boolean up = false;
boolean down = false;
boolean middle = false;
ClickEncoder *encoder;
int16_t last, value;

// Declaration of LCD Variables
const int numOfMainScreens = 3;
const int numOfSettingScreens = 3;
const int numOfTimers = 6;
const int numOfSpeeds = 4;
const int numOfTestMenu = 9;
int currentScreen = 0;
int currentSpeedScreen = 0;
int currentTimerScreen = 0;
int currentSettingScreen = 0;
int currentTestMenuScreen = 0;

String screens[numOfMainScreens][2] = {
  {"SETTINGS", "ENTER TO EDIT"},
  {"RUN AUTO", "ENTER TO RUN AUTO"},
  {"TEST MACHINE", "ENTER TO TEST"}};

String settings[numOfSettingScreens][2] = {
  {"TIMER", "ENTER TO EDIT"},
  {"MOTOR SPEED", "ENTER TO EDIT"},
  {"SAVE SETTINGS", "ENTER TO SAVE"}};

String timers[numOfTimers][2] = {
  {"GRINDING", "MIN"},
  {"COOKING", "MIN"},
  {"DOOR", "SEC"},
  {"COOLING", "MIN"},
  {"DISPENSING", "SEC"},
  {"SAVE TIMERS", "ENTER TO SAVE"}};

String speeds[numOfTimers][2] = {
  {"MIXER / COOKER", "HZ"},
  {"MIXER DISPENSER", "HZ"},
  {"CONVEYOR ", "HZ"},
  {"SAVE SPEED", "ENTER TO SAVE"}};

String TestMenuScreen[numOfTestMenu] = {
  "GRINDER",
  "MIXER COOKER",
  "HEATER",
  "COOKER DOOR",
  "COOLER MIXER",
  "COOLING FAN",
  "DISPENSE",
  "CONVEYOR",
  "BACK TO MAIN MENU"};

double parametersTimer[numOfTimers] = {0, 0, 0, 0, 0};
double parametersTimerMaxValue[numOfTimers] = {120, 120, 60, 60, 4};

double parametersSpeed[numOfSpeeds] = {0, 0, 0};
double parametersSpeedMaxValue[numOfSpeeds] = {120, 120, 60};

bool settingsFlag = false;

bool settingTimerFlag = false;
bool settingEditTimerFlag = false;

bool settingsSpeedFlag = false;
bool settingEditSpeedFlag = false;

bool batchFormulaFlag = false;
bool batchEditFlag = false;
bool motorSpeedFlag = false;
bool motorSpeedEditFlag = false;
bool runAutoFlag = false;
bool testMenuFlag = false;

// Fast Scroll
bool fastScroll = false;

// RELAY DECLARATION
Control Grinder(33, 44, 8);
Control Mixer(32, 47, 8);
Control Heater(35, 44, 8);
Control MixerDoorOpen(34, 33, 8);
Control MixerDoorClose(37, 33, 8);
Control MixerCooler(36, 33, 8);
Control CoolingFan(38, 33, 8);
Control Valve(39, 33, 8);
Control Conveyor(41, 33, 8);

Control TimerDispense(0,0,0);

void setRelays(){
  Grinder.setTimer("000005");
  Mixer.setTimer("000005");
  MixerCooler.setTimer("000005");
  MixerDoorClose.setTimer("000005");
  MixerDoorOpen.setTimer("000005");
  TimerDispense.setTimer("000005");
}


unsigned long previousMillis = 0;  // will store last time LED was updated
// constants won't change:
const long interval = 1000;  // interval at which to blink (milliseconds)
// RUN AUTO FLAG
int RunAutoCommand = 0;
  /*
  RunAutoFlag =  Task
  0           =  Close Door
  1           =  Grinding
  2           =  Mixing / Cooking
  3           =  Open Door
  4           =  DispenseFromCooker
  5           =  Cooling
  6           =  Dispensing
  */

void RunAuto(){
  switch (RunAutoCommand)
  {
  case 0:
    runCloseMixerDoor();
    break;
  case 1:
    runGrinder();
    break;
  case 2:
    runCooking();
    break;
  case 3:
    runOpenMixerDoor();
    break;
  case 4:
    runDispenseFromCooker();
    break;
  case 5:
    runCooling();
    break;
  case 6:
    //Dispense
    break;
  default:
    runAutoFlag = false;
    refreshScreen = true;
    break;
  }
}

void runCloseMixerDoor(){
  MixerDoorClose.run();
  if(MixerDoorClose.isTimerCompleted() == true){
    RunAutoCommand = 1;
    Grinder.start();
  }
}

void runGrinder(){
  Grinder.run();
  if (Grinder.isTimerCompleted() == true)
  {
    RunAutoCommand = 2;
    Mixer.relayOff();
    Heater.relayOff();
    Mixer.start();
  }
  else
  {
    Mixer.relayOn();
    Heater.relayOn();
  }
}


void runCooking(){
  Mixer.run();
  if (Mixer.isTimerCompleted() == true)
  {
    RunAutoCommand = 3;
    Heater.relayOff();
    MixerDoorOpen.start();
  }
  else
  {
    Heater.relayOn();
  }
}

void runOpenMixerDoor(){
  MixerDoorOpen.run();
  if(MixerDoorOpen.isTimerCompleted() == true){
    RunAutoCommand = 4;
    TimerDispense.start();
  }
}

void runDispenseFromCooker(){
  TimerDispense.run();
  if(TimerDispense.isTimerCompleted() == true){
    Mixer.relayOff();
    RunAutoCommand = 5;
    MixerCooler.start();
  }else{
    Mixer.relayOn();
  }
}

void runCooling(){
  MixerCooler.run();
  if (MixerCooler.isTimerCompleted() == true)
  {
    CoolingFan.relayOff();
    RunAutoCommand = -1;
  }
  else
  {
    CoolingFan.relayOn();
  }
}


void PrintRunAuto(String job, char* time){
  lcd.clear();
  lcd.print("Running Auto");
  lcd.setCursor(0,1);
  lcd.print("Status: " + job);
  lcd.setCursor(0,2);
  lcd.print("Timer: ");
  lcd.setCursor(7,2);
  lcd.print(time);
}




//Functions for Rotary Encoder
void timerIsr()
{
  encoder->service();
}

void readRotaryEncoder()
{
  value += encoder->getValue();
  
  if (value/2 > last) {
    last = value/2;
    down = true;
    delay(100);
  }else   if (value/2 < last) {
    last = value/2;
    up = true;
    delay(100);
  }
}

void readButtonEncoder()
{
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {  // Open Bracket for Click
    switch (b) { // Open Bracket for Double Click
      case ClickEncoder::Clicked: 
          middle=true; 
      break;

      case ClickEncoder::DoubleClicked:
      break;

    } 
  }
}

void printScreen()
{

  if (settingsFlag == true)
  {
    if(settingsFlag == true && settingTimerFlag == true){
      lcd.clear();
      lcd.print(timers[currentTimerScreen][0]);
      lcd.setCursor(0, 1);
      if(currentTimerScreen == 5){
        lcd.setCursor(0, 3);
        lcd.write(0);
        lcd.setCursor(2, 3);
        lcd.print("Click to Save All");
      }else{
        lcd.setCursor(0, 1);
        lcd.print(parametersTimer[currentTimerScreen]);
        lcd.print(" ");
        lcd.print(timers[currentTimerScreen][1]);
        lcd.setCursor(0, 3);
        lcd.write(0);
        lcd.setCursor(2, 3);
        if (settingEditTimerFlag == false)
        {
            lcd.print("ENTER TO EDIT");
        }
        else
        {
            lcd.print("ENTER TO SAVE");
        }
        lcd.setCursor(19, 3);
        if (fastScroll == true)
        {
            lcd.write(1);
        }
      }
    }else if(settingsFlag == true && settingsSpeedFlag == true){
      lcd.clear();
      lcd.print(speeds[currentSpeedScreen][0]);
      if(currentSpeedScreen == 3){
        lcd.setCursor(0, 3);
        lcd.write(0);
        lcd.setCursor(2, 3);
        lcd.print("CLICK TO SAVE ALL");
      }else{
        lcd.setCursor(0, 1);
        lcd.print(parametersSpeed[currentSpeedScreen]);
        lcd.print(" ");
        lcd.print(speeds[currentTimerScreen][1]);
        lcd.setCursor(0, 3);
        lcd.write(0);
        lcd.setCursor(2, 3);
        if (settingEditSpeedFlag == false)
        {
            lcd.print("ENTER TO EDIT");
        }
        else
        {
            lcd.print("ENTER TO SAVE");
        }
        lcd.setCursor(19, 3);
        if (fastScroll == true)        {
            lcd.write(1);
        }
      }
    }else{
      lcd.clear();
      lcd.print(settings[currentSettingScreen][0]);
      if(currentSettingScreen == 2){
          lcd.setCursor(0, 2);
        lcd.write(0);
        lcd.setCursor(2, 2);
        lcd.print("ENTER TO SAVE ALL");
        lcd.setCursor(2, 3);
        lcd.print("SETTINGS");
      }else{
        lcd.setCursor(0, 3);
        lcd.write(0);
        lcd.setCursor(2, 3);
        lcd.print("ENTER TO EDIT");
      }
    }
  }
  else if (runAutoFlag == true)
  {
    if (runAutoFlag == true && RunAutoCommand == 0)
    {
      PrintRunAuto("Closing Door", MixerDoorClose.getTimeRemaining());
    }
    else if (runAutoFlag == true && RunAutoCommand == 1)
    {
      PrintRunAuto("Grinding", Grinder.getTimeRemaining());
    }
    else if(runAutoFlag == true && RunAutoCommand == 2)
    {
      PrintRunAuto("Cooking", Mixer.getTimeRemaining());
    }
    else if(runAutoFlag == true && RunAutoCommand == 3)
    {
      PrintRunAuto("Opening Door", MixerDoorOpen.getTimeRemaining());
    }
    else if(runAutoFlag == true && RunAutoCommand == 4)
    {
      PrintRunAuto("To Cooler", TimerDispense.getTimeRemaining());
    }
    else if(runAutoFlag == true && RunAutoCommand == 5)
    {
      PrintRunAuto("Cooling", MixerCooler.getTimeRemaining());
    }
    
  }
  else if (testMenuFlag == true)
  {
    lcd.clear();
    lcd.print(TestMenuScreen[currentTestMenuScreen]);

    if (currentTestMenuScreen == 10)
    {
      lcd.setCursor(0, 3);
      lcd.print("Click to Exit Test");
    }
    else
    {
      lcd.setCursor(0, 3);
      lcd.print("Click to Run Test");
    }
  }
  else
  {
    lcd.clear();
    lcd.print(screens[currentScreen][0]);
    lcd.setCursor(0, 3);
    lcd.write(0);
    lcd.setCursor(2, 3);
    lcd.print(screens[currentScreen][1]);
    refreshScreen = false;
  }
}



void inputCommands()
{
  // LCD Change Function and Values
  //  To Right Rotary
  if (up == 1)
  {
    up = false;
    refreshScreen = true;
    if (settingsFlag == true)
    {
      if (currentSettingScreen == 0 && settingTimerFlag == true){
        if(settingEditTimerFlag == true){
          if (parametersTimer[currentTimerScreen] >= parametersTimerMaxValue[currentTimerScreen] - 1)
          {
              parametersTimer[currentTimerScreen] = parametersTimerMaxValue[currentTimerScreen];
          }
          else
          {
            if (fastScroll == true)
            {
                parametersTimer[currentTimerScreen] += 1;
            }
            else
            {
                parametersTimer[currentTimerScreen] += 0.1;
            }
          }
        }else{
          if (currentTimerScreen == numOfTimers - 1)
          {
              currentTimerScreen = 0;
          }
          else
          {
              currentTimerScreen++;
          }
        }
      }else if (currentSettingScreen == 1 && settingsSpeedFlag == true){
        if(settingEditSpeedFlag == true){
          if (parametersSpeed[currentSpeedScreen] >= parametersSpeedMaxValue[currentSpeedScreen] - 1)
          {
              parametersSpeed[currentSpeedScreen] = parametersSpeedMaxValue[currentSpeedScreen];
          }
          else
          {
            if (fastScroll == true)
            {
                parametersSpeed[currentSpeedScreen] += 1;
            }
            else
            {
                parametersSpeed[currentSpeedScreen] += 0.1;
            }
          }
        }else{
          if (currentSpeedScreen == numOfSpeeds - 1)
          {
              currentSpeedScreen = 0;
          }
          else
          {
              currentSpeedScreen++;
          }
        }
      }
      else{
        if (currentSettingScreen == numOfSettingScreens - 1)
        {
            currentSettingScreen = 0;
        }
        else
        {
            currentSettingScreen++;
        }
      }
    }
    else if (testMenuFlag == true)
    {
        if (currentTestMenuScreen == numOfTestMenu - 1)
        {
            currentTestMenuScreen = 0;
        }
        else
        {
            currentTestMenuScreen++;
        }
    }
    else
    {
        if (currentScreen == numOfMainScreens - 1)
        {
            currentScreen = 0;
        }
        else
        {
            currentScreen++;
        }
    }
  }

  // To Left Rotary
  if (down == 1)
  {
    down = false;
    refreshScreen = true;
     if (settingsFlag == true)
    {
      if (currentSettingScreen == 0 && settingTimerFlag == true){
        if(settingEditTimerFlag == true){
          if (parametersTimer[currentTimerScreen] <= 0)
          {
              parametersTimer[currentTimerScreen] = 0;
          }
          else
          {
            if (fastScroll == true)
            {
                parametersTimer[currentTimerScreen] -= 1;
            }
            else
            {
                parametersTimer[currentTimerScreen] -= 0.1;
            }
          }
        }else{
          if (currentTimerScreen == 0)
          {
              currentTimerScreen = numOfTimers - 1;
          }
          else
          {
              currentTimerScreen--;
          }
        }
      }else if (currentSettingScreen == 1 && settingsSpeedFlag== true){
        if(settingEditSpeedFlag == true){
          if (parametersSpeed[currentSpeedScreen] <= 0 )
          {
              parametersSpeed[currentSpeedScreen] = 0;
          }
          else
          {
            if (fastScroll == true)
            {
                parametersSpeed[currentSpeedScreen] -= 1;
            }
            else
            {
                parametersSpeed[currentSpeedScreen] -= 0.1;
            }
          }
        }else{
          if (currentSpeedScreen == 0)
          {
              currentSpeedScreen = numOfSpeeds - 1;
          }
          else
          {
              currentSpeedScreen--;
          }
        }
      }
      else{
        if (currentSettingScreen == numOfSettingScreens - 1)
        {
            currentSettingScreen = 0;
        }
        else
        {
            currentSettingScreen++;
        }
      }
    }
    else if (testMenuFlag == true)
    {
        if (currentTestMenuScreen == numOfTestMenu - 1)
        {
            currentTestMenuScreen = 0;
        }
        else
        {
            currentTestMenuScreen++;
        }
    }
    else
    {
        if (currentScreen == 0)
        {
            currentScreen = numOfMainScreens - 1;
        }
        else
        {
            currentScreen--;
        }
    }
  }

  // Rotary Button Press
  if (middle == 1)
  {
    middle = false;
    refreshScreen = 1;
    if (currentScreen == 0 && settingsFlag == true)
    {
      if(currentSettingScreen == 0){
        if (currentSettingScreen == 0 && settingTimerFlag == true)
        {
          if(currentTimerScreen == 5 && settingTimerFlag == true){
            settingTimerFlag = false;
            currentTimerScreen = 0;
          }else{
            if(settingEditTimerFlag == true){
              settingEditTimerFlag = false;
            }else{
              settingEditTimerFlag = true;
            }
          }
        }else{
          settingTimerFlag = true;
        }
      }else if(currentSettingScreen == 1){
        if (currentSettingScreen == 1 && settingsSpeedFlag == true)
        {
          if(currentSpeedScreen == 3 && settingsSpeedFlag == true){
            settingsSpeedFlag = false;
            currentSpeedScreen = 0;
          }else{
            if(settingEditSpeedFlag == true){
              settingEditSpeedFlag = false;
            }else{
              settingEditSpeedFlag = true;
            }
          }
        }else{
          settingsSpeedFlag = true;
        }
      }
      else{
        settingsFlag = false;
        currentSettingScreen = 0;
      }
    }
    else if (runAutoFlag == true)
    {
      runAutoFlag = false;
    }
    else if (testMenuFlag == true)
    {
        if(currentTestMenuScreen == 8){
          testMenuFlag = false;
          currentTestMenuScreen = 0;
        }else if(currentTestMenuScreen == 0){
          if (Grinder.getMotorState() == false)
          {
            Grinder.relayOn();
          }
          else
          {
            Grinder.relayOn();
          }
          
        }else if(currentTestMenuScreen == 1){
          if (Mixer.getMotorState() == false)
          {
            Mixer.relayOn();
          }
          else
          {
            Mixer.relayOff();
          }
          
        }else if(currentTestMenuScreen == 2){
          if (Heater.getMotorState() == false)
          {
            Heater.relayOn();
          }
          else
          {
            Heater.relayOff();
          }
          
        }else if(currentTestMenuScreen == 3){
          if (MixerDoorOpen.getMotorState() == false)
          {
            MixerDoorOpen.relayOn();
          }
          else
          {
            MixerDoorOpen.relayOff();
          }
        }else if(currentTestMenuScreen == 4){
          if (MixerCooler.getMotorState() == false)
          {
            MixerCooler.relayOn();
          }
          else
          {
            MixerCooler.relayOff();
          }
        }else if(currentTestMenuScreen == 5){
          if (CoolingFan.getMotorState() == false)
          {
            CoolingFan.relayOn();
          }
          else
          {
            CoolingFan.relayOff();
          }
          
        }else if(currentTestMenuScreen == 6){
          if (Valve.getMotorState() == false)
          {
            Valve.relayOn();
          }
          else
          {
            Valve.relayOff();
          }
        }else if(currentTestMenuScreen == 7){
          if (Valve.getMotorState() == false)
          {
            Valve.relayOn();
          }
          else
          {
            Valve.relayOff();
          }
          
        }
    }
    else
    {
        if (currentScreen == 0)
        {
            settingsFlag = true;
        }
        else if (currentScreen == 1)
        {
            runAutoFlag = true;
            RunAutoCommand = 0;
            MixerDoorClose.start();
            refreshScreen = 1;
        }
        else if (currentScreen == 2)
        {
            testMenuFlag = true;
        }
    }
  }
}

void setup()
{
  //Encoder Setup
  encoder = new ClickEncoder(3,4, 2);
  encoder->setAccelerationEnabled(false);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
  last = encoder->getValue();

  // LCD Setup
  lcd.init();
  lcd.clear();
  lcd.backlight();
 // LogoTitlePrint();
  refreshScreen = true;

  setRelays();

  Serial.begin(9600);

}

void loop()
{
  readRotaryEncoder(); 
  readButtonEncoder();
  inputCommands();
  
  if (refreshScreen == true)
  {
    printScreen();
    refreshScreen = false;
  }

  if(runAutoFlag == true){
    RunAuto();
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;
    printScreen();
    }
  }
    
}
