/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

#include "headers/trackpiece.h"
#include <QString>
#include <QDebug>

TrackPiece::TrackPiece(uint8_t pieceId) {
    this->pieceId = pieceId;

    switch(pieceId) {
    case 36:
        type = SEGMENT_STRAIGHT;
        break;
    case 39:
        type = SEGMENT_STRAIGHT;
        break;
    case 40:
        type = SEGMENT_STRAIGHT;
        break;
    case 20:
        type = SEGMENT_CURVE;
        break;
    case 23:
        type = SEGMENT_CURVE;
        break;
    case 18:
        type = SEGMENT_CURVE;
        break;
    case 17:
        type = SEGMENT_CURVE;
        break;
    case 33:
        type = SEGMENT_START;
        break;
    case 34:
        type = SEGMENT_FINISH;
        break;
    }
}

void TrackPiece::setDirection(Direction direction) {
    this->direction = direction;
}

TrackPiece::Type TrackPiece::getType() const {
    return this->type;
}

QString TrackPiece::toString() {
    QString string = "";

    switch(type) {
    case SEGMENT_STRAIGHT:
        string = string.append("STRAIGHT");
        break;
    case SEGMENT_CURVE:
        string = string.append("CURVE");
        break;
    case SEGMENT_START:
        string = string.append("START");
        break;
    case SEGMENT_FINISH:
        string = string.append("FINISH");
        break;
    }

    switch(direction) {
    case DIRECTION_LEFT:
        string = string.append(" LEFT");
        break;
    case DIRECTION_RIGHT:
        string = string.append(" RIGHT");
        break;
    }

    return string;
}

uint8_t TrackPiece::getPieceId() const {
    return pieceId;
}

TrackPiece::Direction TrackPiece::getDirection() const {
    return direction;
}

void TrackPiece::flipDirection() {
    if (getType() == SEGMENT_CURVE) {
        if (getDirection() == DIRECTION_LEFT) {
            setDirection(DIRECTION_RIGHT);
        }
        else if (getDirection() == DIRECTION_RIGHT) {
            setDirection(DIRECTION_LEFT);
        }
    }
}
