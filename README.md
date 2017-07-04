# Anki OVERDRIVE SDK for C++
This project is a C++ implementation of the official SDK by Anki (written in C) using the Qt framework for various purposes.
It covers the functionality of the official SDK for the most part and also implements some undocumented functions.

*The Anki SDK can be found [here](https://github.com/anki/drive-sdk).*


## Disclaimer
The authors of this software are in no way affiliated to Anki. All naming rights for Anki, Anki Drive and Anki Overdrive are property of [Anki](anki.com).


## About
Additionally to the SDK the project contains a sample application called "Anki OVERDRIVE goes IoT".
This project was developed and tested on Linux **only**. To our best knowledge even the latest version of Qt doesn't currently support BLE on Windows, therefore it is recommended to run this software on Linux - a Raspberry Pi is more than sufficient.
The library used for reading gamepad inputs ([joystick++](https://github.com/drewnoakes/joystick)) also requires a UNIX system.
The sample application, Anki Overdrive goes IoT, establishes connection to up to four cars and enables the user to control them via gamepads. It further establishes connection to a MQTT broker to send status updates or receive commands from the cloud.

## Prerequisites

### Qt 5.7 (or newer)
As already mentioned, the project uses the Qt framework for various purposes. Since there are, at least until today, no appropriate versions of Qt available in the official Raspbian repositories (including Stretch), you have to download, compile and install Qt yourself.
The Qt framework (including the Bluetooth Modules!) is quite large and takes a few hours to compile and install. There are various instructions available on the internet; we recommend [this](http://www.tal.org/building_qt_5_for_raspberrypi_jessie) one.


### BlueZ 5.42 (or newer)
BlueZ is the implementation of the bluetooth protocol stack for Linux. By default Raspbian uses an "unstable" version, which leads to a connection abort after about two minutes. Therefore it is recommended to install at least version 5.42 of BlueZ - which is currently not included in in the official repositories.

To download, compile and install BlueZ 5.42 issue the following commands:

```bash

sudo apt-get install libical-dev
  
sudo cp /lib/systemd/system/hciuart.service /tmp/hciuart.service #Temporary Service-Backup
sudo apt-get purge bluez
  
mkdir -p bluez && cd bluez
wget http://www.kernel.org/pub/linux/bluetooth/bluez-5.42.tar.xz
tar xf bluez-5.42.tar.xz
cd bluez-5.42
  
# Patching source code
wget https://gist.github.com/pelwell/c8230c48ea24698527cd/archive/3b07a1eb296862da889609a84f8e10b299b7442d.zip
unzip 3b07a1eb296862da889609a84f8e10b299b7442d.zip
git apply -v c8230c48ea24698527cd-3b07a1eb296862da889609a84f8e10b299b7442d/*
  
./configure --prefix=/usr --mandir=/usr/share/man --sysconfdir=/etc --localstatedir=/var --enable-experimental --enable-maintainer-mode
  
make && sudo make install
  
sudo cp /tmp/hciuart.service /lib/systemd/system/hciuart.service #Restore previously saved service file
```

Additionally you have to enable the experimental features to use Bluetooth Low Energy:

Open the file /lib/systemd/system/bluetooth.service in your preferred text editor and change the following line
```bash
ExecStart=/usr/local/libexec/bluetooth/bluetoothd
```
to
```bash
ExecStart=/usr/local/libexec/bluetooth/bluetoothd --experimental
```

After that run:

```bash
sudo systemctl unmask bluetooth.service
sudo systemctl enable bluetooth.service
sudo systemctl enable hciuart.service
sudo systemctl daemon-reload
  
sudo reboot
  
sudo hciconfig hci0 up
```


### Microsoft® Xbox 360™ Wireless Receiver for Windows®
To install the appropriate driver to use your Xbox 360 controller via the Xbox 360™ Wireless Receiver for Windows® issue
```bash
sudo apt-get install xboxdrv
```

To test the functionality it is recommended to install the testing tool "joystick".
```bash
sudo apt-get install joystick
jstest /dev/input/js0
```


### MQTT
To be able to user the Mosquitto library for C++ you have to install the following packages:
```bash
sudo apt-get install libmosquittopp1 libmosquittopp-dev
```


## Compile and run
Finally you should be able to build an run the application. Open the cloned project directory and run the following commands to build the project.
```bash
qmake && make
```

Run the tool using
```bash
cd build
./ankioverdrive
```

You might want to change various settings according the usage of MQTT or the number of available cars. Those configurations can be made in the file *drivemode.h*.

## Further steps
The so called "drivemode", which is immediately started after launching the application, provides the possibility to control the anki cars via gamepads. It is an example of how the classes can be used for establishing connection to the cars or controlling them.
If you are interested in implementing your own drivemode, it might be useful to reproduce the processes as implemented in drivemode.cpp.

### (Very) Brief tutorial
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
