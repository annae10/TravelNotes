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

#include <QFile>
#include <thread>
#include <iostream>
#define PORT 4443

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);
    ~Worker(){}
public slots:
    void start_client();
    void process();
signals:
    void finished();
    void error(QString err);
private:
    bool stopServerFlag=false;
};

#endif // WORKER_H
