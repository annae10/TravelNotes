#include "worker.h"
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")

inline void init_openssl() {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    ERR_load_SSL_strings();
    ERR_print_errors_fp(stderr);
}

inline void cleanup_openssl() {
    EVP_cleanup();
}

inline SSL_CTX* create_context() {
    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}

static int password_callback(char *buf, int size, int rwflag, void *user_data){
    const char* password = "1234";
    if(size < static_cast<int>(strlen(password)+1)){
        return 0;
    }
    strcpy(buf, password);
    return strlen(password);
}

inline void configure_context(SSL_CTX *ctx) {
    SSL_CTX_set_options(ctx, SSL_OP_SINGLE_DH_USE);
    SSL_CTX_set_default_passwd_cb(ctx, password_callback);

    if (SSL_CTX_use_certificate_file(ctx, "C:/Users/Professional/Documents/test_ssl/domain.crt", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        abort();
    }
    SSL_CTX_load_verify_locations(ctx,"C:/Users/Professional/Documents/test_ssl/domain.crt",NULL);
    SSL_CTX_set_cipher_list(ctx, "HIGH:!aNULL:!MD5");

    if (SSL_CTX_use_PrivateKey_file(ctx, "C:/Users/Professional/Documents/test_ssl/domain.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        abort();
    }
}

inline void handle_client(SSL_CTX *ctx, SOCKET client_fd)
{
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client_fd);

    if(SSL_accept(ssl)<=0){
        ERR_print_errors_fp(stderr);
    }
    else
    {
        const char reply[] = "New note";
        SSL_write(ssl, reply, sizeof(reply));
    }
    SSL_free(ssl);
    closesocket(client_fd);
}

Worker::Worker(QObject *parent): QObject(parent){

}

void Worker::setText(const char* text){
    emit textChanged(QString::fromUtf8(text));
}

void Worker::start_server()
{
    init_openssl();
    SSL_CTX *ctx = create_context();
    configure_context(ctx);

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(4443);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, SOMAXCONN);

    std::cout << "Server starts at port 4443..." << std::endl;

    while (true) {
        sockaddr_in client;
        int len = sizeof(client);
        SOCKET client_fd = accept(server_fd, (struct sockaddr*)&client, &len);

        if (client_fd == INVALID_SOCKET) {
            std::cerr << "Error accepting connection: " << WSAGetLastError() << std::endl;
            continue;
        }
        handle_client(ctx, client_fd);
    }
    closesocket(server_fd);
    SSL_CTX_free(ctx);
    cleanup_openssl();
    WSACleanup();
}

void Worker::start_client()
{
    init_openssl();
    qDebug()<<"init\n";
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if(!ctx)
    {
        ERR_print_errors_fp(stderr);
        return;
    }
    qDebug()<<"ctx\n";

    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1 | SSL_OP_NO_COMPRESSION | SSL_OP_NO_TICKET);

    if(SSL_CTX_load_verify_locations(ctx, "C:/Users/Professional/Documents/test_ssl/server.crt", NULL)!=1)
    {
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return;
    }

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    qDebug()<<"wsaData\n";

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == INVALID_SOCKET){
        std::cerr<<"Error creating socket: "<<WSAGetLastError()<<"\n";
        SSL_CTX_free(ctx);
        WSACleanup();
        return;
    }

    qDebug()<<"sock\n";

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(4443);
    server.sin_addr.s_addr = inet_addr("192.168.56.101");

    qDebug()<<"server\n";

    if(::connect(sock, (struct sockaddr*)&server, sizeof(server))!=0){
        std::cerr<<"Error connecting to server: "<<WSAGetLastError()<<std::endl;
        closesocket(sock);
        SSL_CTX_free(ctx);
        WSACleanup();
        return;
    };

    qDebug()<<"connect\n";

    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);

    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        int err = SSL_get_error(ssl,SSL_connect(ssl));
        qDebug()<<"SSL_connect error:"<<err;
        SSL_free(ssl);
        closesocket(sock);
        SSL_CTX_free(ctx);
        WSACleanup();
        return;
    } else {
        qDebug()<<"ssl connect\n";
        char buffer[1024] = {0};

        int total_read = 0;
        int bytes_read;

        emit clearText();

        while((bytes_read=SSL_read(ssl, buffer, sizeof(buffer)))>0){
            qDebug()<<"read\n";
            total_read+=bytes_read;

            if(strstr(buffer, "\r\n\r\n")){
                break;
            }
            printf("Buffer data:\n%s\n", buffer);
            setText(buffer);
        }
        if(bytes_read <=0){
            int err = SSL_get_error(ssl, bytes_read);
            if(err == SSL_ERROR_ZERO_RETURN){
                fprintf(stderr, "SSL connection closed by peer\n");
            } else {
                fprintf(stderr, "SSL_read error: %d\n", err);
            }

        }

        if(total_read <0){
            fprintf(stderr, "Failed to read data\n");
        }
        else{
            printf("Successfully read %d bytes\n", total_read);
            printf("Received data:\n%s\n", buffer);
        }

        const char *message = "Client disconnecting...";
        SSL_write(ssl, message, strlen(message));

        SSL_shutdown(ssl);

        std::cout << "Connection completed" << std::endl;

        SSL_free(ssl);
        closesocket(sock);

        cleanup_openssl();

    }
}

void Worker::process()
{
    start_client();
    emit finished();
}

void Worker::process2()
{
    start_server();
}
