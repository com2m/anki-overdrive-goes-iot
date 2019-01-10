/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

#include "headers/ankimessage.h"
#include <QByteArray>
#include <QDebug>

AnkiMessage::AnkiMessage(Type type) {
    configureMessage(type);
}

AnkiMessage::AnkiMessage(Type type, uint16_t velocity, uint16_t acceleration) {
    configureMessage(type);

    setVelocity(velocity, acceleration);
}

AnkiMessage::AnkiMessage(QByteArray response) {
    configureMessage((Type)((int)response[1]), response.length());

    message.replace(message, response);
}

AnkiMessage::AnkiMessage(Type type, float offset, uint16_t velocity, uint16_t acceleration) {
    configureMessage(type);

    if (type == SET_OFFSET_FROM_ROADCENTER)
        setOffsetFromCenter(offset);
    else if (type == CHANGE_LANE)
        setVehicleOffset(offset, velocity, acceleration);
}

void AnkiMessage::configureMessage(Type type, int length) {
    switch (type) {
    case BATTERY_REQUEST:
        message = QByteArray(2, 0x00);
        message[0] = BATTERY_REQUEST_LENGTH;
        message[1] = BATTERY_REQUEST; // Message type 0x1A
        break;

    case SET_VELOCITY:
        message = QByteArray(7, 0x00);
        message[0] = SET_VELOCITY_LENGTH;
        message[1] = SET_VELOCITY; // Message type 0x24
        break;

    case BATTERY_RESPONSE:
        message = QByteArray(length, 0x00);
        break;

    case POSITION_UPDATE:
        message = QByteArray(length, 0x00);
        break;

    case TRANSITION_UPDATE:
        message = QByteArray(length, 0x00);
        break;

    case VEHICLE_INFO:
        message = QByteArray(length, 0x00);
        break;

    case SDK_MODE:
        message = QByteArray(4, 0x00);
        message[0] = SDK_MODE_LENGTH;
        message[1] = SDK_MODE; // Message type 0x90
        message[2] = 0x01; // ON
        message[3] = 0x01; // Override localization flag
        break;

    case SET_OFFSET_FROM_ROADCENTER:
        message = QByteArray(6, 0x00);
        message[0] = SET_OFFSET_FROM_ROADCENTER_LENGTH;
        message[1] = SET_OFFSET_FROM_ROADCENTER; // Message type 0x2C
        break;

    case CHANGE_LANE:
        message = QByteArray(12, 0x00);
        message[0] = CHANGE_LANE_LENGTH;
        message[1] = CHANGE_LANE; // 0x25
        break;

    case CANCEL_LANE_CHANGE:
        message = QByteArray(2, 0x00);
        message[0] = CANCEL_LANE_CHANGE_LENGTH;
        message[1] = CANCEL_LANE_CHANGE; // Message type 0x26
        break;

    case UTURN:
        message = QByteArray(4, 0x00);
        message[0] = UTURN_LENGTH;
        message[1] = UTURN; // Message type 0x32
        message[2] = 0x03; // 0x03 stands for U-Turn
        message[3] = 0x00; // Turn immediately. Alternative: 0x01 for turning at next intersection
        break;

    default:
        message = QByteArray(length, 0x00);
        break;
    }
}

void AnkiMessage::printMessage() {
    qDebug().noquote() << toString();
}

QByteArray AnkiMessage::getMessage() {
    return message;
}

AnkiMessage::Type AnkiMessage::getType() {
    if (message.length() >= 2)
        return (Type)((int)message[1]);
    else
        return NOT_DEFINED;
}

bool AnkiMessage::setVelocity(uint16_t velocity, uint16_t acceleration) {
    if (getType() == SET_VELOCITY) {
        // Arrange velocity and acceleration according to little endian (16-bit)
        message[2] = (velocity & 0xFF);
        message[3] = (velocity >> 8);
        message[4] = (acceleration & 0xFF);
        message[5] = (acceleration >> 8);

        return true;
    }
    else
        return false;
}

