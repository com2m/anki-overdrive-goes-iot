/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

#include "headers/drivemode.h"

#include "headers/joystick.hh"
#include "headers/gamepadmanager.h"

#include "headers/json.h"
#include "headers/tragediyimplementation.h"
#include "headers/ankimessage.h"
#include "headers/mqttclient.h"

#include "unistd.h"
#include <QProcess>


DriveMode::DriveMode(QObject *parent) : QObject(parent) {

    uuid = QUuid::createUuid();

    lanes << 2 << 7 << 12 << 17;

    TragediyImplementation::clearLocationTable();

    gamepadManager = new GamepadManager(numberOfRacecars, this);

    connect(gamepadManager, SIGNAL(turboMode(Racecar*, bool)), this, SLOT(turboMode(Racecar*, bool)));
    connect(gamepadManager, SIGNAL(doUturn(Racecar*, bool)), this, SLOT(doUturn(Racecar*, bool)));
    connect(gamepadManager, SIGNAL(driveLeft(Racecar*, bool)), this, SLOT(driveLeft(Racecar*, bool)));
    connect(gamepadManager, SIGNAL(driveRight(Racecar*, bool)), this, SLOT(driveRight(Racecar*, bool)));
    connect(gamepadManager, SIGNAL(acceleratorChanged(Racecar*, double)), this, SLOT(acceleratorChanged(Racecar*, double)));
    connect(gamepadManager, SIGNAL(changeLane(Racecar*, double)), this, SLOT(changeLane(Racecar*, double)));

    gamepadManager->start();

    for (int i = 0; i < numberOfRacecars; i++) {
        racecarList.append(new Racecar(this));
    }

    bluetoothController = new BluetoothController(racecarList, this);

    foreach (Racecar* racecar, racecarList) {
        connect(racecar, SIGNAL(ready()), this, SLOT(ready()));
        connect(racecar, SIGNAL(batteryLevelUpdate(uint16_t)), this, SLOT(batteryLevelUpdateReceived(uint16_t)));
        connect(racecar, SIGNAL(trackScanCompleted(Track)), this, SLOT(trackScanCompleted(Track)));
        connect(racecar, SIGNAL(finishLine()), this, SLOT(finishLine()));
        connect(racecar, SIGNAL(disconnected()), this, SLOT(disconnected()));
        connect(racecar, SIGNAL(positionUpdate(AnkiMessage)), this, SLOT(positionUpdate(AnkiMessage)));
        connect(racecar, SIGNAL(vehicleInfoUpdate(bool, bool)), this, SLOT(vehicleInfoUpdate(bool, bool)));
        connect(racecar, SIGNAL(sendMessage(QString)), this, SLOT(sendMessage(QString)));
        connect(racecar, SIGNAL(transition()), this, SLOT(transition()));
        connect(racecar, SIGNAL(stoppedAtStart()), this, SLOT(stoppedAtStart()));
        connect(racecar, SIGNAL(velocityUpdate()), this, SLOT(velocityUpdate()));
    }

    if (enableMqtt) {
        while (QProcess::execute(QString("ping -c 1 %0").arg(brokerIp)) != 0)  {
            usleep(10000000);
        }
        mqttClient = new MqttClient(brokerIp, brokerPort, brokerTimeout, brokerUser, brokerPassword, this);
        mqttClient->subscribe(0, c2sChannel.toStdString().c_str());
        mqttClient->subscribe(0, s2cChannel.toStdString().c_str());

        connect(mqttClient, SIGNAL(onMessage(MqttMessage)), this, SLOT(onMqttMessage(MqttMessage)));
    }

    batteryUpdateTimer = new QTimer(this);
    connect(batteryUpdateTimer, SIGNAL(timeout()), this, SLOT(requestBatteryUpdate()));
    batteryUpdateTimer->start(10000);
}

void DriveMode::quit() {
    foreach (Racecar* racecar, racecarList) {
        if (racecar->isAvailable()) {
            publishMessage(Json::getAliveJson(racecar->getAddress(), false, uuid));
        }
    }
}

