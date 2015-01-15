#include "ESP8266_Wifi.h"

ESP8266_Wifi::ESP8266_Wifi() {
    this->out = NULL;
    this->timeout = 10000;
    lineBuffer = "";
}

void ESP8266_Wifi::init() {
  ESP_SERIAL.begin(115200);
  //add Serial.setTimeout here?  Make baud configurable?  
  //I noticed that some people are running their ESP at 9600 on a software UART.
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
  DEBUGLN("(ESP8266 Debug) SENDING: "+data);
  ESP_SERIAL.print(data);
  ESP_SERIAL.print("\r\n");
}

boolean ESP8266_Wifi::stringContains(String needle, String haystack) {
    for (int i=0; i<needle.length()-haystack.length(); i++) {
        String fragment = haystack.substring(i,i+needle.length());
        DEBUG("check frag: ");
        DEBUGLN(fragment);
        if (fragment == needle) return true;
    }
    return false;
}

void ESP8266_Wifi::handleCharacter(char c) {
  lineBuffer += c;
  if (c == '\n') {
    ESP8266_Wifi::flushESPBuffer();
  }
}

void ESP8266_Wifi::flushESPBuffer() {
    DEBUG("(ESP8266 Output) ");
    DEBUG(lineBuffer);
    if (lineBuffer.charAt(lineBuffer.length()-1) != '\n') {
        DEBUGLN("[no nl]");
    }
    lineBuffer = "";
}

boolean ESP8266_Wifi::waitFor(char * waitFor) {
  String s = "";
  return ESP8266_Wifi::waitFor(waitFor,s,timeout,false);
}

boolean ESP8266_Wifi::waitFor(char * waitFor, String &returnValue) {
  return ESP8266_Wifi::waitFor(waitFor,returnValue,this->timeout, false);
}

boolean ESP8266_Wifi::waitFor(char * waitFor, String &returnValue, long timeout, boolean terminateImmediately) {
  long start = millis();
  String buffer = "";
  char c;
  DEBUG("waiting for: ");
  DEBUG(waitFor);
  DEBUG(" (");
  DEBUG(timeout);
  DEBUG(")");
  DEBUGLN();
  boolean found = false;
  while(millis() - start < this->timeout) {
    if (ESP_SERIAL.available()) {
      c = ESP_SERIAL.read();
      returnValue = lineBuffer;
      ESP8266_Wifi::handleCharacter(c);
      buffer += c;
      if (c == '\n' && found) {
          DEBUG("FOUND: ");
          DEBUG(waitFor);
          DEBUG(" (");
          DEBUG(millis()-start);
          DEBUG(")");
          DEBUGLN();
          return true;
      }


      if (buffer.endsWith(waitFor)) {
          if (terminateImmediately) return true;

          found = true;
          //if we've found our sentinel.. but we dont have any available data.. we should stop looking
          if (!ESP_SERIAL.available()) {
              ESP8266_Wifi::flushESPBuffer();
              DEBUG("FOUND: ");
              DEBUG(waitFor);
              DEBUG(" (");
              DEBUG(millis()-start);
              DEBUG(")");
              DEBUGLN();
              return true;
          }
      }
    }
  }
  ERRORLN("ERROR: WaitFor timeout out.");
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
  if (ESP8266_Wifi::waitFor("+CWMODE:",line)) {
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
  //here we wait for an OK but we don't return anything.  add a boolean return?
}

boolean ESP8266_Wifi::connect(String ssid, String password) {
  ESP8266_Wifi::send("AT+CWJAP=\""+ssid+"\",\""+password+"\"");
  boolean success = ESP8266_Wifi::waitFor("OK");
  ESP_SERIAL.flush();
  return success;
}

boolean ESP8266_Wifi::get(String url, String &result) {
    return ESP8266_Wifi::get(url, result, false);
}

boolean ESP8266_Wifi::get(String url, String &result, boolean fullResponse) {
  String protocol = "";
  String host = "";
  String path = "";
  String params = "";
  if (!ESP8266_Wifi::parseUrl(url,protocol,host,path,params)) return false;
  if (!ESP8266_Wifi::start("TCP",host,80)) return false;
  String request = "";
  request += "GET "+path+(params.length() > 0 ? "?"+params : "")+" HTTP/1.1"+"\r\n";
  request += "Host: "+host+"\r\n";
  request += "\r\n";
  if (!ESP8266_Wifi::sendPayload(request)) return false;
  String s = "";
  if (ESP8266_Wifi::waitFor("+IPD,",s,5000,true)) {
    ESP8266_Wifi::flushESPBuffer();
    String buffer = "";
    while(ESP_SERIAL.available()) {
        char c = ESP_SERIAL.read();
        if (c == ':') {
            break;
        }
        buffer += c;
    }
    int responseLength = buffer.toInt();
    buffer = "";
    while(responseLength-- > 0) {
        while(!ESP_SERIAL.available());
        buffer += (char)ESP_SERIAL.read();
    }
    if (fullResponse) {
        result = buffer;
    } else {
        int contentBreak = buffer.indexOf("\r\n\r\n");
        if (contentBreak == -1) return false;
        result = buffer.substring(contentBreak+4);
    }
  }
  if (!ESP8266_Wifi::waitFor("OK")) return false;
  if (!ESP8266_Wifi::close()) return false;
  return true;
}

boolean ESP8266_Wifi::parseUrl(String url,String &protocol,String &host,String &path,String &params) {
    if (!url.startsWith("http")) return false;
    protocol = "http";
    if (url.startsWith("https")) protocol = "https";

    int start = 0;
    start = url.indexOf("/"); //start of hostname
    if (start == -1) return false;
    start += 2;
    int end = url.indexOf("/",start+1);
    if (end == -1) end = url.length();
    host = url.substring(start,end);

    start = end;
    if (start > url.length()) return true;
    end = url.indexOf("?",start);
    if (end == -1) end = url.length();
    path = url.substring(start,end);

    start = end + 1;
    if (start > url.length()) return true;
    params = url.substring(start);

    return true;
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
  boolean success = ESP8266_Wifi::waitFor("> ");
  if (!success) return false;
  ESP8266_Wifi::send(get);
  return ESP8266_Wifi::waitFor("SEND OK");
}

boolean ESP8266_Wifi::close() {
  ESP8266_Wifi::send("AT+CIPCLOSE");
  ESP8266_Wifi::waitFor("Unlink");
  //add a boolean return here?
}

