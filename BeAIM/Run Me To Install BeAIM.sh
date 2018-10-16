#! /bin/sh
#
# BeAIM 1.5.5 Installer
# Get BeAIM at:
# http://www.fifthace.com/beaim/
#

version="1.5.6"
base=`dirname "$0"`

if [ ! -x "$base/Install Files/BeAIM" ]; then
	alert --stop "The BeAIM exectuable was not found!  Have you built the binary yet?" "Exit" > /dev/null
	exit 1
fi

alert --info "Welcome to the BeAIM Installer! Do you want to continue installing BeAIM $version?" "Cancel" "Install" > /dev/null
if [ $? -eq 0 ]
then
	exit 0
fi

alert --info "The BeAIM program files and sound files will now be copied to their correct locations."

# make some directories
rm -rf /boot/apps/BeAIM
mkdir /boot/apps/BeAIM
mkdir /boot/apps/BeAIM/Languages
mkdir /boot/home/config/sounds/BeAIM

# nuke the earlier, stupid directory and links (if there)
rm -rf /boot/beos/apps/BeAIM

cp "$base/Install Files/BeAIM" /boot/apps/BeAIM/
cp "$base/Install Files/README" /boot/apps/BeAIM/
cp "$base/Install Files/Credits" /boot/apps/BeAIM/
cp "$base/Install Files/How to translate BeAIM" /boot/apps/BeAIM/
cp "$base/Install Files/In case of crashes..." /boot/apps/BeAIM/
cp -R "$base/Install Files/Languages" /boot/apps/BeAIM/
cp "$base/Install Files/BeAIM_IdleTime_Filter.so" /boot/home/config/add-ons/input_server/

cp "$base/Install Files/Sounds/AIMDrip" /boot/home/config/sounds/BeAIM
cp "$base/Install Files/Sounds/AIMEnter" /boot/home/config/sounds/BeAIM
cp "$base/Install Files/Sounds/AIMExit" /boot/home/config/sounds/BeAIM
cp "$base/Install Files/Sounds/AIMGotWarned" /boot/home/config/sounds/BeAIM
cp "$base/Install Files/Sounds/AIMNewMessage" /boot/home/config/sounds/BeAIM
cp "$base/Install Files/Sounds/AIMReceive" /boot/home/config/sounds/BeAIM
cp "$base/Install Files/Sounds/AIMSend" /boot/home/config/sounds/BeAIM

alert --idea "Would you like to put a link to BeAIM in your applications menu?" "No" "Yes" > /dev/null
if [ $? -eq 1 ]
then
	rm /boot/home/config/be/Applications/BeAIM
	ln -s /boot/apps/BeAIM/BeAIM /boot/home/config/be/Applications/BeAIM > /dev/NULL
fi

alert --idea "Would you like to put a link to BeAIM on your desktop?" "No" "Yes" > /dev/null
if [ $? -eq 1 ]
then
	rm /boot/home/Desktop/BeAIM
	ln -s /boot/apps/BeAIM/BeAIM /boot/home/Desktop/BeAIM > /dev/NULL
fi

alert --info "The input_server will now be restarted - your mouse and keyboard might not work for a second or two."
/system/servers/input_server -q

alert --info "BeAIM $version is now installed!"
