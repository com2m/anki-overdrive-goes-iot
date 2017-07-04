/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

#ifndef RACECAR_H
#define RACECAR_H

#include "ankicar.h"
#include <QDateTime>


class Racecar : public AnkiCar
{
    Q_OBJECT
public:
    explicit Racecar(QObject *parent = 0);

    void setTmpSpeed(uint16_t value);
    uint16_t getTmpSpeed();

    bool isSteering();
    bool turboIsActive();

    void isSteering(bool value);
    void turboIsActive(bool value);

    bool ignoreInputs();
    void ignoreInputs(bool value);

    void setLastFinishTime(QDateTime finishTime);
    QDateTime getLastFinishTime();

private:
    uint16_t tmpSpeed = 0;
    bool steering = false;
    bool turboMode = false;
    bool ignoreGamepadInputs = true;

    QDateTime lastFinishTime;
};

#endif // RACECAR_H
