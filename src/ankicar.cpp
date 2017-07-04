/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

#include "headers/ankicar.h"
#include <QDebug>
#include <QLowEnergyController>
#include <QLowEnergyCharacteristic>
#include <iostream>
#include "headers/ankimessage.h"
#include "headers/trackpiece.h"
#include <QLowEnergyConnectionParameters>
#include "headers/tragediyimplementation.h"

const QBluetoothUuid AnkiCar::SERVICE_UUID = QBluetoothUuid(QString("BE15BEEF-6186-407E-8381-0BD89C4D8DF4"));
const QBluetoothUuid AnkiCar::CHR_READ_UUID = QBluetoothUuid(QString("BE15BEE0-6186-407E-8381-0BD89C4D8DF4"));
const QBluetoothUuid AnkiCar::CHR_WRITE_UUID = QBluetoothUuid(QString("BE15BEE1-6186-407E-8381-0BD89C4D8DF4"));

AnkiCar::AnkiCar(QObject *parent) : QObject(parent) {

}

AnkiCar::~AnkiCar() {
    if (lowEnergyController != 0) {
        lowEnergyController->disconnectFromDevice();
    }
}

void AnkiCar::init(const QBluetoothDeviceInfo& device) {
    this->address = device.address();

    lowEnergyController = new QLowEnergyController(device, this);
    lowEnergyController->setRemoteAddressType(QLowEnergyController::RandomAddress);

    connect(lowEnergyController, SIGNAL(serviceDiscovered(QBluetoothUuid)), this, SLOT(serviceDiscovered(QBluetoothUuid)));
    connect(lowEnergyController, SIGNAL(discoveryFinished()), this, SLOT(serviceDiscoveryFinished()));
    connect(lowEnergyController, SIGNAL(error(QLowEnergyController::Error)), this, SLOT(controllerError(QLowEnergyController::Error)));
    connect(lowEnergyController, SIGNAL(connected()), this, SLOT(deviceConnected()));
    connect(lowEnergyController, SIGNAL(disconnected()), this, SLOT(deviceDisconnected()));

    lowEnergyController->connectToDevice();
}

void AnkiCar::serviceDiscovered(const QBluetoothUuid &uuid) {
    uuidList.append(uuid);
}

void AnkiCar::serviceDiscoveryFinished() {
    emit sendMessage("[" + getAddress().toString() + "]>> SERVICE DISCOVERY FINISHED.");
    qDebug().noquote().nospace() << "[" + this->getAddress().toString() + "]" << ">> SERVICE DISCOVERY FINISHED.";

    foreach (QBluetoothUuid uuid, uuidList) {
        if (uuid == SERVICE_UUID) {
            emit sendMessage("[" + getAddress().toString() + "]>> FOUND ANKI SERVICE.");
            qDebug().noquote().nospace() << "[" + this->getAddress().toString() + "]" << ">> FOUND ANKI SERVICE.";
            lowEnergyService = lowEnergyController->createServiceObject(uuid);

            connect(lowEnergyService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(serviceStateChanged(QLowEnergyService::ServiceState)));
            connect(lowEnergyService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)), this, SLOT(characteristicWritten(QLowEnergyCharacteristic,QByteArray)));
            connect(lowEnergyService, SIGNAL(characteristicRead(QLowEnergyCharacteristic,QByteArray)), this, SLOT(characteristicRead(QLowEnergyCharacteristic,QByteArray)));
            connect(lowEnergyService, SIGNAL(descriptorWritten(QLowEnergyDescriptor,QByteArray)), this, SLOT(descriptorWritten(QLowEnergyDescriptor,QByteArray)));

            lowEnergyService->discoverDetails();
        }
    }
}

void AnkiCar::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    (void)descriptor;
    (void)newValue;
    emit sendMessage("[" + getAddress().toString() + "]>> DESCRIPTOR SUCCESSFULLY WRITTEN.");
    qDebug().noquote().nospace() << "[" + this->getAddress().toString() + "]" << ">> DESCRIPTOR SUCCESSFULLY WRITTEN.";
}

