/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

#ifndef BLUETOOTHCONTROLLER_H
#define BLUETOOTHCONTROLLER_H

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include "racecar.h"

class BluetoothController : public QObject {
    Q_OBJECT
public:
    BluetoothController(QList<Racecar*> racecarList, QObject *parent = 0);
    BluetoothController(QList<AnkiCar*> ankiCarList, QObject *parent = 0);

private:
    int racecarIndex = 0;

    QBluetoothDeviceDiscoveryAgent *deviceDiscoveryAgent;
    QList<AnkiCar*> ankiCarList;

    // Own, extended AnkiCar-Class. Contains additional properties used for drive mode
    QList<Racecar*> racecarList;

    void init();

signals:    

public slots:
    void addDevice(const QBluetoothDeviceInfo& device);
    void deviceScanError(const QBluetoothDeviceDiscoveryAgent::Error error);
    void scanFinished();
};

#endif // BLUETOOTHCONTROLLER_H
