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
 * Version 1.0 - Henrik Ekblad
 * 
 * DESCRIPTION
 * Example sketch showing how to control physical relays. 
 * This example will remember relay state after power failure.
 * http://www.mysensors.org/build/relay
 */ 

#include <MySensor.h>
#include <ChainableLED.h>
#include <SPI.h>

#define NODE_ID 2

#define CHILD_ID_RGB 0

unsigned long SLEEP_TIME = 5000; //30000; // Sleep time between reads (in milliseconds)

MySensor gw;
ChainableLED leds(7, 8, 1);

void setup()  
{   
  // Initialize library and add callback for incoming messages
  gw.begin(incomingMessage, NODE_ID, true);
  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("RGB Led", "1.0");
  
  gw.present(CHILD_ID_RGB, S_RGB_LIGHT);
  leds.init();
  leds.setColorRGB(0, 0, 0, 0);
}


void loop() 
{
  // Alway process incoming messages whenever possible
  gw.process();
  
  gw.wait(SLEEP_TIME);
}

void incomingMessage(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.sensor==CHILD_ID_RGB && message.type==V_RGB) {
    String hexstring = message.getString();
    long number = (long) strtol( &hexstring[0], NULL, 16);
    int colorR = number >> 16;
    int colorG = number >> 8 & 0xFF;
    int colorB = number & 0xFF;

     leds.setColorRGB(0, colorR, colorG, colorB);
    
     // Write some debug info
     Serial.print("Incoming change for sensor:");
     Serial.print(message.sensor);
     Serial.print(", Red: ");
     Serial.print(colorR);
     Serial.print(", Green: ");
     Serial.print(colorG);
     Serial.print(", Blue: ");
     Serial.print(colorB);
     Serial.print(", New status: ");
     Serial.println(message.getString());
   } 
}

