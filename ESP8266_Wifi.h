#ifndef ESP8266_Wifi_h
#define ESP8266_Wifi_h

#include "Arduino.h"
#include "SoftwareSerial.h"

#define DEBUG_SERIAL Serial
#define ESP_SERIAL Serial1

class ESP8266_Wifi {
public:
    static const int MODE_STA = 1;
    static const int MODE_AP = 2;
    static const int MODE_BOTH = 2;

    ESP8266_Wifi();
    void init();
    boolean connect(String ssid, String password);
    boolean get(String ip, String getString, String &result);

    void debugSerial(SoftwareSerial * serial);
    boolean reset();
    int getMode();
    void setMode(int mode);
    boolean start(String protocol, String ip, int port);
    boolean sendPayload(String get);
    boolean close();

private:
    SoftwareSerial * out;
    long timeout;
    String lineBuffer;

    void send(String data);
    void flush();
    boolean waitFor(char * data);
    boolean waitForReturn(char * waitFor, String &returnValue);
    void handleCharacter(char c);
    boolean stringContains(String needle, String haystack);
};

#endif
