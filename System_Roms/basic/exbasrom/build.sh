#! /bin/bash

ASM=lwasm
CPU="--6309"
#CPU="--6809"

if [ $# == 1 ] ; then
  PROG=$1
else
  PROG=ExBasROM
fi

#PROG=$(echo ${PROG} | tr [:upper:] [:lower:])	# convert to upper case

$ASM $CPU --tabs=8 --list=${PROG}.lst --symbol-dump=${PROG}.sym -f ihex --output=${PROG}.hex ${PROG}.asm
if [ $? != 0 ] ; then
  echo
  echo "Assembly error!"
  echo
  exit 1
fi

if [ -e ${PROG}.hex ] ; then
  # fill in the holes, if any
  # srec_cat ${PROG}.hex -intel --fill 0xff 0x0000 0x10000 -o ${PROG}.ihex -intel -obs=16
  RES=$(echo "ibase=16; $(srec_info ${PROG}.hex -intel | egrep -i data: | awk '{print $2}')" | bc)
  if [ $RES -gt 32768 ] ; then
    OFFSET="-0x8000"	# Re-map image to 0x0000
  else
    OFFSET="0x0000"	# No re-mapping
  fi
fi

if [ -e ${PROG}.hex ] ; then
  # Re-map to start at adress 0x0000 for the EEPROM
  srec_cat ${PROG}.hex -intel -offset ${OFFSET} -o ${PROG}.ihex -intel -obs=16

  # Make a binary image
  srec_cat ${PROG}.ihex -intel -o ${PROG}.bin -binary

  # Make S19 file
  srec_cat ${PROG}.ihex -intel -o ${PROG}.s19 -motorola
  #./makerom < ${PROG}.S19	# make the 32KB ROM image

  echo
  srec_info ${PROG}.hex -intel | sed 1d
  echo
fi
