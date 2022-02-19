#include "EspServer.h"

EspServer::EspServer(uint8_t receivePin, uint8_t transmitPin) : _esp(receivePin, transmitPin)
{
}

void EspServer::init()
{
    _esp.begin(9600);

    sendData(F("AT+RST"));
    sendData(F("AT+UART_DEF=9600,8,1,0,0"));
    sendData(F("AT+CWMODE=2"));
    sendData(F("AT+CIFSR"));          // This will show IP address on the serial monitor
    sendData(F("AT+CIPMUX=1"));       // This will allow multiple connections
    sendData(F("AT+CIPSERVER=1,80")); // This will start the web server on port 80
}

void EspServer::onRequest(String (*callback)(const String &))
{
    String msg = "";
    unsigned long time = millis();
    while ((time + TIMEOUT) > millis())
    {
        while (_esp.available()) // check if there is data available on ESP8266
        {
            char c = _esp.read();
            msg += c;
        }
    }
    if (msg.indexOf("+IPD,") > -1)
    {
        msg = msg.substring(msg.indexOf('/') + 1);
        msg = msg.substring(0, msg.indexOf(' '));

        String resp = callback(msg);
        sendResponse(resp);
    }
}

String EspServer::sendData(const String &command)
{
    String response = "";
    _esp.println(command);
    unsigned long time = millis();
    while ((time + TIMEOUT) > millis())
    {
        while (_esp.available())
        {
            char c = _esp.read();
            response += c;
        }
    }
    Serial.println("\n" + command + " : " + response);
    delay(DELAY);
    return response;
}

void EspServer::sendResponse(const int status, const String &resp)
{
    uint32_t contentLenght = resp.length();
    String okResp = "HTTP/1.1 200 OK\r\nConnection: close\r\nAccess-Control-Allow-Origin: *\r\ncontent-length: ";
    okResp += contentLenght;
    okResp += "\r\ncontent-type: text/plain\r\n\r\n";
    if (contentLenght != 0)
    {
        okResp += resp;
    }

    String len = "";
    len += okResp.length() + 2;
    sendData("AT+CIPSEND=0," + len);
    _esp.find('>');
    sendData(okResp);
    sendData(F("AT+CIPCLOSE=5"));
}

void EspServer::sendResponse(const String &resp)
{
    sendResponse(200, resp);
}

void EspServer::sendResponse()
{
    sendResponse(200, "");
}