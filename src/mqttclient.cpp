/*
 * Copyright (c) 9.11.2016 com2m GmbH.
 * All rights reserved.
 */

#include "headers/mqttclient.h"
#include <QDebug>
#include <QUuid>

MqttClient::MqttClient(QString brokerIp, int port, int keepAlive, QString username, QString password, QObject* parent) : QObject(parent), mosquittopp(QUuid::createUuid().toString().toStdString().c_str()) {
    connectToBroker(brokerIp, port, username, password, keepAlive);
    loop_start();
}

void MqttClient::connectToBroker(QString brokerIp, int port, QString username, QString password, int keepAlive) {
    mosquittopp::username_pw_set(username.toStdString().c_str(), password.toStdString().c_str());
    mosquittopp::connect(brokerIp.toStdString().c_str(), port, keepAlive);
}

void MqttClient::on_message(const struct mosquitto_message *message) {
    emit onMessage(MqttMessage(QString(message->topic), (char*)message->payload, message->payloadlen));
}