void DriveMode::publishMessage(QByteArray message) {
    if (enableMqtt) {
        mqttClient->publish(0, c2sChannel.toStdString().c_str(), message.length(), message);
    }
}

void DriveMode::disconnected() {
    Racecar* racecar = static_cast<Racecar*>(QObject::sender());

    publishMessage(Json::getAliveJson(racecar->getAddress(), false, uuid));
}

void DriveMode::requestBatteryUpdate() {
    foreach (Racecar* racecar, racecarList) {
        if (!racecar->isAvailable()) {
            racecar->reconnect();
        }
        else {
            racecar->requestBatteryLevel();
        }
    }
}

void DriveMode::batteryLevelUpdateReceived(uint16_t value) {

    Racecar* racecar = static_cast<Racecar*>(QObject::sender());

    publishMessage(Json::getBatteryJson(racecar->getAddress(), value, uuid));
}

void DriveMode::ready() {

    Racecar* racecar = static_cast<Racecar*>(QObject::sender());

    Joystick* gamepad = gamepadManager->addGamepad(racecar);

    if (!gamepad->isFound()) {
        sendMessage("[" + racecar->getAddress().toString() + "]>> ATTEMPT TO ACCESS GAMEPAD FAILED.");
        qDebug().noquote().nospace() << "[" + racecar->getAddress().toString() + "]" << ">> ATTEMPT TO ACCESS GAMEPAD FAILED.";
    }

    sendMessage("[" + racecar->getAddress().toString() + "]>> USING GAMEPAD #" + (gamepadManager->getGamepads().indexOf(gamepad) + 1) + ".");
    qDebug().noquote().nospace() << "[" + racecar->getAddress().toString() + "]" << ">> USING GAMEPAD #" << gamepadManager->getGamepads().indexOf(gamepad) + 1 << ".";

    racecar->ignoreInputs(false);

    sendMessage("[" + racecar->getAddress().toString() + "]>> READY.");
    qDebug().noquote().nospace() << "[" + racecar->getAddress().toString() + "]" << ">> READY.";

    publishMessage(Json::getAliveJson(racecar->getAddress(), true, uuid));
}

void DriveMode::finishLine() {

    Racecar* racecar = static_cast<Racecar*>(QObject::sender());

    QDateTime now = QDateTime::currentDateTime();
    qint64 diff;

    if (racecar->getLastFinishTime().isValid()) {
        diff = racecar->getLastFinishTime().msecsTo(now);

        if (diff > 1000 && diff < 20000) {
            publishMessage(Json::getLapTimeJson(racecar->getAddress(), diff));

            QString message = QString("[" + racecar->getAddress().toString() + "]" + ">> FINISH." + ((racecar->getLastFinishTime().isValid())?(" LAP TIME: " + QString::number(diff)):("")) + " MS");
            qDebug().noquote().nospace() << message;

            publishMessage(Json::getMessageJson(uuid, message));
        }
    }

    racecar->setLastFinishTime(now);

    if (racecar->stoppingAtStart()) {
        racecar->ignoreInputs(false);
    }
}

void DriveMode::turboMode(Racecar *racecar, bool value) {
    if (racecar->ignoreInputs())
        return;

    bool changed = false;

    if (value && racecar->getVelocity() != 0) {
        changed = true;
        racecar->setTmpSpeed(racecar->getVelocity());
        racecar->turboIsActive(true);
        racecar->setVelocity(nitroVelocity, (uint16_t)125000);
    }
    else {
        if (!value) {
            if (racecar->getVelocity() != 0) {
                changed = true;
                racecar->setVelocity(racecar->getTmpSpeed());
            }
            else {
                changed = true;
                racecar->setVelocity(0);
            }
            racecar->turboIsActive(false);
        }
    }

    if (changed) {
        publishMessage(Json::getVelocityJson(racecar->getAddress(), racecar->getVelocity()));
    }
}

