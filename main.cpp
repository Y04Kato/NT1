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

    //�|�[�g�ԍ��̓���
    std::cout << "�g�p����|�[�g�ԍ� : ";
    std::cin >> uport;

    //�\�P�b�g�̍쐬
    std::cout << "�\�P�b�g�̍쐬���J�n" << std::endl;
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cerr << "�\�P�b�g�̍쐬�Ɏ��s" << std::endl;
        return;
    }
    std::cout << "�\�P�b�g�̍쐬�ɐ���" << std::endl;

    //�T�[�o�[�̃|�[�g�ԍ���IP�A�h���X��ݒ�
    memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(uport);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    //�\�P�b�g���o�C���h
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "�o�C���h�Ɏ��s" << std::endl;
        closesocket(serverSocket);
        return;
    }
    std::cout << "�o�C���h�ɐ���" << std::endl;

    //�ڑ��ҋ@
    listen(serverSocket, 3);
    std::cout << "�N���C�A���g�ڑ���ҋ@��" << std::endl;

    //�N���C�A���g�̐ڑ����m�F
    clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "�ڑ��Ɏ��s" << std::endl;
        closesocket(serverSocket);
        return;
    }
    std::cout << "�N���C�A���g���ڑ�" << std::endl;

    //���b�Z�[�W���������[�v
    int recvSize;
    while (true) {
        //��M�o�b�t�@�̏�����
        memset(buffer, 0, sizeof(buffer));

        //�N���C�A���g����̃��b�Z�[�W��M
        recvSize = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (recvSize == SOCKET_ERROR) {
            std::cerr << "��M�Ɏ��s" << std::endl;
            break;
        }
        else if (recvSize == 0) {
            std::cout << "�N���C�A���g���ؒf" << std::endl;
            break;
        }

        //��M�f�[�^�ɏI�[������ǉ�
        buffer[recvSize] = '\0';
        std::cout << "�N���C�A���g����̃��b�Z�[�W : " << buffer << std::endl;

        //���b�Z�[�W���N���C�A���g�ɕԐM
        if (send(clientSocket, buffer, strlen(buffer), 0) == SOCKET_ERROR) {
            std::cerr << "�ԐM�̑��M�Ɏ��s���܂���" << std::endl;
            break;
        }
    }

    //�\�P�b�g�����
    closesocket(clientSocket);
    closesocket(serverSocket);
}

void ClientSide() {
    SOCKET clientSocket;
    sockaddr_in serverAddr;
    char buffer[1024];
    char serverIp[16];
    int serverPort;

    //�N���C�A���g�\�P�b�g�̏�����
    std::cout << "�\�P�b�g�̍쐬���J�n" << std::endl;
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cerr << "�\�P�b�g�̍쐬�Ɏ��s" << std::endl;
        return;
    }
    std::cout << "�\�P�b�g�̍쐬�ɐ���" << std::endl;

    //�T�[�o�[��IP�A�h���X�ƃ|�[�g�����
    std::cout << "�ڑ���̃T�[�o�[IP����� : ";
    std::cin >> serverIp;

    std::cout << "�ڑ���̃T�[�o�[�|�[�g����� : ";
    std::cin >> serverPort;
    std::cin.ignore();

    //�T�[�o�[�̃A�h���X�\���̂�ݒ�
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);

    //IP�A�h���X���o�C�i���`���ɕϊ����Đݒ�
    if (inet_pton(AF_INET, serverIp, &serverAddr.sin_addr) <= 0) {
        std::cerr << "IP�A�h���X�̐ݒ�Ɏ��s���܂���" << std::endl;
        closesocket(clientSocket);
        return;
    }

    //�T�[�o�[�ɐڑ�
    std::cout << "�T�[�o�[�ڑ����J�n" << std::endl;
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "�T�[�o�[�ڑ��Ɏ��s" << std::endl;
        closesocket(clientSocket);
        return;
    }
    std::cout << "�T�[�o�[�ڑ��ɐ����A�I������(end)�����" << std::endl;

    //���b�Z�[�W���������[�v
    while (true) {
        std::cout << "���M���郁�b�Z�[�W����� : ";
        std::cin.getline(buffer, 1024);

        //"end"����͂�����ڑ����I��
        if (strcmp(buffer, "end") == 0) {
            std::cout << "�T�[�o�[�ڑ����I�����Đؒf���܂�" << std::endl;
            break;
        }

        //�T�[�o�[�Ƀ��b�Z�[�W�𑗐M
        if (send(clientSocket, buffer, strlen(buffer), 0) == SOCKET_ERROR) {
            std::cerr << "���b�Z�[�W�̑��M�Ɏ��s���܂���" << std::endl;
            break;
        }

        //�T�[�o�[����̃��b�Z�[�W����M
        int recvSize = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (recvSize == SOCKET_ERROR) {
            std::cerr << "���b�Z�[�W�̎�M�Ɏ��s���܂���" << std::endl;
            break;
        }
        else if (recvSize == 0) {
            std::cout << "�T�[�o�[�Ƃ̒ʐM���ؒf����܂���" << std::endl;
            break;
        }

        //��M�f�[�^�ɏI�[������ǉ�
        buffer[recvSize] = '\0';
        std::cout << "�T�[�o�[����̃��b�Z�[�W : " << buffer << std::endl;
    }

    //�\�P�b�g�����
    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    int startUpMode;

    //WinSock�̏�����
    std::cout << "WinSock�̏��������J�n" << std::endl;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WinSock�̏������Ɏ��s���܂���" << std::endl;
        return 1;
    }

    //�T�[�o�[���N���C�A���g����I��
    std::cout << "�T�[�o�[�Ȃ�0����́A�N���C�A���g�Ȃ�1����� : ";
    std::cin >> startUpMode;

    if (startUpMode == 0) {
        ServerSide();
    }
    else {
        ClientSide();
    }

    //WinSock�̏I������
    WSACleanup();
    return 0;
}
