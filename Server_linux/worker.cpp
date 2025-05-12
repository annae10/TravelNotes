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

SSL_CTX *create_context() {
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

void configure_context(SSL_CTX *ctx) {
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1 | SSL_OP_NO_COMPRESSION | SSL_OP_CIPHER_SERVER_PREFERENCE);
    SSL_CTX_set_default_passwd_cb(ctx, password_callback);

    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, "server.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
    }
}


void Worker::start_server()
{
    int server;
    SSL_CTX *ctx;

    init_openssl();
    ctx = create_context();
    configure_context(ctx);

    struct sockaddr_in addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    server = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server, (struct sockaddr*)&addr, sizeof(addr));
    listen(server, 1);

    stringList.push_back("Note1");
    stringList.push_back("Note2");
    stringList.push_back("Note3");

    int num_items = stringList.size();

    while (1) {
        SSL *ssl;
        qDebug()<<"ssl\n";
        int client = accept(server, NULL, NULL);
        if(client<0){
            qDebug()<<"accept failed\n";
        }
        qDebug("accept\n");
        ssl = SSL_new(ctx);
        qDebug("ssl new\n");
        SSL_set_fd(ssl, client);
        if (SSL_accept(ssl) <= 0) {
            qDebug("error:\n");
            ERR_print_errors_fp(stderr);
        } else {

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
                printf("Successfully wrote %d bytes\n", total_written);
            }

            char buf[1024];
            SSL_read(ssl, buf, sizeof(buf));
            printf("buf:%s\n", buf);
        }
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(client);
    }
    close(server);
    SSL_CTX_free(ctx);
    cleanup_openssl();
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
