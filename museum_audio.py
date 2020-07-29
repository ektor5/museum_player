#!/usr/bin/python3.7

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

import re
import os
import random
import configparser

import RPi.GPIO as GPIO  
GPIO.setmode(GPIO.BCM)  
GPIO.setup(23, GPIO.IN, pull_up_down=GPIO.PUD_UP)  

print("Starting museum_audio...")

#SETUP
configfile='/etc/museum_player.conf'
working_dir = "/opt/museum_player/audio"
noise_probability=100

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
    os.system("aplay -D plughw:DEV=0 " + file + " >/dev/null")

while 1:  
  # read a 1 byte packet, wait
  #packet = ser.read()

  #NOISE STATEMENT
  # if packet == p_noise:
  #   print("Noise packet detected")
  #   #choose if play a noise
  #   if random.randint(0,100) < noise_probability:
  #     #choose randomly
  #     noise_chosed = random.choice(noise_list)
  #     play(noise_path+"/"+noise_chosed)
  
  #TRACK STATEMENT
  try:  
    print ("Waiting for falling edge on port 23"  )
    GPIO.wait_for_edge(23, GPIO.FALLING)  
    print ("Falling edge detected."  )
  
    print("Track packet detected")
    #play all tracks
    for track in track_list:
      play(track_path+"/"+track)
      
  except KeyboardInterrupt:  
    GPIO.cleanup()       # clean up GPIO on CTRL+C exit  

GPIO.cleanup()           # clean up GPIO on normal exit  
