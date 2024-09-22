#include <cmath>
#include <sstream>
#include "windows.h"
#include <string>
#include <tchar.h>

#define MIN_ROWS 1
#define MIN_COLS 1
#define MAX_ROWS 20
#define MAX_COLS 20

int nRows = 2;
int nCols = 3;

HWND hEdits[MAX_ROWS][MAX_COLS];

int GetFontSize(HWND hWndEdit) {
    RECT rect;
    GetClientRect(hWndEdit, &rect);

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    int textLength = GetWindowTextLength(hWndEdit);

    return sqrt(width*height/(0.5*textLength + 1))*0.75;
}


void CreateTable(HWND hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    int colWidth = width / nCols;
    int rowHeight = height / nRows;

    for (int row = 0; row < MAX_ROWS; ++row) {
        for (int col = 0; col < MAX_COLS; ++col) {
            if (row < nRows && col < nCols) {
                if (hEdits[row][col] == nullptr) {
                    hEdits[row][col] = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", nullptr,
                                                      WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL,
                                                      col * colWidth, row * rowHeight, colWidth, rowHeight,
                                                      hwnd, (HMENU) (row * MAX_COLS + col), GetModuleHandle(nullptr),
                                                      nullptr);
                }
            } else {
                if (hEdits[row][col] != nullptr) {
                    DestroyWindow(hEdits[row][col]);
                    hEdits[row][col] = nullptr;
                }
            }
        }
    }
}

void ResizeTable(HWND hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    int colWidth = width / nCols;
    int rowHeight = height / nRows;

    for (int row = 0; row < nRows; ++row) {
        for (int col = 0; col < nCols; ++col) {
            SetWindowPos(hEdits[row][col], nullptr, col * colWidth, row * rowHeight,
                         colWidth, rowHeight, SWP_NOZORDER);

            int fontSize = GetFontSize(hEdits[row][col]);

            HFONT newFont = CreateFont(
                   fontSize,
                   0,
                   0,
                   0,
                   FW_NORMAL,
                   FALSE,
                   FALSE,
                   FALSE,
                   DEFAULT_CHARSET,
                   OUT_DEFAULT_PRECIS,
                   CLIP_DEFAULT_PRECIS,
                   DEFAULT_QUALITY,
                   DEFAULT_PITCH | FF_SWISS,
                   "Courier New"
               );
            SendMessage(hEdits[row][col], WM_SETFONT, (WPARAM) newFont, TRUE);
        }
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
                         WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            CreateTable(hWnd);
            break;
        }

        case WM_SIZE: {
            ResizeTable(hWnd);
        }
        break;

        case WM_COMMAND: {
            if (HIWORD(wParam) == EN_UPDATE) {
                int editId = LOWORD(wParam);
                int row = editId / MAX_COLS;
                int col = editId % MAX_COLS;

                int fontSize = GetFontSize(hEdits[row][col]);

                HFONT newFont = CreateFont(
                       fontSize,
                       0,
                       0,
                       0,
                       FW_NORMAL,
                       FALSE,
                       FALSE,
                       FALSE,
                       DEFAULT_CHARSET,
                       OUT_DEFAULT_PRECIS,
                       CLIP_DEFAULT_PRECIS,
                       DEFAULT_QUALITY,
                       DEFAULT_PITCH | FF_SWISS,
                       "Courier New"
                   );
                SendMessage(hEdits[row][col], WM_SETFONT, (WPARAM) newFont, TRUE);
            }
        }
        break;

        case WM_KEYDOWN: {
            switch (wParam) {
                case VK_UP:
                    if (nRows < MAX_ROWS) {
                        nRows++;
                        CreateTable(hWnd);
                        ResizeTable(hWnd);
                    }
                    break;
                case VK_DOWN:
                    if (nRows > MIN_ROWS) {
                        nRows--;
                        CreateTable(hWnd);
                        ResizeTable(hWnd);
                    }
                    break;
                case VK_LEFT:
                    if (nCols > MIN_COLS) {
                        nCols--;
                        CreateTable(hWnd);
                        ResizeTable(hWnd);
                    }
                    break;
                case VK_RIGHT:
                    if (nCols < MAX_COLS) {
                        nCols++;
                        CreateTable(hWnd);
                        ResizeTable(hWnd);
                    }
                    break;
            }
        }
        break;

        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wcex;
    HWND hWnd;
    MSG msg;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_DBLCLKS;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = "lapa2";
    wcex.hIconSm = wcex.hIcon;

    RegisterClassEx(&wcex);
    hWnd = CreateWindow("lapa2", "lapa-2",
                        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0,
                        CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (hWnd == nullptr) {
        MessageBox(nullptr, "Window creation failed!", "Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}
