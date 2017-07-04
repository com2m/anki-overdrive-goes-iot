/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

#include "headers/bluetoothcontroller.h"
#include <QDebug>
#include <QList>

BluetoothController::BluetoothController(QList<Racecar*> racecarList, QObject *parent) : QObject(parent) {
    this->racecarList = racecarList;

    init();
}

BluetoothController::BluetoothController(QList<AnkiCar*> ankiCarList, QObject *parent): QObject(parent) {
    this->ankiCarList = ankiCarList;

    init();
}

void BluetoothController::init() {
    deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);

    connect(deviceDiscoveryAgent, SIGNAL(deviceDiscovered(const QBluetoothDeviceInfo&)),
            this, SLOT(addDevice(const QBluetoothDeviceInfo&)));
    connect(deviceDiscoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)),
            this, SLOT(deviceScanError(const QBluetoothDeviceDiscoveryAgent::Error)));
    connect(deviceDiscoveryAgent, SIGNAL(finished()), this, SLOT(scanFinished()));

    deviceDiscoveryAgent->start();
}

void BluetoothController::addDevice(const QBluetoothDeviceInfo& device) {
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        if (device.name().endsWith("\u0001    Drive")) {
            this->racecarList.at(racecarIndex)->init(device);

            racecarIndex++;

            if (racecarIndex == racecarList.length()) {
                deviceDiscoveryAgent->stop();
            }
        }
    }
}

void BluetoothController::deviceScanError(const QBluetoothDeviceDiscoveryAgent::Error error) {
    switch(error) {
        case QBluetoothDeviceDiscoveryAgent::NoError:
            qDebug() << "No error has occured.";
            break;
        case QBluetoothDeviceDiscoveryAgent::PoweredOffError:
            qDebug() << "The Bluetooth adaptor is powered off.";
            break;
        case QBluetoothDeviceDiscoveryAgent::InputOutputError:
            qDebug() << "Writing or reading from the device resulted in an error.";
            break;
        case QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError:
            qDebug() << "The passed local adapter address does not match the physical adapter address of any local Bluetooth device.";
            break;
        case QBluetoothDeviceDiscoveryAgent::UnsupportedPlatformError:
            qDebug() << "Unsupported plattform.";
            break;
        case QBluetoothDeviceDiscoveryAgent::UnknownError:
            qDebug() << "An unknown error has occured.";
            break;
    }
}

void BluetoothController::scanFinished() {
    qDebug() << ">> SCAN FINISHED.";
}
