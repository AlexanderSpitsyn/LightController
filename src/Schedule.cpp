#include "Schedule.h"

Schedule::Schedule()
{
}

void Schedule::init()
{
    eeprom_read_block((void *)&_phases, PHASES_ADDRESS, sizeof(_phases));

    bool updatePhases = false;
    for (size_t i = 0; i < PHASES_COUNT; i++)
    {
        Phase *phase = &_phases[i];
        if (phase->hour < 0 || phase->hour > 23)
        {
            phase->hour = 0;
            updatePhases = true;
        }
        if (phase->minute < 0 || phase->minute > 59)
        {
            phase->minute = 0;
            updatePhases = true;
        }
        if (phase->color < -1 || phase->color >= COLORS_COUNT)
        {
            phase->color = -1;
            updatePhases = true;
        }
        if (phase->level < 0 || phase->level > 20)
        {
            phase->level = 0;
            updatePhases = true;
        }
    }
    if (updatePhases)
    {
        eeprom_write_block((void *)&_phases, PHASES_ADDRESS, sizeof(_phases));
    }
}

void Schedule::initPhasesFromString(const char *str)
{
    char strCopy[strlen(str)];
    strcpy(strCopy, str);

    char *pch = strtok(strCopy, ";");
    for (size_t i = 0; pch != NULL; i++)
    {
        _phases[i] = toPhase(pch);
        pch = strtok(NULL, ";");
    }
}

void Schedule::save(const char *str)
{
    initPhasesFromString(str);
    eeprom_write_block((void *)&_phases, PHASES_ADDRESS, sizeof(_phases));
}

const Phase *Schedule::getPhases()
{
    return _phases;
}

const uint8_t Schedule::getLevel(const DateTime &nowDateTime, const uint8_t color)
{
    uint16_t nowMinutes = (uint16_t)nowDateTime.hour() * 60 + (uint16_t)nowDateTime.minute();

    Phase *fromPhase = getPhaseByColor(0, color);
    Phase *toPhase = getPhaseByColor(0, color);
    for (uint8_t i = 0; i < PHASES_COUNT; i++)
    {
        Phase *phase = getPhaseByColor(i, color);

        if (phase->minute == nowMinutes)
        {
            return phase->level;
        }

        if (phase->minute < nowMinutes && (phase->minute > fromPhase->minute || fromPhase->minute > nowMinutes))
        {
            fromPhase = phase;
        }
        if (phase->minute > nowMinutes && (phase->minute < toPhase->minute || toPhase->minute < nowMinutes))
        {
            toPhase = phase;
        }
    }

    float k = (float)(toPhase->level - fromPhase->level) / (toPhase->minute - fromPhase->minute);
    float b = (float)toPhase->level - k * toPhase->minute;
    int level = roundf(k * nowMinutes + b);

    if (level < 0)
    {
        return 0;
    }
    else if (level > 255)
    {
        return 255;
    }
    else
    {
        return level;
    }
}

Phase *Schedule::getPhaseByColor(const uint8_t i, const int8_t color)
{
    for (int8_t i = -1; i <= COLORS_COUNT; i++)
    {
        if (color == i)
        {
            return &_phases[i];
        }
    }
    return NULL;
}

Phase Schedule::toPhase(const char *str)
{
    char strCopy[strlen(str)];
    strcpy(strCopy, str);

    char *phaseChars[5];
    char *pch = strtok(strCopy, ":");
    for (size_t i = 0; pch != NULL; i++)
    {
        phaseChars[i] = pch;
        pch = strtok(NULL, ";");
    }

    Phase phase;
    phase.hour = atoi(phaseChars[0]);
    phase.minute = atoi(phaseChars[1]);
    phase.color = atoi(phaseChars[2]);
    if (phase.color >= 0)
    {
        phase.level = atoi(phaseChars[3]);
    }
    else 
    {
        phase.level = 0;
    }

    return phase;
}

String Schedule::toString()
{
    String result = "";
    for (size_t i = 0; i < PHASES_COUNT; i++)
    {
        result += toString(_phases[i], i);
        result += ";";
    }
    return result;
}

String Schedule::toString(const Phase &phase, const uint8_t i)
{
    String result = "";
    result += phase.hour;
    result += ":";
    result += phase.minute;
    result += ":";
    result += phase.color;
    result += ":";
    result += phase.level;

    return result;
}