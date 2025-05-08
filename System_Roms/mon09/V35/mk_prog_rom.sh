#! /bin/bash

# Define valid devices
DEVICE1=2816				# 2K EEPROM
DEVICE2=XL2816A				# 2K EEPROM
DEVICE3=X2816C				# 2K EEPROM

DEVICE4=AT28C64				#  8K EEPROM
DEVICE5=27C64@DIP28			#  8K UVEPROM

DEVICE6=27128A@DIP28		# 16K UVEPROM
DEVICE7=27C128@DIP28		# 16K UVEPROM
DEVICE8=NMC27C128C@DIP28	# 16K UVEPROM
DEVICE9=TMS27PC128@DIP28	# 16K UVEPROM

DEVICE10=AT28C256			# 32K EEPROM
DEVICE11=NM27C256@DIP28		# 32K UVEPROM
DEVICE12=27C256@DIP28		# 32K UVEPROM
DEVICE13=SST27SF256@DIP28	# 32K UVEPROM
DEVICE14=UPD27C256A@DIP28	# 32K UVEPROM

if [ $# == 0 ] ; then
  echo
  echo "Usage: $(basename $0) [Base File Name]"
  echo
else
  TARGET=${1%.*}
  # Create a menu for PROM type
  select DEVICE in $( echo $DEVICE1 $DEVICE2 $DEVICE3 $DEVICE4 $DEVICE5 $DEVICE6 $DEVICE7 \
	$DEVICE8 $DEVICE9 $DEVICE10 $DEVICE11 $DEVICE12 $DEVICE13 $DEVICE14 ) Quit ; do
    if [ "$DEVICE" == "Quit" ] ; then
      echo "Exiting..."
      exit
    fi
    break
  done

  # Set the proper ROM size in bytes
  case $DEVICE in
    2816|XL2816A|X2816C)
      ROM_SIZE=2048		# size of the intended ROM file
    ;;
    AT28C64|27C64@DIP28)
      ROM_SIZE=8192		# size of the intended ROM file
    ;;
    27128A@DIP28|27C128@DIP28|NM27C128C@DIP28|TMS27PC128@DIP28)
      ROM_SIZE=16384		# size of the intended ROM file
    ;;
    AT28C256|27C256@DIP28|SST27SF256@DIP28|NM27C256@DIP28|UPD27C256A@DIP28)
      ROM_SIZE=32768		# size of the intended ROM file
    ;;
    *)
  esac

  echo "Using $DEVICE with size of $ROM_SIZE bytes."
  echo "Exec: minipro -s -y -p "$DEVICE" -w ${TARGET}.bin"
  minipro -s -y -p "$DEVICE" -w ${TARGET}.bin
fi