void DriveMode::doUturn(Racecar *racecar, bool value) {
    if (racecar->ignoreInputs())
        return;

    if (value) {
        if (racecar->getVelocity() != 0) {
            if (racecar->getVelocity() > 500) {
                uint16_t tmp = racecar->getVelocity();
                racecar->setVelocity((uint16_t)500, (uint16_t)3000);
                usleep(12000);
                racecar->doUturn();
                usleep(12000);
                racecar->setVelocity(tmp);
            }
            else {
                racecar->doUturn();
            }
        }
    }
}

void DriveMode::driveLeft(Racecar *racecar, bool value) {
    if (racecar->ignoreInputs())
        return;

    racecar->setOffsetFromRoadCenter(68.0f);
    if (value) {
        racecar->changeLane(-68.0f);
    }
    else {
        racecar->cancelLaneChange();
    }
}

void DriveMode::driveRight(Racecar *racecar, bool value) {
    if (racecar->ignoreInputs())
        return;

    racecar->setOffsetFromRoadCenter(-68.0f);
    if (value) {
        racecar->changeLane(68.0f);
    }
    else {
        racecar->cancelLaneChange();
    }
}

void DriveMode::changeLane(Racecar *racecar, double value){
    if (racecar->ignoreInputs() || (racecar->isSteering() && !(value >= -0.2 && value <= 0.2)))
        return;

    if (value > 0.2) {
        if (!racecar->isSteering()) {
            racecar->setOffsetFromRoadCenter(-68.0f);
            racecar->isSteering(true);

            racecar->changeLane(68.0f, 200);
        }
    }
    else if (value < -0.2) {
        if (!racecar->isSteering()) {
            racecar->setOffsetFromRoadCenter(68.0f);
            racecar->isSteering(true);

            racecar->changeLane(-68.0f, 200);
        }
    }
    else if (racecar->isSteering() && (value >= -0.2 && value <= 0.2)) {
        racecar->isSteering(false);

        racecar->cancelLaneChange();
    }
}

void DriveMode::acceleratorChanged(Racecar *racecar, double value) {
    if (racecar->ignoreInputs() || racecar->isCharging()) {
        return;
    }

    uint16_t newValue = value * maxVelocity;
    bool changed = false;

    if (newValue == 0) {
        changed = true;
        racecar->setVelocity(newValue, (uint16_t)12500);
    }
    else if (newValue == maxVelocity) {
        changed = true;
        racecar->setVelocity(newValue, (uint16_t)12500);
    }
    else if (newValue > racecar->getVelocity() + acceleratorTolerance || newValue < racecar->getVelocity() - acceleratorTolerance) {
        if (!racecar->turboIsActive()) {
            changed = true;
            racecar->setVelocity(newValue, (uint16_t)12500);
        }
    }

    if (changed) {
        velocityChanged(racecar);
    }
}

void DriveMode::trackScanCompleted(Track track) {
    TragediyImplementation::clearLocationTable();

    Racecar* racecar = static_cast<Racecar*>(QObject::sender());

    sendMessage("[" + racecar->getAddress().toString() + "]>> TRACK SCAN COMPLETED: " + track.getTrackString() + ".");
    qDebug().noquote().nospace() << "[" + racecar->getAddress().toString() + "]" << ">> TRACK SCAN COMPLETED: " << track.getTrackString() << ".";

    racecar->ignoreInputs(false);

    QPair<QPair<float, float>, QPair<float, float>> extremeValues = TragediyImplementation::generateTrackScheme(track.getTrackList());

    xMin = extremeValues.first.first;
    yMin = extremeValues.first.second;

    foreach (Racecar* racecar, racecarList) {
        publishMessage(Json::getTrackJson(racecar->getAddress(), track, uuid, extremeValues.first.first, extremeValues.first.second, extremeValues.second.first, extremeValues.second.second));
    }
}

