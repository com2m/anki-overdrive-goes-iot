/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

#include "headers/gamepadmanager.h"
#include <unistd.h>
#include <QDebug>

GamepadManager::GamepadManager(int count, QObject* parent) : QThread(parent) {

}

QList<Joystick*> GamepadManager::getGamepads() {
    return gamepadList;
}

Joystick* GamepadManager::getGamepad(int index) {
    if (index >= 0 && index < gamepadList.length()) {
        return gamepadList.at(index);
    } else {
        return 0;
    }
}

Joystick* GamepadManager::getGamepad(Racecar* racecar) {
    foreach (Joystick* gamepad, gamepadList) {
        if (gamepad->getRacecar() == racecar) {
            return gamepad;
        }
    }

    return 0;
}

void GamepadManager::addGamepad(Joystick* gamepad) {
    gamepadList.append(gamepad);
}

Joystick* GamepadManager::addGamepad(Racecar *racecar) {
    Joystick* gamepad = new Joystick(gamepadList.size());
    gamepad->setRacecar(racecar);

    gamepadList.append(gamepad);

    return gamepad;
}

void GamepadManager::removeGamepad(Joystick* gamepad) {
    gamepadList.removeAll(gamepad);
}

void GamepadManager::removeGamepad(Racecar* racecar) {
    Joystick* gamepad = getGamepad(racecar);

    if (gamepad) {
        gamepadList.removeAll(gamepad);
    }
}

void GamepadManager::run() {
    while (true) {
        usleep(1000);

        if (!gamepadList.empty()) {
            foreach (Joystick* gamepad, gamepadList) {
                JoystickEvent event;

                if (gamepad->sample(&event)) {
                    if (event.isButton()) {
                        switch (event.number) {
                        case 1: // Button B
                            if (!gamepad->buttonBInitialized()) {
                                gamepad->initializeButtonB();
                            }
                            else {
                                emit turboMode(gamepad->getRacecar(), (event.value)?true:false);
                            }
                            break;
                        case 2: // Button X
                            if (!gamepad->buttonXInitialized()) {
                                gamepad->initializeButtonX();
                            }
                            else {
                                emit doUturn(gamepad->getRacecar(), (event.value)?true:false);
                            }
                            break;
                        case 11: // Button Left
                            emit driveLeft(gamepad->getRacecar(), (event.value)?true:false);
                            break;
                        case 12: // Button Right
                            emit driveRight(gamepad->getRacecar(), (event.value)?true:false);
                            break;
                        }
                    }

                    if (event.isAxis()) {
                        switch (event.number) {
                        case 0: // Left joystick, x-axis
                            emit changeLane(gamepad->getRacecar(), mapAxisValue(event.value));
                            break;
                        case 5: // Right trigger
                            if (!gamepad->axisInitialized()) {
                                gamepad->initializeAxis();
                            }
                            else {
                                emit acceleratorChanged(gamepad->getRacecar(), mapTriggerValue(event.value));
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
}

// Convert joystick input to value between -1.0 and 1.0
double GamepadManager::mapAxisValue(short value) {
    return ((double)value / 32767.0);
}

// Convert trigger input to value between 0.0 and 1.0
double GamepadManager::mapTriggerValue(short value) {
    return ((double)(value + 32767) / 65534.0);
}


