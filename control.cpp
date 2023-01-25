#include "control.h"

void RunRelay::Run(){
  // check to see if it's time to change the state of the LED
  unsigned long currentMillisecond = millis();
  if( currentMillisecond - previousMillisecond <= onTime)
  {
    if (runState != 1)
    {
      this->RunRelayOn();  
      secRemaining = ((onTime-(currentMillisecond - previousMillisecond))/1000);
      state = true;
    }else{
      this->RunRelayOff();
    }
  }
  else
  {
    this->RunRelayOff();
    state = false;
    runState = 0;
  }
}

void RunRelay::UpdateTimer(long time){
   onTime = time;
}

void RunRelay::setRunStopState(int state){
   runState = state;
}

void RunRelay::ResetTimer(){
    unsigned long currentMillisecond = millis();
    previousMillisecond = currentMillisecond;
    runState = 0;
}

void RunRelay::setPwmSpeed(int pwm){
   pwmSpeed = pwm;
}

bool RunRelay::getRunState(){
   return state;
}
int RunRelay::getSecRemaining(){
  return secRemaining;
}
void RunRelay::updateState(bool stat){
  state=stat;
}
void RunRelay::RunRelayOn(){
  digitalWrite(relay, LOW);
  digitalWrite(vfd, LOW);
}
void RunRelay::RunRelayOff(){
  digitalWrite(relay, HIGH);
  digitalWrite(vfd, HIGH);
}
void RunRelay::RunRelayOnWithSpeed(int pin){
  digitalWrite(relay, LOW);
  digitalWrite(vfd, LOW);
  analogWrite(vfdSpeed, pwmSpeed);
}
void RunRelay::RunRelayOffWithSpeed(){
  digitalWrite(relay, HIGH);
  digitalWrite(vfd, HIGH);
  analogWrite(vfdSpeed, 0);
}

void RunRelay::runSpeed(){
  analogWrite(vfdSpeed, pwmSpeed);
}

void Sensor::update() {
    // You can handle the debounce of the button directly
    // in the class, so you don't have to think about it
    // elsewhere in your code
    byte newReading = digitalRead(sensor);
    
    if (newReading != lastReading) {
      lastDebounceTime = millis();
    }
    if (millis() - lastDebounceTime > debounceDelay) {
      // Update the 'state' attribute only if debounce is checked
      state = newReading;
    }
    lastReading = newReading;
}

byte Sensor::getState(){
  update();
  return state;
}

bool Sensor::isPressed() {
  return (getState() == LOW);
}
