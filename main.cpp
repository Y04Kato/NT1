#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <cstring>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

void ServerSide(void) {
    SOCKET serverSocket, clientSocket;
    sockaddr_in serverAddr, clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    char buffer[1024];
    u_short uport;

    //ポート番号の入力
    std::cout << "使用するポート番号 : ";
    std::cin >> uport;

    //ソケットの作成
    std::cout << "ソケットの作成を開始" << std::endl;
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cerr << "ソケットの作成に失敗" << std::endl;
        return;
    }
    std::cout << "ソケットの作成に成功" << std::endl;

    //サーバーのポート番号とIPアドレスを設定
    memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(uport);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    //ソケットをバインド
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "バインドに失敗" << std::endl;
        closesocket(serverSocket);
        return;
    }
    std::cout << "バインドに成功" << std::endl;

    //接続待機
    listen(serverSocket, 3);
    std::cout << "クライアント接続を待機中" << std::endl;

    //クライアントの接続を確認
    clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "接続に失敗" << std::endl;
        closesocket(serverSocket);
        return;
    }
    std::cout << "クライアントが接続" << std::endl;

    //メッセージ処理をループ
    int recvSize;
    while (true) {
        //受信バッファの初期化
        memset(buffer, 0, sizeof(buffer));

        //クライアントからのメッセージ受信
        recvSize = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (recvSize == SOCKET_ERROR) {
            std::cerr << "受信に失敗" << std::endl;
            break;
        }
        else if (recvSize == 0) {
            std::cout << "クライアントが切断" << std::endl;
            break;
        }

        //受信データに終端文字を追加
        buffer[recvSize] = '\0';
        std::cout << "クライアントからのメッセージ : " << buffer << std::endl;

        //メッセージをクライアントに返信
        if (send(clientSocket, buffer, strlen(buffer), 0) == SOCKET_ERROR) {
            std::cerr << "返信の送信に失敗しました" << std::endl;
            break;
        }
    }

    //ソケットを閉じる
    closesocket(clientSocket);
    closesocket(serverSocket);
}

void ClientSide() {
    SOCKET clientSocket;
    sockaddr_in serverAddr;
    char buffer[1024];
    char serverIp[16];
    int serverPort;

    //クライアントソケットの初期化
    std::cout << "ソケットの作成を開始" << std::endl;
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cerr << "ソケットの作成に失敗" << std::endl;
        return;
    }
    std::cout << "ソケットの作成に成功" << std::endl;

    //サーバーのIPアドレスとポートを入力
    std::cout << "接続先のサーバーIPを入力 : ";
    std::cin >> serverIp;

    std::cout << "接続先のサーバーポートを入力 : ";
    std::cin >> serverPort;
    std::cin.ignore();

    //サーバーのアドレス構造体を設定
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);

    //IPアドレスをバイナリ形式に変換して設定
    if (inet_pton(AF_INET, serverIp, &serverAddr.sin_addr) <= 0) {
        std::cerr << "IPアドレスの設定に失敗しました" << std::endl;
        closesocket(clientSocket);
        return;
    }

    //サーバーに接続
    std::cout << "サーバー接続を開始" << std::endl;
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "サーバー接続に失敗" << std::endl;
        closesocket(clientSocket);
        return;
    }
    std::cout << "サーバー接続に成功、終了時は(end)を入力" << std::endl;

    //メッセージ処理をループ
    while (true) {
        std::cout << "送信するメッセージを入力 : ";
        std::cin.getline(buffer, 1024);

        //"end"を入力したら接続を終了
        if (strcmp(buffer, "end") == 0) {
            std::cout << "サーバー接続を終了して切断します" << std::endl;
            break;
        }

        //サーバーにメッセージを送信
        if (send(clientSocket, buffer, strlen(buffer), 0) == SOCKET_ERROR) {
            std::cerr << "メッセージの送信に失敗しました" << std::endl;
            break;
        }

        //サーバーからのメッセージを受信
        int recvSize = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (recvSize == SOCKET_ERROR) {
            std::cerr << "メッセージの受信に失敗しました" << std::endl;
            break;
        }
        else if (recvSize == 0) {
            std::cout << "サーバーとの通信が切断されました" << std::endl;
            break;
        }

        //受信データに終端文字を追加
        buffer[recvSize] = '\0';
        std::cout << "サーバーからのメッセージ : " << buffer << std::endl;
    }

    //ソケットを閉じる
    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    int startUpMode;

    //WinSockの初期化
    std::cout << "WinSockの初期化を開始" << std::endl;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WinSockの初期化に失敗しました" << std::endl;
        return 1;
    }

    //サーバーかクライアントかを選択
    std::cout << "サーバーなら0を入力、クライアントなら1を入力 : ";
    std::cin >> startUpMode;

    if (startUpMode == 0) {
        ServerSide();
    }
    else {
        ClientSide();
    }

    //WinSockの終了処理
    WSACleanup();
    return 0;
}
