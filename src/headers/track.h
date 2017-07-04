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
    void setLastDirection(TrackPiece::Direction direction);
    void printTrack();
    bool isComplete(uint8_t trackpieceId);
    QString getTrackString();
    QList<TrackPiece> getTrackList();

private:
    bool containsStart = false;
    bool containsFinish = false;
    QList<TrackPiece> track;

    bool mightBeComplete = false;
};

#endif // TRACK_H
