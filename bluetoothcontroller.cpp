#include "bluetoothcontroller.h"
#include <QDebug>

BluetoothController::BluetoothController(Racecar* racecar, QObject *parent) : QObject(parent)
{
    this->racecar = racecar;

    deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);

    connect(deviceDiscoveryAgent, SIGNAL(deviceDiscovered(const QBluetoothDeviceInfo&)),
            this, SLOT(addDevice(const QBluetoothDeviceInfo&)));
    connect(deviceDiscoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)),
            this, SLOT(deviceScanError(const QBluetoothDeviceDiscoveryAgent::Error)));
    connect(deviceDiscoveryAgent, SIGNAL(finished()), this, SLOT(scanFinished()));

    deviceDiscoveryAgent->start();
}

void BluetoothController::addDevice(const QBluetoothDeviceInfo& device)
{
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        qDebug() << "Discovered LE Device name: " << device.name() << " Address: " << device.address();
        if (device.name().endsWith("Drive"))
        {
            qDebug() << ">> ATTEMPTING TO CONNECT TO" << device.address();

            deviceDiscoveryAgent->stop();
            this->racecar->init(device);
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
    qDebug() << "Scan finished.";
//    this->racecar->init(dev);
}
