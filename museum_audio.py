#!/usr/bin/python3

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
#

import serial   #not compatible with py2?
#import daemon   #not compatible with py3
import re
import os
import random
import configparser


#SETUP
print("Starting museum_audio...")

configfile='/etc/museum_player.conf'


serial_device='/dev/ttymxc3'

serial_baud = 115200
working_dir = "/opt/museum_player/audio"

noise_probability=50
p_track=b't'
p_noise=b'n'

ard_maxThreshold = 0.5
ard_minThreshold = 0.1 
ard_minDistance = 350
ard_population = 500
ard_decr = 1
ard_incr = 50

#CONFIG
try:
  
  config = configparser.ConfigParser()
  
  config.readfp( open(configfile) )
  
  
  serial_device 	= config['player_setup']['serial_device']
  serial_baud 		= int(config['player_setup']['serial_baud'])
  
  working_dir 		= config['player_setup']['working_dir']

  noise_probability 	= int(config['player_setup']['noise_probability'])
  
  ard_maxThreshold 	= float(config['calibration']['maxThreshold'])
  ard_minThreshold 	= float(config['calibration']['minThreshold'])
  ard_minDistance 	= int(config['calibration']['minDistance'])
  ard_population 	= int(config['calibration']['population'])
  ard_decr 		= int(config['calibration']['decr'])
  ard_incr 		= int(config['calibration']['incr'])
  
  p_track=b't'
  p_noise=b'n'
  
except Exception as detail:
 print("WARNING: Cannot use config file " + configfile + ": " + str(detail.args[1]) )

#SERIAL  
try:
  ser = serial.Serial(serial_device, serial_baud)
  ser.flushOutput()
except FileNotFoundError as detail:
  print("FATAL: Cannot find " + serial_device )
  exit(1)
except Exception as detail:
  print("FATAL: " + str(detail.args[1]) )
  exit(1)
  
print("Serial device " + serial_device + " opened")
  
#CALIBRATION

byte1 =int.to_bytes(int(ard_maxThreshold*10) , 1 , byteorder='big')
byte2 =int.to_bytes(int(ard_minThreshold*10) , 1 , byteorder='big')
byte3 =int.to_bytes(int(ard_minDistance/10) , 1 , byteorder='big')
byte4 =int.to_bytes(int(ard_population/10) , 1 , byteorder='big') 
byte5 =int.to_bytes(int(ard_incr) , 1 , byteorder='big')
byte6 =int.to_bytes(int(ard_decr) , 1 , byteorder='big')

#print( byte1 )
#print( byte2 )
#print( byte3 )
#print( byte4 )
#print( byte5 )
#print( byte6 )

ser.write( byte1 ) 
ser.write( byte2 ) 
ser.write( byte3 )
ser.write( byte4 )
ser.write( byte5 )
ser.write( byte6 )

#HOSTNAME
try:
  hostname_file = open("/etc/hostname","r")
  hostname = hostname_file.read()
  hostname_file.close()
except FileNotFoundError:
  print("WARNING: Hostname not found! Defaulting.")
  hostname = "-default" 

#taking last part  
stage = re.split('-',hostname)[ -1 ]
#filtering
stage = re.sub('[^a-zA-Z_0-9]','', stage)

def findaudiofile(pos):
  #audio finder
  for file in os.listdir(pos):
    if os.path.isdir(pos+"/"+file):
      if findaudiofile(pos+"/"+file):
        return(1)
    
    if re.match(".*(mp3|wav|aac|flac)",file,flags=re.IGNORECASE):
      return(1)
    
  return(0)

def checkdir(position):
  #checker
  if not os.path.exists(position):
    return(0)
      
  return(findaudiofile(position))

#track check 
track_path = working_dir + "/track/" + stage

if not checkdir(track_path):
  print("WARNING: " + track_path + " not exists or useless. Defaulting." )
  track_path = working_dir + "/track/default"
  if not checkdir(track_path):
    print("FATAL: No valid track directory found! Check dir structure!") 
    exit(1)


#track load
track_list = os.listdir(track_path)

print("Track path: " + track_path )
print("Track load: " + str(len(track_list)) )

#noise check  
noise_path = working_dir + "/noise/" + stage

if not checkdir(noise_path):
  noise_path = working_dir + "/noise/default"
  if not checkdir(noise_path):
    print("No valid noise directory!") 
    exit(1)
    
#noise load
noise_list = os.listdir(noise_path)

print("Noise path: " + noise_path )
print("Noise load: " + str(len(noise_list)) )

#with daemon.DaemonContext(working_directory=position):
  
def play(file):
  os.system("aplay " + file + " >/dev/null")

while 1:  
  # read a 1 byte packet, wait
  packet = ser.read()

  #NOISE STATEMENT
  if packet == p_noise:
    print("Noise packet detected")
    #choose if play a noise
    if random.randint(0,100) < noise_probability:
      #choose randomly
      noise_chosed = random.choice(noise_list)
      play(noise_path+"/"+noise_chosed)
  
  #TRACK STATEMENT
  if packet == p_track:
    print("Track packet detected")
    #play all tracks
    for track in track_list:
      play(track_path+"/"+track)
      
  #don't listen to other messages    
  ser.flushInput()
  