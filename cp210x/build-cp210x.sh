#/bin/bash
#
# Description:
#    This script patches the 3.x.x in-tree kernel driver CP210X with the 
#    GPIO functionality added in the Silabs driver available here:
#    http://www.silabs.com/products/mcu/pages/usbtouartbridgevcpdrivers.aspx
#
#    Note: Please replace the original driver source file cp210x.c provided
#          with this script with the one from the kernel version you are using.
#
# Last Modified:
#    11/12/2013
#


DRVNAME=cp210x
MAKEFILE=Makefile

# check for necessary files and tools
echo "*** Checking dependencies ***"
if [ ! -f $DRVNAME.c ]; then
   echo "ERROR: original driver file $DRVNAME.c not found"
   exit
fi
if [ ! -f $DRVNAME.patch ]; then
   echo "ERROR: patch file $DRVNAME.patch not found"
   exit
fi
if [ ! -f $MAKEFILE ]; then
   echo "ERROR: make file $MAKEFILE not found"
   exit
fi
if [ -z $(which make) ]; then
   echo "ERROR: make tool is not installed"
   exit
fi
#TODO: check for kernel headers

# patch the driver
echo "*** Patching original driver ***"
patch $DRVNAME.c < $DRVNAME.patch

# check if patch applied successfully
if [ -f $DRVNAME.c.rej ]; then
   echo "ERROR: Patch did not apply correctly, see $DRVNAME.c.rej"
   exit
fi

# build the driver
echo "*** Building patched driver ***"
make

# check if make was successful
if [ ! -f $DRVNAME.ko ]; then
   echo "ERROR: building the patched driver failed"
   exit
fi

# replace original kernel module with new one
echo "*** Installing patched driver (need to have root privilages) ***"
sudo mv /lib/modules/$(uname -r)/kernel/drivers/usb/serial/$DRVNAME.ko /lib/modules/$(uname -r)/kernel/drivers/usb/serial/$DRVNAME.ko.orig
sudo cp $DRVNAME.ko /lib/modules/$(uname -r)/kernel/drivers/usb/serial
sudo depmod -a

echo "*** Done ***"
echo
echo "You can now load the new module with \"modprobe $DRVNAME\""
