// From: https://github.com/drewnoakes/joystick
// Modified by: com2m

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>

#include "headers/joystick.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sstream>
#include "unistd.h"
#include <QDebug>
#include <QFileInfo>
#include <errno.h>

Joystick::Joystick() {
  openPath("/dev/input/js0");
}

Joystick::Joystick(int joystickNumber) {
  this->joystickNumber = joystickNumber;
  std::stringstream sstm;
  sstm << "/dev/input/js" << joystickNumber;
  openPath(sstm.str());
}

Joystick::Joystick(const std::string& devicePath) {
  openPath(devicePath);
}

void Joystick::openPath(const std::string& devicePath) {
  errno = 0;
  _fd = open(devicePath.c_str(), O_RDONLY | O_NONBLOCK);
}

bool Joystick::sample(JoystickEvent* event) {
  std::stringstream sstm;
  sstm << "/dev/input/js" << joystickNumber;
  QString deviceName = QString::fromStdString(sstm.str());
  QFileInfo check_device(deviceName);
  
  if (!reInit && !check_device.exists()) {
    close(_fd);
    reInit = true;
  } else {
      if (reInit && check_device.exists()) {
          reInit = false;
    _fd = 0;
          openPath(deviceName.toStdString());
      }
  }

  int bytes = read(_fd, event, sizeof(*event));
  
  if (bytes == -1)
    return false;

  // NOTE if this condition is not met, we're probably out of sync and this
  // Joystick instance is likely unusable
  return bytes == sizeof(*event);
}

bool Joystick::isFound() {
  return _fd >= 0;
}

Joystick::~Joystick() {
  close(_fd);
}

void Joystick::setRacecar(Racecar *racecar) {
    this->racecar = racecar;
}

Racecar* Joystick::getRacecar() {
    return this->racecar;
}

void Joystick::initializeAxis() {
    axisInit = true;
}

bool Joystick::axisInitialized() {
    return axisInit;
}

void Joystick::initializeButtonB() {
    buttonBInit = true;
}

bool Joystick::buttonBInitialized() {
    return buttonBInit;
}

void Joystick::initializeButtonY() {
    buttonYInit = true;
}

bool Joystick::buttonYInitialized() {
    return buttonYInit;
}

void Joystick::initializeButtonX() {
    buttonXInit = true;
}

bool Joystick::buttonXInitialized() {
    return buttonXInit;
}

int Joystick::getJoyStickNumber() {
    return joystickNumber;
}
