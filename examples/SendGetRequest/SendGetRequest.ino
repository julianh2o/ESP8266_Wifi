#include <SoftwareSerial.h>
#include <ESP8266_Wifi.h>

SoftwareSerial out(3, 2); // RX, TX for debug serial (optional)
ESP8266_Wifi wifi;

void setup() {
  out.begin(9600);
  wifi.debugSerial(&out); //optional
  wifi.init();

  unsigned long time = millis();
  wifi.reset();
  wifi.setMode(1);
  wifi.connect("insert_ssid_here","insert_password_here");
  //should check to see if connect failed; print error
  out.print("Connected in ");
  out.print((millis() - time));
  out.println("ms");
}

int count = 0;
void loop() {
  String get = "GET /update?key=INSERT_THINGSPEAK_KEY&field1=";
  //make they Key and Field into variables
  get += count;
  
  unsigned long time = millis();
  wifi.start("TCP","184.106.153.149",80);  //connect to thingspeak
  wifi.sendPayload(get);
  wifi.close();
  //how do we know the above was successful?
  out.print("Connected in ");
  out.print((millis() - time));
  out.println("ms");

  count++;
  delay(5000);
}

