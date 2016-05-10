//
// Created by chmielok on 5/3/16.
//

#ifndef TIN_REMOTE_ACCESS_MANAGER_SERVER_H
#define TIN_REMOTE_ACCESS_MANAGER_SERVER_H

#include <iostream>
#include <string>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


using namespace std;

class Server {
public:
    // zmienia domyślny port dla serwera
    static bool setDefaultPort(int);
    // zmienia liczbę możliwych oczekujących połączeń
    static bool setMaxWaitingConns(int);
    // pobierz instancję serwera (singleton)
    static Server & getServer();
    // uruchom serwer
    void runServer();
    ~Server();

private:
    // zablokowany konstruktor domyślny (singleton)
    Server(){};
    // nasłuchuj i twórz wątki
    void listenForClients();
    // przygotuj serwer pod przyjmowanie klientów
    void prepare();

    // initialize openSSL library
    void initializeSSL();
    // initializes SSL conntext parameters
    void initializeSSL_CTX();

    static bool running;
    static int port;
    static int maxWaitingConns;
    int serverSocket;

    // SSL parameters
    SSL *ssl;
    SSL_CTX *sslctx;
    int certfd;
    int keyfd;

    static constexpr char *certPath = (char*)".ssl/cert.pem";
    static constexpr char *keyPath = (char*)".ssl/key.pem";


    struct ServerException {
        enum ErrorCode {
            SOCKET_FAILURE,
            BIND_FAILURE,
            LISTEN_FAILURE,
            ACCEPT_FAILURE
        } errorCode;
        ServerException(ErrorCode code) : errorCode(code) {};
    };
};


#endif //TIN_REMOTE_ACCESS_MANAGER_SERVER_H