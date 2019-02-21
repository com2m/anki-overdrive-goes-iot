/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

#include <QCoreApplication>
#include "headers/drivemode.h"

#include "headers/joystick.hh"
#include "headers/gamepadmanager.h"
#include "headers/ConsoleReader.h"

#include "headers/json.h"
#include "headers/tragediyimplementation.h"
#include "headers/ankimessage.h"
#include "headers/mqttclient.h"
#include "headers/rgbled.h"

#include "unistd.h"
#include <QProcess>
#include <QFileInfo>


DriveMode::DriveMode(QObject *parent) : QObject(parent) {
    uuid = QUuid::createUuid();
    
    if (enableRGBLed) {
        statusLED = new RGBLed(3, 12, 13);
        statusLED->setColor(Qt::red); 
    }
    if (enableBackgroundMusic) {
        
        QSoundEffect *startup = new QSoundEffect;
        QUrl _url(QUrl::fromLocalFile(QFileInfo("media/Startup.wav").absoluteFilePath()));
        startup->setSource(_url); 
        startup->setLoopCount(1);
        startup->setVolume(1.0f);
        startup->play();

        playlist = new QMediaPlaylist();
        _url = QUrl::fromLocalFile(QFileInfo("media/AnkiOveride.mp3").absoluteFilePath());
        playlist->addMedia(_url);
        playlist->setPlaybackMode(QMediaPlaylist::Loop);
        player = new QMediaPlayer;
        player->setPlaylist(playlist);
        player->setVolume(60);
        // qDebug().noquote().nospace() << "player plays: " << player->currentMedia().canonicalUrl().toString() << " from url=" << _url <<  ", error: " << player->error();

        sleep(5);  // allow some time for the mp3 to load ...
        if (enableRGBLed) statusLED->setColor(QColor("#ffa500")); // orange, see SVG colors reference  https://doc.qt.io/qt-5/qml-color.html#svg-color-reference
        sleep(5);
        if (enableRGBLed) statusLED->setColor(Qt::green); 
        sleep(5);
        
        fireSoundEffect1 = new QSoundEffect;
        _url = QUrl::fromLocalFile(QFileInfo("media/Duff1.wav").absoluteFilePath());
        fireSoundEffect1->setSource(_url); 
        fireSoundEffect1->setLoopCount(1);
        fireSoundEffect1->setVolume(1.0f);

        fireSoundEffect2 = new QSoundEffect;
        _url = QUrl::fromLocalFile(QFileInfo("media/Duff2.wav").absoluteFilePath());
        fireSoundEffect2->setSource(_url); 
        fireSoundEffect2->setLoopCount(1);
        fireSoundEffect2->setVolume(1.0f);
    }
    if (enableRGBLed) statusLED->setColor(Qt::green); 

    lanes << 2 << 7 << 12 << 17;

    TragediyImplementation::clearLocationTable();

    for (int i = 0; i < numberOfRacecars; i++) {
        racecarList.append(new Racecar(this));
        qDebug().noquote().nospace() << "[racecar #" << i << "]" << ">> listed.";
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

    gamepadManager = new GamepadManager(numberOfRacecars, this);
    
    connect(gamepadManager, SIGNAL(turboMode(Racecar*, bool)), this, SLOT(turboMode(Racecar*, bool)));
    connect(gamepadManager, SIGNAL(doUturn(Racecar*, bool)), this, SLOT(doUturn(Racecar*, bool)));
    connect(gamepadManager, SIGNAL(driveLeft(Racecar*, bool)), this, SLOT(driveLeft(Racecar*, bool)));
    connect(gamepadManager, SIGNAL(driveRight(Racecar*, bool)), this, SLOT(driveRight(Racecar*, bool)));
    connect(gamepadManager, SIGNAL(acceleratorChanged(Racecar*, double)), this, SLOT(acceleratorChanged(Racecar*, double)));
    connect(gamepadManager, SIGNAL(changeLane(Racecar*, double)), this, SLOT(changeLane(Racecar*, double)));
    connect(gamepadManager, SIGNAL(scanTrack()), this, SLOT(scanTrack()));
    gamepadManager->start();

    consoleReader = 0;
    eventReader = 0;
    if (enableKeyboard) 
    {
        if (isatty(STDIN_FILENO)) { 
            qDebug().noquote().nospace() << "Trying to use ConsoleReader, as we are a TTY";
            consoleReader = new ConsoleReader();
            connect(consoleReader, SIGNAL(KeyPressed(char)), this, SLOT(OnConsoleKeyPressed(char)));
            consoleReader->start();
        }
        else {
            qDebug().noquote().nospace() << "Trying to use EventReader, as we are not a TTY";
            eventReader = new EventReader(eventDevice);
            // eventReader = new EventReader("/dev/input/event1");  // = console keyboard 
            connect(eventReader, SIGNAL(KeyPressed(char)), this, SLOT(OnConsoleKeyPressed(char)));
            eventReader->start();
        }
    }

    if (enableMqtt) {
        qDebug() << ">> CHECKING MQTT BROKER AVAILABILITY...";
        while (QProcess::execute(QString("ping -c 1 %0").arg(brokerIp)) != 0)  {
            usleep(10000000);
        }
        qDebug() << ">> MQTT BROKER IS AVAILABLE.";
        mqttClient = new MqttClient(brokerIp, brokerPort, brokerTimeout, brokerUser, brokerPassword, this);
        mqttClient->subscribe(0, c2sChannel.toStdString().c_str());
        mqttClient->subscribe(0, s2cChannel.toStdString().c_str());

        qDebug() << ">> ATTEMPTING TO CONNECT TO MQTT BROKER...";
        connect(mqttClient, SIGNAL(onMessage(MqttMessage)), this, SLOT(onMqttMessage(MqttMessage)));
        qDebug() << ">> SUCCESSFULLY CONNECTED TO MQTT BROKER.";
    }

    batteryUpdateTimer = new QTimer(this);
    connect(batteryUpdateTimer, SIGNAL(timeout()), this, SLOT(requestBatteryUpdate()));
    batteryUpdateTimer->start(10000);
    // qDebug().noquote().nospace() << "[DriveMode created]";
}

void DriveMode::quit() {
    foreach (Racecar* racecar, racecarList) {
        if (racecar->isAvailable()) {
            publishMessage(Json::getAliveJson(racecar->getAddress(), false, uuid));
        }
    }
    if (enableRGBLed) delete statusLED;
    if (enableBackgroundMusic) player->stop();
    if (consoleReader != 0) {
        consoleReader->quit();
        consoleReader->terminate();
        delete consoleReader;
    }
    if (eventReader != 0) {
        eventReader->quit();
        eventReader->terminate();
        delete eventReader;
    }
    qDebug().noquote().nospace() << "[DriveMode quit() done]";
}

void DriveMode::publishMessage(QByteArray message) {
    if (enableMqtt) {
        mqttClient->publish(0, c2sChannel.toStdString().c_str(), message.length(), message);
    }
}

void DriveMode::disconnected() {
    Racecar* racecar = static_cast<Racecar*>(QObject::sender());
    if (enableRGBLed) statusLED->setColor(Qt::green);
    if (enableBackgroundMusic) {
        player->stop();
        qDebug().noquote().nospace() << "background music stopped.";
    }
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
        if (enableRGBLed) {
            double level = racecar->getBatteryLevel() * 100.0/maximumBatteryLevel;
            if (0.0 < level && level < 50) {
                statusLED->setColorForPeriod(Qt::red, -20*level + 1200);  // 10% = 1s, 50% = 200ms
                qDebug().noquote().nospace() << "[" + racecar->getName() + "]>> battery level = " << racecar->getBatteryLevel() * 100.0/maximumBatteryLevel << "%";
            }
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
        sendMessage("[" + racecar->getName() + "]>> ATTEMPT TO ACCESS GAMEPAD FAILED.");
        qDebug().noquote().nospace() << "[" + racecar->getName() + "]" << ">> ATTEMPT TO ACCESS GAMEPAD FAILED.";
    }
    else {
        sendMessage("[" + racecar->getName() + "]>> USING GAMEPAD #" + (gamepadManager->getGamepads().indexOf(gamepad) + 1) + ".");
        qDebug().noquote().nospace() << "[" + racecar->getName() + "]" << ">> USING GAMEPAD #" << gamepadManager->getGamepads().indexOf(gamepad) + 1 << ".";
    }
    
    if (enableKeyboard) {
        sendMessage("[" + racecar->getName() + "]>> USING KEYBOARD.");
        qDebug().noquote().nospace() << "[" + racecar->getName() + "]" << ">> USING KEYBOARD.";
    }

    racecar->ignoreInputs(false);
    QColor color(Qt::white);
    if (racecar->getName().contains("Ground Shock")) color = Qt::blue;
    if (racecar->getName().contains("Skull")) color = Qt::red;
    racecar->setEngineLight(color.red(), color.green(), color.blue());

    sendMessage("[" + racecar->getName() + "]>> READY.");
    qDebug().noquote().nospace() << "[" + racecar->getName() + "]" << ">> READY.";
    qDebug().noquote().nospace() << "[" + racecar->getName() + "]" << ">> setEngineLight = " << color.name();
    
    // LED is blue
    if (enableRGBLed) statusLED->setColor(Qt::blue);
    // start music 
    if (enableBackgroundMusic) {
        player->play();
        qDebug().noquote().nospace() << "background music started.";
    }

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

            QString message = QString("[" + racecar->getName() + "]" + ">> FINISH." + ((racecar->getLastFinishTime().isValid())?(" LAP TIME: " + QString::number(diff)):("")) + " MS");
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
    qDebug().noquote().nospace() << "changeLane: ";
    if (racecar->ignoreInputs() || (racecar->isSteering() && !(value >= -0.2 && value <= 0.2)))
        return;

    if (value > 0.2) {
        if (!racecar->isSteering()) {
            racecar->setOffsetFromRoadCenter(-68.0f);
            racecar->isSteering(true);

            racecar->changeLane(68.0f, 200);
            qDebug().noquote().nospace() << "changeLane(+68.0f, 200)";

        }
    }
    else if (value < -0.2) {
        if (!racecar->isSteering()) {
            racecar->setOffsetFromRoadCenter(68.0f);
            racecar->isSteering(true);

            racecar->changeLane(-68.0f, 200);
            qDebug().noquote().nospace() << "changeLane(-68.0f, 200)";
        }
    }
    else if (racecar->isSteering() && (value >= -0.2 && value <= 0.2)) {
        racecar->isSteering(false);

        racecar->cancelLaneChange();
    }
}


void DriveMode::acceleratorUp(Racecar *racecar) {
    double speed = 1.0 * racecar->getVelocity() / maxVelocity;
    speed += (speed < 1.0) ? (acceleratorTolerance + 10.0)/ maxVelocity: 0.0;  // increase by acceleratorTolerance
    acceleratorChanged(racecar, speed); 
    qDebug().noquote().nospace() << "[" + racecar->getName() + "]>> Speed set to " << speed << " (velocity now = " << racecar->getVelocity() << ")";
}

void DriveMode::acceleratorDown(Racecar *racecar) {
    double speed = 1.0 * racecar->getVelocity() / maxVelocity;
    speed -= (speed > 0.0) ? (acceleratorTolerance + 10.0)/ maxVelocity: 0.0;  // decrease by acceleratorTolerance
    if (speed < 0.0) speed = 0;
    acceleratorChanged(racecar, speed); 
    qDebug().noquote().nospace() << "[" + racecar->getName() + "]>> Speed set to " << speed << " (velocity now = " << racecar->getVelocity() << ")";
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

    qDebug("TRACKSCAN complete.");
    track.printTrack();

    Racecar* racecar = static_cast<Racecar*>(QObject::sender());

    sendMessage("[" + racecar->getName() + "]>> TRACK SCAN COMPLETED: " + track.getTrackString() + ".");
    qDebug().noquote().nospace() << "[" + racecar->getName() + "]" << ">> TRACK SCAN COMPLETED: " << track.getTrackString() << ".";

    racecar->ignoreInputs(false);

    QPair<QPair<float, float>, QPair<float, float>> extremeValues = TragediyImplementation::generateTrackScheme(track.getTrackList());

    xMin = extremeValues.first.first;
    yMin = extremeValues.first.second;

    foreach (Racecar* racecar, racecarList) {
        publishMessage(Json::getTrackJson(racecar->getAddress(), track, uuid, extremeValues.first.first, extremeValues.first.second, extremeValues.second.first, extremeValues.second.second));
        racecar->setLights(AnkiMessage::lightFeature::BRAKE_LITE_OFF);
    }
}

void DriveMode::stoppedAtStart() {
    Racecar* racecar = static_cast<Racecar*>(QObject::sender());
    setLightsForPeriod(racecar, AnkiMessage::lightFeature::BRAKE_LITE_ON, 2000, AnkiMessage::lightFeature::BRAKE_LITE_OFF);  
    racecar->cancelLaneChange();  
     
    publishMessage(Json::getEventJson(racecar->getAddress(), "startposition", 0));
    QString message = QString("[" + racecar->getName() + "]>> REACHED START POSITION.");
    qDebug().noquote().nospace() << message;
    publishMessage(Json::getMessageJson(uuid, message));

}

void DriveMode::positionUpdate(AnkiMessage ankiMessage) {
    Racecar* racecar = static_cast<Racecar*>(QObject::sender());

    AnkiLocationTableEntry entry = TragediyImplementation::getAnkiLocationTableEntry(ankiMessage);

    entry.setX(entry.getX() + xMin * (-1));
    entry.setY(entry.getY() + yMin * (-1));

    publishMessage(Json::getPositionJson(racecar->getAddress(), entry));
}


bool DriveMode::setLightsForPeriod(Racecar* racecar, AnkiMessage::lightFeature lightsOn, int ms, AnkiMessage::lightFeature lightsOff) { 

    // qDebug().noquote().nospace() << "setLightsForPeriod, On= 0x"  << hex << lightsOn << ", off= 0x" << hex << lightsOff << " for ms=" << ms;

    periodTimer = new QTimer(this);
    connect(periodTimer, &QTimer::timeout, [=] { lightsPeriodUpdate(racecar, lightsOff); });
    periodTimer->setSingleShot(true);
    periodTimer->start(ms);
    racecar->setLights(lightsOn);
    return true;
}

void DriveMode::lightsPeriodUpdate(Racecar* racecar, AnkiMessage::lightFeature lightsOff ) {
    racecar->setLights(lightsOff);
}


void DriveMode::OnConsoleKeyPressed(char c){

    int car = 0;
    int lane = 0;
    int retcode = 10;
    static int leftCar = 0;
    static int counter = 0;
    uint8_t lightValue = 0;
    QSoundEffect *shutdown;
    QUrl _url;

   /*
   qDebug().noquote().nospace() << "OnConsoleKeyPressed: '"  << c << "'";
   foreach (Racecar* racecar, racecarList) {
      qDebug().noquote().nospace() << "[" + racecar->getName() + "]>> seen";
   }
   */
    
    switch (c) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
        usingSpeed = (c - '0')*(1.0/5);
        foreach (Racecar* racecar, racecarList) {
            emit acceleratorChanged(racecar, usingSpeed); 
            qDebug().noquote().nospace() << "[" + racecar->getName() + "]>> Speed set to " << usingSpeed;
        }
        break;
    case 'G':   // F5    drive to start - does only work if track scan previously was successful 
        if (TragediyImplementation::ankiLocationTableCount() == 0) break;
        foreach (Racecar* racecar, racecarList) {
            QString carAddress = racecar->getAddress().toString();
            if (carAddress != "00:00:00:00:00:00" ) {
                if (racecar->ignoreInputs() || racecar->isCharging() || !racecar->isOnTrack()) {
                    signalStatus(racecar);
                    continue;
                }
                racecar->setReverseDriving(false);
                racecar->ignoreInputs(true);
                lane = lanes.at(3 - racecarList.indexOf(racecar));
                racecar->driveToStart(lane);
                qDebug().noquote().nospace() << "[" + racecar->getName() + "]>> Drive to Start, using lane " << lane;
            }
        }
        break;
    case 'C':   // F8    clear road (drive furthest left and right)
        foreach (Racecar* racecar, racecarList) {
            QString carAddress = racecar->getAddress().toString();
            if (carAddress != "00:00:00:00:00:00" ) {
                if (racecar->ignoreInputs() || racecar->isCharging() || !racecar->isOnTrack()) {
                    signalStatus(racecar);
                    continue;
                }
                if (racecarList.indexOf(racecar) == leftCar) {
                    emit driveLeft(racecar, true); // drive furthest left
                    qDebug().noquote().nospace() << "[" + racecar->getName() + "]>> [F8] Drive to left";
                }
                if (racecarList.indexOf(racecar) != leftCar) {
                    emit driveRight(racecar, true);  // drive farthest right 
                    qDebug().noquote().nospace() << "[" + racecar->getName() + "]>> [F8] Drive to right";
                }
            }
        }
        leftCar = 1 - leftCar;
        break;
    case 'w':
    case 'W':
        car++;
    case '^':
        foreach (Racecar* racecar, racecarList) {
            QString carAddress = racecar->getAddress().toString();
            if (carAddress != "00:00:00:00:00:00" && racecarList.indexOf(racecar) == car) {
                if (racecar->isOnTrack()) {
                    emit acceleratorUp(racecar); 
                }
                else if (racecar->ignoreInputs() || racecar->isCharging() || !racecar->isOnTrack()) {
                    signalStatus(racecar);
                }
            }
        }
        car = 0;
        break;
    case 's':
    case 'S':
        car++;
    case 'v':
        foreach (Racecar* racecar, racecarList) {
            QString carAddress = racecar->getAddress().toString();
            if (carAddress != "00:00:00:00:00:00" && racecarList.indexOf(racecar) == car) {
                if (racecar->isOnTrack()) {
                    emit acceleratorDown(racecar); 
                    setLightsForPeriod(racecar, AnkiMessage::lightFeature::BRAKE_LITE_ON, 400, AnkiMessage::lightFeature::BRAKE_LITE_OFF);  
                }
                else if (racecar->ignoreInputs() || racecar->isCharging() || !racecar->isOnTrack()) {
                    signalStatus(racecar);
                }
            }
        }
        car = 0;
        break;
    case 'a':
    case 'A':
        car++;
    case '<':
        foreach (Racecar* racecar, racecarList) {
            QString carAddress = racecar->getAddress().toString();
            if (carAddress != "00:00:00:00:00:00" && racecarList.indexOf(racecar) == car) {
                if (racecar->isOnTrack()) {
                    racecar->setOffsetFromRoadCenter(0.0f);  
                    racecar->changeLane(-23.0f, 250, 1000);   // Offset (68, 23, -23, 68 seem to be lane values 1-4), 
                                                              // max speed = 250 , max acceleration = 2500)
                }
                else if (racecar->ignoreInputs() || racecar->isCharging() || !racecar->isOnTrack()) {
                    signalStatus(racecar);
                }
            }
        }
        car = 0;
        break;
    case 'd':
    case 'D':
        car++;
    case '>':
        foreach (Racecar* racecar, racecarList) {
            QString carAddress = racecar->getAddress().toString();
            if (carAddress != "00:00:00:00:00:00" && racecarList.indexOf(racecar) == car) {
                if (racecar->isOnTrack()) {
                    racecar->setOffsetFromRoadCenter(0.0f);  
                    racecar->changeLane(+23.0f);   // +9.06f ?
                }
                else if (racecar->ignoreInputs() || racecar->isCharging() || !racecar->isOnTrack()) {
                    signalStatus(racecar);
                }
            }
        }
        car = 0;
        break;
    case ' ':     // fire
        car++;
    case 'I':
        foreach (Racecar* racecar, racecarList) {
            QString carAddress = racecar->getAddress().toString();
            if (carAddress != "00:00:00:00:00:00" && racecarList.indexOf(racecar) == car) {
                setLightsForPeriod(racecar, AnkiMessage::lightFeature::HEAD_LITE_FLASH, 500, AnkiMessage::lightFeature::HEAD_LITE_OFF);  
                if (car == 0 && fireSoundEffect1 != 0) fireSoundEffect1->play();
                if (car == 1 && fireSoundEffect2 != 0) fireSoundEffect2->play();
                // qDebug().noquote().nospace() << "[" + racecar->getName() + "]>> fire for car " << car;
            }
        }
        car = 0;
        break;
    case 'l':     // testing lights
        car++;
    case 'L':
        foreach (Racecar* racecar, racecarList) {
            QString carAddress = racecar->getAddress().toString();
            if (carAddress != "00:00:00:00:00:00" && racecarList.indexOf(racecar) == car) {
                if (counter % 6 == 0)      lightValue = AnkiMessage::lightFeature::BRAKE_LITE_ON;
                else if (counter % 6 == 1) lightValue = AnkiMessage::lightFeature::BRAKE_LITE_OFF;
                else if (counter % 6 == 2) lightValue = AnkiMessage::lightFeature::BRAKE_LITE_FLASH;
                else if (counter % 6 == 3) lightValue = AnkiMessage::lightFeature::BRAKE_LITE_OFF;
                else if (counter % 6 == 4) lightValue = AnkiMessage::lightFeature::HEAD_LITE_FLASH;
                else if (counter % 6 == 5) lightValue = AnkiMessage::lightFeature::HEAD_LITE_OFF;
                qDebug().noquote().nospace() << "[" + racecar->getName() + "]>> set lights using value 0x" << hex << lightValue;
                racecar->setLights(lightValue);
                counter++;
            }
        }
      car = 0;
      break;
    case 't': 
    case 'T': 
        usingTurboMode = !usingTurboMode;
        if (usingTurboMode) {
            sendMessage(">> NO LIMIT MODE ON.");
            qDebug().noquote().nospace() << ">> NO LIMIT MODE ON.";
            maxVelocity = 2000;
        }
        else {
            sendMessage(">> NO LIMIT MODE OFF.");
            qDebug().noquote().nospace() << ">> NO LIMIT MODE OFF.";
            maxVelocity = 800;
        }
        nitroVelocity = (uint16_t)(maxVelocity * 1.5);
        acceleratorTolerance = (int)(maxVelocity / 8);
        break;
    case 'h':     // scan track (Home, Pos1)
        // qDebug().noquote().nospace() << ">> pressed Home ";
        foreach (Racecar* racecar, racecarList) {
            QString carAddress = racecar->getAddress().toString();
            if (carAddress != "00:00:00:00:00:00") {
                if (racecar->ignoreInputs() || racecar->isCharging() || !racecar->isOnTrack()) {
                    signalStatus(racecar);
                    qDebug().noquote().nospace() << "[" + racecar->getName() + "]>> TRACK SCAN not feasible, cars not ready.";
                    continue;
                }
                qDebug().noquote().nospace() << "[" + racecar->getName() + "]>> TRACK SCAN INITIATED.";
                racecar->setLights(AnkiMessage::lightFeature::BRAKE_LITE_FLASH);
                racecar->ignoreInputs(true);
                racecar->scanTrack();
                break;
            }
        }
      break;
    case 'p':
    case 'P':
        if (gamePaused) {
            gamePaused = false;
        }
        else {
            gamePaused = true;
        }
        foreach (Racecar* racecar, racecarList) {
            if (gamePaused) {
                racecar->setTmpSpeed(racecar->getVelocity());
                racecar->setVelocity(0);
                racecar->ignoreInputs(true);
            }
            else {
                racecar->ignoreInputs(false);
                racecar->setVelocity(racecar->getTmpSpeed());
            }
            qDebug().noquote().nospace() << "[" + racecar->getName() + "]>> processing Pause, velocity set to " << racecar->getVelocity();
        }
        break;
    case '?':      // F1 
        foreach (Racecar* racecar, racecarList) {
            QString carName = racecar->getName();
            requestBatteryUpdate();
            racecar->reconnect();
            racecar->ignoreInputs(false);
            if (racecar->getAddress().toString() != "00:00:00:00:00:00") {
                qDebug().noquote().nospace() << "[" + carName + "]>> ignoreInputs = " << racecar->ignoreInputs() 
                                                                 << ", isCharging = " << racecar->isCharging()
                                                                 << ", onTrack = " << racecar->isOnTrack()
                                                                 << ", turboIsActive = " << racecar->turboIsActive()
                                                                 << ", battery level = " << racecar->getBatteryLevel() * 100.0/maximumBatteryLevel << "%";
            }
        }
        break;
    case 'M':     // U-turn
        car++;
    case 'm':
        foreach (Racecar* racecar, racecarList) {
            QString carAddress = racecar->getAddress().toString();
            if (carAddress != "00:00:00:00:00:00" && racecarList.indexOf(racecar) == car) {
                doUturn(racecar, true);
            }
        }
        car = 0;
        break;
    case 'Q':   // F12
        retcode += 2;
    case 'R':   // F10
        if (enableRGBLed) statusLED->setColor(Qt::red);
        shutdown = new QSoundEffect;
        _url = QUrl::fromLocalFile(QFileInfo("media/ByeBye.wav").absoluteFilePath());
        shutdown->setSource(_url); 
        shutdown->setLoopCount(1);
        shutdown->setVolume(1.0f);
        shutdown->play();
        for (int i=0; i < 15; i++) { // 15 * 0.2s = 3s blinking red
            if (enableRGBLed) statusLED->setColorForPeriod(Qt::black, 100);  // 0.1s = 100ms
            QThread::msleep(100); // 100 + 100 msecs = 0.2s
            if (enableRGBLed) statusLED->setColor(Qt::red);
            QThread::msleep(100); 
        }
        qDebug().noquote().nospace() << "[QUIT drive mode] with return code = " << retcode;
        this->quit();
        // this->deleteLater();  // QThread: Destroyed while thread is still running, and does not really terminated.
        QCoreApplication::exit(retcode);
        break;
    default:
        break;
    }
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

                QString statusMessage = QString("[" + racecar->getName() + "]>> " + ((message["enabled"].toBool()) ? ("ENABLED"):("DISABLED")) + ".");
                publishMessage(Json::getMessageJson(uuid, statusMessage));
            }
        } else {
            Racecar* racecar = getRacecarByAddress(QBluetoothAddress(message["address"].toString()));

            if (racecar == 0 || racecar->isCharging()) {
                return;
            }

            racecar->ignoreInputs(!(message["enabled"].toBool()));

            QString statusMessage = QString("[" + racecar->getName() + "]>> " + ((message["enabled"].toBool()) ? ("ENABLED"):("DISABLED")) + ".");
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

                        sendMessage("[" + racecar->getName() + "]>> USING GAMEPAD #" + (index + 1) + ".");
                        qDebug().noquote().nospace() << "[" + racecar->getName() + "]" << ">> USING GAMEPAD #" << index + 1 << ".";

                        index++;
                    }
                }

                foreach (Racecar* racecar, racecarList) {
                    if (racecar->isCharging()) {
                        gamepadManager->getGamepad(index)->setRacecar(racecar);

                        sendMessage("[" + racecar->getName() + "]>> USING GAMEPAD #" + (index + 1) + ".");
                        qDebug().noquote().nospace() << "[" + racecar->getName() + "]" << ">> USING GAMEPAD #" << index + 1 << ".";

                        index++;
                    }
                }
            }
        }
    }
}

void DriveMode::scanTrack(){
	Racecar* racecar = 0;
	foreach (Racecar* tmpRacecar, racecarList) {
		if (tmpRacecar->isOnTrack() && (!tmpRacecar->ignoreInputs() |  !tmpRacecar->isCharging())) {
			qDebug("SCANNING TRACK INITIATED");
            racecar->ignoreInputs(true);
			tmpRacecar->scanTrack();
			return;
		}

		racecar = tmpRacecar;
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

void DriveMode::signalStatus(Racecar* racecar) {
    if (enableRGBLed) {
        if (racecar->isCharging()) {
            statusLED->setColorForPeriod(Qt::cyan, 1000);  // 1s = 1000ms
        }
        else if (!racecar->isOnTrack()) {
            statusLED->setColorForPeriod(Qt::yellow, 1000); 
        }
    }
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