uint16_t AnkiMessage::getVelocity() {
    int velocity = 0;

    switch (getType()) {
    case SET_VELOCITY:
        velocity = message[3] << 8;
        velocity = velocity | message[2];
        break;
    case POSITION_UPDATE:
        velocity = message[9] << 8;
        velocity = velocity | message[8];
        break;
    case TRANSITION_UPDATE:
        velocity = message[14] << 8;
        velocity = velocity | message[13];
        break;
    case NOT_DEFINED:
    case CHANGE_LANE:
    case CANCEL_LANE_CHANGE:
    case SET_OFFSET_FROM_ROADCENTER:
    case UTURN:
    case BATTERY_REQUEST:
    case BATTERY_RESPONSE:
    case VEHICLE_INFO:
    case SDK_MODE:
        break;
    }

    return velocity;
}

float AnkiMessage::getOffset() {
    if (getType() == POSITION_UPDATE) {
        float offset;
        char offset_array[4];

        for (int i = 0; i < 4; i++) {
            offset_array[i] = message[4 + i];
        }

        memcpy(&offset, offset_array, sizeof offset);

        return offset;  // offset from road center [mm], signed
    }
    else
        return 0.0f;
}

bool AnkiMessage::setOffsetFromCenter(float offset) {
    if (getType() == SET_OFFSET_FROM_ROADCENTER) {
        // Transform float to little endian (32-bit)

        QByteArray offsetArray(reinterpret_cast<const char*>(&offset), sizeof(offset));

        message[2] = offsetArray[0];
        message[3] = offsetArray[1];
        message[4] = offsetArray[2];
        message[5] = offsetArray[3];

        return true;
    }
    else
        return false;
}

bool AnkiMessage::setVehicleOffset(float offset, uint16_t velocity, uint16_t acceleration) {
    if (getType() == CHANGE_LANE) {
        QByteArray offsetArray(reinterpret_cast<const char*>(&offset), sizeof(offset));

        // Transform velocity, acceleration, offset to little endian

        message[2] = (velocity & 0xFF);
        message[3] = (velocity >> 8);
        message[4] = (acceleration & 0xFF);
        message[5] = (acceleration >> 8);
        message[6] = offsetArray[0];
        message[7] = offsetArray[1];
        message[8] = offsetArray[2];
        message[9] = offsetArray[3];

        return true;
    }
    else
        return false;
}

uint16_t AnkiMessage::getBattery() {
    if (getType() == BATTERY_RESPONSE) {
        // Reassemble battery from 2x4 bit to int
        return (message[3] << 8) | message[2];
    }
    else
        return -1;
}

QString AnkiMessage::toString() {
    QString output;

    for (int i = 0; i < message.length(); i++) {
        output = output.append(QString("0x%1").arg(message.at(i), 2, 16, QLatin1Char('0')) + " ");
    }

    return output;
}

uint8_t AnkiMessage::getLocationId() {
    if (getType() == POSITION_UPDATE) {
        return message[2];
    }
    return 0;
}

uint8_t AnkiMessage::getPieceId() {
    if (getType() == POSITION_UPDATE) {
        return message[3];
    }
    return 0;
}

uint8_t AnkiMessage::getRoadPieceIdx() {
    if (getType() == TRANSITION_UPDATE) {
        return message[2];
    }
    return 0;
}

uint8_t AnkiMessage::getRoadPieceIdxPrev() {
    if (getType() == TRANSITION_UPDATE) {
        return message[3];
    }
    return 0;
}

uint8_t AnkiMessage::getDrivingDirection() {
    if (getType() == TRANSITION_UPDATE) {
        return message[8];
    }
    return 0;
}

uint8_t AnkiMessage::getLeftWheelDisplacement() {
    if (getType() == TRANSITION_UPDATE) {
        return message[16];
    }
    return 0;
}

uint8_t AnkiMessage::getRightWheelDisplacement() {
    if (getType() == TRANSITION_UPDATE) {
        return message[17];
    }
    return 0;
}

int AnkiMessage::getNumBits() {
    if (getType() == POSITION_UPDATE) {
        return message[10] & 0x0F;
    }
    return 0;
}

bool AnkiMessage::reverseParsing() {
    if (getType() == POSITION_UPDATE) {
        return (bool)(message[10] & 0x40);
    }
    return 0;
}

bool AnkiMessage::reverseDriving() {
    if (getType() == POSITION_UPDATE) {
        return (bool)(message[10] & 0x20);
    }
    return 0;
}

bool AnkiMessage::onTrack() {
    if (getType() == VEHICLE_INFO) {
        return (bool)(message[2]);
    }
    return 0;
}

bool AnkiMessage::charging() {
    if (getType() == VEHICLE_INFO) {
        return (bool)(message[3]);
    }
    return 0;
}
