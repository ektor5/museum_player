
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
unsigned int maxThreshold	 = 9000 ;
unsigned int minThreshold	 = 1000 ;
unsigned int population 	 = 10000 ;
unsigned int buttonTimeDown 	 = 1500 ;

//DEFAULT
unsigned int minDistance 	= 150;
unsigned int buttonTimeUp	= 150;
unsigned int sensorTimeUp     	= 500;
unsigned int sensorTimeDown   	= 5000;
unsigned int frequency  	= 10;
boolean debug          		= 0;
boolean irEnable		= 1;

//VARS
unsigned int periodDelay;
unsigned int periodLoop;
unsigned int sensorIncr;
unsigned int sensorDecr;
unsigned int buttonIncr;
unsigned int buttonDecr;

long sensorDist;
unsigned int buttonGoodValues;
unsigned int sensorGoodValues;

boolean buttonState;
boolean buttonHitThreshold;
boolean sensorHitThreshold;
boolean result;

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
  digitalWrite(buttonPin, HIGH);
  
  Serial.flush();
}

void loop() {
  
  //int lastMillis = millis();
  
  if ( Serial.available() == 6 ) 
  { 
    
    unsigned int buttonTimeUp_	=     (unsigned int)Serial.read() * 100 ;
    unsigned int sensorTimeUp_ =      (unsigned int)Serial.read() * 100 ;  
    unsigned int sensorTimeDown_ =    (unsigned int)Serial.read() * 1000 ;
    unsigned int minDistance_ =       (unsigned int)Serial.read() ;
    unsigned int frequency_ =         (unsigned int)Serial.read() ;
    bool irEnable_ =                  (bool)Serial.read();
   
    digitalWrite(ledPin, HIGH);
    delay(1000);
    digitalWrite(ledPin, LOW);

    Serial.println(buttonTimeUp_);
    Serial.println(sensorTimeUp_);
    Serial.println(sensorTimeDown_);
    Serial.println(minDistance_);
    Serial.println(frequency_);
    Serial.println(irEnable_);

    buttonTimeUp =      buttonTimeUp_?buttonTimeUp_:buttonTimeUp;
    sensorTimeUp =      sensorTimeUp_?sensorTimeUp_:sensorTimeUp;
    sensorTimeDown =    sensorTimeDown_?sensorTimeDown_:sensorTimeDown;
    minDistance =       minDistance_?minDistance_:minDistance;
    frequency =         frequency_?frequency_:frequency;
    irEnable =          irEnable_;

    //reset
    setupVars();
   
    Serial.flush();
  }
  
  //BUTTON
  buttonState = ( digitalRead(buttonPin) == LOW )? true : false;
  
  buttonGoodValues = updateValues( buttonState, buttonGoodValues, buttonIncr, buttonDecr );
  
  if ( buttonGoodValues > maxThreshold ){
    if ( buttonHitThreshold != true){
      sendTrack();
      buttonHitThreshold = true;
    }
  } else 
      if ( buttonGoodValues < minThreshold ){
        buttonHitThreshold = false;
      }
      
  //SENSOR
  sensorDist = distancePing();
  
  if(debug){
    Serial.print(sensorDist);
  }
  
  bool result = (sensorDist < minDistance) ? true : false ;
  
  sensorGoodValues = updateValues( result, sensorGoodValues, sensorIncr, sensorDecr );
  
  if ( sensorGoodValues > maxThreshold ){
    if ( sensorHitThreshold != true){
      sendNoise();
      sensorHitThreshold = true;
      digitalWrite(ledPin, HIGH);
    }
  } else 
      if ( sensorGoodValues < minThreshold ){
        sensorHitThreshold = false;
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
   Serial.write("t");
   delay(100);
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

unsigned int updateValues( boolean result, float values, int incr, int decr )
{
  if ( result ) 
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

  sensorIncr = (int)((float)maxThreshold / (float)sensorTimeUp) * ((float)periodDelay);
  sensorDecr = (int)(( (float)(population - minThreshold) / (float)sensorTimeDown ) * (float)periodDelay);
  buttonIncr = (int)((float)maxThreshold / (float)buttonTimeUp) * ((float)periodDelay);
  buttonDecr = (int)(( (float)(population - minThreshold) / (float)buttonTimeDown ) * (float)periodDelay);
  
 
  //RESET VAR

  sensorDist = 500;
  sensorGoodValues = 1;  
  buttonGoodValues = 1;
  buttonHitThreshold = false;
  sensorHitThreshold = false;
  
  if (debug) {
    Serial.print("maxThreshold: ");
    Serial.println(maxThreshold);
    Serial.print("minThreshold: ");
    Serial.println(minThreshold);
    Serial.print("population: ");
    Serial.println(population);
    Serial.print("minDistance: ");
    Serial.println(minDistance);
    Serial.print("sensorTimeUp: ");
    Serial.println(sensorTimeUp);
    Serial.print("sensorTimeDown: ");
    Serial.println(sensorTimeDown); 
    Serial.print("frequency: ");
    Serial.println(frequency);
    Serial.print("periodDelay: ");
    Serial.println(periodDelay);
    Serial.print("sensorIncr: ");
    Serial.println(sensorIncr);
    Serial.print("sensorDecr: ");
    Serial.println(sensorDecr);

  }
}
