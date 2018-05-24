/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

#ifndef DRIVEMODE_H
#define DRIVEMODE_H

#include <QObject>
#include "joystick.hh"
#include "bluetoothcontroller.h"
#include "racecar.h"
#include "gamepadmanager.h"
#include "mqttclient.h"
#include <QTimer>
#include "track.h"

class DriveMode : public QObject {
    Q_OBJECT
private:
    // Configuration:
    const bool enableMqtt = false;

    const QString brokerIp = "127.0.0.1";
    const int brokerPort = 1883;
    const int brokerTimeout = 30;
    const QString brokerUser = "user";
    const QString brokerPassword = "password";
    // Channel used for transmission of car updates
    const QString c2sChannel = "cars";
    // Channel used for car control
    const QString s2cChannel = "control";

    const int numberOfRacecars = 4;

    uint16_t maxVelocity = 800;
    uint16_t nitroVelocity = 1200;
    int acceleratorTolerance = 100;


    QUuid uuid;

    QList<int> lanes;

    QTimer* batteryUpdateTimer;

    BluetoothController* bluetoothController;

    QList<Racecar*> racecarList;

    GamepadManager* gamepadManager;

    QThread* gamepadThread;

    MqttClient *mqttClient;

    void publishMessage(QByteArray message);

    Racecar* getRacecarByAddress(QBluetoothAddress address);
    void velocityChanged(Racecar* racecar);

    float xMin = 0.0f;
    float yMin = 0.0f;

public:
    explicit DriveMode(QObject *parent = 0);
    void quit();

signals:

public slots:
    void sendMessage(QString messagePayload);

    void turboMode(Racecar* racecar, bool value);
    void doUturn(Racecar* racecar, bool value);
    void driveLeft(Racecar* racecar, bool value);
    void driveRight(Racecar* racecar, bool value);

    void changeLane(Racecar* racecar, double value);
    void acceleratorChanged(Racecar* racecar, double value);

    void ready();
    void requestBatteryUpdate();
    void batteryLevelUpdateReceived(uint16_t value);
    void trackScanCompleted(Track track);
    void positionUpdate(AnkiMessage message);
    void finishLine();
    void disconnected();
    void vehicleInfoUpdate(bool onTrack, bool charging);
    void transition();
    void stoppedAtStart();
    void velocityUpdate();

    void onMqttMessage(MqttMessage mqttMessage);
};

#endif // DRIVEMODE_H
