#include "ESP8266_Wifi.h"

ESP8266_Wifi::ESP8266_Wifi() {
    this->out = NULL;
    this->timeout = 5000;
    lineBuffer = "";
}

void ESP8266_Wifi::init() {
  ESP_SERIAL.begin(115200);
}

//TODO remove me?
void ESP8266_Wifi::debugSerial(SoftwareSerial * serial) {
    this->out = serial;
}

void ESP8266_Wifi::flush() {
  char c;
  while(ESP_SERIAL.available()) {
      c = ESP_SERIAL.read();
      ESP8266_Wifi::handleCharacter(c);
  }
}

void ESP8266_Wifi::send(String data) {
  DEBUG_SERIAL.println("(ESP8266 Debug) SENDING: "+data);
  ESP_SERIAL.print(data);
  ESP_SERIAL.print("\r\n");
}

boolean ESP8266_Wifi::stringContains(String needle, String haystack) {
    for (int i=0; i<needle.length()-haystack.length(); i++) {
        String fragment = haystack.substring(i,i+needle.length());
        DEBUG_SERIAL.print("check frag: ");
        DEBUG_SERIAL.println(fragment);
        if (fragment == needle) return true;
    }
    return false;
}

boolean ESP8266_Wifi::waitFor(char * waitFor) {
  String dest = "";
  return ESP8266_Wifi::waitForReturn(waitFor,dest);
}

void ESP8266_Wifi::handleCharacter(char c) {
  lineBuffer += c;
  if (c == '\n') {
    DEBUG_SERIAL.print("(ESP8266 Output) ");
    DEBUG_SERIAL.print(lineBuffer);
    lineBuffer = "";
  }
}

boolean ESP8266_Wifi::waitForReturn(char * waitFor, String &returnValue) {
  long start = millis();
  String buffer = "";
  char c;
  DEBUG_SERIAL.print("waiting for: ");
  DEBUG_SERIAL.println(waitFor);
  boolean found = false;
  while(millis() - start < this->timeout) {
    if (ESP_SERIAL.available()) {
      c = ESP_SERIAL.read();
      returnValue = lineBuffer;
      ESP8266_Wifi::handleCharacter(c);
      buffer += c;
      //DEBUG_SERIAL.print("char: ");
      //DEBUG_SERIAL.println(c);
      if (c == '\n' && found) {
          DEBUG_SERIAL.print("FOUND: ");
          DEBUG_SERIAL.println(waitFor);
          return true;
      }


      if (buffer.endsWith(waitFor)) {
          found = true;
      }
    }
  }
  DEBUG_SERIAL.println("ERROR: WaitFor timeout out.");
  return found;
}

boolean ESP8266_Wifi::reset() {
  ESP8266_Wifi::send("AT+RST");
  boolean success = ESP8266_Wifi::waitFor("ready");
  ESP8266_Wifi::flush();
  return success;
}

int ESP8266_Wifi::getMode() {
  ESP8266_Wifi::send("AT+CWMODE?");
  String line = "";
  if (ESP8266_Wifi::waitForReturn("+CWMODE:",line)) {
    int mode = line.substring(line.indexOf('+')+8).toInt();
    ESP8266_Wifi::flush();
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
  ESP_SERIAL.flush();
}

boolean ESP8266_Wifi::connect(String ssid, String password) {
  ESP8266_Wifi::send("AT+CWJAP=\""+ssid+"\",\""+password+"\"");
  boolean success = ESP8266_Wifi::waitFor("OK");
  ESP_SERIAL.flush();
  return success;
}

boolean ESP8266_Wifi::start(String protocol, String ip, int port) {
  ESP8266_Wifi::send("AT+CIPSTART=\""+protocol+"\",\""+ip+"\","+port);
  boolean success = ESP8266_Wifi::waitFor("Linked");
  ESP_SERIAL.flush();
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

