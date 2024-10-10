#include <windows.h>
#include <mmsystem.h>
#include <commdlg.h>
#include <stdio.h>

#pragma comment(lib, "winmm.lib")

#define SAMPLE_RATE 44100
#define BITS_PER_SAMPLE 16
#define NUM_CHANNELS 2
#define BUFFER_SIZE 44100

HWAVEOUT hWaveOut;
WAVEFORMATEX waveFormat;
WAVEHDR waveHeader;
short buffer[BUFFER_SIZE];

char file1[260], file2[260]; // Пути к файлам

// Структура для хранения WAV-данных
typedef struct {
    short *data;
    int dataSize;
    WAVEFORMATEX format;
} WAVFile;

// Простая функция микширования двух аудиопотоков
void mix_audio(short *buffer1, short *buffer2, short *outputBuffer, int numSamples) {
    for (int i = 0; i < numSamples; i++) {
        int mixedSample = buffer1[i] + buffer2[i];
        if (mixedSample > 32767) mixedSample = 32767;
        if (mixedSample < -32768) mixedSample = -32768;
        outputBuffer[i] = static_cast<short>(mixedSample);
    }
}

// Функция загрузки WAV-файла
WAVFile load_wav_file(const char *filename) {
    WAVFile wav = {0};
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Error opening WAV file.\n");
        return wav;
    }

    // Чтение заголовка
    char chunkId[4];
    int chunkSize;
    short audioFormat, numChannels, bitsPerSample;
    int sampleRate, byteRate, dataSize;

    fread(chunkId, sizeof(char), 4, file); // "RIFF"
    fread(&chunkSize, sizeof(int), 1, file);
    fread(chunkId, sizeof(char), 4, file); // "WAVE"

    // Чтение fmt chunk
    fread(chunkId, sizeof(char), 4, file); // "fmt "
    fread(&chunkSize, sizeof(int), 1, file);
    fread(&audioFormat, sizeof(short), 1, file);
    fread(&numChannels, sizeof(short), 1, file);
    fread(&sampleRate, sizeof(int), 1, file);
    fread(&byteRate, sizeof(int), 1, file);
    fread(&wav.format.nBlockAlign, sizeof(short), 1, file);
    fread(&bitsPerSample, sizeof(short), 1, file);

    if (audioFormat != 1) {
        printf("Only PCM format is supported.\n");
        fclose(file);
        return wav;
    }

    // Пропускаем любое возможное дополнительное поле в заголовке
    fseek(file, chunkSize - 16, SEEK_CUR);

    // Чтение data chunk
    fread(chunkId, sizeof(char), 4, file); // "data"
    fread(&dataSize, sizeof(int), 1, file);

    // Выделение памяти для данных
    wav.data = (short *) malloc(dataSize);
    fread(wav.data, sizeof(char), dataSize, file);
    wav.dataSize = dataSize / sizeof(short);

    // Настройка формата
    wav.format.wFormatTag = WAVE_FORMAT_PCM;
    wav.format.nChannels = numChannels;
    wav.format.nSamplesPerSec = sampleRate;
    wav.format.wBitsPerSample = bitsPerSample;
    wav.format.nBlockAlign = (wav.format.wBitsPerSample / 8) * wav.format.nChannels;
    wav.format.nAvgBytesPerSec = wav.format.nSamplesPerSec * wav.format.nBlockAlign;
    wav.format.cbSize = 0;

    fclose(file);
    return wav;
}

// Диалог выбора файла
BOOL open_file_dialog(char *fileName, int maxSize) {
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = fileName;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = maxSize;
    ofn.lpstrFilter = "WAV Files\0*.WAV\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    return GetOpenFileName(&ofn);
}

// Функция воспроизведения
void play_mixed_audio() {
    // Загрузка WAV-файлов
    WAVFile wav1 = load_wav_file(file1);
    WAVFile wav2 = load_wav_file(file2);
    if (wav1.data == NULL || wav2.data == NULL) {
        MessageBox(NULL, "Error loading WAV files.", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Настраиваем формат звука
    waveFormat = wav1.format;
    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveFormat, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        MessageBox(NULL, "Error opening wave output device.", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Микшируем два файла
    int numSamples = (wav1.dataSize < wav2.dataSize) ? wav1.dataSize : wav2.dataSize;
    mix_audio(wav1.data, wav2.data, buffer, numSamples);

    // Настраиваем заголовок буфера
    waveHeader.lpData = (LPSTR) buffer;
    waveHeader.dwBufferLength = numSamples * sizeof(short);
    waveHeader.dwFlags = 0;
    waveHeader.dwLoops = 0;

    // Подготавливаем и отправляем буфер для воспроизведения
    waveOutPrepareHeader(hWaveOut, &waveHeader, sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &waveHeader, sizeof(WAVEHDR));

    // Ожидание завершения воспроизведения
    Sleep(5000);

    // Закрываем устройство
    waveOutClose(hWaveOut);

    // Освобождаем память
    free(wav1.data);
    free(wav2.data);
}

// Обработчик сообщений для главного окна
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            // Создаем кнопки
            CreateWindow("BUTTON", "Choose files", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                         50, 50, 150, 30, hwnd, (HMENU)1, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Play", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                         50, 100, 150, 30, hwnd, (HMENU)2, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            break;
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case 1: {
                    // Обработчик для кнопки "Выбрать файлы"
                    if (open_file_dialog(file1, sizeof(file1)) && open_file_dialog(file2, sizeof(file2))) {
                        MessageBox(hwnd, "Files were chosen", "Information", MB_OK);
                    } else {
                        MessageBox(hwnd, "Error selecting files", "Fail", MB_OK | MB_ICONERROR);
                    }
                    break;
                }
                case 2: {
                    // Обработчик для кнопки "Воспроизвести"
                    play_mixed_audio();
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
