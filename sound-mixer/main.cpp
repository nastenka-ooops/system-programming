#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <cstdio>

#include "AudioBuffer.h"
#include "AudioSource.h"
#include "AudioMixer.h"
#include "TrackControls.h"
#include "AudioRecorder.h"

#pragma comment(lib, "winmm.lib")

#define SAMPLE_RATE 44100
#define BITS_PER_SAMPLE 16
#define NUM_CHANNELS 2
#define BUFFER_SIZE 4096

AudioMixer mixer(SAMPLE_RATE, BITS_PER_SAMPLE, NUM_CHANNELS, 8, BUFFER_SIZE);
AudioRecorder recorder(SAMPLE_RATE, BITS_PER_SAMPLE, NUM_CHANNELS, 8, BUFFER_SIZE);
HWND hLoadButton, hSaveButton, hStopButton, hPauseButton, hMixButton, hRecordButton;
std::vector<TrackControls *> tracksControls;
bool isSaving = false;

// Обработчик сообщений для главного окна
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            hLoadButton = CreateWindowW(L"BUTTON", L"Load", WS_CHILD | WS_VISIBLE,
                                        10, 10, 100, 30, hwnd, (HMENU)1,
                                        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

            hMixButton = CreateWindow("BUTTON", "Mix", WS_TABSTOP | WS_VISIBLE | WS_CHILD,
                                      130, 10, 100, 30, hwnd, (HMENU)2,
                                      (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

            hSaveButton = CreateWindow("BUTTON", "Save",
                                       WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | BS_PUSHLIKE,
                                       490, 10, 100, 30, hwnd, (HMENU)3,
                                       (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

            hRecordButton = CreateWindow("BUTTON", "Record",
                                         WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | BS_PUSHLIKE,
                                         610, 10, 100, 30, hwnd, (HMENU)4,
                                         (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

            hStopButton = CreateWindow("BUTTON", "Stop", WS_TABSTOP | WS_VISIBLE | WS_CHILD,
                                       250, 10, 100, 30, hwnd, (HMENU)5,
                                       (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

            hPauseButton = CreateWindow("BUTTON", "Pause", WS_TABSTOP | WS_VISIBLE | WS_CHILD,
                                        370, 10, 100, 30, hwnd, (HMENU)6,
                                        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);


            break;
        case WM_TIMER: {
            for (auto &controls: tracksControls) {
                controls->UpdateProgressBar();
            }
            break;
        }
        case WM_HSCROLL: {
            // Передача обработки события изменения позиции прогресс-бара
            for (auto &controls: tracksControls) {
                if (controls->IsControlRelevant((WPARAM) GetDlgCtrlID((HWND) lParam))) {
                    controls->HandleScroll((HWND) lParam);
                }
            }
            break;
        }
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case 1: {
                    OPENFILENAMEW ofn;
                    WCHAR filename[260] = {};
                    ZeroMemory(&ofn, sizeof(ofn));
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = hwnd;
                    ofn.lpstrFile = filename;
                    ofn.nMaxFile = MAX_PATH;
                    ofn.lpstrFilter = L"WAV Files\0*.wav\0";
                    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

                    if (GetOpenFileNameW(&ofn)) {
                        auto buffer = new AudioBuffer();
                        if (buffer->load(filename)) {
                            auto source = mixer.create(buffer, filename);
                            int index = tracksControls.size();
                            auto controls = new TrackControls(hwnd, index, source);
                            tracksControls.push_back(controls);
                        } else {
                            MessageBox(hwnd, "Ошибка загрузки файла!", "Ошибка", MB_ICONERROR);
                        }
                    }
                    break;
                }
                case 2: {
                    for (auto source: mixer.getSources()) {
                        source->play();
                    }
                    break;
                }
                case 3: {
                    if (!isSaving) {
                        OPENFILENAME ofn = {};
                        char szFileName[MAX_PATH] = "";
                        ofn.lStructSize = sizeof(OPENFILENAME);
                        ofn.hwndOwner = hwnd; // hwnd вашего основного окна
                        ofn.lpstrFilter = "Wave Files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0";
                        ofn.lpstrFile = szFileName;
                        ofn.nMaxFile = MAX_PATH;
                        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
                        ofn.lpstrDefExt = "wav";

                        if (GetSaveFileName(&ofn)) {
                            mixer.startRecording(szFileName);
                            for (auto source: mixer.getSources()) {
                                source->play();
                            }
                            isSaving = true;
                        }
                    } else {
                        mixer.stopRecording();
                        for (auto source: mixer.getSources()) {
                            source->stop();
                        }
                        isSaving = false;
                    }
                    break;
                }
                case 4: {
                    const wchar_t *filename = L"record.wav";

                    if (!recorder.recording) {
                        recorder.startRecording(filename);
                    } else {
                        recorder.stopRecording();

                        auto buffer = new AudioBuffer();
                        if (buffer->load(filename)) {
                            auto source = mixer.create(buffer, filename);
                            int index = tracksControls.size();
                            auto controls = new TrackControls(hwnd, index, source);
                            tracksControls.push_back(controls);
                        }
                    }
                }
                case 5: {
                    for (auto source: mixer.getSources()) {
                        source->stop();
                    }
                }
                case 6: {
                    for (auto source: mixer.getSources()) {
                        source->pause();
                    }
                }
                default: {
                    for (auto controls: tracksControls) {
                        if (controls->IsControlRelevant(wParam)) {
                            controls->HandleCommand(wParam);
                            break;
                        }
                    }
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
                        WS_OVERLAPPEDWINDOW, 50, 50,
                        1200, 600, nullptr, nullptr, hInstance, nullptr);

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
