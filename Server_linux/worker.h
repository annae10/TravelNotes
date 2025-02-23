#ifndef WORKER_H
#define WORKER_H

#pragma once

#include <QApplication>
#include <QPushButton>
#include <QWidget>
#include <QThread>
#include <QDebug>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <arpa/inet.h>

#include <iostream>
#define PORT 4443

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);
    ~Worker(){}
public slots:
    void start_server();
    void start_client();
    void process();
    void process2();

signals:
    void finished();
    void error(QString err);
};

#endif // WORKER_H
