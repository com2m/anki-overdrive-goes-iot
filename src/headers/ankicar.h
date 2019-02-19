/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

#ifndef ANKICAR_H
#define ANKICAR_H

#include <QObject>
#include <QString>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include "track.h"
#include "ankimessage.h"
#include <QBluetoothAddress>

class AnkiCar : public QObject {
    Q_OBJECT
public:
    explicit AnkiCar(QObject *parent = 0);
    ~AnkiCar();

    void init(const QBluetoothDeviceInfo& address);

    void setVelocity(uint16_t velocity = 0, uint16_t acceleration = 1000);
    void setLights(uint8_t lightValue = 0x88);
    void setEngineLight(uint8_t red, uint8_t green, uint8_t blue);
    uint16_t getVelocity();
    int getLane();

    void requestBatteryLevel();
    uint16_t getBatteryLevel();

    float getOffset();

    void setVehicleOffset(float offset = 0.0f, uint16_t velocity = 0, uint16_t acceleration = 2500);
    void setOffsetFromRoadCenter(float = 0.0f);

    void changeLane(float offset = 0.0f, uint16_t velocity = 250, uint16_t acceleration = 2500);
    void changeLane(int lane);

    void doUturn();

    void stop();

    void scanTrack();

    void cancelLaneChange();

    QBluetoothAddress getAddress();
    QString getName();

    TrackPiece::Type getCurrentSegment();

    bool stoppingAtStart();

    bool isCharging();
    bool isOnTrack();

    void driveToStart(int lane = 17);

    bool isAvailable();

    void reconnect();

    void setReverseDriving(bool value);

private:
    bool slowedDown = false;

    static const QBluetoothUuid SERVICE_UUID;
    static const QBluetoothUuid CHR_READ_UUID;
    static const QBluetoothUuid CHR_WRITE_UUID;

    QLowEnergyController* lowEnergyController = 0;
    QList<QBluetoothUuid> uuidList;

    QLowEnergyService* lowEnergyService = 0;
    QLowEnergyCharacteristic readCharacteristic;
    QLowEnergyCharacteristic writeCharacteristic;

    void sdkModeOn();
    void processIncomingMessage(AnkiMessage message);

    uint16_t velocity = 0;
    uint8_t lightValue = 0;
    uint16_t batteryLevel = 0;
    float offset = 0.0f;

    bool scanMode = false;
    bool initialized = false;

    Track track = Track();

    void sendMessage(AnkiMessage message);

    QBluetoothAddress address = QBluetoothAddress();
    QString name;

    TrackPiece::Type currentSegment = TrackPiece::SEGMENT_NOT_DEFINED;
    uint8_t currentPieceId = 0;

    bool lastSegmentBeforeFinishLine = false;
    bool stopAtStart = false;

    int destinationLane = 0;
    int currentLane = 0;

    bool charging = false;
    bool onTrack = true;

    bool available = false;

    bool changingLane = false;

    bool reverseDriving = false;

signals:
    // Signal which can be received by any class as soon as the ankicar object is ready (connected, calibrated, whatever)
    void ready();
    void disconnected();
    void batteryLevelUpdate(uint16_t value);
    void velocityUpdate();
    void trackScanCompleted(Track track);
    void finishLine();
    void positionUpdate(AnkiMessage message);
    void vehicleInfoUpdate(bool onTrack, bool charging);
    void sendMessage(QString message);
    void transition();
    void stoppedAtStart();

public slots:
    void serviceDiscovered(const QBluetoothUuid& uuid);
    void serviceDiscoveryFinished();
    void controllerError(const QLowEnergyController::Error& error);
    void deviceConnected();
    void deviceDisconnected();
    void serviceStateChanged(const QLowEnergyService::ServiceState& state);
    void characteristicWritten(const QLowEnergyCharacteristic& characteristic, const QByteArray& newValue);
    void characteristicRead(const QLowEnergyCharacteristic& characteristic, const QByteArray& value);
    void characteristicChanged(const QLowEnergyCharacteristic& characteristic, const QByteArray& value);
    void descriptorWritten(const QLowEnergyDescriptor& descriptor, const QByteArray& newValue);
};


#endif // ANKICAR_H
