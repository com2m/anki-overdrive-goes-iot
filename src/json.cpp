/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

#include "headers/json.h"

#include <QBluetoothAddress>
#include "headers/tragediyimplementation.h"
#include <QVariant>

QByteArray Json::getVelocityJson(QBluetoothAddress address, uint16_t speed) {
    QJsonObject jsonObject;

    jsonObject.insert("address", address.toString());
    jsonObject.insert("velocity", speed);

    QJsonDocument jsonDocument(jsonObject);

    return jsonDocument.toJson(QJsonDocument::Compact);
}

QByteArray Json::getAliveJson(QBluetoothAddress address, bool alive, QUuid uuid) {
    QJsonObject jsonObject;

    jsonObject.insert("address", address.toString());
    jsonObject.insert("alive", alive);
    jsonObject.insert("uuid", uuid.toString());

    QJsonDocument jsonDocument(jsonObject);

    return jsonDocument.toJson(QJsonDocument::Compact);
}

QByteArray Json::getBatteryJson(QBluetoothAddress address, uint16_t batteryLevel, QUuid uuid) {
    QJsonObject jsonObject;

    jsonObject.insert("address", address.toString());
    jsonObject.insert("battery", batteryLevel);
    jsonObject.insert("uuid", uuid.toString());

    QJsonDocument jsonDocument(jsonObject);

    return jsonDocument.toJson(QJsonDocument::Compact);
}


QByteArray Json::getTrackJson(QBluetoothAddress address, Track track, QUuid uuid, float xMin, float yMin, float xMax, float yMax) {
    QJsonObject jsonObject;

    jsonObject.insert("address", address.toString());
    jsonObject.insert("track", track.getTrackString());
    jsonObject.insert("uuid", uuid.toString());
    jsonObject.insert("xMin", xMin);
    jsonObject.insert("yMin", yMin);
    jsonObject.insert("xMax", xMax);
    jsonObject.insert("yMax", yMax);

    QString trackId;

    foreach (TrackPiece trackPiece, track.getTrackList()) {
        trackId.append(QString::number(trackPiece.getPieceId()));
    }

    jsonObject.insert("trackId", trackId);

    QJsonDocument jsonDocument(jsonObject);

    return jsonDocument.toJson(QJsonDocument::Compact);
}

QByteArray Json::getPositionJson(QBluetoothAddress address, AnkiLocationTableEntry entry) {
    QJsonObject jsonObject;

    jsonObject.insert("address", address.toString());
    jsonObject.insert("x", entry.getX());
    jsonObject.insert("y", entry.getY());
    jsonObject.insert("lane", entry.getLane());
    jsonObject.insert("distance", entry.getDistance());
    jsonObject.insert("backward", (entry.getBackward() != 0)?"true":"false");

    QJsonDocument jsonDocument(jsonObject);

    return jsonDocument.toJson(QJsonDocument::Compact);
}

QByteArray Json::getVehicleInfoJson(QBluetoothAddress address, bool charging, bool onTrack) {
    QJsonObject jsonObject;

    jsonObject.insert("address", address.toString());
    jsonObject.insert("charging", charging);
    jsonObject.insert("onTrack", onTrack);

    QJsonDocument jsonDocument(jsonObject);

    return jsonDocument.toJson(QJsonDocument::Compact);
}

QByteArray Json::getMessageJson(QUuid uuid, QString message) {
    QJsonObject jsonObject;

    jsonObject.insert("uuid", uuid.toString());
    jsonObject.insert("message", message);

    QJsonDocument jsonDocument(jsonObject);

    return jsonDocument.toJson(QJsonDocument::Compact);
}

QByteArray Json::getEventJson(QBluetoothAddress address, QString event, uint16_t velocity) {
    QJsonObject jsonObject;

    jsonObject.insert("address", address.toString());
    jsonObject.insert("event", event);
    jsonObject.insert("velocity", velocity);

    QJsonDocument jsonDocument(jsonObject);

    return jsonDocument.toJson(QJsonDocument::Compact);
}

QByteArray Json::getLapTimeJson(QBluetoothAddress address, qint64 milliseconds) {
    QJsonObject jsonObject;

    jsonObject.insert("address", address.toString());
    jsonObject.insert("milliseconds", milliseconds);

    QJsonDocument jsonDocument(jsonObject);

    return jsonDocument.toJson(QJsonDocument::Compact);
}

QVariantMap Json::parseJson(QString payload) {
    QJsonDocument jsonDocument = QJsonDocument::fromJson(payload.toUtf8());

    if (jsonDocument.isObject()) {
        return jsonDocument.toVariant().toMap();
    }
    else {
        return QVariantMap();
    }
}
