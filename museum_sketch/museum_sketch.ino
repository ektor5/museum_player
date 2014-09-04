
/*
#  This file is part of museum_player
#
#  Copyright (C) 2014 Ettore Chimenti <ek5.chimenti@gmail.com>
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Library General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Library General Public License for more details.
#
#  You should have received a copy of the GNU Library General Public License
#  along with this library; see the file COPYING.LIB.  If not, write to
#  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#  Boston, MA 02110-1301, USA.
*/

#include <SharpIR.h>

//PINOUT
const int ledPin = 53;
const int pingPin = 7;
const int buttonPin = 6;
const int irPin = 11;

//HARDCODED
unsigned int maxThreshold = 9000 ;
unsigned int minThreshold = 1000 ;
unsigned int population = 10000; 

//DEFAULT
unsigned int minDistance = 120;
unsigned int timeUp     = 500;
unsigned int timeDown   = 5000;
unsigned int frequency  = 10;
boolean debug          	= 0;
boolean irEnable	= 1;


//VARS
unsigned int periodDelay;
unsigned int periodLoop;
unsigned int incr;
unsigned int decr;

long dist;
unsigned int goodValues;
boolean hitThreshold;
boolean canSendTrack;

void setup() {
  // initialize serial:
  Serial.begin(115200);
  Serial.println("START");
  Serial.flush();
  
  setupVars();
  
  // set pinout
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(buttonPin, INPUT);
  attachInterrupt(buttonPin, sendTrack, RISING);
  
  Serial.flush();
}

void loop() {
  
  //int lastMillis = millis();
  
  if ( Serial.available() == 5 ) 
  { 
    
    unsigned int timeUp_ =      (unsigned int)Serial.read() * 100 ;  
    unsigned int timeDown_ =    (unsigned int)Serial.read() * 1000 ;
    unsigned int minDistance_ = (unsigned int)Serial.read() ;
    unsigned int frequency_ =   (unsigned int)Serial.read() ;
    bool irEnable_ =            (bool)Serial.read();
   
    digitalWrite(ledPin, HIGH);
    delay(1000);
    digitalWrite(ledPin, LOW);

    Serial.println(timeUp_);
    Serial.println(timeDown_);
    Serial.println(minDistance_);
    Serial.println(frequency_);
    Serial.println(irEnable_);

    timeUp=      timeUp_?timeUp_:timeUp;
    timeDown=    timeDown_?timeDown_:timeDown;
    minDistance= minDistance_?minDistance_:minDistance;
    frequency=   frequency_?frequency_:frequency;
    irEnable=    irEnable_;

    //reset
    setupVars();
   
    Serial.flush();
  }
  
  if ( canSendTrack ) 
  {
    Serial.write("t");
    delay(100);
    canSendTrack = false;
  }
  
  dist = distancePing();
  
  if(debug){
    Serial.print(dist);
  }
  
  goodValues = updateValues( dist, goodValues );
  
  if ( goodValues > maxThreshold ){
    if ( hitThreshold != true){
      sendNoise();
      hitThreshold = true;
      digitalWrite(ledPin, HIGH);
    }
  } else 
      if ( goodValues < minThreshold ){
        hitThreshold = false;
        digitalWrite(ledPin, LOW);
      }
  
  //Serial.println(millis() - lastMillis);
  
  delay(periodDelay);
  
} 

long microsecondsToCentimeters(long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}

void sendTrack()
{
   canSendTrack = true;
}

void sendNoise()
{
   Serial.write("n"); 
   delay(100);
}

long distancePing()
{
  if (irEnable) {
    SharpIR sharp(irPin, 15, 93, 20150);
    return (long)sharp.distance();
  }else{
    pinMode(pingPin, OUTPUT);
    digitalWrite(pingPin, LOW);
    delayMicroseconds(2);
    digitalWrite(pingPin, HIGH);
    delayMicroseconds(5);
    digitalWrite(pingPin, LOW);
    
    pinMode(pingPin, INPUT);
    long duration = pulseIn(pingPin, HIGH);
    
    return microsecondsToCentimeters(duration);
  }
}

unsigned int updateValues( long dist, float values )
{
  if ( dist < minDistance ) 
  {
    values = (values > population-incr ) ? population : (values+incr) ; 
  } else { 
    values = (values < decr) ? 0 : (values-decr);
  }

  return values;
}  

void setupVars(){
  
  //SET 
 
  frequency = (frequency<70)?frequency:70;
  
  periodDelay = (int)(( 1.0 / (float)frequency ) * 1000.0 ) ;
  incr = (int)((float)maxThreshold / (float)timeUp) * ((float)periodDelay);
  decr = (int)(( (float)(population - minThreshold) / (float)timeDown ) * (float)periodDelay);
  
  //RESET VAR

  dist = 500;
  goodValues = 1;
  hitThreshold = false;
  canSendTrack = false;
  
  if (debug) {
    Serial.print("maxThreshold: ");
    Serial.println(maxThreshold);
    Serial.print("minThreshold: ");
    Serial.println(minThreshold);
    Serial.print("population: ");
    Serial.println(population);
    Serial.print("minDistance: ");
    Serial.println(minDistance);
    Serial.print("timeUp: ");
    Serial.println(timeUp);
    Serial.print("timeDown: ");
    Serial.println(timeDown); 
    Serial.print("frequency: ");
    Serial.println(frequency);
    Serial.print("periodDelay: ");
    Serial.println(periodDelay);
    Serial.print("incr: ");
    Serial.println(incr);
    Serial.print("decr: ");
    Serial.println(decr);

  }
}
