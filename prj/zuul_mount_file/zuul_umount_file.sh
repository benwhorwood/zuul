#!/bin/bash

# Script vars
ENCFILE="/path/to/encdata.dat"
ENCLUKS="zuul-cont1"
MOUNTPT="/mnt/tmp"

# Root user verification
RUNUSER=`whoami` # Get current user

# Make sure running as root
# From: http://www.linuxforums.org/forum/linux-programming-scripting/11092-script-determine-if-current-user-root.html
if [ "$RUNUSER" != "root" ]
    then
    echo "This program must be executed as root"
    exit 1
fi

#echo "Running as $RUNUSER, good and verified"

# Unmount mapped encrypted loo device
echo -n "Unmounting loop device ${MOUNTPT}... "
umount $MOUNTPT
echo "Done"

# Close encrypted loop device
echo -n "Closing encrypted loop device ${ENCLUKS}... " 
cryptsetup luksClose "${ENCLUKS}"
echo "Done."

# Close loop device
#LOOP=$(losetup -a | grep "${ENCFILE}" | awk -F ":" '{print $1}' | head -n 1)
#echo -n "Removing loop device ${LOOP}... "
#losetup -d $LOOP
#echo "Done"
