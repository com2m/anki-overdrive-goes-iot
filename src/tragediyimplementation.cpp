/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

// Tragediy from: https://github.com/tpreclik/drive-sdk/tree/tragediy
// and: https://github.com/NoveroResearch/tragediy

#include "headers/tragediyimplementation.h"
#include <QDebug>
#include <stdio.h>
#include <locale.h>
#include "headers/ankimessage.h"
#include <QFile>
#include <unistd.h>
#include <QProcess>
#include <QDir>
#include <QMap>

QMap<QString, AnkiLocationTableEntry*> TragediyImplementation::ankiLocationTable;
//AnkiLocationTableEntry TragediyImplementation::ankiLocationTable[2][8][256][256];

TragediyImplementation::TragediyImplementation(QObject *parent) : QObject(parent) {

}

QPair<QPair<float, float>, QPair<float, float>> TragediyImplementation::generateTrackScheme(QList<TrackPiece> track) {
    QString trackScheme = "";

    trackScheme.append(QString::number(track.length()) + "\n");

    foreach (TrackPiece trackPiece, track) {
        bool rightCurve = (trackPiece.getDirection() == TrackPiece::DIRECTION_RIGHT)?true:false;
        
        // second parameter is always 7 (not track.length()), according to the modularRoadPieceDefinitionFiles 0_7_*.txt
        QString newLine = QString("0 %1 %2 1000 %3").arg(7).arg(trackPiece.getPieceId()).arg(rightCurve?1:0);
        trackScheme.append(newLine + "\n");
    }

    trackScheme.append(QString::number(track.length()) + "\n");

    int counter = 0;

    foreach (TrackPiece trackPiece, track) {
        bool rightCurve = (trackPiece.getDirection() == TrackPiece::DIRECTION_RIGHT)?true:false;

        QString newLine = QString("%1 %2 %3 %4").arg(counter).arg(rightCurve?0:1).arg((counter + 1) % track.length()).arg((track.at((counter + 1) % track.length()).getDirection() == TrackPiece::DIRECTION_RIGHT)?1:0);

        trackScheme.append(newLine + "\n");

        counter++;
    }

    trackScheme.append("1 0 0.0 0.15\n");
    trackScheme.append("0.0 0.0 0.0\n");

    return writeTrackSchemeToFile(trackScheme);
}

QPair<QPair<float, float>, QPair<float, float>> TragediyImplementation::writeTrackSchemeToFile(QString trackScheme) {
    QFile file("track.txt");

    if (file.exists()) {
        file.remove();
    }

    if (file.open(QIODevice::ReadWrite)) {
        QTextStream stream(&file);
        stream << trackScheme << endl;
    }

    return generateLocationTable();
}

QPair<QPair<float, float>, QPair<float, float>> TragediyImplementation::generateLocationTable() {
    QFile file("track_location-table.csv");

    if (file.isOpen()) {
        return QPair<QPair<float, float>, QPair<float, float>>();
    }

    QProcess tragediy;

    tragediy.setProgram(QDir(QDir::currentPath()).absolutePath() + QDir::separator() + "tragediy");
    tragediy.setArguments(QStringList() << "-I" << "com.anki.drive" << "-p" << "track" << "-j" << "track.txt");

    tragediy.start();

    tragediy.waitForFinished();

    return importLocationTable();
}

QPair<QPair<float, float>, QPair<float, float>> TragediyImplementation::importLocationTable() {
    FILE *fin = fopen("track_location-table.csv", "r");
    if (!fin) {
        qDebug() << ">> CAN'T OPEN LOCATION TABLE.";
        return QPair<QPair<float, float>, QPair<float, float>>();
    }

    float xMin = 0.0;
    float yMin = 0.0;
    float xMax = 0.0;
    float yMax = 0.0;

    int reverse, numbits, roadPieceId, locationId, lane, backward, numberOfEntries = 0;
    float x, y, dist;
    
    char const *oldLocale = setlocale(LC_ALL, 0);
    setlocale(LC_ALL, "C");
    // setlocale(LC_NUMERIC, "C")
    // int nnn = fscanf(fin, "%d %d %d %d %f %f %d %f %d", &reverse, &numbits, &roadPieceId, &locationId, &x, &y, &lane, &dist, &backward);
    // qDebug() << ">> found fields: " << nnn; 

    while (fscanf(fin, "%d %d %d %d %f %f %d %f %d", &reverse, &numbits, &roadPieceId, &locationId, &x, &y, &lane, &dist, &backward) == 9)
    {
        if (reverse < 0 || reverse > 1 || numbits <= 0 || numbits > 8 || roadPieceId < 0 || roadPieceId >= 256 || locationId < 0 || locationId >= 256 || backward < 0 || backward > 1)
        {
            qDebug() << ">> INVALID LOCATION TABLE ENTRY ENCOUNTERED.";
            continue;
        }

        // qDebug() << ">> entry: " << reverse << numbits << roadPieceId << locationId << x << y << lane << dist << backward << "#=" << numberOfEntries;
        xMin = qMin(xMin, x);
        yMin = qMin(yMin, y);

        xMax = qMax(xMax, x);
        yMax = qMax(yMax, y);

        AnkiLocationTableEntry* entry = new AnkiLocationTableEntry();
        entry->setX(x);
        entry->setY(y);
        entry->setLane(lane);
        entry->setDistance(dist);
        entry->setBackward(backward);

        TragediyImplementation::ankiLocationTable.insert(QString(reverse).append(QString(numbits)).append(QString(roadPieceId)).append(QString(locationId)), entry);

        ++numberOfEntries;
    }

    qDebug() << ">> SUCCESSFULLY IMPORTED" << numberOfEntries << "LOCATION TABLE ENTRIES.";

    fclose(fin);

    QPair<float, float> minValues = QPair<float, float>(xMin, yMin);
    QPair<float, float> maxValues = QPair<float, float>(xMax + xMin * (-1), yMax + yMin * (-1));

    QPair<QPair<float, float>, QPair<float, float>> extremeValues;

    extremeValues.first = minValues;
    extremeValues.second = maxValues;

    setlocale(LC_ALL, oldLocale);
    return extremeValues;
}

void TragediyImplementation::clearLocationTable() {
    TragediyImplementation::ankiLocationTable.clear();
}

int TragediyImplementation::ankiLocationTableCount() {
    return ankiLocationTable.count();
}

AnkiLocationTableEntry TragediyImplementation::getAnkiLocationTableEntry(AnkiMessage message) {
    if (message.getNumBits() == 0 || message.getNumBits() > 8) {
        return AnkiLocationTableEntry();
    }
    else {
        QString key = QString(((message.reverseParsing() != 0x00) ? 0x01 : 0x00)).append(QString(message.getNumBits())).append(QString(message.getPieceId())).append(QString(message.getLocationId()));

        if (TragediyImplementation::ankiLocationTable.count(key) != 0) {
            AnkiLocationTableEntry tmpEntry;

            AnkiLocationTableEntry* entry = TragediyImplementation::ankiLocationTable[key];

            tmpEntry.setX(entry->getX());
            tmpEntry.setY(entry->getY());
            tmpEntry.setLane(entry->getLane());
            tmpEntry.setDistance(entry->getDistance());
            tmpEntry.setBackward(entry->getBackward());

            return tmpEntry;

        }
        return AnkiLocationTableEntry();
    }
}
