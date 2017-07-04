/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

#include "headers/racecar.h"

Racecar::Racecar(QObject *parent) : AnkiCar(parent)
{
    lastFinishTime = QDateTime();
}

void Racecar::setTmpSpeed(uint16_t value) {
    tmpSpeed = value;
}

uint16_t Racecar::getTmpSpeed() {
    return tmpSpeed;
}

bool Racecar::isSteering() {
    return steering;
}

bool Racecar::turboIsActive() {
    return turboMode;
}

void Racecar::isSteering(bool value) {
    steering = value;
}

void Racecar::turboIsActive(bool value) {
    turboMode = value;
}

bool Racecar::ignoreInputs() {
    return ignoreGamepadInputs;
}

void Racecar::ignoreInputs(bool value) {
    ignoreGamepadInputs = value;
}

void Racecar::setLastFinishTime(QDateTime finishTime) {
    lastFinishTime = finishTime;
}

QDateTime Racecar::getLastFinishTime() {
    return lastFinishTime;
}
