#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <iostream>
#include <string>

#pragma comment (lib, "WSock32.lib")

//サーバー関数
void ChatServer(void){
    SOCKET listen_s;
    SOCKET s;
    SOCKADDR_IN saddr;
    SOCKADDR_IN from;
    int fromlen;
    u_short uport;

    //ポート番号の入力
    std::cout << "使用するポート番号 : ";
    std::cin >> uport;
    std::cin.ignore();

    //リスンソケットをオープン
    listen_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_s == INVALID_SOCKET) {
        std::cout << "リスンソケットオープンエラー" << std::endl;
        WSACleanup();
        return;
    }

    std::cout << "リスンソケットをオープンしました" << std::endl;

    //ソケットに名前を付ける
    memset(&saddr, 0, sizeof(SOCKADDR_IN));
    saddr.sin_family = AF_INET;//アドレスファミリ
    saddr.sin_port = htons(uport);//ポート番号
    saddr.sin_addr.s_addr = INADDR_ANY;//任意のIPアドレスを受け入れる

    if (bind(listen_s, (SOCKADDR*)&saddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
        std::cout << "bindエラー" << std::endl;
        closesocket(listen_s);
        return;
    }
    std::cout << "bind成功です" << std::endl;

    //クライアントからの接続待ちの状態にする
    if (listen(listen_s, SOMAXCONN) == SOCKET_ERROR) {
        std::cout << "listenエラー" << std::endl;
        closesocket(listen_s);
        return;
    }

    std::cout << "listen成功" << std::endl;

    //接続待機する
    std::cout << "クライアント接続を待機します" << std::endl;

    fromlen = (int)sizeof(from);
    s = accept(listen_s, (SOCKADDR*)&from, &fromlen);  // 修正

    if (s == INVALID_SOCKET) {
        std::cout << "クライアント接続エラー" << std::endl;
        closesocket(listen_s);
        return;
    }

    std::cout << inet_ntoa(from.sin_addr) << "が接続してきました" << std::endl;
    std::cout << "クライアント接続に成功" << std::endl;

    //リスンソケットはもう不要
    closesocket(listen_s);

    //チャット開始
    std::cout << "チャット開始" << std::endl;

    char buffer[1024];
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(s, buffer, sizeof(buffer), 0);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            std::cout << "接続が切断されました" << std::endl;
            break;
        }

        std::cout << "クライアント : " << buffer << std::endl;

        std::cout << "サーバー : ";
        std::string message;
        std::getline(std::cin, message);

        //データ送信
        send(s, message.c_str(), (int)message.length(), 0);
    }

    //ソケットを閉じる
    closesocket(s);
}

//クライアント関数
void ChatClient(){
    SOCKET s;
    SOCKADDR_IN saddr;
    u_short uport;
    char szServer[1024] = { 0 };
    unsigned int addr;

    //ポート番号の入力
    std::cout << "使用するポート番号 : ";
    std::cin >> uport;
    std::cin.ignore();

    //サーバのIPアドレスを入力
    std::cout << "IPアドレス : ";
    std::cin >> szServer;
    std::cin.ignore();

    //ソケットをオープン
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        std::cout << "ソケットオープンエラー" << std::endl;
        return;
    }

    //サーバーを名前で取得する
    HOSTENT* lpHost;
    lpHost = gethostbyname(szServer);

    if (lpHost == NULL) {
        // サーバーをIPアドレスで取得する
        addr = inet_addr(szServer);
        lpHost = gethostbyaddr((char*)&addr, 4, AF_INET);
    }

    //クライアントソケットをサーバーに接続
    memset(&saddr, 0, sizeof(SOCKADDR_IN));
    saddr.sin_family = AF_INET;//アドレスファミリ
    saddr.sin_port = htons(uport);//ポート番号
    memcpy(&saddr.sin_addr, lpHost->h_addr_list[0], lpHost->h_length);//サーバーのIPアドレスをセット

    if (connect(s, (SOCKADDR*)&saddr, sizeof(saddr)) == SOCKET_ERROR) {
        std::cout << "サーバーと接続失敗" << std::endl;
        closesocket(s);
        return;
    }

    std::cout << "サーバーに接続成功" << std::endl;

    char buffer[1024];
    while (1) {
        std::cout << "クライアント : ";
        std::string message;
        std::getline(std::cin, message);

        //データ送信
        send(s, message.c_str(), (int)message.length(), 0);

        memset(buffer, 0, sizeof(buffer));
        
        //データ受信
        int bytesReceived = recv(s, buffer, sizeof(buffer), 0);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            std::cout << "接続が切断されました" << std::endl;
            break;
        }

        std::cout << "サーバー : " << buffer << std::endl;
    }

    //ソケットを閉じる
    closesocket(s);
}

//メイン関数
int main(void)
{
    WSADATA wsaData;
    int mode;

    //WinSockの初期化
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
        // 初期化エラー
        std::cout << "WinSockの初期化に失敗しました" << std::endl;
        return 1;
    }

    //サーバーか or クライアント
    std::cout << "サーバーなら0を入力 クライアントなら1を入力 : ";
    std::cin >> mode;
    std::cin.ignore();

    if (mode == 0) {
        //サーバーとして起動
        ChatServer();
    }
    else {
        //クライアントとして起動
        ChatClient();
    }

    //WinSockの終了処理
    WSACleanup();

    return 0;
}
