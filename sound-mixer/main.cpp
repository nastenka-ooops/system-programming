#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <cstdio>

#include "AudioBufffer.h"
#include "AudioMixer.h"

#pragma comment(lib, "winmm.lib")

#define SAMPLE_RATE 44100
#define BITS_PER_SAMPLE 16
#define NUM_CHANNELS 2
#define BUFFER_SIZE 44100

// Обработчик сообщений для главного окна
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            CreateWindow("BUTTON", "Play", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                         50, 100, 150, 30, hwnd, (HMENU)2, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            break;
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case 2: {
                    static const int PROGRESS_BAR_LENGTH = 20;
                    const char* filename = "C:/Users/madam/CLionProjects/sys-prog/sound-mixer/sample-3s.wav";
                    const char* filename2 = "C:/Users/madam/CLionProjects/sys-prog/sound-mixer/test.wav";

                    AudioMixer mixer(44100, 16, 2, 8, 4096);

                    AudioSource* source = mixer.play(filename2);
                    mixer.play(filename);
                    if (source == NULL) {
                        printf("Failed to load '%s'.\n", filename);
                        return 1;
                    }

                    printf("Filename  '%s'\n", filename);
                    printf("Duration  %.2d:%.2d\n", (int)source->getTotalSeconds() / 60, (int)source->getTotalSeconds() % 60);
                    printf("Volume    %.2f\n", source->getVolume());
                    printf("Pan       %.2f\n", source->getPan());
                    printf("Speed     %.2fx\n", source->getSpeed());

                    while (!source->finished()) {
                        const float progress = source->getProgress();
                        const int left = progress * PROGRESS_BAR_LENGTH;
                        const int right = PROGRESS_BAR_LENGTH - left;
                        printf("\33[2K\rProgress  [%*s%*s", left, "*", right, "]");
                        _sleep(10);
                    }
                    printf("\nFinished playing\n");

                    break;
                }
            }
            break;
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
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = "sound-mixer";
    wcex.hIconSm = wcex.hIcon;

    RegisterClassEx(&wcex);
    hWnd = CreateWindow("sound-mixer", "sound-mixer",
                        WS_OVERLAPPEDWINDOW, 300, 100,
                        500, 400, nullptr, nullptr, hInstance, nullptr);

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
