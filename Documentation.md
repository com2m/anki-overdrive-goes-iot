
Anki Overdrive C++
==================
*as of 2017 (Raspbian Jessie)*

####Table of Contents

1. [Qt 5.7](#qt5.7)
2. [BlueZ 5.42](#bluez5.42)
3. [Building/launching the application](#build)
4. [Start application after boot](#autostart)

Getting started on Raspberry Pi
-------------------------------

Since the communication between the Anki cars and their controller is based on Bluetooth LE, it is necessary to use either the Raspberry Pi 3B or an appropriate Bluetooth LE adaptor.
However, the procedure has only been tested on Raspberry Pi 3B running Raspbian Jessie until now.


<div id="qt5.7" />
#### 1. Qt 5.7

Since the classes which are used to communicate with the Anki cars is based on Qt 5.7, the framework has to be installed in order to build the project.

Be aware that this whole process might take a lot of time. Configuring, compiling and installing may take up to 9 hours.
It is recommended to use a clean installation of Raspbian Jessie and a 32 GB microSD card.

Note: You may want to try to cross compile the source code, although this approach wasn't tested.


##### 1.1 Prerequisites

```bash
sudo apt-get update && sudo apt-get upgrade
sudo apt-get dist-upgrade
```

##### 1.2 Installing dependencies

```bash
sudo apt-get install libfontconfig1-dev libdbus-1-dev libfreetype6-dev libudev-dev libicu-dev libsqlite3-dev libxslt1-dev libssl-dev libasound2-dev libavcodec-dev libavformat-dev libswscale-dev libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev gstreamer-tools gstreamer0.10-plugins-good gstreamer0.10-plugins-bad libraspberrypi-dev libpulse-dev libx11-dev libglib2.0-dev libcups2-dev freetds-dev libsqlite0-dev libpq-dev libiodbc2-dev libmysqlclient-dev firebird-dev libpng12-dev libjpeg62-turbo-dev libgst-dev libxext-dev libxcb1 libxcb1-dev libx11-xcb1 libx11-xcb-dev libxcb-keysyms1 libxcb-keysyms1-dev libxcb-image0 libxcb-image0-dev libxcb-shm0 libxcb-shm0-dev libxcb-icccm4 libxcb-icccm4-dev libxcb-sync1 libxcb-sync-dev libxcb-render-util0 libxcb-render-util0-dev libxcb-xfixes0-dev libxrender-dev libxcb-shape0-dev libxcb-randr0-dev libxcb-glx0-dev libxi-dev libdrm-dev flex ruby gperf bison libts-dev libxcb-xinerama0 libxcb-xinerama0-dev libbluetooth-dev
sudo apt-get clean
```

##### 1.3 Getting Qt 5.7 source

```bash
mkdir -p qt && cd qt
wget http://download.qt.io/official_releases/qt/5.7/5.7.0/single/qt-everywhere-opensource-src-5.7.0.tar.gz
tar zxvf qt-everywhere-opensource-src-5.7.0.tar.gz
cd qt-everywhere-opensource-src-5.7.0
```

##### 1.4 Configuration

Qt will be configured to be installed at /usr/local/qt5.

```bash
./configure -v -opengl es2 -tslib -force-pkg-config -device linux-rasp-pi-g++ -device-option CROSS_COMPILE=/usr/bin/ -opensource -confirm-license -optimized-qmake -reduce-exports -release -qt-pcre -make libs -no-use-gold-linker -prefix /usr/local/qt5 2>&1 | tee config.out
```

##### 1.5 Compilation

```bash
time make -j3 2>&1 | tee make.out
```

##### 1.6 Installation

```bash
sudo make install
```

##### 1.7 Setting environment variables

```bash
export LD_LIBRARY_PATH=/usr/local/qt5/lib
export PATH=/usr/local/qt5/bin:$PATH
```

##### 1.8 Done

At this point the Qt framework should be installed. A reboot might be necessary.



<div id="bluez5.42" />
#### 2. BlueZ 5.42

BlueZ is the API which is used by Qt 5.7 to communicate with Bluetooth LE device. Unfortunately version 5.23 which natively comes with Raspbian Jessie contains a bug which causes the connection to be aborted. This bug doesn't (at least) occur in version 5.42.

##### 2.1 Prerequisites

```bash
sudo apt-get install libical-dev
```

##### 2.2 hciuart.service backup

```bash
sudo cp /lib/systemd/system/hciuart.service /tmp/hciuart.service
```

##### 2.3 Removing current version of bluez

```bash
sudo apt-get purge bluez
```

##### 2.4 Getting BlueZ 5.42 source

```bash
mkdir -p bluez && cd bluez
wget http://www.kernel.org/pub/linux/bluetooth/bluez-5.42.tar.xz
tar xf bluez-5.42.tar.xz
cd bluez-5.42
```

##### 2.5 Patching source code

```bash
wget https://gist.github.com/pelwell/c8230c48ea24698527cd/archive/3b07a1eb296862da889609a84f8e10b299b7442d.zip
unzip 3b07a1eb296862da889609a84f8e10b299b7442d.zip
git apply -v c8230c48ea24698527cd-3b07a1eb296862da889609a84f8e10b299b7442d/*
```

##### 2.6 Configuration

```bash
./configure --prefix=/usr --mandir=/usr/share/man --sysconfdir=/etc --localstatedir=/var --enable-experimental --enable-maintainer-mode
```

##### 2.7 Compilation and installation

```bash
make && sudo make install
```

##### 2.8 Restoring hciuart.service

```bash
sudo cp /tmp/hciuart.service /lib/systemd/system/hciuart.service
```

##### 2.9 Activating experimental features

```bash
sudo nano /lib/systemd/system/bluetooth.service
```

Change the following line

```bash
ExecStart=/usr/local/libexec/bluetooth/bluetoothd
```

to

```bash
ExecStart=/usr/local/libexec/bluetooth/bluetoothd --experimental
```

##### 2.10 Restarting and enabling services

```bash
sudo systemctl unmask bluetooth.service
sudo systemctl enable bluetooth.service
sudo systemctl enable hciuart.service
sudo systemctl daemon-reload
```

##### 2.11 Reboot

```bash
sudo reboot
```

##### 2.12 Activate Bluetooth

```bash
sudo hciconfig hci0 up
```

<div id="build" />
#### 3. Building/launching the application

Assuming that you have already cloned the repostiory, you have to run the following commands to build and run the application.

```bash
qmake
make
./ankioverdrive
```

<div id="autostart" />
#### 4. Start application after boot

To active the bluetooth adaptor after boot create the file /etc/udev/10-local.rules with the following content:

```bash
# Set bluetooth power up
ACTION=="add", KERNEL=="hci0", RUN+="/usr/bin/hciconfig hci0 up"
```

Save ankioverdrive.service to /etc/systemd/system:

```bash
[Unit]
Description=Anki Overdrive Service

[Service]
Type=idle
ExecStart=/home/pi/ankioverdrive/ankioverdrive
ExecStop=/usr/bin/pkill -f ankioverdrive

[Install]
WantedBy=multi-user.target
```

Run the following commands:

```bash
sudo chmod 664 /etc/systemd/system/ankioverdrive.service
sudo systemctl daemon-reload
sudo systemctl enable ankioverdrive.service
```