void DriveMode::stoppedAtStart() {
    Racecar* racecar = static_cast<Racecar*>(QObject::sender());

    publishMessage(Json::getEventJson(racecar->getAddress(), "startposition", 0));

    QString message = QString("[" + racecar->getAddress().toString() + "]>> REACHED START POSITION.");
    publishMessage(Json::getMessageJson(uuid, message));

}

void DriveMode::positionUpdate(AnkiMessage ankiMessage) {
    Racecar* racecar = static_cast<Racecar*>(QObject::sender());

    AnkiLocationTableEntry entry = TragediyImplementation::getAnkiLocationTableEntry(ankiMessage);

    entry.setX(entry.getX() + xMin * (-1));
    entry.setY(entry.getY() + yMin * (-1));


    publishMessage(Json::getPositionJson(racecar->getAddress(), entry));
}

void DriveMode::onMqttMessage(MqttMessage mqttMessage) {
    if (mqttMessage.getTopic() != s2cChannel)
        return;

    QVariantMap message = Json::parseJson(mqttMessage.getPayload());

    if (message.contains("velocity")) {
        if (message["address"].toString() == "*") {
            foreach (Racecar* racecar, racecarList) {
                if (racecar->ignoreInputs() || racecar->isCharging()) {
                    continue;
                }

                racecar->setVelocity(message["velocity"].toInt(), (uint16_t)12500);
                velocityChanged(racecar);
            }
        }
        else {
            Racecar* racecar = getRacecarByAddress(QBluetoothAddress(message["address"].toString()));

            if (racecar == 0 || racecar->ignoreInputs() || racecar->isCharging()) {
                return;
            }

            racecar->setVelocity(message["velocity"].toInt(), (uint16_t)12500);
            velocityChanged(racecar);
        }
    }

    if (message.contains("lane")) {
        Racecar* racecar = getRacecarByAddress(QBluetoothAddress(message["address"].toString()));

        if (racecar == 0 || racecar->ignoreInputs() || racecar->isCharging()) {
            return;
        }

        racecar->changeLane(message["lane"].toInt());
    }

    if (message.contains("enabled")) {

        if (message["address"].toString() == "*") {
            foreach (Racecar* racecar, racecarList) {
                racecar->ignoreInputs(!(message["enabled"].toBool()));

                QString statusMessage = QString("[" + racecar->getAddress().toString() + "]>> " + ((message["enabled"].toBool()) ? ("ENABLED"):("DISABLED")) + ".");
                publishMessage(Json::getMessageJson(uuid, statusMessage));
            }
        } else {
            Racecar* racecar = getRacecarByAddress(QBluetoothAddress(message["address"].toString()));

            if (racecar == 0 || racecar->isCharging()) {
                return;
            }

            racecar->ignoreInputs(!(message["enabled"].toBool()));

            QString statusMessage = QString("[" + racecar->getAddress().toString() + "]>> " + ((message["enabled"].toBool()) ? ("ENABLED"):("DISABLED")) + ".");
            publishMessage(Json::getMessageJson(uuid, statusMessage));
        }
    }

    if (message.contains("noLimits")) {
        if (message["uuid"].toString() == uuid.toString() || message["uuid"].toString() == "*") {
            if (message["noLimits"].toBool()) {
                sendMessage(">> NO LIMIT MODE ON.");
                qDebug() << ">> NO LIMIT MODE ON.";
                maxVelocity = 2000;
            }
            else {
                sendMessage(">> NO LIMIT MODE OFF.");
                qDebug() << ">> NO LIMIT MODE OFF.";
                maxVelocity = 800;
            }

            nitroVelocity = (uint16_t)(maxVelocity * 1.5);
            acceleratorTolerance = (int)(maxVelocity / 8);
        }
    }

    if (message.contains("command")) {
        if (message["command"].toString() == "scan") {

            Racecar* racecar = 0;

            if (message["address"].toString() == "*") {
                foreach (Racecar* tmpRacecar, racecarList) {
                    if (tmpRacecar->ignoreInputs() ||  tmpRacecar->isCharging()) {
                        continue;
                    }

                    racecar = tmpRacecar;
                }
            }
            else {
                Racecar* tmpRacecar = getRacecarByAddress(QBluetoothAddress(message["address"].toString()));

                if (tmpRacecar == 0 || tmpRacecar->ignoreInputs() || tmpRacecar->isCharging()) {
                    return;
                }

                racecar = tmpRacecar;
            }

            if (racecar) {
                sendMessage(">> TRACK SCAN INITIATED.");
                racecar->ignoreInputs(true);
                racecar->scanTrack();
            }
        }
        else if (message["command"].toString() == "lineup") {

            if (message["address"].toString() == "*") {
                sendMessage(">> LINE UP INITIATED.");

                foreach (Racecar* racecar, racecarList) {
                    if (racecar->ignoreInputs() || racecar->isCharging()) {
                        continue;
                    }

                    racecar->setReverseDriving(false);
                    racecar->ignoreInputs(true);
                    racecar->driveToStart(lanes.at(3 - racecarList.indexOf(racecar)));
                }
            }
            else {
                Racecar* racecar = getRacecarByAddress(QBluetoothAddress(message["address"].toString()));

                if (racecar == 0 || racecar->ignoreInputs() || racecar->isCharging()) {
                    return;
                }

                sendMessage(">> LINE UP INITIATED.");

                racecar->setReverseDriving(false);
                racecar->ignoreInputs(true);
                racecar->driveToStart(lanes.at(3 - racecarList.indexOf(racecar)));
            }
        }
        else if (message["command"].toString() == "reassign") {
            if (message["uuid"].toString() == "*" || message["uuid"].toString() == uuid.toString()) {
                qDebug() << ">> REASSIGNING GAMEPADS.";
                sendMessage(">> REASSIGNING GAMEPADS.");

                foreach (Joystick* gamepad, gamepadManager->getGamepads()) {
                    gamepad->setRacecar(NULL);
                }

                int index = 0;

                foreach (Racecar* racecar, racecarList) {
                    if (!racecar->isCharging()) {
                        gamepadManager->getGamepad(index)->setRacecar(racecar);

                        sendMessage("[" + racecar->getAddress().toString() + "]>> USING GAMEPAD #" + (index + 1) + ".");
                        qDebug().noquote().nospace() << "[" + racecar->getAddress().toString() + "]" << ">> USING GAMEPAD #" << index + 1 << ".";

                        index++;
                    }
                }

                foreach (Racecar* racecar, racecarList) {
                    if (racecar->isCharging()) {
                        gamepadManager->getGamepad(index)->setRacecar(racecar);

                        sendMessage("[" + racecar->getAddress().toString() + "]>> USING GAMEPAD #" + (index + 1) + ".");
                        qDebug().noquote().nospace() << "[" + racecar->getAddress().toString() + "]" << ">> USING GAMEPAD #" << index + 1 << ".";

                        index++;
                    }
                }
            }
        }
    }
}

Racecar* DriveMode::getRacecarByAddress(QBluetoothAddress address) {
    foreach (Racecar* racecar, racecarList) {
        if (racecar->getAddress() == address) {
            return racecar;
        }
    }

    return 0;
}

void DriveMode::velocityChanged(Racecar *racecar) {
    publishMessage(Json::getVelocityJson(racecar->getAddress(), racecar->getVelocity()));
}

void DriveMode::velocityUpdate() {
    velocityChanged(static_cast<Racecar*>(QObject::sender()));
}

void DriveMode::vehicleInfoUpdate(bool onTrack, bool charging) {
    Racecar* racecar = static_cast<Racecar*>(QObject::sender());

    publishMessage(Json::getVehicleInfoJson(racecar->getAddress(), charging, onTrack));
}

void DriveMode::sendMessage(QString messagePayload) {
    publishMessage(Json::getMessageJson(uuid, messagePayload));
}

void DriveMode::transition() {
    Racecar* racecar = static_cast<Racecar*>(QObject::sender());

    publishMessage(Json::getEventJson(racecar->getAddress(), "transition", racecar->getVelocity()));
}
