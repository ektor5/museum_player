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

#default
HOST=192.168.0.57
KEY="/home/ektor-5/.ssh/chiave-ssh"

error() {
  #error($E_TEXT,$E_CODE)

  local E_TEXT=$1
  local E_CODE=$2
  
  [[ -z $E_CODE ]] && E_CODE=1
  [[ -z $E_TEXT ]] || echo $E_TEXT
  exit $E_CODE
}

usage(){
cat <<EOF
museum_player rsync script
usage: $0 -h [HOST] -k [KEY] 
EOF

exit
}

for arg 
do
shift
  case $arg in
  
  "--help") usage ;;
  
  "-h")
    #HOST 
    [ -n "$1" ] || usage
    HOST="$1" 
    ;;
    
  "-k") 
    #KEY
    [ -n "$1" ] || usage
    [ -e "$1" ] || error "Can't find key $1" 
    KEY="$1" 
    ;;
  *) ;;
  esac
  
done

rsync 	-e "ssh -i $KEY" \
	-vvtuLpr --delete-after  \
	opt/museum_player/ root@$HOST:/opt/museum_player/ && 

rsync 	-e "ssh -i $KEY" \
	-vvtLpr --size-only \
	etc/ root@$HOST:/etc/ && 

ssh -i "$KEY" root@$HOST "sync && 
  service museum_audio restart ; 
  service museum_video restart " 
