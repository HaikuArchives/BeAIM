#!/bin/sh

# A quick script to build a release of BeAIM.  Answer questions,
# get a installable zipfile...
#
# (c) 2003 gile@uselink.net

version="1.5.6"
base="`dirname "$0"`"
cd "$base"

x86_netkit() {
	alert --idea "Which system/networking kit is this for?" "R4-5/net_server" "R4-5/BONE" "Dano/BONE"
	case $? in
		0) proj="$proj net_server" ;;
		1) proj="$proj BONE" ;;
		2) proj="$proj Dano" ;;
	esac
}

ppc_netkit() {
	# i think PPC is only net_server, tell me if i'm wrong!
	proj="$proj net_server";
}

if [ ! -x "obj.x86/BeAIM" -a ! -x "obj.ppc/BeAIM" ]; then
	alert --stop "BeAIM binary is non-existant.  Open the project file appropriate to the platform you are building for, and press Alt-M to build it." "Close"
	exit 1
fi

if [ ! -x "Idle Time Filter/obj.x86/BeAIM_IdleTime_Filter.so" -a ! -x "Idle Time Filter/obj.ppc/BeAIM_IdleTime_Filter.so" ]; then
	alert --stop "Idle time filter is non-existant.  Open the project file appropriate to the platform you are building for, and press Alt-M to build it." "Close"
	exit 1
fi

proj="BeAIM $version"

alert --idea "For which platform is this installer being made for?" "Intel x86" "PowerPC"

case $? in
	0) proj="$proj x86" ; id='x86' ; x86_netkit ;;
	1) proj="$proj PPC" ; id='ppc' ; ppc_netkit ;;
esac

mkdir "tmp.$proj"

copyattr -d -r "BeAIM Installer" "tmp.$proj/BeAIM $version Installer"
copyattr -d "obj.$id/BeAIM" "tmp.$proj/BeAIM $version Installer/Install Files/BeAIM"
copyattr -d "Idle Time Filter/obj.$id/BeAIM_IdleTime_Filter.so" "tmp.$proj/BeAIM $version Installer/Install Files/BeAIM_IdleTime_Filter.so"

cd "tmp.$proj"

rm -f "../$proj.zip"
zip -r -9 -q "../$proj.zip" "BeAIM $version Installer"

cd ..

rm -rf "tmp.$proj"
