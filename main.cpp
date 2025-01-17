#define STRICT
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <WinSock2.h>
#include <Windows.h>
#include <tchar.h>
#include <thread>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

#include "resource.h"

#define PORT 8000
#define BUFFER_SIZE 1024
#define SYNC_TIMER_ID 1
#define SYNC_INTERVAL_MS 30  // 30ミリ秒ごとに同期

HINSTANCE hInstanceGlobal;
HBITMAP serverBitmap, clientBitmap;
POINT serverBitmapPos = { 50, 50 };
POINT clientBitmapPos = { 150, 50 };
SOCKET clientSocket;
bool isRunning = true;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void LoadBitmaps() {
    serverBitmap = LoadBitmap(hInstanceGlobal, MAKEINTRESOURCE(101));  // サーバー用ビットマップ
    clientBitmap = LoadBitmap(hInstanceGlobal, MAKEINTRESOURCE(102));  // クライアント用ビットマップ

    if (!serverBitmap || !clientBitmap) {
        MessageBox(NULL, _T("ビットマップのロードに失敗"), _T("エラー"), MB_ICONERROR);
    }
}

void NetworkSyncThread() {
    char buffer[BUFFER_SIZE];

    while (isRunning) {
        // サーバー側のビットマップ位置を送信
        sprintf_s(buffer, "%d %d", serverBitmapPos.x, serverBitmapPos.y);
        if (send(clientSocket, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {
            std::cerr << "データ送信に失敗しました。" << std::endl;
            break;
        }

        // クライアント側のビットマップ位置を受信
        if (recv(clientSocket, buffer, sizeof(buffer), 0) > 0) {
            sscanf_s(buffer, "%d %d", &clientBitmapPos.x, &clientBitmapPos.y);
        }
        else {
            std::cerr << "データ受信に失敗しました。" << std::endl;
            break;
        }

        Sleep(SYNC_INTERVAL_MS);
    }

    closesocket(clientSocket);
}

int APIENTRY main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
    hInstanceGlobal = hInstance;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        MessageBox(NULL, _T("WSAStartupに失敗"), _T("エラー"), MB_ICONERROR);
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        MessageBox(NULL, _T("ソケットの作成に失敗"), _T("エラー"), MB_ICONERROR);
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        MessageBox(NULL, _T("ソケットのバインドに失敗"), _T("エラー"), MB_ICONERROR);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 1) == SOCKET_ERROR) {
        MessageBox(NULL, _T("ソケットのリスンに失敗"), _T("エラー"), MB_ICONERROR);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    MessageBox(NULL, _T("クライアントの接続を待機中..."), _T("サーバー"), MB_OK);

    sockaddr_in clientAddr = {};
    int clientAddrSize = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket == INVALID_SOCKET) {
        MessageBox(NULL, _T("クライアントの接続に失敗"), _T("エラー"), MB_ICONERROR);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    closesocket(serverSocket);

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = _T("ServerWindow");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(wc.lpszClassName, _T("サーバー: Network Bitmap"), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    LoadBitmaps();

    std::thread networkThread(NetworkSyncThread);
    networkThread.detach();

    SetTimer(hwnd, SYNC_TIMER_ID, SYNC_INTERVAL_MS, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    isRunning = false;
    closesocket(clientSocket);
    WSACleanup();

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
    case WM_TIMER: {
        InvalidateRect(hwnd, NULL, TRUE);
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        HDC hdcMem = CreateCompatibleDC(hdc);

        SelectObject(hdcMem, serverBitmap);
        BitBlt(hdc, serverBitmapPos.x, serverBitmapPos.y, 100, 100, hdcMem, 0, 0, SRCCOPY);

        SelectObject(hdcMem, clientBitmap);
        BitBlt(hdc, clientBitmapPos.x, clientBitmapPos.y, 100, 100, hdcMem, 0, 0, SRCCOPY);

        DeleteDC(hdcMem);
        EndPaint(hwnd, &ps);
        break;
    }
    case WM_KEYDOWN: {
        switch (wparam) {
        case VK_LEFT: serverBitmapPos.x -= 5; break;
        case VK_RIGHT: serverBitmapPos.x += 5; break;
        case VK_UP: serverBitmapPos.y -= 5; break;
        case VK_DOWN: serverBitmapPos.y += 5; break;
        }
        InvalidateRect(hwnd, NULL, TRUE);
        break;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}