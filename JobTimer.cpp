#include "JobTimer.h"

JobTimer::JobTimer()
{
  _previousMillis = 0;
  _currentCountTime = 0;
  _countTime = 0;
  _isCounterCompleted = false;
  _isStopped = true;
  _startCountTime = 0;
}

JobTimer::~JobTimer()
{
}

void JobTimer::setTimer(uint16_t hours, uint8_t minutes, uint8_t seconds)
{
  if (hours > COUNTIMER_MAX_HOURS) {
    hours = COUNTIMER_MAX_HOURS;
  }

  if (minutes > COUNTIMER_MAX_MINUTES_SECONDS) {
    minutes = COUNTIMER_MAX_MINUTES_SECONDS;
  }

  if (seconds > COUNTIMER_MAX_MINUTES_SECONDS) {
    seconds = COUNTIMER_MAX_MINUTES_SECONDS;
  }

  _currentCountTime = ((hours * 3600L) + (minutes * 60L) + seconds) * 1000L;
  _countTime = _currentCountTime;

  _startCountTime = _currentCountTime;

}

void JobTimer::start()
{
  _isStopped = false;
  if(_isCounterCompleted)
    _isCounterCompleted = false;
}

void JobTimer::stop()
{
  _isStopped = true;
  _isCounterCompleted = true;
  _currentCountTime = _countTime;
}

void JobTimer::run()
{
  // timer is running only if is not completed or not stopped.
  if (_isCounterCompleted || _isStopped)
    return;

  if (millis() - _previousMillis >= _interval) {
        countDown();
    _previousMillis = millis();
  }
}

bool JobTimer::isTimerCompleted()
{
  return _isCounterCompleted;
}

bool JobTimer::isStopped()
{
  return _isStopped;
}

void JobTimer::countDown()
{
  if (_currentCountTime > 0)
  {
    _currentCountTime -= _interval;
  }
  else
  {
    stop();
  }
}

char* JobTimer::getCurrentTime()
{
  sprintf(_formatted_time, "%02d:%02d:%02d", getCurrentHours(), getCurrentMinutes(), getCurrentSeconds());
  return _formatted_time;
}

uint16_t JobTimer::getCurrentHours()
{
  return _currentCountTime / 1000 / 3600;
}

uint8_t JobTimer::getCurrentMinutes()
{
  return _currentCountTime / 1000 % 3600 / 60;
}

uint8_t JobTimer::getCurrentSeconds()
{
  return _currentCountTime / 1000 % 3600 % 60 % 60;
}