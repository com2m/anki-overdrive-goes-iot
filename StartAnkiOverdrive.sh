#!/bin/sh
echo "About to start Anki Overdrive"; sleep 3
cd /home/pi/Projects/anki-overdrive-goes-iot/build
[ -f AnkiOverdrive.log ] && cp AnkiOverdrive.log AnkiOverdrive.log.bak
[ -f btmon.log ] && cp btmon.log btmon.log.bak
echo starting AnkiOverdrive in $PWD >>StartAnkiOverdrive.log
date >>StartAnkiOverdrive.log
date >btmon.log
ps -ef | grep btmon >>btmon.log
sudo btmon >>btmon.log 2>&1 &
date >AnkiOverdrive.log
ps -ef | grep ankioverdrive >>AnkiOverdrive.log
./ankioverdrive >>AnkiOverdrive.log 2>&1
if [ $? -eq 12 ]
then
   date >>StartAnkiOverdrive.log
   echo "Anki Overdrive finished with shutdown [F12].">>AnkiOverdrive.log
   sudo shutdown -h now
fi
date >>StartAnkiOverdrive.log
echo "Anki Overdrive finished.">>AnkiOverdrive.log
echo killall btmon>>AnkiOverdrive.log
sudo killall btmon>>AnkiOverdrive.log
echo "Anki Overdrive finished."; sleep 1
