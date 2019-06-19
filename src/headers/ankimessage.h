/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

#ifndef ANKIMESSAGE_H
#define ANKIMESSAGE_H

#include <QByteArray>
#include <QString>

#define LIGHT_ANKI_VEHICLE_MSG_IS_VALID(messageBits, LIGHT_ID) (((messageBits >> LIGHT_ID)  & 1) == TRUE)
#define LIGHT_ANKI_VEHICLE_MSG_GET_VALUE(messageBits, LIGHT_ID) ((messageBits >> (4 + LIGHT_ANKI_VEHICLE_MSG_HEADLIGHTS) & 1))
#define LIGHT_EFFECT_SET_VALUE(LIGHT_SOURCE, LIGHT_EFFECT)  (LIGHT_EFFECT << 4 | 1 << LIGHT_SOURCE)   
   // node.js: LIGHT_EFFECT_SET_VALUE(LIGHT_ENGINE, EFFECT_THROB):     (2 << 4 | 1 << 3).toString('16') = '28' 
   //                                                                   ^             ^
   //                                                        EFFECT_THROB      |      LIGHT_ENGINE 


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
        // Lights 
        SET_LIGHTS = 0x1d,    
        LIGHTS_PATTERN  = 0x33, 

        // SDK-Mode
        SDK_MODE = 0x90
    };

    enum lightFeature {
       BRAKE_LITE_ON =    0x22, // Turn on the back tail lights.  This is a solid red light 
       BRAKE_LITE_FLASH = 0x88, // Flash the back tail lights.  This is a blinking red light.
       BRAKE_LITE_OFF   = 0x02, // Turn off the back tail lights.
       HEAD_LITE_FLASH  = 0x44, // Turn on the front flashing headlights.  This simulates the car shooting forward weapons.
       HEAD_LITE_OFF    = 0x04  // Turn off the front flashing headlights.
    };
    
    enum lightSource {
                           // Bits 0-3 light source
        HEADLIGHTS   = 0,  // ie. 0x_0
        BRAKELIGHTS  = 1,  //     0x_2
        FRONTLIGHTS  = 2,  //     0x_4
        ENGINE       = 3   //     0x_8
    };
    
    enum ligthEffect {
                          // Bits 4-7 light effect 
        STEADY      = 0,  // ie. 0x0_   // Simply set the light intensity to 'start' value              
        FADE        = 1,  //     0x1_   // Fade intensity from 'start' to 'end'                        
        THROB       = 2,  //     0x2_   // Fade intensity from 'start' to 'end' and back to 'start'
        FLASH       = 3,  //     0x3_   // Turn on LED between time 'start' and time 'end' inclusive
        RANDOM      = 4   //     0x4_   // Flash the LED erratically - ignoring start/end
    };
    

    // Create AnkiMessage from response/existing message
    explicit AnkiMessage(QByteArray response);

    // Create empty AnkiMessage for certain message type
    explicit AnkiMessage(Type type);

    // Create SET_LIGHTS AnkiMessage
    AnkiMessage(Type type, uint8_t lightValue);

    // Create LIGHTS_PATTERN AnkiMessage
    AnkiMessage(Type type, uint8_t red, uint8_t green, uint8_t blue);

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
    
    // Set lights if message type equals SET_LIGHTS
    bool setLights(uint8_t lightValue);

    // Set lights if message type equals LIGHTS_PATTERN 
    bool setEngineLight(uint8_t red, uint8_t green, uint8_t blue);

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

    QString getReadableMessage();

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
        SET_LIGHTS_LENGTH = 0x02, 
        LIGHTS_PATTERN_LENGTH = 0x11  
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
