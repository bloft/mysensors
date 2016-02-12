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
 * Version 1.1 - Bjarne Loft
 * 
 * DESCRIPTION
 * Use this sensor to measure KWH and Watt of your house meter
 * You need to set the correct pulsefactor of your meeter (blinks per KWH).
 * The sensor starts by fetching current KWH value from gateway.
 * Reports both KWH and Watt back to gateway.
 *
 * Unfortunately millis() won't increment when the Arduino is in 
 * sleepmode. So we cannot make this sensor sleep if we also want 
 * to calculate/report watt-number.
 * http://www.mysensors.org/build/pulse_power
 */

#include <SPI.h>
#include <MySensor.h>

#define NODE_ID 4

#define CHILD_ID_ENERGY 1

#define ENERGY_DIGITAL_PIN 3  // The digital input you attached your light sensor.  (Only 2 and 3 generates interrupt!)
#define PULSE_FACTOR 1000       // Nummber of blinks per KWH of your meeter
#define MAX_WATT 10000          // Max watt value to report. This filetrs outliers.


unsigned long SEND_FREQUENCY = 20000; // Minimum time between send (in milliseconds). We don't want to spam the gateway.
MySensor gw;
double ppwh = ((double)PULSE_FACTOR)/1000; // Pulses per watt hour
volatile unsigned long pulseCount = 0;   
volatile unsigned long lastBlink = 0;
volatile unsigned long watt = 0;
unsigned long oldPulseCount = 0;
unsigned long oldWatt = 0;
double oldKwh;
unsigned long lastSend = 0;

MyMessage wattMsg(CHILD_ID_ENERGY, V_WATT);
MyMessage kwhMsg(CHILD_ID_ENERGY, V_KWH);
MyMessage pcMsg(CHILD_ID_ENERGY ,V_VAR1);

void setup() {
  gw.begin(incomingMessage, NODE_ID, true);

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Energy Meter", "1.1");

  // Register this device as power sensor
  gw.present(CHILD_ID_ENERGY, S_POWER);

  // Fetch last known pulse count value from gw
  gw.request(CHILD_ID_ENERGY, V_VAR1);
  
  attachInterrupt(ENERGY_DIGITAL_PIN-2, onPulse, RISING);
  lastSend=millis();
}


void loop() {
  gw.process();
  unsigned long now = millis();
  // Only send values at a maximum frequency or woken up from sleep
  if ((now - lastSend) > SEND_FREQUENCY) {
    lastSend=now;
	 // New watt value has been calculated  
	 if (watt != oldWatt && watt<((unsigned long)MAX_WATT)) {
		  gw.send(wattMsg.set(watt));
		  Serial.print("Watt:");
		  Serial.println(watt);
		  oldWatt = watt;
	 }
  
	 if(pulseCount > 0) {
		// Pulse cout has changed
		if (pulseCount != oldPulseCount) {
		  gw.send(pcMsg.set(pulseCount));  // Send pulse count value to gw
		  Serial.print("pulseCount:");
		  Serial.println(pulseCount);
		  double kwh = ((double)pulseCount/((double)PULSE_FACTOR));
		  oldPulseCount = pulseCount;
		  if (kwh != oldKwh) {
			 gw.send(kwhMsg.set(kwh, 4));  // Send kwh value to gw 
			 Serial.print("kwh:");
			 Serial.println(kwh);
			 oldKwh = kwh;
		  }
		}
	 } else {
		// No count received. Try requesting it again
		gw.request(CHILD_ID_ENERGY, V_VAR1);
	 }
  }
  gw.wait(SEND_FREQUENCY);
}

void incomingMessage(const MyMessage &message) {
  if (message.sensor==CHILD_ID_ENERGY && message.type==V_VAR1) {  
    pulseCount = oldPulseCount = message.getLong();
    Serial.print("Received last pulse count from gw:");
    Serial.println(pulseCount);
  }
}

void onPulse() {
  unsigned long newBlink = micros();  
  unsigned long interval = newBlink-lastBlink;
  if (interval<10000L) { // Sometimes we get interrupt on RISING
	 return;
  }
  watt = (3600000000.0 /interval) / ppwh;
  lastBlink = newBlink;
  if(pulseCount > 0) {
	 pulseCount++;
  }
}
