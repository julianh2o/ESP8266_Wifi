#include "ESP8266_Wifi.h"

ESP8266_Wifi::ESP8266_Wifi() {
    this->out = NULL;
}

void ESP8266_Wifi::init() {
  Serial.begin(115200);
}

void ESP8266_Wifi::debugSerial(SoftwareSerial * serial) {
    this->out = serial;
}

void ESP8266_Wifi::send(String data) {
  if (out != NULL) out->println("(ESP8266 Debug) SENDING: "+data);
  Serial.print(data);
  Serial.print("\r\n");
}

boolean ESP8266_Wifi::waitFor(char * data) {
  return Serial.find(data);
}

boolean ESP8266_Wifi::reset() {
  ESP8266_Wifi::send("AT+RST");
  boolean success = ESP8266_Wifi::waitFor("ready");
  Serial.flush();
  return success;
}

int ESP8266_Wifi::getMode() {
  ESP8266_Wifi::send("AT+CWMODE?");
  if (ESP8266_Wifi::waitFor("+CWMODE:")) {
    int mode = Serial.parseInt();
    Serial.flush();
    return mode;
  } else {
    return -1;
  }
}

void ESP8266_Wifi::setMode(int mode) {
  int currentMode = ESP8266_Wifi::getMode();
  if (currentMode == mode) {
    return;
  }
  String cmd = "AT+CWMODE=";
  cmd += mode;
  ESP8266_Wifi::send(cmd);
  ESP8266_Wifi::waitFor("OK");
  Serial.flush();
}

boolean ESP8266_Wifi::connect(String ssid, String password) {
  ESP8266_Wifi::send("AT+CWJAP=\""+ssid+"\",\""+password+"\"");
  boolean success = ESP8266_Wifi::waitFor("OK");
  Serial.flush();
  return success;
}

boolean ESP8266_Wifi::start(String protocol, String ip, int port) {
  ESP8266_Wifi::send("AT+CIPSTART=\""+protocol+"\",\""+ip+"\","+port);
  boolean success = ESP8266_Wifi::waitFor("Linked");
  Serial.flush();
  return success;
}

boolean ESP8266_Wifi::sendPayload(String get) {
  String cmd = "AT+CIPSEND=";
  cmd += (get.length() + 2);
  ESP8266_Wifi::send(cmd);
  ESP8266_Wifi::waitFor(">");
  ESP8266_Wifi::send(get);
}

boolean ESP8266_Wifi::close() {
  ESP8266_Wifi::send("AT+CIPCLOSE");
  ESP8266_Wifi::waitFor("Unlink");
}

