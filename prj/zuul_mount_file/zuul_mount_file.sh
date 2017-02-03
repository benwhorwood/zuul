#!/bin/sh

KEYFILE="/tmp/keyfile1"
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


echo -n "Writing keyfile to $KEYFILE... "
./zuul_write_keyfile $KEYFILE
echo "Done"

echo -n "Mounting encrypted file system... "

# Map loop device to file
LOOP=$(losetup -f)
losetup $LOOP $ENCFILE

cryptsetup -d $KEYFILE luksOpen $LOOP $ENCLUKS
mount /dev/mapper/$ENCLUKS $MOUNTPT
echo "Done"

rm $KEYFILE
