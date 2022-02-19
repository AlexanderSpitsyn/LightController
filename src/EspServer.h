#ifndef EspServer_h
#define EspServer_h
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>

#define TIMEOUT 1000
#define DELAY 1000

class EspServer
{
public:
    EspServer(uint8_t receivePin, uint8_t transmitPin);
    void init();
    String sendData(const String &command);
    void onRequest(String (*callback)(const String &));
    void sendResponse(const int status, const String &resp);
    void sendResponse(const String &resp);
    void sendResponse();

private:
    SoftwareSerial _esp;
};

#endif