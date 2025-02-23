#ifndef WORKER_H
#define WORKER_H

#pragma once

#include <QApplication>
#include <QPushButton>
#include <QWidget>
#include <QThread>
#include <QDebug>

#include <winsock2.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <iostream>
#include <QObject>

class Worker: public QObject{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);
    void setText(const char* text);

public slots:
    void start_server();
    void start_client();
    void process();
    void process2();
signals:
    void textChanged(const QString &text);
    void clearText();
    void finished();
    void error(QString err);
};

#endif // WORKER_H