// This function is called as soon as there are incoming messages from the vehicle
void AnkiCar::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value) {
    (void)characteristic;
    AnkiMessage response(value);

    processIncomingMessage(response);
}

// Get characteristics and enable notifications
void AnkiCar::serviceStateChanged(const QLowEnergyService::ServiceState &state) {
    if (state == QLowEnergyService::ServiceDiscovered) {
        foreach (QLowEnergyCharacteristic characteristic, lowEnergyService->characteristics()) {
            if (characteristic.uuid() == CHR_READ_UUID) {
                readCharacteristic = characteristic;

                if (characteristic.isValid()) {

                    QLowEnergyDescriptor notification = characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);

                    if (notification.isValid()) {
                        connect(lowEnergyService, &QLowEnergyService::characteristicChanged, this, &AnkiCar::characteristicChanged);

                        lowEnergyService->writeDescriptor(notification, QByteArray::fromHex("0100"));
                    }
                }
            }
            else if (characteristic.uuid() == CHR_WRITE_UUID) {
                writeCharacteristic = characteristic;
            }
        }

        emit sendMessage("[" + getAddress().toString() + "]>> SERVICE CHARACTERISTICS DISCOVERED.");
        qDebug().noquote().nospace() << "[" + this->getAddress().toString() + "]" << ">> SERVICE CHARACTERISTICS DISCOVERED.";

        sdkModeOn();
    }
}

void AnkiCar::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &value) {
    (void)characteristic;
    (void)value;
    emit sendMessage("[" + getAddress().toString() + "]>> READ ANKI CHARACTERISTIC.");
    qDebug() << "CHARACTERISTIC READ.";
}

void AnkiCar::sdkModeOn() {
    AnkiMessage sdkMessage(AnkiMessage::SDK_MODE);

    sendMessage(sdkMessage);

    emit sendMessage("[" + getAddress().toString() + "]>> SDK MODE ON.");
    qDebug().noquote().nospace() << "[" + this->getAddress().toString() + "]" << ">> SDK ON.";
}

void AnkiCar::setVelocity(uint16_t velocity, uint16_t acceleration) {
    AnkiMessage sdkMessage(AnkiMessage::SET_VELOCITY, velocity, acceleration);

    this->velocity = velocity;

    sendMessage(sdkMessage);
}

uint16_t AnkiCar::getVelocity() {
    return this->velocity;
}

uint16_t AnkiCar::getBatteryLevel() {
    return this->batteryLevel;
}

float AnkiCar::getOffset() {
    return this->offset;
}

void AnkiCar::doUturn() {
    AnkiMessage sdkMessage(AnkiMessage::UTURN);

    sendMessage(sdkMessage);
}

void AnkiCar::setVehicleOffset(float offset, uint16_t velocity, uint16_t acceleration) {
    AnkiMessage sdkMessage(AnkiMessage::CHANGE_LANE, offset, velocity, acceleration);

    sendMessage(sdkMessage);
}

void AnkiCar::setOffsetFromRoadCenter(float offset) {
    AnkiMessage sdkMessage(AnkiMessage::SET_OFFSET_FROM_ROADCENTER, offset);

    sendMessage(sdkMessage);
}

void AnkiCar::requestBatteryLevel() {
    AnkiMessage sdkMessage(AnkiMessage::BATTERY_REQUEST);

    sendMessage(sdkMessage);
}

void AnkiCar::changeLane(float offset, uint16_t velocity, uint16_t acceleration) {
    setVehicleOffset(offset, velocity, acceleration);
}

// This function is called after a message has been sent to the vehicle
void AnkiCar::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    (void)characteristic;

    AnkiMessage message(newValue);

    if (!initialized) {
        initialized = true;
        available = true;
        emit ready();
    }
}

