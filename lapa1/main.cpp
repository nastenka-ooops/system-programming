#include <string>
#include "atlbase.h"
#include "atlwin.h"
#include "windows.h"
#include "gdiplus.h"
#include <algorithm>

#define _USE_MATH_DEFINES

#include <math.h>

using namespace Gdiplus;

#define ID_ACCEL_LEFT  5001
#define ID_ACCEL_RIGHT 5002

ACCEL accelTable[] = {
        {FSHIFT | FVIRTKEY, VK_LEFT,  ID_ACCEL_LEFT},
        {FSHIFT | FVIRTKEY, VK_RIGHT, ID_ACCEL_RIGHT}
};

const int IMAGE_WIDTH = 200;
const int IMAGE_HEIGHT = 200;
const int DRAGGING_OFFSET = 50;
const int MOVE_SPEED = 10;
const std::wstring PATH_TO_OPEN_MOUTH_CAT_IMAGE = L"C:\\Users\\madam\\CLionProjects\\sys-prog\\lapa1\\resources\\images\\catWithOpenMouse.jpg";
const std::wstring PATH_TO_CLOSE_MOUTH_CAT_IMAGE = L"C:\\Users\\madam\\CLionProjects\\sys-prog\\lapa1\\resources\\images\\catWithCLoseMouse.jpg";
const std::wstring PATH_TO_TRIANGLE_IMAGE = L"C:\\Users\\madam\\CLionProjects\\sys-prog\\lapa1\\resources\\images\\triangle.png";

Image *normalImage = nullptr;
Image *clickedImage = nullptr;
bool isImageDragging = false;
bool isImageChanged = false;
bool isTriangleImage = false;

int imageCenterX = 150;
int imageCenterY = 150;

int angle = 0;

Image *LoadImageFromFile(const std::wstring &filePath) {
    return new Image(filePath.c_str());
}

void InitGDIPlus(ULONG_PTR *gdiplusToken) {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(gdiplusToken, &gdiplusStartupInput, nullptr);
}

void ShutdownGDIPlus(ULONG_PTR *gdiplusToken) {
    GdiplusShutdown(*gdiplusToken);
}

void UpdateSpritePosition(HWND hWnd) {
    InvalidateRect(hWnd, nullptr, TRUE);
}

bool IsEnglishKeyboardLayout() {
    HKL layout = GetKeyboardLayout(0);
    return LOWORD(layout) == MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
}

float findMinElement(float arr[], int size) {
    float minElement = arr[0];

    for (int i = 1; i < size; ++i) {
        if (arr[i] < minElement) {
            minElement = arr[i];
        }
    }

    return minElement;
}

float findMaxElement(float arr[], int size) {
    float maxElement = arr[0];

    for (int i = 1; i < size; ++i) {
        if (arr[i] > maxElement) {
            maxElement = arr[i];
        }
    }

    return maxElement;
}

double TriangleArea(POINT A, POINT B, POINT C) {
    return 0.5 * abs(A.x * (B.y - C.y) + B.x * (C.y - A.y) + C.x * (A.y - B.y));
}

bool IsPointInTriangle(POINT P, POINT A, POINT B, POINT C) {
    double areaABC = TriangleArea(A, B, C);

    double areaPAB = TriangleArea(P, A, B);
    double areaPBC = TriangleArea(P, B, C);
    double areaPCA = TriangleArea(P, C, A);

    return fabs(areaABC - (areaPAB + areaPBC + areaPCA)) < 1e-5;
}

void RotatePoints(float &x, float &y, int centerX, int centerY, int angle) {
    float angleRadians = angle * static_cast<float>(M_PI) / 180.0f;

    float dx = x - centerX;
    float dy = y - centerY;

    float angleCos = cos(angleRadians);
    float angleSin = sin(angleRadians);

    float newX = dx * angleCos - dy * angleSin;
    float newY = dx * angleSin + dy * angleCos;

    x = newX + centerX;
    y = newY + centerY;
}

bool CheckMouseCoordinatesForSquare(float mouseX, float mouseY) {
    RotatePoints(mouseX, mouseY, imageCenterX, imageCenterY, -angle);

    return (mouseX >= imageCenterX - IMAGE_WIDTH / 2 && mouseX <= imageCenterX + IMAGE_WIDTH / 2 &&
            mouseY >= imageCenterY - IMAGE_HEIGHT / 2 && mouseY <= imageCenterY + IMAGE_HEIGHT / 2);

}

void CheckRotatedSquareBounds(HWND hWnd) {
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);

    float topLeftX = imageCenterX - IMAGE_WIDTH / 2;
    float topLeftY = imageCenterY - IMAGE_HEIGHT / 2;

    float topRightX = imageCenterX + IMAGE_WIDTH / 2;
    float topRightY = imageCenterY - IMAGE_HEIGHT / 2;

    float bottomLeftX = imageCenterX - IMAGE_WIDTH / 2;
    float bottomLeftY = imageCenterY + IMAGE_HEIGHT / 2;

    float bottomRightX = imageCenterX + IMAGE_WIDTH / 2;
    float bottomRightY = imageCenterY + IMAGE_HEIGHT / 2;

    RotatePoints(topLeftX, topLeftY, imageCenterX, imageCenterY, angle);
    RotatePoints(topRightX, topRightY, imageCenterX, imageCenterY, angle);
    RotatePoints(bottomLeftX, bottomLeftY, imageCenterX, imageCenterY, angle);
    RotatePoints(bottomRightX, bottomRightY, imageCenterX, imageCenterY, angle);

    float minX = findMinElement(new float[]{topLeftX, topRightX, bottomLeftX, bottomRightX}, 4);
    float minY = findMinElement(new float[]{topLeftY, topRightY, bottomLeftY, bottomRightY}, 4);
    float maxX = findMaxElement(new float[]{topLeftX, topRightX, bottomLeftX, bottomRightX}, 4);
    float maxY = findMaxElement(new float[]{topLeftY, topRightY, bottomLeftY, bottomRightY}, 4);

    if (minX < 0) {
        imageCenterX += -minX;
    }
    if (maxX > clientRect.right) {
        imageCenterX -= (maxX - clientRect.right);
    }
    if (minY < 0) {
        imageCenterY += -minY;
    }
    if (maxY > clientRect.bottom) {
        imageCenterY -= (maxY - clientRect.bottom);
    }

}

