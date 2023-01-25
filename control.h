#include <Arduino.h>
class RunRelay {
    private:
    int relay;
    long onTime;
    int secRemaining;
    unsigned long previousMillisecond;
    int vfd;
    int vfdSpeed;
    int pwmSpeed;
    bool state;
    int runState;

    // Constructor
    public:
    RunRelay(int pin, long on, int pinVfd, int pinVfdSpeed, int pwmSpeed, bool stat)
    {
      relay = pin;
      vfd = pinVfd;
      onTime = on;
      vfdSpeed = pinVfdSpeed;
      pwmSpeed = pwmSpeed;
      state = stat;
      pinMode(relay, OUTPUT);
      digitalWrite(relay, HIGH);
      pinMode(vfd, OUTPUT);
      digitalWrite(vfd, HIGH);
      pinMode(vfdSpeed, OUTPUT);     
      previousMillisecond = 0;
    }

    void Run();
    void UpdateTimer(long time);
    void ResetTimer();
    void setPwmSpeed(int pwm);
    bool getRunState();
    int getSecRemaining();
    void updateState(bool stat);
    void RunRelayOn();
    void RunRelayOff();
    void RunRelayOnWithSpeed(int pwm);
    void RunRelayOffWithSpeed();
    void runSpeed();
    void setRunStopState(int state);
};

class Sensor {
    private:
    int sensor;
    bool state;
    byte lastReading;
    unsigned long lastDebounceTime = 0;
    unsigned long debounceDelay = 50;

    public:
    Sensor(int pin){
      sensor = pin;
      pinMode(sensor, INPUT_PULLUP);
    }

    void update();
    byte getState();
    bool isPressed();
};
