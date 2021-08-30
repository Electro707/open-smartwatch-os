#ifdef E707_REV2_EDITION
#include <Arduino.h>
#include <RtcDateTime.h>
#include <RV-3028-C7.h>
#else
#include <RtcDS3231.h>
#endif
#include <Wire.h>
#include <config.h>
#include <osw_config.h>
#include <time.h>

#include <map>
#include <string>

#include "osw_hal.h"
#ifdef E707_REV2_EDITION
RV3028 Rtc;
#else
RtcDS3231<TwoWire> Rtc(Wire);
#endif

void OswHal::setupTime(void) {
  Wire.begin(SDA, SCL, 100000L);

#ifdef E707_REV2_EDITION
  if(Rtc.begin() == false){
    Serial.println("Error while beginning the RTC!!!!!");
    return;
  }
  Rtc.set24Hour();
  Rtc.clearInterrupts();
  Rtc.disableTrickleCharge();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Rtc.setTime(compiled.Second(), compiled.Minute(), compiled.Hour(), compiled.DayOfWeek(), compiled.Day(), compiled.Month(), compiled.Year());
  Rtc.updateTime();
#else

  Rtc.Begin();
  Rtc.Enable32kHzPin(false);
  if (!Rtc.LastError()) {
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

    if (!Rtc.IsDateTimeValid()) {
      Rtc.SetDateTime(compiled);
    }
    if (!Rtc.GetIsRunning()) {
      Serial.println("RTC was not actively running, starting now");
      Rtc.SetIsRunning(true);
    }
  }
#endif

  // how to register interrupts:
  // pinMode(RTC_INT, INPUT);
  // attachInterrupt(RTC_INT, isrAlarm, FALLING);
  // Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeAlarmBoth);
  // RtcDateTime now = Rtc.GetDateTime();
  // RtcDateTime alarmTime = now + 10;  // into the future
  // DS3231AlarmOne alarm1(alarmTime.Day(), alarmTime.Hour(), alarmTime.Minute(), alarmTime.Second(),
  //                       DS3231AlarmOneControl_HoursMinutesSecondsMatch);
  // Rtc.SetAlarmOne(alarm1);
}

bool OswHal::hasDS3231(void) { return getUTCTime() > 0; }

#ifdef E707_REV2_EDITION
uint32_t OswHal::getUTCTime(void) {
  bool e = Rtc.updateTime();
#ifdef DEBUG
  if(e == false){Serial.println("Error while reading RTC time");}
#endif
  RtcDateTime t = RtcDateTime(Rtc.getYear(), Rtc.getMonth(), Rtc.getWeekday(), Rtc.getHours(), Rtc.getMinutes(), Rtc.getSeconds());
  return t.Epoch32Time(); 
}
#else
uint32_t OswHal::getUTCTime(void) { return Rtc.GetDateTime().Epoch32Time(); }
#endif
uint32_t OswHal::getLocalTime(void) { return getUTCTime() + 3600 * OswConfigAllKeys::timeZone.get() + (long)(3600 * OswConfigAllKeys::daylightOffset.get()); }

void OswHal::setUTCTime(long epoch) {
  RtcDateTime t = RtcDateTime();
  t.InitWithEpoch32Time(epoch);
  #ifdef E707_REV2_EDITION
  Rtc.setTime(t.Second(), t.Minute(), t.Hour(), t.DayOfWeek(), t.Day(), t.Month(), t.Year());
  #else
  Rtc.SetDateTime(t);
  #endif
}

void OswHal::getUTCTime(uint32_t *hour, uint32_t *minute, uint32_t *second) {
  RtcDateTime d = RtcDateTime();
  d.InitWithEpoch32Time(getUTCTime());
  *hour = d.Hour();
  *minute = d.Minute();
  *second = d.Second();
}

void OswHal::getLocalTime(uint32_t *hour, uint32_t *minute, uint32_t *second) {
  RtcDateTime d = RtcDateTime();
  d.InitWithEpoch32Time(getLocalTime());
  if (!OswConfigAllKeys::timeFormat.get()) {
    if (d.Hour() > 12) {
      *hour = d.Hour() - 12;
    } else if (d.Hour() == 0) {
      *hour = 12;
    } else {
      *hour = d.Hour();
    }
  } else {
    *hour = d.Hour();
  }
  *minute = d.Minute();
  *second = d.Second();
}

void OswHal::getLocalTime(uint32_t *hour, uint32_t *minute, uint32_t *second, bool *afterNoon) {
  RtcDateTime d = RtcDateTime();
  d.InitWithEpoch32Time(getLocalTime());
  if (!OswConfigAllKeys::timeFormat.get()) {
    if (d.Hour() > 12) {
      *hour = d.Hour() - 12;
      *afterNoon = true;
    } else if (d.Hour() == 0) {
      *hour = 12;
      *afterNoon = false;
    } else if (d.Hour() == 12) {
      *hour = d.Hour();
      *afterNoon = true;
    } else {
      *hour = d.Hour();
      *afterNoon = false;
    }
  } else {
    *hour = d.Hour();
    *afterNoon = false;
  }
  *minute = d.Minute();
  *second = d.Second();
}

void OswHal::getDate(uint32_t *day, uint32_t *weekDay) {
  RtcDateTime d = RtcDateTime();
  d.InitWithEpoch32Time(getLocalTime());
  *weekDay = d.DayOfWeek();
  *day = d.Day();
}

void OswHal::getDate(uint32_t *day, uint32_t *month, uint32_t *year) {
  RtcDateTime d = RtcDateTime();
  d.InitWithEpoch32Time(getLocalTime());
  *day = d.Day();
  *month = d.Month();
  *year = d.Year();
}

void OswHal::getWeekdayString(int firstNChars, string *output) {
  uint32_t day = 0;
  uint32_t weekDay = 0;
  getDate(&day, &weekDay);

  std::map<int, std::string> dayMap;

  dayMap[0] = LANG_SUNDAY;
  dayMap[1] = LANG_MONDAY;
  dayMap[2] = LANG_TUESDAY;
  dayMap[3] = LANG_WEDNESDAY;
  dayMap[4] = LANG_THURSDAY;
  dayMap[5] = LANG_FRIDAY;
  dayMap[6] = LANG_SATURDAY;

  string value = dayMap[weekDay];
  int valueLength = value.length();

  if (firstNChars == 0 || valueLength <= firstNChars) {
    *output = value;
  }

  *output = value.substr(0, firstNChars);
}