void AnkiCar::controllerError(const QLowEnergyController::Error &error) {
    switch (error) {
    case QLowEnergyController::NoError:
        qDebug("No Error");
        break;
    case QLowEnergyController::UnknownError:
        qDebug("Unknown Error");
        initialized = false;
        break;
    case QLowEnergyController::UnknownRemoteDeviceError:
        qDebug("The remote Bluetooth LE device with the address pass to the constructor of this class cannot be found.");
        break;
    case QLowEnergyController::NetworkError:
        qDebug("Read/Write failed");
        break;
    case QLowEnergyController::InvalidBluetoothAdapterError:
        qDebug("Bluetooth adaptor not found");
        break;
    case QLowEnergyController::ConnectionError:
        qDebug("Connection attempt failed");
        break;
    case QLowEnergyController::AdvertisingError:
        qDebug("Advertising failed");
        break;
    }
}

void AnkiCar::deviceConnected() {
    emit sendMessage("[" + getAddress().toString() + "]>> CONNECTED.");
    qDebug().noquote().nospace() << "[" << getAddress().toString() << "]" << ">> CONNECTED.";
    lowEnergyController->discoverServices();
}

void AnkiCar::deviceDisconnected() {
    emit sendMessage("[" + getAddress().toString() + "]>> DISCONNECTED.");
    qDebug().noquote().nospace() << "[" + getAddress().toString() + "]" << ">> DISCONNECTED.";

    lowEnergyService->disconnect();

    uuidList.clear();
    velocity = 0;
    batteryLevel = 0;
    offset = 0.0f;

    scanMode = false;
    initialized = false;
    currentSegment = TrackPiece::SEGMENT_NOT_DEFINED;
    lastSegmentBeforeFinishLine = false;
    stopAtStart = false;
    destinationLane = 0;
    currentLane = 0;
    charging = false;
    onTrack = true;
    available = false;
    slowedDown = false;
    currentPieceId = 0;
    changingLane = false;
    reverseDriving = false;

    emit disconnected();
}

void AnkiCar::scanTrack() {
    track.clear();
    scanMode = true;
    setVelocity(300);
}

void AnkiCar::cancelLaneChange() {
    AnkiMessage sdkMessage(AnkiMessage::CANCEL_LANE_CHANGE);

    sendMessage(sdkMessage);
}

QBluetoothAddress AnkiCar::getAddress() {
    return this->address;
}

