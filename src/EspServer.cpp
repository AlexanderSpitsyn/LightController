#include "EspServer.h"

EspServer::EspServer(uint8_t receivePin, uint8_t transmitPin) : _esp(receivePin, transmitPin)
{
    _esp.begin(115200);

    sendData("AT+CWMODE=2", 1000);
    sendData("AT+CIFSR", 1000);          // This will show IP address on the serial monitor
    sendData("AT+CIPMUX=0", 1000);       // This will allow multiple connections
    sendData("AT+CIPSERVER=1,80", 1000); // This will start the web server on port 80
}

void EspServer::onRequest(String (*callback)(const String &))
{
    if (_esp.available()) // check if there is data available on ESP8266
    {
        if (_esp.find("+IPD,")) // if there is a new command
        {
            _esp.find("/"); // run cursor until command is found
            String msg = _esp.readStringUntil(' ');

            sendResponse(callback(msg));
        }
    }
}

String EspServer::sendData(const String &command, const int timeout)
{
    String response = "";
    _esp.println(command);
    long int time = millis();
    while ((time + timeout) > millis())
    {
        while (_esp.available())
        {
            char c = _esp.read();
            response += c;
        }
    }
    return response;
}

void EspServer::sendResponse(const int status, const String &resp)
{
    uint32_t contentLenght = resp.length();
    String okResp = "HTTP/1.1 200 OK\r\nConnection: close\r\ncontent-length: ";
    okResp += contentLenght;
    okResp += "\r\ncontent-type: text/plain\r\n\r\n";
    if (contentLenght != 0)
    {
        okResp += resp;
    }

    String len = "";
    len += okResp.length() + 2;
    sendData("AT+CIPSEND=0," + len, 1000);
    _esp.find(">");
    sendData(okResp, 1000);
    sendData("AT+CIPCLOSE=5", 1000);
}

void EspServer::sendResponse(const String &resp)
{
    sendResponse(200, resp);
}

void EspServer::sendResponse()
{
    sendResponse(200, "");
}