#ifndef RACECAR_H
#define RACECAR_H

#include <QObject>
#include <QLowEnergyController>
#include <QLowEnergyService>

class Racecar : public QObject
{
    Q_OBJECT
public:
    explicit Racecar(QObject *parent = 0);
    void init(const QBluetoothDeviceInfo& address);

private:
    static const QBluetoothUuid SERVICE_UUID;
    static const QBluetoothUuid CHR_READ_UUID;
    static const QBluetoothUuid CHR_WRITE_UUID;

    QLowEnergyController* lowEnergyController;
    QList<QBluetoothUuid> uuidList;

    QLowEnergyService* lowEnergyService;
    QLowEnergyCharacteristic readCharacteristic;
    QLowEnergyCharacteristic writeCharacteristic;

    void sdkModeOn();
    void setSpeed();

signals:

public slots:
    void serviceDiscovered(const QBluetoothUuid& uuid);
    void serviceDiscoveryFinished();
    void controllerError(const QLowEnergyController::Error& error);
    void deviceConnected();
    void deviceDisconnected();
    void serviceStateChanged(const QLowEnergyService::ServiceState& state);
    void characteristicWritten(const QLowEnergyCharacteristic& characteristic, const QByteArray& newValue);
    void characteristicRead(const QLowEnergyCharacteristic& characteristic, const QByteArray& value);
};

#endif // RACECAR_H