void AnkiCar::processIncomingMessage(AnkiMessage message) {

    if (scanMode && (message.getType() == AnkiMessage::POSITION_UPDATE || message.getType() == AnkiMessage::TRANSITION_UPDATE)) {
        if (message.getType() == AnkiMessage::POSITION_UPDATE) {            
            if (track.isComplete(message.getPieceId())) {
                stop();
                scanMode = false;

                emit trackScanCompleted(track);
            }
            else {
                track.append(message.getPieceId());

                if (message.reverseParsing()) {
                    track.setLastDirection(TrackPiece::DIRECTION_RIGHT);
                }
                else {
                    track.setLastDirection(TrackPiece::DIRECTION_LEFT);
                }
            }
        }
// Alternative solution based on wheel displacement

//        else if (message.getType() == AnkiMessage::TRANSITION_UPDATE) {
//            if (message.getLeftWheelDisplacement() < message.getRightWheelDisplacement()) {
//                track.setLastDirection(TrackPiece::DIRECTION_LEFT);
//            }
//            else {
//                track.setLastDirection(TrackPiece::DIRECTION_RIGHT);
//            }
//        }
    }

    if (message.getType() == AnkiMessage::BATTERY_RESPONSE) {
        batteryLevel = message.getBattery();
        emit batteryLevelUpdate(batteryLevel);
    }

    if (message.getType() == AnkiMessage::TRANSITION_UPDATE) {
        bool emitTransition = true;

        if ((message.getLeftWheelDisplacement() < 0x27) && (message.getLeftWheelDisplacement() > 0x20) && (message.getRightWheelDisplacement() < 0x27) && (message.getRightWheelDisplacement() > 0x20)) {
            emit finishLine();

            if (slowedDown) {
                currentPieceId = 34;
                stop();
                destinationLane = 0;
                stopAtStart = false;
                slowedDown = false;

                emit stoppedAtStart();

                emitTransition = false;
            }
        }

        if (emitTransition) {
            emit transition();
        }

        if (track.getTrackList().length() > 0) {
            if (!scanMode && this->currentPieceId == track.getTrackList().last().getPieceId()) {
                lastSegmentBeforeFinishLine = true;
            }
        }
    }

    if (message.getType() == AnkiMessage::POSITION_UPDATE) {

        if (stopAtStart && TragediyImplementation::getAnkiLocationTableEntry(message).getDistance() < 200 && !slowedDown) {
            setVelocity(100, 800);
            slowedDown = true;
        }

        this->reverseDriving = message.reverseDriving() ? true : false;


        this->offset = message.getOffset();
        this->currentSegment = (TrackPiece::Type)(message.getPieceId());
        this->currentPieceId = message.getPieceId();

        emit positionUpdate(message);

        if (changingLane) {

            currentLane = TragediyImplementation::getAnkiLocationTableEntry(message).getLane();
            int reverseDriving = message.reverseDriving() ? (-1) : (1);

            if (currentLane < destinationLane) {
                setOffsetFromRoadCenter(0.0f);
                changeLane((reverseDriving) * (destinationLane - currentLane) * 9.06, velocity);
            }
            else if (currentLane > destinationLane) {
                setOffsetFromRoadCenter(0.0f);
                changeLane((reverseDriving) * (-1)*(currentLane - destinationLane) * 9.06, velocity);
            }
            else {
                changingLane = false;
            }
        }
    }

    if (message.getType() == AnkiMessage::VEHICLE_INFO) {
        onTrack = message.onTrack();
        charging = message.charging();

        if (message.charging() && !message.onTrack()) {
            emit sendMessage("[" + getAddress().toString() + "]>> CHARGING, NOT ON TRACK.");
            qDebug().noquote().nospace() << "[" << getAddress().toString() << "]" << ">> CHARGING, NOT ON TRACK.";
        }
        else if (!message.charging() && !message.onTrack()){
            emit sendMessage("[" + getAddress().toString() + "]>> NOT CHARGING, NOT ON TRACK.");
            qDebug().noquote().nospace() << "[" << getAddress().toString() << "]" << ">> NOT CHARGING, NOT ON TRACK.";
        }
        else if (!message.charging() && message.onTrack()) {
            emit sendMessage("[" + getAddress().toString() + "]>> NOT CHARGING, ON TRACK.");
            qDebug().noquote().nospace() << "[" << getAddress().toString() << "]" << ">> NOT CHARGING, ON TRACK.";
        }

        emit vehicleInfoUpdate(message.onTrack(), message.charging());
    }
}

void AnkiCar::changeLane(int lane) {
    if (lane > 17) {
        destinationLane = 17;
    }
    else if (lane < 2) {
        destinationLane = 2;
    }
    else {
        destinationLane = lane;
    }

    changingLane = true;
}

void AnkiCar::sendMessage(AnkiMessage message) {
    lowEnergyService->writeCharacteristic(lowEnergyService->characteristic(CHR_WRITE_UUID), message.getMessage());
}

void AnkiCar::stop() {
    setVelocity(0);
    emit velocityUpdate();
}

TrackPiece::Type AnkiCar::getCurrentSegment() {
    return this->currentSegment;
}

bool AnkiCar::stoppingAtStart() {
    return stopAtStart;
}

bool AnkiCar::isCharging() {
    return charging;
}

bool AnkiCar::isOnTrack() {
    return onTrack;
}

void AnkiCar::driveToStart(int lane) {
    setVelocity(300, (uint16_t)12500);
    changingLane = true;
    stopAtStart = true;
    this->destinationLane = lane;
}

bool AnkiCar::isAvailable() {
    return available;
}

void AnkiCar::reconnect() {
    lowEnergyController->connectToDevice();
}

void AnkiCar::setReverseDriving(bool value) {
    reverseDriving = value;
}
