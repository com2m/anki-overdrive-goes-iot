/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

#ifndef TRACKPIECE_H
#define TRACKPIECE_H

#include <stdint.h>
#include <QString>

class TrackPiece {
public:
    enum Type {
        SEGMENT_NOT_DEFINED = 0x00,

        SEGMENT_STRAIGHT = 0x01,


        SEGMENT_CURVE = 0x02,
        SEGMENT_CURVE_A = 0x11,
        SEGMENT_CURVE_B = 0x12,
        SEGMENT_CURVE_C = 0x14,

        SEGMENT_START = 0x21,
        SEGMENT_FINISH = 0x22,

    };

    enum Direction {
        DIRECTION_NONE = 0x00,
        DIRECTION_LEFT = 0x01,
        DIRECTION_RIGHT = 0x02
    };

    explicit TrackPiece(uint8_t pieceId);
    void setDirection(Direction direction);
    Direction getDirection() const;

    uint8_t getPieceId() const;
    Type getType() const;

    QString toString();

    void flipDirection();

private:
    uint8_t pieceId = 0;
    Type type = SEGMENT_NOT_DEFINED;
    Direction direction = DIRECTION_NONE;
};

#endif // TRACKPIECE_H
