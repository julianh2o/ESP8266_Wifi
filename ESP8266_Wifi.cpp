#include "ESP8266_Wifi.h"

ESP8266_Wifi::ESP8266_Wifi() {
    this->out = NULL;
}

void ESP8266_Wifi::init() {
  Serial.begin(115200);
  //add Serial.setTimeout here?  Make baud configurable?  
  //I noticed that some people are running their ESP at 9600 on a software UART.
}

void ESP8266_Wifi::debugSerial(SoftwareSerial * serial) {
    this->out = serial;
}

void ESP8266_Wifi::send(String data) {
  if (out != NULL) out->println("(ESP8266 Debug) SENDING: "+data);
  Serial.print(data);
  Serial.print("\r\n");
  //the above line prints a \r\n which implies that it's the end of a 
  //command, but there is no check to see if the comamnd worked.  Is this 
  //function just too general to know what to expect?
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
  //here we wait for an OK but we don't return anything.  add a boolean return?
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
  //add a check to see if this worked?
}

boolean ESP8266_Wifi::close() {
  ESP8266_Wifi::send("AT+CIPCLOSE");
  ESP8266_Wifi::waitFor("Unlink");
  //add a boolean return here?
}

