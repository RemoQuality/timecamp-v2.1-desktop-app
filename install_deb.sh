#!/bin/bash

sudo apt-get update
sudo apt-get install libc-bin

sudo pkill -f TimeCampDesktop

sudo dpkg -i ./*.deb

#xdg-icon-resource install --novendor --size 256 /usr/share/timecamp/icons/tc_tracking.png tc_tracking
#xdg-icon-resource install --novendor --size 256 /usr/share/timecamp/icons/tc_tracking_task.png tc_tracking_task
#xdg-icon-resource install --novendor --size 256 /usr/share/timecamp/icons/tc_not_tracking.png tc_not_tracking

/usr/bin/TimeCampDesktop &
