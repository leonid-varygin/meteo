/**
   BasicHTTPSClient.ino

    Created on: 20.08.2018

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

Adafruit_BME280 bme;
#define SEALEVELPRESSURE_HPA (1013.25)
#define sp_relay_pin 13
#define bme_relay_pin 12

String temperature, humidity, pressure, altitude, battery, sp;
float involtage, voltage;
const String mySSID = "mySSID", myPass = "myPass", myKey = "mySuperKey",
  myUrl = "https://meteo.alexanderkif.now.sh/data";

void setup() {

//  Serial.begin(115200);

  bme.begin(0x76);

  pinMode(bme_relay_pin, OUTPUT);
  digitalWrite(bme_relay_pin, 1); // bme power on
  
  pinMode(sp_relay_pin, OUTPUT); // low impedance
  digitalWrite(sp_relay_pin, 0); // solar panel can charge now
  int involt = analogRead(A0);
  digitalWrite(sp_relay_pin, 1); //solar panel unplugged
  pinMode(sp_relay_pin, INPUT); // high impedance
  int volt = analogRead(A0);

  involtage = involt * 4.45 / 1023; //used 125kOm 220kOm 100kOm resistors = 445kOm
  voltage = volt * 4.45 / 1023; // 0V - 4.45V
  sp = String(involtage, 3);
  battery = String(voltage, 3);
      
  temperature = String(bme.readTemperature(), 3);
  humidity = String(bme.readHumidity(), 2);
  pressure = String(bme.readPressure() / 100.0F * 0.7500637554192, 2);
  altitude = String(bme.readAltitude(SEALEVELPRESSURE_HPA), 2);
  
  digitalWrite(bme_relay_pin, 0); // bme power off

  WiFi.mode(WIFI_STA);
  WiFi.begin( mySSID, myPass);
//  Serial.println("Wait for WiFi connection");
  int count = 0;
  while (WiFi.status() != WL_CONNECTED && count < 60) {
    delay(100);
//    Serial.print(".");
    count++;
  }

  if (WiFi.status() == WL_CONNECTED) {  
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);  
    client->setInsecure();  
    HTTPClient https;  
//    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, myUrl)) {  
      https.addHeader("Content-Type", "application/x-www-form-urlencoded");      
      String postMessage = "temperature="+temperature+"&humidity="+humidity+"&pressure="+pressure+"&altitude="+altitude+"&battery="+battery+"&sp="+sp+"&key="+myKey;
//      Serial.println(postMessage);      
      int httpCode = https.POST(postMessage);   //Send the request   
//      Serial.println(httpCode);   //Print HTTP return code  
      https.end();
    } else {
//      Serial.printf("[HTTPS] Unable to connect\n");
    }  
    WiFi.disconnect();
  }
  
  if (voltage < 4.2){
    pinMode(sp_relay_pin, OUTPUT); // low impedance
    digitalWrite(sp_relay_pin, 0); // solar panel can charge now
    delay(100);
  }
  
//  Serial.println("Wait 5 min before next round...\n");
//  system_deep_sleep_set_option(2); //no RF calibration
  system_deep_sleep_instant(300e6);
}

void loop() {
}
