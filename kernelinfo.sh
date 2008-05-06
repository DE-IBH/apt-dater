#!/bin/sh
# Returns a string at stdout that represents the version of the running kernel in comparision to the latest installed standard kernel
# Return Codes: 0 - The running kernel is the latest standard kernel. No reboot required.
#               1 - The running kernel is an standard kernel but it's older then the latest installed. A reboot is recommended.
#               2 - No standard kernel is running.
#               9 - An error occured when running the script.

infostr='KERNELINFO:'
verfile='/proc/version'

if [ ! -f $verfile ] 
then
  echo $infostr '9'
  exit 0
fi

buff=$(eval "sed 's/).*//;s/^.*(//' $verfile | grep Debian | awk '{print $2}' " )
if [ -z "$buff" ]
then
  echo $infostr '2'
  exit 0
fi

REBOOT="0"
for ver in $(dpkg-query -W -f='${Version} ${Status;20} ${Maintainer}\n' 'linux-image*' | grep 'install ok installed Debian Kernel Team'|awk '{print $1}')
do
  dpkg --compare-versions $buff lt $ver && REBOOT="1"
done

echo $infostr $REBOOT
