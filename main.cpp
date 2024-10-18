#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <iostream>
#include <string>

#pragma comment (lib, "WSock32.lib")

//�T�[�o�[�֐�
void ChatServer(void){
    SOCKET listen_s;
    SOCKET s;
    SOCKADDR_IN saddr;
    SOCKADDR_IN from;
    int fromlen;
    u_short uport;

    //�|�[�g�ԍ��̓���
    std::cout << "�g�p����|�[�g�ԍ� : ";
    std::cin >> uport;
    std::cin.ignore();

    //���X���\�P�b�g���I�[�v��
    listen_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_s == INVALID_SOCKET) {
        std::cout << "���X���\�P�b�g�I�[�v���G���[" << std::endl;
        WSACleanup();
        return;
    }

    std::cout << "���X���\�P�b�g���I�[�v�����܂���" << std::endl;

    //�\�P�b�g�ɖ��O��t����
    memset(&saddr, 0, sizeof(SOCKADDR_IN));
    saddr.sin_family = AF_INET;//�A�h���X�t�@�~��
    saddr.sin_port = htons(uport);//�|�[�g�ԍ�
    saddr.sin_addr.s_addr = INADDR_ANY;//�C�ӂ�IP�A�h���X���󂯓����

    if (bind(listen_s, (SOCKADDR*)&saddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
        std::cout << "bind�G���[" << std::endl;
        closesocket(listen_s);
        return;
    }
    std::cout << "bind�����ł�" << std::endl;

    //�N���C�A���g����̐ڑ��҂��̏�Ԃɂ���
    if (listen(listen_s, SOMAXCONN) == SOCKET_ERROR) {
        std::cout << "listen�G���[" << std::endl;
        closesocket(listen_s);
        return;
    }

    std::cout << "listen����" << std::endl;

    //�ڑ��ҋ@����
    std::cout << "�N���C�A���g�ڑ���ҋ@���܂�" << std::endl;

    fromlen = (int)sizeof(from);
    s = accept(listen_s, (SOCKADDR*)&from, &fromlen);  // �C��

    if (s == INVALID_SOCKET) {
        std::cout << "�N���C�A���g�ڑ��G���[" << std::endl;
        closesocket(listen_s);
        return;
    }

    std::cout << inet_ntoa(from.sin_addr) << "���ڑ����Ă��܂���" << std::endl;
    std::cout << "�N���C�A���g�ڑ��ɐ���" << std::endl;

    //���X���\�P�b�g�͂����s�v
    closesocket(listen_s);

    //�`���b�g�J�n
    std::cout << "�`���b�g�J�n" << std::endl;

    char buffer[1024];
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(s, buffer, sizeof(buffer), 0);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            std::cout << "�ڑ����ؒf����܂���" << std::endl;
            break;
        }

        std::cout << "�N���C�A���g : " << buffer << std::endl;

        std::cout << "�T�[�o�[ : ";
        std::string message;
        std::getline(std::cin, message);

        //�f�[�^���M
        send(s, message.c_str(), (int)message.length(), 0);
    }

    //�\�P�b�g�����
    closesocket(s);
}

//�N���C�A���g�֐�
void ChatClient(){
    SOCKET s;
    SOCKADDR_IN saddr;
    u_short uport;
    char szServer[1024] = { 0 };
    unsigned int addr;

    //�|�[�g�ԍ��̓���
    std::cout << "�g�p����|�[�g�ԍ� : ";
    std::cin >> uport;
    std::cin.ignore();

    //�T�[�o��IP�A�h���X�����
    std::cout << "IP�A�h���X : ";
    std::cin >> szServer;
    std::cin.ignore();

    //�\�P�b�g���I�[�v��
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        std::cout << "�\�P�b�g�I�[�v���G���[" << std::endl;
        return;
    }

    //�T�[�o�[�𖼑O�Ŏ擾����
    HOSTENT* lpHost;
    lpHost = gethostbyname(szServer);

    if (lpHost == NULL) {
        // �T�[�o�[��IP�A�h���X�Ŏ擾����
        addr = inet_addr(szServer);
        lpHost = gethostbyaddr((char*)&addr, 4, AF_INET);
    }

    //�N���C�A���g�\�P�b�g���T�[�o�[�ɐڑ�
    memset(&saddr, 0, sizeof(SOCKADDR_IN));
    saddr.sin_family = AF_INET;//�A�h���X�t�@�~��
    saddr.sin_port = htons(uport);//�|�[�g�ԍ�
    memcpy(&saddr.sin_addr, lpHost->h_addr_list[0], lpHost->h_length);//�T�[�o�[��IP�A�h���X���Z�b�g

    if (connect(s, (SOCKADDR*)&saddr, sizeof(saddr)) == SOCKET_ERROR) {
        std::cout << "�T�[�o�[�Ɛڑ����s" << std::endl;
        closesocket(s);
        return;
    }

    std::cout << "�T�[�o�[�ɐڑ�����" << std::endl;

    char buffer[1024];
    while (1) {
        std::cout << "�N���C�A���g : ";
        std::string message;
        std::getline(std::cin, message);

        //�f�[�^���M
        send(s, message.c_str(), (int)message.length(), 0);

        memset(buffer, 0, sizeof(buffer));
        
        //�f�[�^��M
        int bytesReceived = recv(s, buffer, sizeof(buffer), 0);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            std::cout << "�ڑ����ؒf����܂���" << std::endl;
            break;
        }

        std::cout << "�T�[�o�[ : " << buffer << std::endl;
    }

    //�\�P�b�g�����
    closesocket(s);
}

//���C���֐�
int main(void)
{
    WSADATA wsaData;
    int mode;

    //WinSock�̏�����
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
        // �������G���[
        std::cout << "WinSock�̏������Ɏ��s���܂���" << std::endl;
        return 1;
    }

    //�T�[�o�[�� or �N���C�A���g
    std::cout << "�T�[�o�[�Ȃ�0����� �N���C�A���g�Ȃ�1����� : ";
    std::cin >> mode;
    std::cin.ignore();

    if (mode == 0) {
        //�T�[�o�[�Ƃ��ċN��
        ChatServer();
    }
    else {
        //�N���C�A���g�Ƃ��ċN��
        ChatClient();
    }

    //WinSock�̏I������
    WSACleanup();

    return 0;
}
