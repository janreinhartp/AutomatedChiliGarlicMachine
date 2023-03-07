#include <string.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  String time = "123456";
  setTimer(time);
}

void loop() {
  // put your main code here, to run repeatedly:
  
}


void setTimer(String timer){
  uint16_t h = atoi (timer.substring(0,2).c_str());
  uint8_t m = atoi (timer.substring(2,4).c_str());
  uint8_t s = atoi (timer.substring(4,6).c_str());
  Serial.println(h);
  Serial.println(m);
  Serial.println(s);
}
