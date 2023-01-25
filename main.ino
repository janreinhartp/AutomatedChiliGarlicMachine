// Declaration of Libraries
// LCD
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
bool refreshScreen = false;

// Button
#include <Button.h>
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
const int numOfTestMenu = 11;
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
  {"SAVE TIMERS", "ENTER TO SAVE"}};

String TestMenuScreen[numOfTestMenu] = {
  "GRINDER",
  "MIXER COOKER",
  "HEATER",
  "COOKER DOOR",
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


// LOGO MACKY
byte chr[1][8][8] = {
    {{B11100, B11110, B11111, B11011, B11001, B11000, B11010, B11011},
     {B00001, B00011, B00011, B10001, B11000, B11101, B01101, B00001},
     {B10000, B11000, B11000, B10001, B00011, B10111, B10110, B10000},
     {B00111, B01111, B11111, B11011, B00011, B00011, B01011, B10011},
     {B11010, B11010, B11010, B11010, B11010, B11010, B11010, B11011},
     {B10001, B01001, B00101, B00001, B00001, B11101, B00101, B11101},
     {B10001, B10010, B10100, B10100, B10111, B10000, B10000, B10111},
     {B00011, B00011, B00011, B00011, B11011, B01011, B01011, B11011}}};

byte enterChar[] = {0x10, 0x10, 0x10, 0x14, 0x16, 0x1F, 0x06, 0x04};
byte fastChar[] = {0x04, 0x0E, 0x1F, 0x00, 0x04, 0x0E, 0x1F, 0x00};

// Function Intro
void LogoTitlePrint(){
   // LCD Setup
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("*------------------*");
  lcd.setCursor(0, 1);
  lcd.print("|");
  lcd.setCursor(2, 1);

  lcd.print("AUTOMATED CHILI");
  lcd.setCursor(19, 1);
  lcd.print("|");
  lcd.setCursor(0, 2);
  lcd.print("|");
  lcd.setCursor(2, 2);
  lcd.print("GARLIC MACHINE");
  lcd.setCursor(19, 2);
  lcd.print("|");

  lcd.setCursor(0, 3);
  lcd.print("*------------------*");
  delay(2000);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("*------------------*");
  lcd.setCursor(0, 1);
  lcd.print("|");
  lcd.setCursor(3, 1);
  lcd.print("FABRICATED AND");
  lcd.setCursor(19, 1);
  lcd.print("|");
  lcd.setCursor(0, 2);
  lcd.print("|");
  lcd.setCursor(3, 2);
  lcd.print("PROGRAMMED BY:");
  lcd.setCursor(19, 2);
  lcd.print("|");
  lcd.setCursor(0, 3);
  lcd.print("*------------------*");

  delay(2000);
  int x = 7;
  for (int f = 0; f < 1; f++)
  {
    lcd.clear();
    for (int i = 0; i < 8; i++)
    {
        lcd.createChar(i, chr[f][i]);
    }
    for (int c = 0; c < 4; c++)
    {
      int xc = x + c;
      if ((xc >= 0) && (xc < 16))
      {
          lcd.setCursor(x + c, 0);
          lcd.write(byte(c));
          lcd.setCursor(x + c, 1);
          lcd.write(byte(c + 4));
      }
    }
    lcd.setCursor(2, 2);
    lcd.print("MACKY INDUSTRIAL");
    lcd.setCursor(0, 3);
    lcd.print("AND GENERAL SERVICES");
  }
  lcd.clear();
}


//Functions for Rotary Encoder
void timerIsr(){
  encoder->service();
}

void readRotaryEncoder(){
  value += encoder->getValue();
  
  if (value/2 > last) {
    last = value/2;
    down = true;
    //delay(100);
  }else   if (value/2 < last) {
    last = value/2;
    up = true;
    //delay(100);
  }
}

void readButtonEncoder(){
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

void printScreen(){

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
            lcd.print("Click to Edit");
        }
        else
        {
            lcd.print("Click to Save");
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
    }else{
      lcd.clear();
      lcd.print(settings[currentSettingScreen][0]);
    }
  }
  else if (runAutoFlag == true)
  {
    /* code */
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

void inputCommands(){
  // LCD Change Function and Values
  //  To Right Rotary
  if (up == 1)
  {
    up = false;
    refreshScreen = true;
    if (settingsFlag == true)
    {
      if (currentSettingScreen == 1 && settingTimerFlag == true){
        if(settingEditTimerFlag == true){
          if (parametersTimer[currentTimerScreen] >= parametersTimerMaxValue[currentTimerScreen] - 1)
          {
              parametersTimer[currentTimerScreen] = parametersTimerMaxValue[currentTimerScreen];
          }
          else
          {
            if (fastScroll == true)
            {
                parametersTimer[currentSettingScreen] += 1;
            }
            else
            {
                parametersTimer[currentSettingScreen] += 0.1;
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
      }else if (currentSettingScreen == 2 && settingEditSpeedFlag == true){
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
      if (currentSettingScreen == 1 && settingTimerFlag == true){
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
      }else if (currentSettingScreen == 2 && settingEditSpeedFlag == true){
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
        if (settingTimerFlag == true)
        {
          Serial.println("Test Click Inside Setting Edit Timer");
          if(currentTimerScreen == 5){
            settingTimerFlag = false;
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
        if (settingsSpeedFlag == true)
        {
          if(currentSpeedScreen == 3){
            settingsSpeedFlag = false;
          }else{
            if(settingEditSpeedFlag == true){
              settingEditSpeedFlag = false;
            }else{
              settingsSpeedFlag = true;
            }
          }
        }else{
          settingsSpeedFlag = true;
        }
      }
      else{
        settingsFlag = false;
      }
    }
    else if (runAutoFlag == true)
    {
      runAutoFlag = false;
    }
    else if (testMenuFlag == true)
    {
        
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
  encoder = new ClickEncoder(2,3, 4);
  encoder->setAccelerationEnabled(false);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
  last = encoder->getValue();

  // LCD Setup
  lcd.init();
  lcd.clear();
  lcd.backlight();
  LogoTitlePrint();
  lcd.createChar(0, enterChar);
  lcd.createChar(1, fastChar);
  refreshScreen = true;

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

    Serial.println("Current Screen: " + String(currentScreen));
    Serial.println("Current Setting: " + String(currentSettingScreen));
    Serial.println("Current Setting Timer: " + String(currentTimerScreen));
    Serial.println("Current Setting Speed: " + String(currentSpeedScreen));


    Serial.println("Current Setting STAT: " + String(settingsFlag));
    Serial.println("Current Setting Timer Stat: " + String(settingTimerFlag));
    Serial.println("Current Setting Timer Edit Stat: " + String(settingEditTimerFlag));
  }
    
}