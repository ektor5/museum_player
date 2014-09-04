
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

const int ledPin = 53;
const int pingPin = 7;
const int buttonPin = 6;

float maxThreshold = 0.5 ;
float minThreshold = 0.1 ;
long minDistance = 350;
unsigned int population = 500; 
unsigned int incr = 50;
unsigned int decr = 1;

long dist;
unsigned int goodValues = 1;
boolean hitThreshold = false;
boolean canSendTrack = false;
float meanValue;

void setup() {
  // initialize serial:
  Serial.begin(115200);
  Serial.println("START");
  Serial.flush();
  
  // set pinout
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(buttonPin, INPUT);
  attachInterrupt(buttonPin, sendTrack, RISING);
}

void loop() {
  
  while ( Serial.available() == 6 ) 
  {  
    maxThreshold =   (float)((unsigned int)Serial.read()) / 10;  
    minThreshold =   (float)((unsigned int)Serial.read()) / 10;
    minDistance =    (unsigned int)Serial.read() * 10;
    population =     (unsigned int)Serial.read() * 10;
    incr = 	     (unsigned int)Serial.read();
    decr =           (unsigned int)Serial.read();
    
    digitalWrite(ledPin, HIGH);
    delay(1000);
    digitalWrite(ledPin, LOW);

    //reset
    dist = 500 ;
    goodValues = 1;
    meanValue = 0 ;
    hitThreshold = false;
    canSendTrack = false;
    
    Serial.println(maxThreshold);
    Serial.println(minThreshold);
    Serial.println(minDistance);
    Serial.println(population);
    Serial.println(decr);
    Serial.println(incr);
    
  }
  
  if ( canSendTrack ) 
  {
    Serial.write("t");
    delay(1000);
    canSendTrack = false;
  }
  
  dist = distancePing();
  goodValues = updateValues( dist, goodValues );
  //Serial.println(goodValues);
  meanValue = calcMeanValue( goodValues );
  
  if ( meanValue > maxThreshold ){
    if ( hitThreshold != true){
      sendNoise();
      hitThreshold = true;
      digitalWrite(ledPin, HIGH);
    }
  } else 
      if ( meanValue < minThreshold ){
        hitThreshold = false;
        digitalWrite(ledPin, LOW);
      }
  delay(5);
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
}

long distancePing()
{
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

unsigned int updateValues( long dist, unsigned int values )
{
  if ( dist < minDistance ) 
  {
    values = (values > population-incr ) ? population : (values+incr) ; 
  } else { 
    values = (values < decr) ? 0 : (values-decr);
  }

  return values;
}  

float calcMeanValue( int goodValues ) 
{
  return (float)goodValues / (float)population;
}
