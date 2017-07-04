/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

#include "headers/track.h"
#include "headers/trackpiece.h"
#include <QList>
#include <QDebug>

Track::Track() {

}

void Track::clear() {
    track.clear();
    containsStart = false;
    containsFinish = false;
}

void Track::append(const uint8_t trackpieceId) {
    if (!track.isEmpty() && track.last().getPieceId() == trackpieceId)
        return;

    track.append(TrackPiece(trackpieceId));

    if (trackpieceId == TrackPiece::SEGMENT_START)
        containsStart = true;
    else if (trackpieceId == TrackPiece::SEGMENT_FINISH)
        containsFinish = true;
}

void Track::setLastDirection(TrackPiece::Direction direction) {
    if (track.length() > 0) {
        if (track[track.length() - 1].getType() == TrackPiece::SEGMENT_CURVE) {
            track[track.length() - 1].setDirection(direction);
        }
    }
}

QList<TrackPiece> Track::getTrackList() {
    return track;
}

void Track::printTrack() {
    qDebug() << getTrackString();
}

bool Track::isComplete(uint8_t trackpieceId) {

    if (!(containsStart && containsFinish)) {
        return false;
    }

    if (mightBeComplete) {
        if (track.first().getPieceId() == TrackPiece::SEGMENT_START && track.last().getPieceId() == TrackPiece::SEGMENT_FINISH) {
            track.insert(0, track.last());
            track.removeLast();

            return true;
        }

        // Avoid index out of range
        if (track.length() < 2) {
            mightBeComplete = false;
            return false;
        }

        if (track.at(1).getPieceId() == trackpieceId) {
            track.removeLast();

            while (!getTrackString().startsWith("F")) {
                track.append(track.mid(0, 1));
                track.removeFirst();
            }

            if (getTrackString().startsWith("F") && getTrackString().endsWith("S")) {
                track.append(track.at(0));
                track.removeFirst();

                QList<TrackPiece> tmpTrack;

                for (int i = 0; i < track.length(); i++) {
                    tmpTrack.append(track.at(track.length() - 1 - i));
                    tmpTrack[i].flipDirection();
                }

                track = tmpTrack;
            }

            int startCounter = 0;
            int finishCounter = 0;

            foreach (TrackPiece trackPiece, track) {
                if (trackPiece.getPieceId() == TrackPiece::SEGMENT_START) {
                    startCounter++;
                }
                else if (trackPiece.getPieceId() == TrackPiece::SEGMENT_FINISH) {
                    finishCounter++;
                }
            }

            if (startCounter != 1 || finishCounter != 1) {
                track.clear();
                mightBeComplete = false;

                return false;
            }

            return true;
        }
        else {
            mightBeComplete = false;
        }
    }

    int startCounter = 0;
    int finishCounter = 0;

    foreach (TrackPiece trackPiece, track) {
        if (trackPiece.getPieceId() == TrackPiece::SEGMENT_START) {
            startCounter++;
        }
        else if (trackPiece.getPieceId() == TrackPiece::SEGMENT_FINISH) {
            finishCounter++;
        }
    }

    if (track.at(0).getPieceId() != TrackPiece::SEGMENT_FINISH && ((startCounter >= 2) || (finishCounter >= 2))) {
        track.clear();
        mightBeComplete = false;

        return false;
    }

    if (track.at(0).getPieceId() == trackpieceId) {
        mightBeComplete = true;
    }

    return false;
}

QString Track::getTrackString() {
    QString trackString = "";

    foreach (TrackPiece trackPiece, track) {
        switch (trackPiece.getType()) {
        case TrackPiece::SEGMENT_CURVE:
            if (trackPiece.getDirection() == TrackPiece::DIRECTION_LEFT) {
                trackString.append("L");
            }
            else if (trackPiece.getDirection() == TrackPiece::DIRECTION_RIGHT) {
                trackString.append("R");
            }
            break;
        case TrackPiece::SEGMENT_START:
            trackString.append("S");
            break;
        case TrackPiece::SEGMENT_FINISH:
            trackString.append("F");
            break;
        case TrackPiece::SEGMENT_STRAIGHT:
            trackString.append("E");
            break;
        }
    }

    return trackString;
}
