/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

#ifndef JSON_H
#define JSON_H

#include <QJsonObject>
#include <QJsonDocument>
#include <QBluetoothAddress>
#include "track.h"
#include "tragediyimplementation.h"
#include <QUuid>


class Json {
public:
    static QByteArray getVelocityJson(QBluetoothAddress address, uint16_t speed);
    static QByteArray getAliveJson(QBluetoothAddress address, bool alive, QUuid uuid);
    static QByteArray getBatteryJson(QBluetoothAddress address, uint16_t batteryLevel, QUuid uuid);
    static QByteArray getTrackJson(QBluetoothAddress address, Track track, QUuid uuid, float xMin, float yMin, float xMax, float yMax);
    static QByteArray getPositionJson(QBluetoothAddress address, AnkiLocationTableEntry entry);
    static QByteArray getVehicleInfoJson(QBluetoothAddress address, bool charging, bool onTrack);
    static QByteArray getMessageJson(QUuid uuid, QString message);
    static QByteArray getEventJson(QBluetoothAddress address, QString event, uint16_t velocity);
    static QByteArray getLapTimeJson(QBluetoothAddress address, qint64 milliseconds);
    static QVariantMap parseJson(QString payload);
};

#endif // JSON_H
