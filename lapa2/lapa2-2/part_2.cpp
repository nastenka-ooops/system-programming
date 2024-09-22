#include <windows.h>
#include <cmath>
#include <commctrl.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static const int TEXT_BUFFER_SIZE = 256;
    static char text[TEXT_BUFFER_SIZE];
    static int textLength = 0;
    static int charWidth = 25;

    switch (uMsg)
    {
        case WM_CREATE:
        {
            CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                         50, 50, 200, 20, hwnd, (HMENU)1, nullptr, nullptr);
            CreateWindow("BUTTON", "Set Text", WS_CHILD | WS_VISIBLE,
                         260, 50, 80, 25, hwnd, (HMENU)2, nullptr, nullptr);
            return 0;
        }

        case WM_COMMAND:
        {
            if (LOWORD(wParam) == 2)
            {
                GetDlgItemText(hwnd, 1, text, TEXT_BUFFER_SIZE);
                textLength = strlen(text);
                InvalidateRect(hwnd, nullptr, TRUE); // Перерисовать окно
            }
            return 0;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rect;
            GetClientRect(hwnd, &rect);

            int radius = textLength * charWidth / (2 * M_PI);
            int centerX = (rect.right - rect.left)/2;
            int centerY = (rect.bottom - rect.top)/2;

            if (textLength > 0)
            {
                double angleStep = 2 * M_PI / textLength;

                SetTextAlign(hdc, TA_CENTER | TA_BASELINE);

                for (int i = 0; i < textLength; ++i)
                {
                    double angle = i * angleStep;

                    int x = centerX + static_cast<int>(radius * cos(angle));
                    int y = centerY + static_cast<int>(radius * sin(angle));

                    TextOutA(hdc, x, y, &text[i], 1);
                }
            }

            EndPaint(hwnd, &ps);
        }
        return 0;

        case WM_SIZE:
        {
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
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
