#include <Arduino.h>
#include <SPI.h>
#include <TimerMs.h>
#include <RTClib.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <Wire.h>

#include "EspServer.h"
#include "Schedule.h"

#define DEBUG true
#define GET_TIMESTAMP_PARAM "get_timestamp"
#define SET_TIMESTAMP_PARAM "set_timestamp"
#define GET_PHASES_PARAM "get_phases"
#define SET_PHASES_PARAM "set_phases"
#define CURRENT_VALUES_PARAM "current_values"


#define BLUE_PIN 5
#define WHITE_PIN 6
#define DUTY_MULTIPLICATOR 12.75

EspServer esp(2, 3);
Schedule schedule;
RTC_DS3231 rtc;
TimerMs levelLogTimer[COLORS_COUNT];

boolean showCurrentLevels = false;
uint8_t blueCurrentLevel = 0;
uint8_t whiteCurrentLevel = 0;
DateTime now;


void log(const String &message)
{
  if (DEBUG)
  {
    Serial.println(message);
  }
}

void printDateTime(const DateTime &now)
{
  if (DEBUG)
  {
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
  }
}


void getCommandFromMessage(const String &message, String &param, String &value)
{
  log(message);
  int sepIdx = message.indexOf('=');
  param = message.substring(0, sepIdx);
  value = message.substring(sepIdx + 1);
}


void showLightCurrentLevels(const String &value)
{
  uint8_t sepIdx = value.indexOf(':');
  uint8_t color = value.substring(0, sepIdx).toInt();
  uint8_t level = value.substring(sepIdx + 1).toInt();
  if (color == BLUE_COLOR)
  {
    blueCurrentLevel = level;
  }
  else if (color = WHITE_COLOR)
  {
    whiteCurrentLevel = level;
  }
  showCurrentLevels = true;
}

String processRequest(const String &msg)
{
  if (msg == GET_TIMESTAMP_PARAM)
  {
    return String(now.unixtime());
  }

  String param, value;
  getCommandFromMessage(msg, param, value);

  if (param == SET_TIMESTAMP_PARAM)
  {
    uint32_t valueInt = strtoul(value.c_str(), NULL, 10);
    rtc.adjust(DateTime(valueInt));
    printDateTime(rtc.now());
  }
  else if (param == SET_PHASES_PARAM)
  {
    schedule.save(value.c_str());
    log(schedule.toString());
  }
  else if (param == CURRENT_VALUES_PARAM)
  {
    showLightCurrentLevels(value);
  }
  return "";
}

void setup()
{
  if (DEBUG)
  {
    Serial.begin(115200);
    Serial.println("TEST");
    for (uint8_t i = 0; i < COLORS_COUNT; i++)
    {
      levelLogTimer[i].setPeriodMode();
      levelLogTimer[i].setTime(5000);
      levelLogTimer[i].start();
    }
  }

  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    while (1)
      ;
  }

  if (rtc.lostPower())
  {
    Serial.println("RTC lost power, lets set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  printDateTime(rtc.now());

  schedule.init();
  log(schedule.toString());
}

void loop()
{
  now = rtc.now();
  esp.onRequest(processRequest);

  if (showCurrentLevels)
  {
    analogWrite(BLUE_PIN, blueCurrentLevel * DUTY_MULTIPLICATOR);
    analogWrite(WHITE_PIN, whiteCurrentLevel * DUTY_MULTIPLICATOR);
  }
  else
  {
    analogWrite(BLUE_PIN, schedule.getLevel(now, BLUE_COLOR) * DUTY_MULTIPLICATOR);
    analogWrite(WHITE_PIN, schedule.getLevel(now, WHITE_COLOR) * DUTY_MULTIPLICATOR);
  }

  delay(100);
}