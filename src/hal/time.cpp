#include <RV8523.h>
#include <Wire.h>
#include <config.h>
#include <osw_config.h>
#include <time.h>

#include <map>
#include <string>

#include "osw_hal.h"

void _rv8526_i2c_read(int read_how_much, uint8_t *data_buff);
void _rv8526_i2c_write(uint8_t *data, uint8_t len);

RV8523 Rtc;

// void RV8523::i2c_begin(void){
//   Wire.begin(SDA, SCL, 100000L);
// } 


void OswHal::setupTime(void) {
  Wire.begin(SDA, SCL, 100000L);

  Rtc.i2c_read = &_rv8526_i2c_read;
  Rtc.i2c_write = &_rv8526_i2c_write;


  Rtc.begin();
  Rtc.start();

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

bool OswHal::hasRV8523(void) { return getUTCTime() > 0; }

uint32_t OswHal::getUTCTime(void) { return Rtc.GetDateTime().Epoch32Time(); }
uint32_t OswHal::getLocalTime(void) { return getUTCTime() + 3600 * OswConfigAllKeys::timeZone.get() + (long)(3600 * OswConfigAllKeys::daylightOffset.get()); }

void OswHal::setUTCTime(long epoch) {
  RtcDateTime t = RtcDateTime();
  t.InitWithEpoch32Time(epoch);
  Rtc.SetDateTime(t);
}

void OswHal::getUTCTime(uint32_t *hour, uint32_t *minute, uint32_t *second) {
  RtcDateTime d = RtcDateTime();
  d.InitWithEpoch32Time(getUTCTime());
  *hour = d.Hour();
  *minute = d.Minute();
  *second = d.Second();
}

void OswHal::getLocalTime(uint32_t *hour, uint32_t *minute, uint32_t *second) {
  rv8523_time_struct time;
  Rtc.get_time(&time);
  if (!OswConfigAllKeys::timeFormat.get()) {
    if (time.hours > 12) {
      *hour = time.hours - 12;
    } else if (time.hours == 0) {
      *hour = 12;
    } else {
      *hour = time.hours;
    }
  } else {
    *hour = time.hours;
  }
  *second = time.seconds;
  *minute = time.minutes;
}

void OswHal::getLocalTime(uint32_t *hour, uint32_t *minute, uint32_t *second, bool *afterNoon) {
  rv8523_time_struct time;
  Rtc.get_time(&time);
  if (!OswConfigAllKeys::timeFormat.get()) {
    if (time.hours > 12) {
      *hour = time.hours - 12;
      *afterNoon = true;
    } else if (time.hours == 0) {
      *hour = 12;
      *afterNoon = false;
    } else if (time.hours == 12) {
      *hour = time.hours;
      *afterNoon = true;
    } else {
      *hour = time.hours;
      *afterNoon = false;
    }
  } else {
    *hour = time.hours;
    *afterNoon = false;
  }
  *second = time.seconds;
  *minute = time.minutes;
}

void OswHal::getDate(uint32_t *day, uint32_t *weekDay) {
  rv8523_time_struct time;
  Rtc.get_time(&time);
  *day = time.date;
  *weekDay = time.weekday;
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

void OswHal::updateTimeViaNTP(long gmtOffset_sec, int daylightOffset_sec, uint32_t timeout_sec) {
  long start = millis();
  if (getWiFi()->isConnected()) {
    // this configures the timezone and sets the esps time to UTC
    configTime(gmtOffset_sec + 3600, daylightOffset_sec, "pool.ntp.org", "time.nist.gov");

    Serial.println("Waiting for time");

    while (!time(nullptr) && millis() - start < timeout_sec * 1000) {
      Serial.print(".");
      delay(1000);
    }

    // sometimes time(nullptr) returns seconds since boot
    // so check the request was resolved
    if (time(nullptr) > 1600000000) {
      Serial.println(time(nullptr));
      setUTCTime(time(nullptr));
    }
  }
}

void _rv8526_i2c_read(int read_how_much, uint8_t *data_buff){
  int i = 0;
  Wire.requestFrom(RV8523_I2C_ADDRESS, read_how_much);
  while(Wire.available()){
    data_buff[i++] = Wire.read();
  }
}

void _rv8526_i2c_write(uint8_t *data, uint8_t len){
  Wire.beginTransmission(RV8523_I2C_ADDRESS);
  for(int i=0;i<len;i++){
    Wire.write(data[i]);
  }
  Wire.endTransmission();
}