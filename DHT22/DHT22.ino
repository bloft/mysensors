/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0 - Henrik EKblad
 * 
 * DESCRIPTION
 * This sketch provides an example how to implement a humidity/temperature
 * sensor using DHT11/DHT-22 
 * http://www.mysensors.org/build/humidity
 */
 
#include <SPI.h>
#include <MySensor.h>
#include <Bounce2.h>
#include <DHT.h>  

#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define CHILD_ID_RELAY 2

#define HUMIDITY_DIGITAL_PIN 3
#define RELAY_DIGITAL_PIN 4

unsigned long SLEEP_TIME = 5000; //30000; // Sleep time between reads (in milliseconds)

MySensor gw;
DHT dht;
Bounce debouncer = Bounce();
float lastTemp;
float lastHum;
boolean metric = true;

MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);

void setup() { 
  gw.begin(incomingMessage, AUTO, true);
  
  dht.setup(HUMIDITY_DIGITAL_PIN);
  
  digitalWrite(RELAY_DIGITAL_PIN, RELAY_ON); // Make sure relays are off when starting up
  pinMode(RELAY_DIGITAL_PIN, OUTPUT); // Then set relay pins in output mode

  // Send the Sketch Version Information to the Gateway
  gw.sendSketchInfo("Prototype", "1.1");

  // Register all sensors to gw (they will be created as child devices)
  gw.present(CHILD_ID_HUM, S_HUM);
  gw.present(CHILD_ID_TEMP, S_TEMP);
  gw.present(CHILD_ID_RELAY, S_BINARY);
  
  metric = gw.getConfig().isMetric;
}

void loop() {
  gw.process();
  gw.wait(dht.getMinimumSamplingPeriod());
  
  float temperature = dht.getTemperature();
  if (isnan(temperature)) {
      Serial.println("Failed reading temperature from DHT");
  } else if (temperature != lastTemp) {
    lastTemp = temperature;
    if (!metric) {
      temperature = dht.toFahrenheit(temperature);
    }
    gw.send(msgTemp.set(temperature, 1));
    Serial.print("T: ");
    Serial.println(temperature);
  }
  
  float humidity = dht.getHumidity();
  if (isnan(humidity)) {
      Serial.println("Failed reading humidity from DHT");
  } else if (humidity != lastHum) {
      lastHum = humidity;
      gw.send(msgHum.set(humidity, 1));
      Serial.print("H: ");
      Serial.println(humidity);
  }

  gw.wait(SLEEP_TIME); //sleep a bit
}

void incomingMessage(const MyMessage &message) {
  if (message.type == V_STATUS) {
     digitalWrite(RELAY_DIGITAL_PIN, message.getBool()?1:0);
    
     // Write some debug info
     Serial.print("Incoming change for sensor:");
     Serial.print(message.sensor);
     Serial.print(", New status: ");
     Serial.println(message.getBool());
   } 
}

