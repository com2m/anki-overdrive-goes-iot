/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

#ifndef TRAGEDIYIMPLEMENTATION_H
#define TRAGEDIYIMPLEMENTATION_H

#include <QObject>
#include "trackpiece.h"
#include "ankimessage.h"
#include <QMap>

class AnkiLocationTableEntry {

public:
    AnkiLocationTableEntry() {
        distance = -1.0f;
        x = -1.0f;
        y = -1.0f;
        lane = -1;
        backward = -1;
    }

    float getDistance() {
        return distance;
    }

    void setDistance(float value) {
        distance = value;
    }

    float getX() {
        return x;
    }

    void setX(float value) {
        x = value;
    }

    float getY() {
        return y;
    }

    void setY(float value) {
        y = value;
    }

    int getLane() {
        return lane;
    }

    void setLane(int value) {
        lane = value;
    }

    int getBackward() {
        return backward;
    }

    void setBackward(int value) {
        backward = value;
    }

private:
    float distance;
    float x;
    float y;
    int lane;
    int backward;
};

class TragediyImplementation : public QObject {
    Q_OBJECT
public:
    explicit TragediyImplementation(QObject *parent = 0);

    static QPair<QPair<float, float>, QPair<float, float>> generateTrackScheme(QList<TrackPiece> track);
    static QPair<QPair<float, float>, QPair<float, float>> importLocationTable();
    static void clearLocationTable();
    static QPair<QPair<float, float>, QPair<float, float>> writeTrackSchemeToFile(QString trackScheme);
    static QPair<QPair<float, float>, QPair<float, float>> generateLocationTable();
    static AnkiLocationTableEntry getAnkiLocationTableEntry(AnkiMessage message);
    static int ankiLocationTableCount();

private:
//    static AnkiLocationTableEntry ankiLocationTable[2][8][256][256];
    static QMap<QString, AnkiLocationTableEntry*> ankiLocationTable;
signals:

public slots:
};

#endif // TRAGEDIYIMPLEMENTATION_H
