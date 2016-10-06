#ifndef BLUETOOTHCONTROLLER_H
#define BLUETOOTHCONTROLLER_H

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include "racecar.h"

class BluetoothController : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothController(Racecar* racecar, QObject *parent = 0);

private:
    QBluetoothDeviceDiscoveryAgent *deviceDiscoveryAgent;
    Racecar* racecar;

signals:    

public slots:
    void addDevice(const QBluetoothDeviceInfo& device);
    void deviceScanError(const QBluetoothDeviceDiscoveryAgent::Error error);
    void scanFinished();
};

#endif // BLUETOOTHCONTROLLER_H
