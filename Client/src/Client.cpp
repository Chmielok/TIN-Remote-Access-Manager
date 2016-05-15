//
// Created by tomasz on 10.05.16.
//

#include "Client.h"
#include "../../Shared/Message.h"

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <netdb.h>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <cstring>
#include <climits>


using namespace std;

bool Client::running = false;
int Client::port = 8080;
string Client::hostname = "localhost";

Client::~Client() {
    close(clientSocket);
}

Client &Client::getClient() {
    static Client client;
    return client;
}

void Client::runClient() {

}

void Client::prepare(){
    // Initialize SSL configuration and variables
    initializeSSL();
    initializeSSL_CTX();
    initializeSSL_BIO();

    cout<<"PREPARE"<<endl;
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
        throw ClientException(ClientException::ErrorCode::SOCKET_FAILURE);

    server_addr.sin_family = AF_INET;
    server_hostent = gethostbyname(hostname.c_str());
    if (server_hostent == (struct hostent *) 0)
        throw ClientException(ClientException::ErrorCode::NO_SERVER);

    memcpy((char *) &server_addr.sin_addr, (char *) server_hostent->h_addr, server_hostent->h_length);

    server_addr.sin_port = htons(port);

    if (connect(clientSocket, (struct sockaddr *) &server_addr, sizeof server_addr) == -1) {
        throw ClientException(ClientException::ErrorCode::CONNECT_FAILURE);
    }

    // perform SSL handshake
    if(SSL_connect(ssl) != 1) {
        // @FIXME
        exit(1000);
    }
}

void Client::connectWithMainServer(){
    try {
        prepare();
        sendData();
    } catch (ClientException e) {
        cerr << "FAILURE. Source: " << e.errorCode << endl;
    }
    close (clientSocket);
}

void Client::sendData() {

    Message message;
    char c;
    cout<<"Co chcesz wyslac"<<endl;
    cout<<"0 - LOGGING"<<endl;
    cout<<"1 - BOOKING"<<endl;
    cout<<"2 - ACCESS_REQUEST"<<endl;
    cout<<"3 - FAIL"<<endl;
    cout<<"4 - SUCCESS"<<endl;
    cout<<"5 - MACHINE_DATA"<<endl;
    cout<<"6 - BOOKING_LOG"<<endl;
    c = cin.get();
    cin.ignore(INT_MAX,'\n');
    switch (c){
        case '0':
            message.messageType = Message::MessageType::LOGGING;
            Message::MessageData::LoggingMessage loggingMessage;
            cout<<"Podaj login"<<endl;
            char login[32];
            char password[20];
            cin >> login;
            cout<<"Podaj haslo"<<endl;
            cin >> password;
            strcpy(loggingMessage.login ,login);
            strcpy(loggingMessage.password, password);
            message.messageData.loggingMessage = loggingMessage;
            break;

        case '1':
            message.messageType = Message::MessageType::BOOKING;
            Message::MessageData::BookingMessage bookingMessage;
            cout<<"Podaj id"<<endl;
            uint32_t id;
            cin >> id;
            bookingMessage.id = id;
            time(&bookingMessage.data);
            message.messageData.bookingMessage = bookingMessage;
            break;

        case '2':
            message.messageType = Message::MessageType::ACCESS_REQUEST;
            break;

        case '3':
            message.messageType = Message::MessageType::FAIL;
            cout<<"Podaj wiadomosc bledu"<<endl;
            char failMessage[64];
            cin >> failMessage;
            strcpy(message.messageData.failMessage ,failMessage);
            break;

        case '4':
            message.messageType = Message::MessageType::SUCCESS;
            cout<<"Podaj wiadomosc sukcesu"<<endl;
            char successMessage[64];
            cin >> successMessage;
            strcpy(message.messageData.successMessage ,successMessage);
            break;

        case '5':
            message.messageType = Message::MessageType::MACHINE_DATA;
            Message::MessageData::MachineDataMessage machineDataMessage;
            cout<<"Podaj id"<<endl;
            //uint32_t id;
            cin >> id;
            cout<<"Podaj informacje"<<endl;
            char information[32];
            cin >> information;
            strcpy(machineDataMessage.information ,information);
            machineDataMessage.id = id;
            message.messageData.machineDataMessage  = machineDataMessage;
            break;

        case '6':
            message.messageType = Message::MessageType::BOOKING_LOG;
            break;

    }
    void* pointer = (void*) &message;
    if (SSL_write(ssl, pointer, sizeof (Message)) == -1)
        perror("writing on stream socket");
}

void Client::initializeSSL() {
    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();

}

void Client::initializeSSL_CTX() {
    sslctx = SSL_CTX_new(SSLv23_client_method());

    // Load OpenSSL cerrificate file
    if(SSL_CTX_use_certificate_file(sslctx, certPath, SSL_FILETYPE_PEM) <= 0){
        ERR_print_errors_fp(stderr);
        exit(1);
    }
    // Load OpenSSL Private Key
    if(SSL_CTX_use_PrivateKey_file(sslctx, keyPath, SSL_FILETYPE_PEM) <= 0){
        ERR_print_errors_fp(stderr);
        exit(1);
    }

    /* Load trusted CA. */
    if (!SSL_CTX_load_verify_locations(sslctx, caCertPath, NULL)) {
        ERR_print_errors_fp(stderr);
        exit(1);
    }

    ssl = SSL_new(sslctx);
}

void Client::initializeSSL_BIO() {
    bio = BIO_new_socket(clientSocket, BIO_NOCLOSE);
    SSL_set_bio(ssl, bio, bio);
}

bool Client::setServerPortAndName(int p, string name){
    if(!running){
        port = p;
        hostname = name;
    }
    else
        return false;
    return true;
}