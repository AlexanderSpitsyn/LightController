#ifndef Schedule_h
#define Schedule_h
#include <Arduino.h>
#include <RTClib.h>
#include <EEPROM.h>
#include <SPI.h>

#define BLUE_COLOR 0
#define WHITE_COLOR 1
#define COLORS_COUNT 2
#define PHASES_COUNT 9
#define PHASES_ADDRESS 0

struct Phase
{
    uint8_t hour;
    uint8_t minute;
    int8_t color;
    uint8_t level;
};

class Schedule
{
public:
    Schedule();
    void init();
    void initPhasesFromString(const char *str);
    void save(const char *str);
    const Phase *getPhases();
    const uint8_t getLevel(const DateTime &nowDateTime, const uint8_t color);
    String toString();

private:
    Phase _phases[PHASES_COUNT];

    Phase toPhase(const char *str);
    String toString(const Phase &phase, const uint8_t i);
    Phase *getPhaseByColor(const uint8_t i, const int8_t color);
};

#endif