#ifndef SSLSERVER_H
#define SSLSERVER_H

#include "clienthandler.h"

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

#include <QTimer>
#include <fcntl.h>
#include <QFile>


#define PORT 4443

class SslServer : public QThread {

    Q_OBJECT

public:
    SslServer(SSL_CTX *ctx, QObject *parent = nullptr) : QThread(parent), server_fd(-1), is_running(false), ctx_(ctx){}


    inline void init_openssl() {
        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();
    }

    inline void cleanup_openssl() {
        EVP_cleanup();
    }

    inline SSL_CTX *create_context() {
        SSL_CTX *ctx;

        ctx = SSL_CTX_new(TLS_server_method());
        if (!ctx) {
            perror("Unable to create SSL context");
            ERR_print_errors_fp(stderr);
            exit(EXIT_FAILURE);
        }
        return ctx;
    }

    static int password_callback(char *buf, int size, int rwflag, void *user_data){
        const char* password = "";
        if(size < static_cast<int>(strlen(password)+1)){
            return 0;
        }
        strcpy(buf, password);
        return strlen(password);
    }

    inline void configure_context(SSL_CTX *ctx) {
        SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1 | SSL_OP_NO_COMPRESSION | SSL_OP_CIPHER_SERVER_PREFERENCE);
        SSL_CTX_set_default_passwd_cb(ctx, password_callback);

        if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0) {
            ERR_print_errors_fp(stderr);
        }
        if (SSL_CTX_use_PrivateKey_file(ctx, "server.pem", SSL_FILETYPE_PEM) <= 0) {
            ERR_print_errors_fp(stderr);
        }
    }
    inline void run() override {

        init_openssl();

        struct sockaddr_in addr;
        server_fd = socket(AF_INET, SOCK_STREAM, 0);

        addr.sin_family = AF_INET;
        addr.sin_port = htons(PORT);
        addr.sin_addr.s_addr = INADDR_ANY;

        bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
        listen(server_fd, 3);

        is_running=true;

        while (is_running){

            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);

            int client = accept(server_fd, NULL, NULL);
            if(client<0){

                if(errno == EINTR)continue;
                perror("Accept failed");
                continue;
            }

            handleClient(client, ctx_);
        }

        close(server_fd);
        SSL_CTX_free(ctx_);
        cleanup_openssl();
    }

    inline void stop()
    {
        is_running = false;
        if(server_fd != -1){
            close(server_fd);
            server_fd = -1;
        }
        wait();
    }

private:
    void handleClient(int client, SSL_CTX *ctx){

        ClientHandler *handler = new ClientHandler(client, ctx);
        QObject::connect(handler, &ClientHandler::finished, handler, &QObject::deleteLater);

        QThread *thread = new QThread;
        handler->moveToThread(thread);

        QObject::connect(thread, &QThread::started, handler, &ClientHandler::process);
        QObject::connect(handler, &ClientHandler::finished, thread, &QThread::quit);
        QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
        thread->start();
    }

    int server_fd;
    bool is_running;
    SSL_CTX *ctx_;
};

#endif // SSLSERVER_H
