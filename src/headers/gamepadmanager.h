/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

#ifndef GAMEPADMANAGER_H
#define GAMEPADMANAGER_H

#include <QObject>
#include "joystick.hh"
#include <QThread>
#include "racecar.h"

class GamepadManager : public QThread {
    Q_OBJECT
private:
    QList<Joystick*> gamepadList;
public:
    explicit GamepadManager(int count, QObject* parent = 0);

    QList<Joystick*> getGamepads();
    Joystick* getGamepad(int index);
    Joystick* getGamepad(Racecar* racecar);
    void addGamepad(Joystick* gamepad);
    Joystick* addGamepad(Racecar* racecar);
    void removeGamepad(Joystick* gamepad);
    void removeGamepad(Racecar* racecar);
    double mapAxisValue(short value);
    double mapTriggerValue(short value);

    void run() Q_DECL_OVERRIDE;
signals:
    void turboMode(Racecar* racecar, bool value);
    void doUturn(Racecar* racecar, bool value);
    void driveLeft(Racecar* racecar, bool value);
    void driveRight(Racecar* racecar, bool value);

    void changeLane(Racecar* racecar, double value);
    void acceleratorChanged(Racecar* racecar, double value);

public slots:
};

#endif // GAMEPADMANAGER_H
