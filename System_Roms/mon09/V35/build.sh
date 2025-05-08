#! /bin/bash
# This build script is specialized for the MON09 code

ASM=lwasm
CPU="--6309"
#CPU="--6809"

DATEFILE=datestring.asm

if [ $# == 1 ] ; then
  PROG=$1
else
  PROG=mon09v35
fi

#PROG=$(echo ${PROG} | tr [:upper:] [:lower:])	# convert to upper case

# Build a date string to be included in the MON09 sign-on message
#echo -e "\tFCC\t'Build: $(date +%c )'" > $DATEFILE
echo -e "\tFCC\t'Build: $(date +%Y-%m%d) @$(date +%H:%M:%S) $(date +%Z)'" > $DATEFILE

$ASM $CPU --tabs=8 --list=${PROG}.lst --symbol-dump=${PROG}.sym -f ihex --output=${PROG}.hex ${PROG}.asm
if [ $? != 0 ] ; then
  echo
  echo "Assembly error!"
  echo
  exit 1
fi

# Create the MON09 routine entry points for user programs
API=${PROG}_API.inc
echo "; MON09 ROM ENTRY POINTS FOR USER PROGRAMS" > ${API}
egrep "MON09_" < ${PROG}.sym | sort | awk '{print $1 "\t" $2 "\t" $3}' >> ${API}

RES=$(echo "ibase=16; $(srec_info ${PROG}.hex -intel | egrep -i data: | awk '{print $2}')" | bc)
if [ $RES -gt 32768 ] ; then
  OFFSET="-0x8000"	# Re-map image to 0x0000
else
  OFFSET="0x0000"	# No re-mapping
fi

#echo $OFFSET ; exit

if [ -e ${PROG}.hex ] ; then
  # Re-map to start at adress 0x0000 for the EEPROM
  srec_cat ${PROG}.hex -intel -offset ${OFFSET} -o ${PROG}.ihex -intel -obs=16

  # Make a binary image
  srec_cat ${PROG}.ihex -intel -o ${PROG}.bin -binary

  # Make S19 file
  srec_cat ${PROG}.hex -intel -o ${PROG}.s10 -motorola

  echo
  srec_info ${PROG}.hex -intel | sed 1d
  echo
fi
