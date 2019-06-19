/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

#ifndef TRACK_H
#define TRACK_H

#include <QObject>
#include <QList>
#include "trackpiece.h"


class Track {
public:
    Track();
    void clear();
    void append(const uint8_t trackpieceId);
    void append(const uint8_t trackpieceId, const TrackPiece::Direction direction);
    void setLastDirection(TrackPiece::Direction direction);
    void printTrack();
    bool isComplete(uint8_t trackpieceId);
    QString getTrackString();
    QString getTrackPieceString(const uint8_t trackpieceId);

    QList<TrackPiece> getTrackList();
    int samePieceCounter = 1;

private:
    bool containsStart = false;
    bool containsFinish = false;
    QList<TrackPiece> track = QList<TrackPiece>();

    bool mightBeComplete = false;
};

#endif // TRACK_H
