#ifndef ESP8266_Wifi_h
#define ESP8266_Wifi_h

#include "Arduino.h"
#include "SoftwareSerial.h"

class ESP8266_Wifi {
public:
    ESP8266_Wifi();
    void init();
    void debugSerial(SoftwareSerial * serial);
    void send(String data);
    boolean waitFor(char * data);
    boolean reset();
    int getMode();
    void setMode(int mode);
    boolean connect(String ssid, String password);
    boolean start(String protocol, String ip, int port);
    boolean sendPayload(String get);
    boolean close();

private:
    SoftwareSerial * out;
};

#endif
