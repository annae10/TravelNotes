#include "mainwindow.h"

#include <QApplication>
#include "sslserver.h"

int password_callback(char *buf, int size, int rwflag, void *user_data){
    const char* password = "";
    if(size < static_cast<int>(strlen(password)+1)){
        return 0;
    }
    strcpy(buf, password);
    return strlen(password);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    SSL_CTX *ctx;

    ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1 | SSL_OP_NO_COMPRESSION | SSL_OP_CIPHER_SERVER_PREFERENCE);
    SSL_CTX_set_default_passwd_cb(ctx, password_callback);

    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, "server.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
    }

    SslServer sslServer(ctx);
    sslServer.start();

    return a.exec();
}
