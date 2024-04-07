#! /bin/bash
# MODIFIED FOR THE HB63C09 ROM BUILD.
#  Currently supports up to 4 images.
# NOTES:
#	1) In this build, eFORTH requires running from RAM, thus
#		the eFORTH image must be offset to its intended binary
#		storage address located in the master ROM image.
#	2) The start address of all images is obtained from the
#		MON09 symbol reference file as it is the primary module
#		utilized at the RESET vector.

# MODIFIED FOR THE HB63C09 ROM BUILD.

DEBUG=0

MAXADDR=65536		# Maximum address space of the target CPU

#ROM_SIZE=8192		# 2xC64 - size of the intended ROM file
ROM_SIZE=16384		# 2xC128 - size of the intended ROM file
#ROM_SIZE=32768		# 2xC256 - size of the intended ROM file

# Symbol table holding start addresses
STARTADDRFILE=mon09/V37/mon09v37.sym

#define the SOURCE file PREFIX names
TGT1=mon09/V37/mon09v37						# MON09 Monitor
TGT1LA=$(egrep -i "CODE" < ${STARTADDRFILE} | awk '{print $3}' | sed 's/\$/0x/g')

#TGT2=basic/exbasrom/ExBasROM					# 6809 MS FP BASIC
#TGT2LA=$(egrep -i "FPBASSTRT" < ${STARTADDRFILE} | awk '{print $3}' | sed 's/\$/0x/g')

TGT2=basic/DigiCoolExBasic/ExtendedBasic09					# 6809 MS FP BASIC (DigiCoolExBasic)
TGT2LA=$(egrep -i "FPBASSTRT" < ${STARTADDRFILE} | awk '{print $3}' | sed 's/\$/0x/g')

#RES=$(egrep -m1 -i "JQ_EFORTH	EQU" <${TGT1}.asm | awk '{print $3}' )
#if [ $RES == 1 ] ; then		#Which FORTH?
#  TGT3=forth/eForth/V11/ef09						# eFORTH
#else
#  TGT3=forth/z79forth/V12/forth					# Z79 FORTH
#fi
#TGT3LA=$(egrep -i "FTHIMGSTRT" < ${STARTADDRFILE} | awk '{print $3}' | sed 's/\$/0x/g')

#TGT4=basic/basic/basic						# TinyBASIC
#TGT4LA=$(egrep -i "TNYBASSTRT" < ${STARTADDRFILE} | awk '{print $3}' | sed 's/\$/0x/g')

#define the TARGET and SOURCE file PREFIX names and TARGET load addresses

TGT9=BIOS								# Destination image
TGT9OS="-0x$(echo "obase=16; $MAXADDR - $ROM_SIZE" | bc )"
FILL="-fill 0xff 0x0000 0x$(printf '%x\n' ${ROM_SIZE})"

#
# ANSI COLORS
# source $(which colordefs.sh)		# source color definitions from file
#CRE="$(echo -e '\r\033[K')"
NML="$(echo -e '\033[0;39m')"
RED="$(echo -e '\033[1;31m')"
GRN="$(echo -e '\033[1;32m')"
YEL="$(echo -e '\033[1;33m')"
BLU="$(echo -e '\033[1;34m')"
MAG="$(echo -e '\033[1;35m')"
CYN="$(echo -e '\033[1;36m')"
WHT="$(echo -e '\033[1;37m')"
GRY="$(echo -e '\033[0;37m')"
INV0="$(echo -e '\033[1;27m')"		# inverse video off
INV1="$(echo -e '\033[1;7m')"		# inverse video on
BLNK0="$(echo -e '\033[25m')"		# blinking video off
BLNK1="$(echo -e '\033[5m')"		# blinking video on

#-----------------------------------------------------------------
# First insure that the segment images are up-to-date
for DIR in $TGT1 $TGT2 $TGT3 $TGT4 ; do
  if [ ! -z $DIR ] && [ -d $(dirname $DIR) ] ; then
    pushd $(dirname $DIR) >/dev/null
    echo "Rebuilding $(basename $DIR) ..."
    if [ -e build.sh ] ; then source build.sh ; fi
    if [ -e Makefile ] ; then make clean && make ; fi
    popd >/dev/null
    echo
    echo "Build completed"
    echo "**************************************************"
  fi
