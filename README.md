![](https://img.shields.io/badge/racing%20game-good-brightgreen.svg) ![](https://img.shields.io/badge/C%2B%2B%20Qt-mature-blue.svg)
![](https://img.shields.io/badge/Raspberry%20Pi-3B-orange.svg)

## Ready, steady, ...
![Anki Setup](http://www.t-h-schmidt.de/anki-overdrive-goes-iot/media/AnkiSetup.jpp "Anki Setup")
## Go!
![Go!](http://www.t-h-schmidt.de/anki-overdrive-goes-iot/media/AnkiGo.jpp "Go!")

# Anki OVERDRIVE Raspi gaming (and SDK for C++)
With an Anki Overdrive Starter set, a Raspberyy Pi 3B and a keyboard you can drive exciting races. Connecting speakers and a three color LED will add to the thrill.
This project uses a C++ implementation of the official Anki SDK made by [com2m](https://github.com/com2m/anki-overdrive-goes-iot).

*The Anki SDK can be found [here](https://github.com/anki/drive-sdk).*

## Getting Started
These instructions will get you a copy of the project up and running on your Raspberry Pi 3B for gaming and development purposes. See "Playing the game" for notes on how to run the  project on a live system.

### Prerequisites

#### Project sources
I used the directory `~/Projects/anki-overdrive-goes-iot` to build and run the project.
```bash
md ~/Projects/anki-overdrive-goes-iot
cd ~/Projects/anki-overdrive-goes-iot
git clone git@github.com/ThomasHeinrichSchmidt/anki-overdrive-goes-iot.git
```

#### Qt 5.7 (or newer)
The project uses the Qt framework for various purposes. It is available in the official Raspbian Stretch repositories (installed from ([NOOBS_v3_0_0.zip](https://www.raspberrypi.org/downloads/noobs/))
```bash
pi@raspberrypi:~ $ sudo apt-get install qt5-default  qt5-qmake  qtconnectivity5-dev
```

#### BlueZ 5.42 (or newer)
BlueZ is the implementation of the bluetooth protocol stack for Linux. Raspian Stretch comes with BlueZ 5.43, you may check using
```bash
pi@raspberrypi:~ $ systemctl status bluetooth         # shows "Bluetooth daemon 5.43"
pi@raspberrypi:~ $ sudo apt-get install pi-bluetooth  # only if Bluetooth needs to be re-installed
```
The Bluetooth stack does not seem to be 100% stable, therefor it is recommended to run the Bluetooth monitor `btmon` parallel to `ankioverdrive` (as shown in `StartAnkiOverdrive.sh` see below)

```bash
pi@raspberrypi:~ sudo btmon >>btmon.log 2>&1 &
```

#### Microsoft® Xbox 360™ Wireless Receiver for Windows®
To install the appropriate driver to use your Xbox 360 controller via the Xbox 360™ Wireless Receiver for Windows® issue
```bash
sudo apt-get install xboxdrv
```

To test the functionality it is recommended to install the testing tool "joystick".
```bash
sudo apt-get install joystick
jstest /dev/input/js0
```

#### MQTT
To be able to use the [Mosquitto](https://mosquitto.org/) library for C++ you have to install the following packages:
```bash
pi@raspberrypi:~ $ sudo apt-get install libmosquittopp1 libmosquittopp-dev

```

#### TRAGEDIY
The Anki Track Generator tool needs to be present in `~/Projects/anki-overdrive-goes-iot/build` for all functions of ankioverdrive to work properly. [Tragediy](https://github.com/NoveroResearch/tragediy) is available from Github.
```bash
sudo apt-get install cmake
sudo apt-get install libboost-dev libboost-filesystem-dev libboost-program-options-dev
git clone git@github.com:NoveroResearch/tragediy.git
cd tragediy
mkdir build
cd build
cmake ..
make
cp tragediy ~/Projects/anki-overdrive-goes-iot/build
```
TRAGEDIY assumes you extracted various track data from the Anki Android app. We don't provide this data as we assume it falls under the Anki copyright. Once the Anki Overdrive App is installed you'll find a folder /sdcard/Android/data/com.anki.overdrive on your phone. From there you need to copy
      `com.anki.overdrive/files/expansion/assets/rams/overdrive/basestation`
to the directory where your `ankioverdrive` executable is located, e.g. copy it to
      `~/Projects/anki-overdrive-goes-iot/build/com.anki.overdrive/files/expansion/assets/resources/basestation`
Note that you have to use `resources` as copy target instead of `rams`.
In this way the directory name for the tragediy -I parameter (used by `ankioverdrive`) is `com.anki.overdrive`      

#### USB Keyboard
Instead of using Xbox 360 controllers you can also connect a standard USB keyboard for controlling two cars. This even works when running headless (with no screen attached) by using the Linux `/dev/input/eventX` interface. You may disable this feature in `src/headers/drivemode.h`:  `enableKeyboard = false;`. To change the keyboard input device used go to `src/headers/EventReader.h`: `char *device = "/dev/input/event0";`

#### RGB LED     
To be able to see some game status you may connect an RGB LED to the Raspi's GPIO connector. I was using a LED with built-in resistors so that I do not need a breadboard but just 4 wires to connect the LED to GPIO22, GPIO10, GPIO9 + GND using the wiringPi pin numbers 3, 12, 13. You may switch off the feature uisng `src/headers/drivemode.h`: `enableRGBLed = false;` but if you don't you will just not see the LED signals if you do not connect one.

#### Background music
The game plays background music and some appropriate sounds if you press the fire buttons on the keyboard.
The music files are located in `anki-overdrive-goes-iot/build/media`:
*      AnkiOveride.mp3 - background music
*      Startup.wav, ByeBye.wav - what the name says
*      Duff1.wav, Duff2.wav - fire sounds for cars 1, 2

(You may switch off the music in  `src/headers/drivemode.h` using `enableBackgroundMusic = true;` or just by not connecting any speakers).

To make the music work for a Qt application you need to install
```bash
pi@raspberrypi:~ $ sudo apt-get install pulseaudio pulseaudio-module-zeroconf
pi@raspberrypi:~ $ pulseaudio -vvvv
pi@raspberrypi:~ $ ps -ae | grep pulseaudio              # should show as running
pi@raspberrypi:~ $ sudo apt-get install libqt5multimedia5-plugins
pi@raspberrypi:~ $ sudo apt-get install qtgstreamer-plugins-qt5
pi@raspberrypi:~ $ sudo apt-get install gstreamer1.0-fluendo-mp3
pi@raspberrypi:  $ sudo apt-get install gstreamer0.10-pulseaudio
pi@raspberrypi:~ $ sudo apt-get install gstreamer1.0-pulseaudio
```

#### Compile and run
Finally you should be able to build and run the application. Open the cloned project directory and run the following commands to build the project.
```bash
pi@raspberrypi:~/Projects/anki-overdrive-goes-iot $ qmake && make
```
The error message `/usr/include/c++/6/cstdlib:75:25: fatal error: stdlib.h: file or directory not found` can be removed
by adding   `QMAKE_CFLAGS_ISYSTEM = -I`   to `~/Projects/anki-overdrive-goes-iot/ankioverdrive.pro` according to [this arcticle](https://stackoverflow.com/questions/52532936/usr-include-c-7-cstdlib7515-fatal-error-stdlib-h-no-such-file-or-directo)

#### AutoStart
To play the game stand-alone (just the Raspi, an RGB LED, the keyboard, and speakers) you may prepare for a headless AutoStart. Open a terminal session and edit the file `~/.profile`: `nano ~/.profile`.
Add the following line to the end of the file, using `flock` to avoid starting the game twice, e.g. if using a SSH console to log in.
`flock -n $HOME/StartAnkiOverdrive.lockfile $HOME/StartAnkiOverdrive.sh`
The script itself looks like this
```bash
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
```
You need to make the script executable
```bash
pi@raspberrypi:~ $ chmod +x /home/pi/StartAnkiOverdrive.sh
```

## Playing the game
Generally you may run the game by starting the exectuable using
`cd ~/Projects/anki-overdrive-goes-iot/build` and `./ankioverdrive`

But the easiest way to play is "headless" without a screen using the AutoStart as described above.
* Build a track, switch on cars, put on track
* Remove screen, add USB keyboard, RGB LED, and speakers to Raspberry Pi, restart
* Play game using keyboard or gamepads

### Using the Keyboard
Use the following keys for controlling the cars (&uarr; &darr; ... are the cursor keys)

| Car  #1       | Car #2        | Effect |
|:------------: |:-------------:| -----   |
| W             |    &uarr;     | faster |
| S             |    &darr;     | slower |
| A             |    &larr;     | left   |
| D             |    &rarr;     | right  |
| space bar     |    Ins        | fire   |
| enter key     | keypad enter  | do U-turn |

| Key  | Effect |
|:------------: |-----   |
|  F1           | cars status (on console or log)|
| Home (Pos1)   | scan track (using tragediy) |
| F5            | drive to start </b>  (if track was previously scanned using Home) |
| F8            | clear road |
| F10           | quit game and reboot Raspi |
| F12           | quit game and shutdown Raspi |
| P             | pause game |
| 0 - 5         | speed 0%, 20%, ... 100% for all cars |
| T             | toggle turbo speed |
| Pause         | quit game (== Ctrl-C, ^C, SIGINT), return to console |


### Understanding the RGB LED     
The LED shows different colours depending on the status of the game.
* Scanning for cars (<font color="LawnGreen">green</font>)
* Bluetooth is active and cars were detected (<font color="DeepSkyBlue  ">blue</font>)
* if car is not on track (<font color="yellow">yellow</font> for 1s)
* if car is charging (<font color="cyan">cyan</font> for 1s)
* low battery (short blinking <font color="red">red</font> if battery < 50%, increase <font color="red">red</font> period if less battery)


### Bluethooth problems
* Cars sometimes stop and do no longer respond to the Bluetooth game commands. In this case reboot the Raspi by pressing F10 on the keyboard.
* If Bluetooth LE fails cars do no longer connect. Sometimes they start blinking red. After this they will no longer be able to connect themselves. If switching off and on again (using the tiny white plastic button under the car) does not help put them on the charger to reset. Remove after some time, switch off, put on charger again until cars no longer blink red.

## Built With

* [Qt 5](https://doc.qt.io/qt-5/index.html) - The Qt framework
* [Tragediy](https://github.com/NoveroResearch/tragediy) - Tool for constructing Anki tracks
* [Mosquitto](https://mosquitto.org/) - message broker for the MQTT protocol

## About
  The [com2m](https://github.com/com2m/anki-overdrive-goes-iot) SDK covers the functionality of the official Anki SDK for the most part and also implements some undocumented functions. Additionally to the SDK the project contains a sample application called "Anki OVERDRIVE goes IoT".
  The SDK project was developed and tested on Linux **only**. To our best knowledge even the latest version of Qt doesn't currently support BLE on Windows, therefore it is recommended to run this software on Linux - a Raspberry Pi is more than sufficient.
  The library used for reading gamepad inputs ([joystick++](https://github.com/drewnoakes/joystick)) also requires a UNIX system.
  The sample application, Anki Overdrive goes IoT, establishes connection to up to four cars and enables the user to control them via gamepads. It further establishes connection to a MQTT broker to send status updates or receive commands from the cloud.
  You may (alternatively) use a USB keyboard to control the cars, attach speakers to the audio jack for background music, and wire a RGB status LED to see Bluetooth connection, battery life etc. without the need of a screen attached to the Raspberry Pi.

### Further steps
You might want to change various settings according the usage of MQTT or the number of available cars. Those configurations can be made in the file *`drivemode.h`*.

The so called "drivemode", which is immediately started after launching the application, provides the possibility to control the anki cars via gamepads or keyboard. It is an example of how the classes can be used for establishing connection to the cars or controlling them.
If you are interested in implementing your own drivemode, it might be useful to reproduce the processes as implemented in `drivemode.cpp`.

### (Very) Brief tutorial for using the SDK
If you just want to get started with coding, this is the basic usage of the classes implemented in our project. It shows how to use the BluetoothController class to establish a connection and control the anki car afterwards.
```cpp
QList<AnkiCar*> ankiCarList;
ankiCarList.append(new AnkiCar());

BluetoothController* bluetoothController = new BluetoothController(ankiCarList, this);
...
AnkiCar* ankiCar = ankiCarList.at(0);
ankiCar->setVelocity(300);
ankiCar->changeLane(68.0f); //drive right
ankiCar->doUturn();
ankiCar->stop();
```

## License
This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Disclaimer
The authors of this software are in no way affiliated to Anki. All naming rights for Anki, Anki Drive and Anki Overdrive are property of [Anki](anki.com).

## Acknowledgments
* integrated improvements for `DriveMode::scanTrack()` from [Renji3](https://github.com/Renji3/anki-overdrive-goes-iot)
