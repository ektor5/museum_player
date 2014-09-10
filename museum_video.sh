#!/bin/bash

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


VIDEO_ROOT="/opt/museum_player/video"
PLAYER="/usr/bin/gst-launch-0.10"
DEFAULT="default"

HOSTNAME=`cat /etc/hostname`

declare -i STATION
STATION=${HOSTNAME##*-}

#set alpha to 0
set_alpha fb0 0 

error() 
{
	echo $1 >&2                                                                                                 
	exit 1
}

#FATAL
if [[ ! -d $VIDEO_ROOT ]]
then 
	error "$VIDEO_ROOT does not exist"
fi

if [[ ! -x $PLAYER ]]  
then 
	error "I cannot execute $PLAYER"
fi

VIDEO_DIR="$VIDEO_ROOT/$STATION" 

if [[ $STATION != 0 ]] && [[ -d "$VIDEO_DIR" ]] && [[ "$(ls -A $VIDEO_DIR/*)" ]]
then
	VIDEO_DIR="$VIDEO_ROOT/$STATION" 
else
	VIDEO_DIR="$VIDEO_ROOT/$DEFAULT"
fi

if [[ ! "$(ls -A $VIDEO_DIR/*)" ]] 
then
	error "No file available"
fi

while [ 1 ]
do
	for i in $VIDEO_DIR/*
	do
		$PLAYER playbin2 uri=file://$i 
	done
done

