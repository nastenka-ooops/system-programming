#include <string>
#include "atlbase.h"
#include "atlwin.h"
#include "windows.h"
#include "gdiplus.h"
#define _USE_MATH_DEFINES
#include <math.h>

using namespace Gdiplus;

#define ID_ACCEL_LEFT  5001
#define ID_ACCEL_RIGHT 5002

ACCEL accelTable[] = {
        { FSHIFT | FVIRTKEY, VK_LEFT,  ID_ACCEL_LEFT},
        { FSHIFT | FVIRTKEY, VK_RIGHT, ID_ACCEL_RIGHT}
};

const int IMAGE_WIDTH = 200;
const int IMAGE_HEIGHT = 200;
const int DRAGGING_OFFSET = 50;
const int MOVE_SPEED = 10;
const std::wstring PATH_TO_OPEN_MOUTH_CAT_IMAGE = L"C:\\Users\\madam\\CLionProjects\\sys-prog\\lapa1\\resources\\images\\catWithOpenMouse.jpg";
const std::wstring PATH_TO_CLOSE_MOUTH_CAT_IMAGE = L"C:\\Users\\madam\\CLionProjects\\sys-prog\\lapa1\\resources\\images\\catWithCLoseMouse.jpg";

Image *normalImage = nullptr;
Image *clickedImage = nullptr;
bool isImageDragging = false;
bool isImageChanged = false;

int imageX = 50;
int imageY = 50;
int angle = 0;

Image *LoadImageFromFile(const std::wstring &filePath) {
    return new Image(filePath.c_str());
}

// Инициализация GDI+
void InitGDIPlus(ULONG_PTR *gdiplusToken) {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(gdiplusToken, &gdiplusStartupInput, nullptr);
}

// Завершение работы с GDI+
void ShutdownGDIPlus(ULONG_PTR *gdiplusToken) {
    GdiplusShutdown(*gdiplusToken);
}

void UpdateSpritePosition(HWND hWnd) {
    InvalidateRect(hWnd, nullptr, TRUE);
}

bool IsEnglishKeyboardLayout() {
    HKL layout = GetKeyboardLayout(0);
    return LOWORD(layout) == 0x409;  // 0x409 — код английского (США) языка
}

void CheckWindowBounds(HWND hWnd) {
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);

    if (imageX < 0) imageX = 0;
    if (imageY < 0) imageY = 0;
    if (imageX + IMAGE_WIDTH > clientRect.right) {
        imageX = clientRect.right - IMAGE_WIDTH;
    }
    if (imageY + IMAGE_HEIGHT > clientRect.bottom) {
        imageY = clientRect.bottom - IMAGE_HEIGHT;
    }
}

