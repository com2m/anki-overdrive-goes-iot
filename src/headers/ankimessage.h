/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

#ifndef ANKIMESSAGE_H
#define ANKIMESSAGE_H

#include <QByteArray>
#include <QString>


class AnkiMessage {
public:
    enum Type {
        NOT_DEFINED = 0x00,

        // Driving commands
        SET_VELOCITY = 0x24,
        CHANGE_LANE = 0x25,
        CANCEL_LANE_CHANGE = 0x26,
        SET_OFFSET_FROM_ROADCENTER = 0x2C,
        UTURN = 0x32,

        // Battery level
        BATTERY_REQUEST = 0x1A,
        BATTERY_RESPONSE = 0x1B,

        // Vehicle position updates
        POSITION_UPDATE = 0x27,
        TRANSITION_UPDATE = 0x29,

        VEHICLE_INFO = 0x3f,

        // SDK-Mode
        SDK_MODE = 0x90
    };

    // Create AnkiMessage from response/existing message
    explicit AnkiMessage(QByteArray response);

    // Create empty AnkiMessage for certain message type
    explicit AnkiMessage(Type type);

    // Create SET_VELOCITY AnkiMessage
    AnkiMessage(Type type, uint16_t velocity, uint16_t acceleration = 1000);

    // Create CHANGE_LANE AnkiMessage
    AnkiMessage(Type type, float offset, uint16_t velocity = 250, uint16_t acceleration = 3000);

    Type getType();
    QByteArray getMessage();

    QString toString();
    void printMessage();

    uint16_t getBattery();

    // Set velocity if message type equals SET_VELOCITY
    bool setVelocity(uint16_t velocity = 0, uint16_t acceleration = 1000);

    // Get velocity if message type equals SET_VELOCITY
    uint16_t getVelocity();

    // Get offset if message type equals POSITION_UPDATE
    float getOffset();


    // POSITION_UPDATE
    uint8_t getLocationId();
    uint8_t getPieceId();

    // TRANSITION_UPDATE
    uint8_t getRoadPieceIdx();
    uint8_t getRoadPieceIdxPrev();

    uint8_t getDrivingDirection();

    uint8_t getLeftWheelDisplacement();
    uint8_t getRightWheelDisplacement();

    int getNumBits();
    bool reverseDriving();
    bool reverseParsing();

    // VEHICLE_INFO
    bool onTrack();
    bool charging();

private:
    enum Length {
        BATTERY_REQUEST_LENGTH = 0x01,
        SET_VELOCITY_LENGTH = 0x06,
        SDK_MODE_LENGTH = 0x03,
        CANCEL_LANE_CHANGE_LENGTH = 0x01,
        SET_OFFSET_FROM_ROADCENTER_LENGTH = 0x05,
        CHANGE_LANE_LENGTH = 0x0B,
        UTURN_LENGTH = 0x03,
    };

    enum DrivingDirection {
        FORWARD = 0x00,
        REVERSE = 0x01
    };

    QByteArray message;

    void configureMessage(Type type, int length = -1);

    // Set offset values if message type equals CHANGE_LANE/SET_OFFSET_FROM_ROADCENTER
    bool setOffsetFromCenter(float offset = 0.0f);
    bool setVehicleOffset(float offset = 0.0f, uint16_t velocity = 250, uint16_t acceleration = 1000);
};

#endif // ANKIMESSAGE_H