void calculateTrianglePoints(POINT points[3], int centerX, int centerY, double angle) {

    points[0] = {0, (LONG) (-IMAGE_HEIGHT / 2)};
    points[1] = {(LONG) (-IMAGE_WIDTH / 2), (LONG) (IMAGE_HEIGHT / 2)};
    points[2] = {(LONG) (IMAGE_WIDTH / 2), (LONG) (IMAGE_HEIGHT / 2)};

    float angleRadians = angle * static_cast<float>(M_PI) / 180.0f;

    for (int i = 0; i < 3; ++i) {
        double x = points[i].x * cos(angleRadians) - points[i].y * sin(angleRadians);
        double y = points[i].x * sin(angleRadians) + points[i].y * cos(angleRadians);
        points[i].x = (LONG) (centerX + x);
        points[i].y = (LONG) (centerY + y);
    }
}

void CheckTriangleBounds(HWND hWnd) {
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);

    POINT points[3];
    calculateTrianglePoints(points, imageCenterX, imageCenterY, angle);

    for (int i = 0; i < 3; ++i) {
        if (points[i].x < 0) imageCenterX += MOVE_SPEED;
        if (points[i].x > clientRect.right) imageCenterX -= MOVE_SPEED;
        if (points[i].y < 0) imageCenterY += MOVE_SPEED;
        if (points[i].y > clientRect.bottom) imageCenterY -= MOVE_SPEED;
    }
}

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

            graphics.TranslateTransform(imageCenterX, imageCenterY);
            graphics.RotateTransform(angle);
            graphics.TranslateTransform(-imageCenterX, -imageCenterY);

            if (isImageChanged && clickedImage) {
                graphics.DrawImage(clickedImage, imageCenterX - IMAGE_WIDTH / 2, imageCenterY - IMAGE_HEIGHT / 2,
                                   IMAGE_WIDTH, IMAGE_HEIGHT);
            } else if (normalImage) {
                graphics.DrawImage(normalImage, imageCenterX - IMAGE_WIDTH / 2, imageCenterY - IMAGE_HEIGHT / 2,
                                   IMAGE_WIDTH, IMAGE_HEIGHT);
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
            POINT clickPoint;
            clickPoint.x = LOWORD(lParam);
            clickPoint.y = HIWORD(lParam);

            int mouseX = LOWORD(lParam);
            int mouseY = HIWORD(lParam);

            POINT points[3];
            calculateTrianglePoints(points, imageCenterX, imageCenterY, angle);

            if (isTriangleImage) {

                if (IsPointInTriangle(clickPoint, points[0], points[1], points[2])) {
                    isImageChanged = true;
                    isImageDragging = true;
                };
            } else {
                if (CheckMouseCoordinatesForSquare(mouseX, mouseY)) {
                    isImageChanged = true;
                    isImageDragging = true;
                }
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
                imageCenterX = LOWORD(lParam) - DRAGGING_OFFSET;
                imageCenterY = HIWORD(lParam) - DRAGGING_OFFSET;

                if (isTriangleImage) {
                    CheckTriangleBounds(hWnd);
                } else {
                    CheckRotatedSquareBounds(hWnd);
                }
                UpdateSpritePosition(hWnd);
            }
            break;
        }
        case WM_KEYDOWN: {

            if (!IsEnglishKeyboardLayout()) {
                break;
            }

            switch (wParam) {
                case 'T':
                    normalImage = LoadImageFromFile(PATH_TO_TRIANGLE_IMAGE);
                    isTriangleImage = true;
                    break;
                case 'C':
                    normalImage = LoadImageFromFile(PATH_TO_CLOSE_MOUTH_CAT_IMAGE);
                    isTriangleImage = false;
                    break;
                case 'W':
                case VK_UP:
                    imageCenterY -= MOVE_SPEED;
                    break;
                case 'S':
                case VK_DOWN:
                    imageCenterY += MOVE_SPEED;
                    break;
                case 'A':
                case VK_LEFT:
                    imageCenterX -= MOVE_SPEED;
                    break;
                case 'D':
                case VK_RIGHT:
                    imageCenterX += MOVE_SPEED;
                    break;
                case VK_ESCAPE: {
                    int result = MessageBox(hWnd, "Are you sure you want to exit", "Confirmation of exit",
                                            MB_YESNO | MB_ICONQUESTION);
                    if (result == IDYES) {
                        PostQuitMessage(0);
                    }
                    break;
                }
            }

            if (isTriangleImage) {
                CheckTriangleBounds(hWnd);
            } else {
                CheckRotatedSquareBounds(hWnd);
            }
            UpdateSpritePosition(hWnd);
            break;
        }
        case WM_MOUSEWHEEL: {
            int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

            if (GetKeyState(VK_SHIFT) & 0x8000) {
                imageCenterX += (zDelta > 0 ? -MOVE_SPEED : MOVE_SPEED);
            } else {
                imageCenterY += (zDelta > 0 ? -MOVE_SPEED : MOVE_SPEED);
            }

            if (isTriangleImage) {
                CheckTriangleBounds(hWnd);
            } else {
                CheckRotatedSquareBounds(hWnd);
            }
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