// Обработчик сообщений окна
LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
                         WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            normalImage = LoadImageFromFile(PATH_TO_CLOSE_MOUTH_CAT_IMAGE);
            clickedImage = LoadImageFromFile(PATH_TO_OPEN_MOUTH_CAT_IMAGE);

            if (!normalImage || normalImage->GetLastStatus() != Ok || !clickedImage ||
                clickedImage->GetLastStatus() != Ok) {
                MessageBox(hWnd, "Не удалось загрузить изображения!", "Ошибка", MB_OK | MB_ICONERROR);
                PostQuitMessage(0);
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, ps.rcPaint.right - ps.rcPaint.left,
                                                       ps.rcPaint.bottom - ps.rcPaint.top);
            SelectObject(memDC, memBitmap);

            Graphics graphics(memDC);

            HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(memDC, &ps.rcPaint, hBrush);
            DeleteObject(hBrush);

            float centerX = imageX + IMAGE_WIDTH / 2.0f;
            float centerY = imageY + IMAGE_HEIGHT / 2.0f;

            graphics.TranslateTransform(centerX, centerY);
            graphics.RotateTransform(angle);
            graphics.TranslateTransform(-centerX, -centerY);

            if (isImageChanged && clickedImage) {
                graphics.DrawImage(clickedImage, imageX, imageY, IMAGE_WIDTH, IMAGE_HEIGHT);
            } else if (normalImage) {
                graphics.DrawImage(normalImage, imageX, imageY, IMAGE_WIDTH, IMAGE_HEIGHT);
            }



            BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left,
                   ps.rcPaint.bottom - ps.rcPaint.top, memDC, 0, 0, SRCCOPY);

            DeleteObject(memBitmap);
            DeleteDC(memDC);

            EndPaint(hWnd, &ps);
            break;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_LBUTTONDOWN: {
            int mouseX = LOWORD(lParam);
            int mouseY = HIWORD(lParam);
            if (mouseX >= imageX && mouseX <= imageX + IMAGE_WIDTH &&
                mouseY >= imageY && mouseY <= imageY + IMAGE_HEIGHT) {
                isImageChanged = true;
                isImageDragging = true;
            }
            UpdateSpritePosition(hWnd);
            break;
        }
        case WM_LBUTTONUP: {
            isImageChanged = false;
            isImageDragging = false;
            UpdateSpritePosition(hWnd);
            break;
        }
        case WM_MOUSEMOVE: {
            if (isImageDragging) {
                imageX = LOWORD(lParam) - DRAGGING_OFFSET;
                imageY = HIWORD(lParam) - DRAGGING_OFFSET;

                CheckWindowBounds(hWnd);

                UpdateSpritePosition(hWnd);
            }
            break;
        }
        case WM_KEYDOWN: {
//TODO change
            if (!IsEnglishKeyboardLayout()) {
                break;
            }

            switch (wParam) {
                case 'W':  // Вверх
                case VK_UP:
                    imageY -= MOVE_SPEED;
                    break;
                case 'S':  // Вниз
                case VK_DOWN:
                    imageY += MOVE_SPEED;
                    break;
                case 'A':  // Влево
                case VK_LEFT:
                    imageX -= MOVE_SPEED;
                    break;
                case 'D':  // Вправо
                case VK_RIGHT:
                    imageX += MOVE_SPEED;
                    break;
                case VK_ESCAPE: {
                    int result = MessageBox(hWnd, "Are you sure you want to exit", "Confirmation of exit", MB_YESNO | MB_ICONQUESTION);
                    if (result == IDYES) {
                        PostQuitMessage(0);
                    }
                    break;
                }
            }

            CheckWindowBounds(hWnd);

            UpdateSpritePosition(hWnd);
            break;
        }
        case WM_MOUSEWHEEL: {
            int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

            if (GetKeyState(VK_SHIFT) & 0x8000) {
                imageX += (zDelta > 0 ? -MOVE_SPEED : MOVE_SPEED);
            } else {
                imageY += (zDelta > 0 ? -MOVE_SPEED : MOVE_SPEED);
            }

            CheckWindowBounds(hWnd);
            UpdateSpritePosition(hWnd);
            break;
        }
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case ID_ACCEL_LEFT:
                    angle -= 5;
                    if (angle < 0) angle += 360;
                    break;
                case ID_ACCEL_RIGHT:
                    angle += 5;
                    if (angle >= 360) angle -= 360;
                    break;
            }

            UpdateSpritePosition(hWnd);
            break;
        }
        case WM_DESTROY: {

            if (normalImage) {
                delete normalImage;
            }
            if (clickedImage) {
                delete clickedImage;
            }

            PostQuitMessage(0);
            break;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Точка входа
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    ULONG_PTR gdiplusToken;
    InitGDIPlus(&gdiplusToken);

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
    wcex.lpszClassName = "lapa1";
    wcex.hIconSm = wcex.hIcon;

    RegisterClassEx(&wcex);
    hWnd = CreateWindow("lapa1", "lapa-1",
                        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0,
                        CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (hWnd == nullptr) {
        MessageBox(nullptr, "Window creation failed!", "Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    HACCEL hAccel = CreateAcceleratorTable(accelTable, sizeof(accelTable) / sizeof(ACCEL));

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(hWnd, hAccel, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    ShutdownGDIPlus(&gdiplusToken);

    return (int) msg.wParam;
}

