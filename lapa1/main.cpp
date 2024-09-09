#include <string>
#include "atlbase.h"
#include "atlwin.h"
#include "windows.h"
#include "gdiplus.h"

using namespace Gdiplus;

const int IMAGE_WIDTH = 200;
const int IMAGE_HEIGHT = 200;
const int DRAGGING_OFFSET = 50;
const std::wstring PATH_TO_OPEN_MOUTH_CAT_IMAGE = L"C:\\Users\\madam\\CLionProjects\\sys-prog\\lapa1\\resources\\images\\catWithOpenMouse.jpg";
const std::wstring PATH_TO_CLOSE_MOUTH_CAT_IMAGE = L"C:\\Users\\madam\\CLionProjects\\sys-prog\\lapa1\\resources\\images\\catWithCLoseMouse.jpg";

Image *normalImage = nullptr;
Image *clickedImage = nullptr;
bool isImageDragging = false;
bool isImageChanged = false;

int imageX = 50;
int imageY = 50;

Image *LoadImageFromFile(const std::wstring &filePath) {
    return new Image(filePath.c_str());
}

// Инициализация GDI+
void InitGDIPlus(ULONG_PTR *gdiplusToken) {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(gdiplusToken, &gdiplusStartupInput, NULL);
}

// Завершение работы с GDI+
void ShutdownGDIPlus(ULONG_PTR *gdiplusToken) {
    GdiplusShutdown(*gdiplusToken);
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

            // Создаем контекст устройства для памяти (double buffering)
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top);
            SelectObject(memDC, memBitmap);

            Graphics graphics(memDC);

            HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(memDC, &ps.rcPaint, hBrush);
            DeleteObject(hBrush);

            // Рисуем изображение: исходное или измененное, в зависимости от состояния
            if (isImageChanged && clickedImage) {
                graphics.DrawImage(clickedImage, imageX, imageY, IMAGE_WIDTH, IMAGE_HEIGHT);
            } else if (normalImage) {
                graphics.DrawImage(normalImage, imageX, imageY, IMAGE_WIDTH, IMAGE_HEIGHT);
            }

            // Копируем результат из памяти на экран
            BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top, memDC, 0, 0, SRCCOPY);

            // Освобождаем ресурсы
            DeleteObject(memBitmap);
            DeleteDC(memDC);

            EndPaint(hWnd, &ps);
            break;
        }
        case WM_LBUTTONDOWN: {
            // Когда пользователь нажимает на изображение, переключаемся на другое
            int mouseX = LOWORD(lParam);
            int mouseY = HIWORD(lParam);
            if (mouseX >= imageX && mouseX <= imageX + IMAGE_WIDTH &&
                mouseY >= imageY && mouseY <= imageY + IMAGE_HEIGHT) {
                isImageChanged = true;
                isImageDragging = true;
                InvalidateRect(hWnd, NULL, TRUE); // Перерисовываем окно
            }
            break;
        }
        case WM_LBUTTONUP: {
            // Когда пользователь отпускает мышь, возвращаем исходное изображение
            isImageChanged = false;
            isImageDragging = false;
            InvalidateRect(hWnd, NULL, TRUE); // Перерисовываем окно
            break;
        }
        case WM_MOUSEMOVE: {
            if (isImageDragging) {
                imageX = LOWORD(lParam) - DRAGGING_OFFSET;  // Простое смещение, если перемещаем объект
                imageY = HIWORD(lParam) - DRAGGING_OFFSET;
                InvalidateRect(hWnd, NULL, TRUE); // Перерисовываем окно
            }
            break;
        }
        case WM_DESTROY: {
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

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    ShutdownGDIPlus(&gdiplusToken);

    return (int) msg.wParam;
}

