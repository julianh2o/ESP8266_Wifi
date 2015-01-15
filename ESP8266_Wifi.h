#ifndef ESP8266_Wifi_h
#define ESP8266_Wifi_h

#include "Arduino.h"
#include "SoftwareSerial.h"

//#define DEBUG
#define ERROR

#define DEBUG_SERIAL Serial
#define ESP_SERIAL Serial1

#ifdef DEBUG
    #define DEBUG(x) Serial.print(x)
    #define DEBUGLN(x) Serial.println(x)
#else
    #define DEBUG(x)
    #define DEBUGLN(x)
#endif

#ifdef ERROR
    #define ERROR(x) Serial.print(x)
    #define ERRORLN(x) Serial.println(x)
#else
    #define ERROR(x)
    #define ERRORLN(x)
#endif

class ESP8266_Wifi {
public:
    static const int MODE_STA = 1;
    static const int MODE_AP = 2;
    static const int MODE_BOTH = 2;

    ESP8266_Wifi();
    void init();
    boolean connect(String ssid, String password);
    boolean get(String url, String &result);
    boolean get(String url, String &result, boolean fullResponse);

    void debugSerial(SoftwareSerial * serial);
    boolean reset();
    int getMode();
    void setMode(int mode);

private:
    SoftwareSerial * out;
    long timeout;
    String lineBuffer;

    void send(String data);
    void flush();
    boolean waitFor(char * waitFor);
    boolean waitFor(char * waitFor, String &returnValue);
    boolean waitFor(char * waitFor, String &returnValue, long timeout, boolean terminateImmediately);
    void handleCharacter(char c);
    void flushESPBuffer();
    boolean stringContains(String needle, String haystack);
    boolean parseUrl(String url,String &protocol,String &host,String &path,String &params);
    boolean start(String protocol, String ip, int port);
    boolean sendPayload(String get);
    boolean close();
};

#endif
