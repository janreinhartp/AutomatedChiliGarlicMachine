#pragma once

#ifndef JOBTIMER_H
#define JOBTIMER_H

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif


#define COUNTIMER_MAX_HOURS 999
#define COUNTIMER_MAX_MINUTES_SECONDS 59


class JobTimer
{
private:
    uint16_t hours;
    uint8_t minutes;
    uint8_t seconds;

    bool _isCounterCompleted;
    bool _isTimerCompleted;
    bool _isStopped;
    uint32_t _countTime;
    uint32_t _currentCountTime;
    uint32_t _startCountTime;
  
    uint32_t _interval = 1000;
    uint32_t _previousMillis;

    void countDown();
    void complete();

    char _formatted_time[10];
    
public:
    JobTimer();
    ~JobTimer();
    void setTimer(uint16_t hours, uint8_t minutes, uint8_t seconds);
    void run();
    void start();
    void stop();
    void restart();
    bool isStopped();

    bool isTimerCompleted();
    
    char* getCurrentTime();
    uint16_t getCurrentHours();
    uint8_t getCurrentMinutes();
    uint8_t getCurrentSeconds();
};
#endif