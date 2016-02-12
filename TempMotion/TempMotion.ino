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
#include <DHT.h>

#define NODE_ID 3

#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define CHILD_ID_MOTION 2
#define CHILD_ID_LIGHT 3

#define MOTION_DIGITAL_PIN 2
#define HUMIDITY_DIGITAL_PIN 3
#define LIGHT_ANALOG_PIN 4

unsigned long SLEEP_TIME = 5000; //30000; // Sleep time between reads (in milliseconds)

MySensor gw;
DHT dht;
float lastTemp;
float lastHum;
int lastLight;
boolean lastMotion;

boolean metric = true;

MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgMotion(CHILD_ID_MOTION, V_TRIPPED);
MyMessage msgLight(CHILD_ID_LIGHT, V_LIGHT_LEVEL);

void setup() { 
  gw.begin(incomingMessage, NODE_ID, false);
  
  dht.setup(HUMIDITY_DIGITAL_PIN);

  pinMode(MOTION_DIGITAL_PIN, INPUT);

  // Send the Sketch Version Information to the Gateway
  gw.sendSketchInfo("Temp+Hum+Motion", "0.1");

  // Register all sensors to gw (they will be created as child devices)
  gw.present(CHILD_ID_HUM, S_HUM);
  gw.present(CHILD_ID_TEMP, S_TEMP);
  gw.present(CHILD_ID_MOTION, S_MOTION);
  gw.present(CHILD_ID_LIGHT, S_LIGHT_LEVEL);
  
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
  
  boolean motion = digitalRead(MOTION_DIGITAL_PIN) == HIGH;
  if(motion != lastMotion) {
	 lastMotion = motion;
	 Serial.print("M: ");
    Serial.println(motion?"1":"0");
	 gw.send(msgMotion.set(motion?"1":"0"));
  }

  int lightLevel = analogRead(LIGHT_ANALOG_PIN)/10.23; 
  if (lightLevel != lastLight) {
	 lastLight = lightLevel;
    Serial.print("L: ");
    Serial.println(lightLevel);
    gw.send(msgLight.set(lightLevel));
  }

  gw.sleep(MOTION_DIGITAL_PIN-2, CHANGE, SLEEP_TIME); //sleep a bit
}

void incomingMessage(const MyMessage &message) {
}

