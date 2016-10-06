#include "racecar.h"
#include <QDebug>
#include <QLowEnergyController>
#include <QLowEnergyCharacteristic>
#include <iostream>

using namespace std;

const QBluetoothUuid Racecar::SERVICE_UUID = QBluetoothUuid(QString("BE15BEEF-6186-407E-8381-0BD89C4D8DF4"));
const QBluetoothUuid Racecar::CHR_READ_UUID = QBluetoothUuid(QString("BE15BEE0-6186-407E-8381-0BD89C4D8DF4"));
const QBluetoothUuid Racecar::CHR_WRITE_UUID = QBluetoothUuid(QString("BE15BEE1-6186-407E-8381-0BD89C4D8DF4"));

Racecar::Racecar(QObject *parent) : QObject(parent)
{

}

void Racecar::init(const QBluetoothDeviceInfo& device)
{
    lowEnergyController = new QLowEnergyController(device, this);
    lowEnergyController->setRemoteAddressType(QLowEnergyController::RandomAddress);

    connect(lowEnergyController, SIGNAL(serviceDiscovered(QBluetoothUuid)),
            this, SLOT(serviceDiscovered(QBluetoothUuid)));
    connect(lowEnergyController, SIGNAL(discoveryFinished()),
            this, SLOT(serviceDiscoveryFinished()));
    connect(lowEnergyController, SIGNAL(error(QLowEnergyController::Error)),
            this, SLOT(controllerError(QLowEnergyController::Error)));
    connect(lowEnergyController, SIGNAL(connected()),
            this, SLOT(deviceConnected()));
    connect(lowEnergyController, SIGNAL(disconnected()),
            this, SLOT(deviceDisconnected()));

    lowEnergyController->connectToDevice();
}

void Racecar::serviceDiscovered(const QBluetoothUuid &uuid)
{
    uuidList.append(uuid);
}

void Racecar::serviceDiscoveryFinished()
{
    qDebug() << ">> SCAN DONE.";

    foreach (QBluetoothUuid uuid, uuidList)
    {
        if (uuid == SERVICE_UUID)
        {
            qDebug() << ">> FOUND ANKI SERVICE.";
            lowEnergyService = lowEnergyController->createServiceObject(uuid, this);

            connect(lowEnergyService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(serviceStateChanged(QLowEnergyService::ServiceState)));
            connect(lowEnergyService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)), this, SLOT(characteristicWritten(QLowEnergyCharacteristic,QByteArray)));
            connect(lowEnergyService, SIGNAL(characteristicRead(QLowEnergyCharacteristic,QByteArray)), this, SLOT(characteristicRead(QLowEnergyCharacteristic,QByteArray)));

            lowEnergyService->discoverDetails();
        }
    }
}

void Racecar::serviceStateChanged(const QLowEnergyService::ServiceState &state)
{
    if (state == QLowEnergyService::ServiceDiscovered)
    {
        foreach (QLowEnergyCharacteristic characteristic, lowEnergyService->characteristics())
        {
            if (characteristic.uuid() == CHR_READ_UUID) {
                readCharacteristic = characteristic;
            }
            else if (characteristic.uuid() == CHR_WRITE_UUID)
                writeCharacteristic = characteristic;
        }

        qDebug() << ">> SERVICE CHARACTERISTICS DISCOVERED.";

        sdkModeOn();
    }
}

void Racecar::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    qDebug() << "CHARACTERISTIC READ.";
}

void Racecar::sdkModeOn()
{
    QByteArray sdkMessage(4, 0x0);
    sdkMessage[0] = 0x03;
    sdkMessage[1] = 0x90;
    sdkMessage[2] = 0x01;
    sdkMessage[3] = 0x01;

    lowEnergyService->writeCharacteristic(lowEnergyService->characteristic(CHR_WRITE_UUID), sdkMessage);

    qDebug() << ">> SDK ON.";
}

void Racecar::setSpeed()
{
    QByteArray sdkMessage(7, 0x0);
    sdkMessage[0] = 0x06;
    sdkMessage[1] = 0x24;
    sdkMessage[2] = 0x2C;
    sdkMessage[3] = 0x01;
    sdkMessage[4] = 0xE8;
    sdkMessage[5] = 0x03;
    sdkMessage[6] = 0x0;

    lowEnergyService->writeCharacteristic(lowEnergyService->characteristic(CHR_WRITE_UUID), sdkMessage);
}

void Racecar::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    qDebug() << "Characteristic written" << characteristic.uuid();
}

void Racecar::controllerError(const QLowEnergyController::Error &error)
{
    switch (error) {
        case QLowEnergyController::NoError:
            qDebug("No Error");
            break;
        case QLowEnergyController::UnknownError:
            qDebug("Unknown Error");
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
            qDebug("Connection attemp failed");
            break;
        case QLowEnergyController::AdvertisingError:
            qDebug("Advertising failed");
            break;
    }
}

void Racecar::deviceConnected()
{
    lowEnergyController->discoverServices();
}

void Racecar::deviceDisconnected()
{
    qDebug() << "Remote device disconnected.";
}
