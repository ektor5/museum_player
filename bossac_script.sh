#!/bin/sh

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

# BOSSAC Flasher script

cd /opt/museum_player/sketch/

# museum_audio dialogs with the serial, must stop
service museum_audio stop

# flash
./bossac -e -w -v -p ttymxc3 -R -b ./museum_sketch.cpp.bin

# restart museum_audio
service museum_audio start

