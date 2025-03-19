#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <QMutex>
#include <QThread>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

#include <QDebug>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include "worker.h"

#include <fcntl.h>
#include <QFile>

class ClientHandler : public QObject
{
    Q_OBJECT
public:
    explicit ClientHandler(int client_fd, SSL_CTX *ctx, QObject *parent = nullptr):QObject(parent), client_fd_(client_fd), ctx_(ctx){}

    void process(){
        SSL *ssl = SSL_new(ctx_);
        SSL_set_fd(ssl, client_fd_);

        if(client_fd_ < 0){
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            emit finished();
            return;
        }

        int ret = SSL_accept(ssl);
        if(ret <= 0){
            int error = SSL_get_error(ssl, ret);
            if(error != SSL_ERROR_WANT_READ && error != SSL_ERROR_WANT_WRITE){
                ERR_print_errors_fp(stderr);
            }
            SSL_free(ssl);
            close(client_fd_);
            emit finished();
            return;
        }
        handleConnection(ssl);
        SSL_free(ssl);
        close(client_fd_);
        emit finished();
    }

private:
    void handleConnection(SSL *ssl){
        QStringList stringList;
        QFile file("data.txt");
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream in(&file);
            while(!in.atEnd()){
                QString line = in.readLine();
                qDebug()<<"line:"<<line<<"\n";
                stringList.push_back(line);
            }
            file.close();
        }
        int num_items = stringList.size();
        int total_written=0;

        for(int i=0;i<num_items; i++){

            int len = stringList[i].size();
            QByteArray byteArray = stringList[i].toUtf8();
            int written = SSL_write(ssl, byteArray.constData(), len);

            if(written <=0){
                int err=SSL_get_error(ssl, written);
                fprintf(stderr, "SSL_write error: %d\n",err);
                return;
            }
            total_written+=written;
        }
        if(total_written < 0){
            fprintf(stderr, "Failed to wrote data\n");
        }else{
            qDebug()<<"Successfully wrote"<<total_written<<"bytes\n";
            printf("Successfully wrote %d bytes\n", total_written);
        }
    }

    int client_fd_;
    SSL_CTX *ctx_;
public:
    Q_SIGNAL void finished();
};

#endif // CLIENTHANDLER_H
