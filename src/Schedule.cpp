#include "Schedule.h"

Schedule::Schedule()
{
}

void Schedule::init()
{
    eeprom_read_block((void *)&_phases, PHASES_ADDRESS, sizeof(_phases));
}

void Schedule::initPhasesFromString(const char *str)
{
    char *strCopy = strdup(str);

    char *pch = strtok(strCopy, ";");
    for (size_t i = 0; pch != NULL; i++)
    {
        _phases[i] = toPhase(pch);
        pch = strtok(NULL, ";");
    }

    free(strdup);
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
    for (size_t i = 0; i < COLORS_COUNT; i++)
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
    char *strCopy = strdup(str);

    char *phaseChars[5];
    char *pch = strtok(strCopy, ":");
    for (size_t i = 0; pch != NULL; i++)
    {
        phaseChars[i] = pch;
        pch = strtok(NULL, ";");
    }

    Phase phase;
    phase.color = atoi(phaseChars[0]);
    if (phase.color >= 0)
    {
        phase.hour = atoi(phaseChars[1]);
        phase.minute = atoi(phaseChars[2]);
        phase.level = atoi(phaseChars[3]);
    }

    free(strdup);
}

const char *Schedule::toString()
{
    char result[15 * PHASES_COUNT + PHASES_COUNT];
    for (size_t i = 0; i < PHASES_COUNT; i++)
    {
        strcat(result, toString(_phases[i], i));
        strcat(result, ";");
    }
    return result;
}

const char *Schedule::toString(const Phase &phase, const uint8_t i)
{
    char result[15];
    char buf[2];

    itoa(i, buf, DEC);
    strcat(result, buf);

    itoa(phase.color, buf, DEC);
    strcat(result, ":");
    strcat(result, buf);

    itoa(phase.hour, buf, DEC);
    strcat(result, ":");
    strcat(result, buf);

    itoa(phase.minute, buf, DEC);
    strcat(result, ":");
    strcat(result, buf);

    itoa(phase.level, buf, DEC);
    strcat(result, ":");
    strcat(result, buf);

    return result;
}