done

if [[ "$TGT3" == *eForth* ]] ; then
  #----------------------------------------------------------------
  #Special handling for the eFORTH image since it is stored in ROM
  #  and copied to RAM for execution.
  #
  #Fetch the load address from the S19 file
  TGT3OS=$(srec_info ${TGT3}.s19 -motorola | egrep -i data: | awk '{print $2}')
  TGT3OS=$(echo "ibase=16; $(echo ${TGT3LA} | sed 's/0x//g') - ${TGT3OS}" | bc)
  #Now offset the image to the intended ROM start address
  srec_cat ${TGT3}.s19 -motorola -offset ${TGT3OS} -o ${TGT3}.hex -intel -obs=16
elif [[ "$TGT3" == *z79forth* ]] ; then
  #Special handling for the Z79-FORTH image. Strips the RAM address contents.
  #
  #Fetch the load address from the HEX file
  TGT3OS=$(srec_info ${TGT3}.s19 -motorola | egrep -i "$(echo ${TGT3LA} | sed 's/0x//g')" | awk '{print $1}')
  #Now crop the ROM data
  srec_cat ${TGT3}.s19 -motorola -crop 0x8000 0xffff -o ${TGT3}.hex -intel -obs=16
fi

#----------------------------------------------------------------
#Now build the final ROM image from the individual source images.
echo
if [ -e ${TGT1}.hex ] || \
[ -e ${TGT2}.hex ] || \
[ -e ${TGT3}.hex ] || \
[ -e ${TGT4}.hex ] ; then
  echo
  echo "${MAG}Generating combined HEX and BIN files for ${YEL}${ROM_SIZE} ${MAG}byte image:"
  echo -e "\t${YEL}$TGT1${NML}"
  if [ -e ${TGT2}.hex ] ; then echo -e "\t ${MAG}and ${YEL}$TGT2${NML}" ; fi
  if [ -e ${TGT3}.hex ] ; then echo -e "\t ${MAG}and ${YEL}$TGT3${NML}" ; fi
  if [ -e ${TGT4}.hex ] ; then echo -e "\t ${MAG}and ${YEL}$TGT4${NML}" ; fi
  echo -e "\ton ${BLU}$(date)${NML}"
  echo

  if [ $DEBUG == 1 ] ; then
    echo "ROM_SIZE  = $ROM_SIZE"
    echo "TGT1   = ${TGT1}"
    echo "TGT1LA =${TGT1LA}"
    echo
    echo "TGT2   = ${TGT2}"
    echo "TGT2LA =${TGT2LA}"
    echo
    echo "TGT3   = ${TGT3}"
    echo "TGT3LA =${TGT3LA}"
    echo
    echo "TGT4   = ${TGT4}"
    echo "TGT4LA =${TGT4LA}"
    echo
    echo "TGT9   = $TGT9"
    echo "TGT9OS = $TGT9OS"
    echo
    echo "FILL   = ${FILL}"
  fi

  echo "${BLU}Combining source files to ${RED}${TGT9}${NML}"
  # Concatentate the targets
  srec_cat $TGT1.hex -intel $(if [ -e $TGT2.hex ] ; then echo "$TGT2.hex -intel" ; fi) \
  $(if [ -e $TGT3.hex ] ; then echo "$TGT3.hex -intel" ; fi) \
  $(if [ -e $TGT4.hex ] ; then echo "$TGT4.hex -intel" ; fi) \
  -o _$TGT9.hex -intel -obs=16
  srec_cat _$TGT9.hex -intel -offset ${TGT9OS} -o $TGT9.hex -intel -obs=16
  srec_cat $TGT9.hex -intel ${FILL} -o $TGT9.BIN -binary
  echo

  # delete programming files
  rm -f _$TGT9.hex
  for TGT in $TGT1 $TGT2 $TGT3 $TGT4 ; do
    if [ ${#TGT} -gt 0 ] ; then
      rm -f ${TGT}.bin ${TGT}.hex ${TGT}.ihex ${TGT}.s19
    fi
  done
  


else
  echo
  echo "${RED}Either ${YEL}$TGT1.hex ${RED}or ${YEL}$TGT2.hex ${RED}or ${YEL}$TGT3.hex ${RED}or ${YEL}$TGT4.hex not found!"
  echo
fi
