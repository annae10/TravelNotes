#include "worker.h"

Worker::Worker(QObject *parent): QObject(parent)
{

}

void init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl() {
    EVP_cleanup();
}

void Worker::start_client()
{
    SSL_CTX *ctx;
    SSL *ssl;
    int server;
    struct sockaddr_in addr;

    init_openssl();
    ctx = SSL_CTX_new(TLS_client_method());

    server = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "ip", &addr.sin_addr);

    ::connect(server, (struct sockaddr*)&addr, sizeof(addr));

    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, server);
    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
    } else {

        char buf[1024];
        SSL_read(ssl, buf, sizeof(buf));
        printf("%s\n", buf);

        int len = strlen(buf);
        QString str = QString::fromUtf8(buf, len);
        qDebug()<<"received message:"<<str<<"\n";

        QFile file("data.txt");
        if(file.open(QIODevice::Append | QIODevice::Text)){
            QTextStream out(&file);
            out << str << "\n";
        }
        file.close();

    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(server);
    SSL_CTX_free(ctx);
    cleanup_openssl();
}

void Worker::process()
{
    while(1){
        start_client();
        sleep(1);
    }
}
