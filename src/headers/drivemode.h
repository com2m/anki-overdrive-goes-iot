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
#include "ConsoleReader.h"
#include "EventReader.h"
#include "mqttclient.h"
#include <QTimer>
#include "track.h"
#include "rgbled.h"
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QSoundEffect>



class DriveMode : public QObject {
    Q_OBJECT
private:
    // Configuration:
    const bool enableMqtt = false;
    const bool enableKeyboard = true;
    const bool enableRGBLed = true;
    const bool enableBackgroundMusic = true;

    const QString brokerIp = "127.0.0.1";
    const int brokerPort = 1883;
    const int brokerTimeout = 30;
    const QString brokerUser = "user";
    const QString brokerPassword = "password";
    // Channel used for transmission of car updates
    const QString c2sChannel = "cars";
    // Channel used for car control
    const QString s2cChannel = "control";

    const int numberOfRacecars = 2;

    uint16_t maxVelocity = 800;    // slowly: 600
    uint16_t nitroVelocity = 1200; // slowly: 800
    int acceleratorTolerance = 100;


    QUuid uuid;

    QList<int> lanes;

    QTimer* batteryUpdateTimer;
    QTimer* periodTimer;

    BluetoothController* bluetoothController;

    QList<Racecar*> racecarList;

    GamepadManager* gamepadManager;
    ConsoleReader* consoleReader;
    EventReader* eventReader;
    const QString eventDevice = "/dev/input/event0";

    QThread* gamepadThread;

    MqttClient *mqttClient;

    QMediaPlayer *player;
    QMediaPlaylist *playlist;
    QSoundEffect *fireSoundEffect1 = 0;
    QSoundEffect *fireSoundEffect2 = 0;
    
    void publishMessage(QByteArray message);

    Racecar* getRacecarByAddress(QBluetoothAddress address);
    void signalStatus(Racecar* racecar);
    void velocityChanged(Racecar* racecar);
    bool usingTurboMode = false;
    double usingSpeed = 0.0;
    bool gamePaused = false;
    RGBLed* statusLED;
    const double maximumBatteryLevel = 4200;
    
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
    void acceleratorUp(Racecar* racecar);
    void acceleratorDown(Racecar *racecar);

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
    bool setLightsForPeriod(Racecar* racecar, AnkiMessage::lightFeature lightsOn, int ms, AnkiMessage::lightFeature lightsOff);  
    void lightsPeriodUpdate(Racecar* racecar, AnkiMessage::lightFeature lightsOff); 
    void OnConsoleKeyPressed(char); 

    void onMqttMessage(MqttMessage mqttMessage);

    void scanTrack();
};

#endif // DRIVEMODE_H
