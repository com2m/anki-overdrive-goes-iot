/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <mosquittopp.h>
#include <QDebug>
#include <QObject>

class MqttMessage {
public:
    MqttMessage(QString topic = "", QString payload = "", int size = 0) {
        this->topic = topic;
        this->payload = payload;
        this->size = size;
    }

    void setTopic(QString topic) {
        this->topic = topic;
    }

    QString getTopic() {
        return topic;
    }

    void setPayload(QString payload) {
        this->payload = payload;
    }

    QString getPayload() {
        return payload;
    }

    void setSize(int size) {
        this->size = size;
    }

    int getSize() {
        return size;
    }

private:
    QString topic;
    QString payload;
    int size;
};

using namespace mosqpp;

class MqttClient: public QObject, public mosquittopp {
    Q_OBJECT
public:
    MqttClient(QString brokerIp, int port = 1883, int keepAlive = 60, QString username = "", QString password = "", QObject* parent = 0);
    void on_message(const struct mosquitto_message *message);

private:
    void connectToBroker(QString brokerIp, int port, QString username, QString password, int keepAlive = 60);

signals:
    void onMessage(MqttMessage message);
};

#endif // MQTTCLIENT_H
