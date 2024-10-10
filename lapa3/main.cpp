#include <string>
#include <windows.h>

HWND hEditKey, hEditValueName, hEditValueData;
HWND hButtonCreateKey, hButtonSetValue, hButtonQueryValue, hButtonDeleteKey;
HKEY hKey;

void CreateRegistryKey(const std::string &subKey) {
    LONG result = RegCreateKeyExA(HKEY_CURRENT_USER, subKey.c_str(), 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    if (result == ERROR_SUCCESS) {
        MessageBoxA(NULL, "Key created or opened successfully!", "Success", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxA(NULL, "Error creating key!", "Fail", MB_OK | MB_ICONERROR);
    }
}

void SetRegistryValue(const std::string &valueName, DWORD valueData) {
    LONG result = RegSetValueExA(HKEY_CURRENT_USER, valueName.c_str(), 0, REG_DWORD, (BYTE *) &valueData, sizeof(valueData));
    if (result == ERROR_SUCCESS) {
        MessageBoxA(NULL, "The value is set!", "Success", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxA(NULL, "Error setting value!", "Fail", MB_OK | MB_ICONERROR);
    }
}

void QueryRegistryValue(const std::string &valueName) {
    DWORD data;
    DWORD dataSize = sizeof(data);
    LONG result = RegQueryValueExA(HKEY_CURRENT_USER, valueName.c_str(), NULL, NULL, (LPBYTE) &data, &dataSize);
    if (result == ERROR_SUCCESS) {
        std::string message = "Value: " + std::to_string(data);
        MessageBoxA(NULL, message.c_str(), "Key value", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxA(NULL, "Error getting value!", "Fail", MB_OK | MB_ICONERROR);
    }
}

void DeleteRegistryKey(const std::string &subKey) {
    LONG result = RegDeleteKeyA(HKEY_CURRENT_USER, subKey.c_str());
    if (result == ERROR_SUCCESS) {
        MessageBoxA(NULL, "Key removed!", "Success", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxA(NULL, "Error deleting key!", "Fail", MB_OK | MB_ICONERROR);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            // Поля для ввода ключа и значения
            CreateWindow("STATIC", "Registry key:", WS_VISIBLE | WS_CHILD, 20, 20, 100, 20, hwnd, NULL, NULL, NULL);
            hEditKey = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 150, 20, 300, 20, hwnd, NULL, NULL,
                                    NULL);

            CreateWindow("STATIC", "Value name:", WS_VISIBLE | WS_CHILD, 20, 60, 100, 20, hwnd, NULL, NULL, NULL);
            hEditValueName = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 150, 60, 300, 20, hwnd, NULL,
                                          NULL, NULL);

            CreateWindow("STATIC", "Value data:", WS_VISIBLE | WS_CHILD, 20, 100, 100, 20, hwnd, NULL, NULL, NULL);
            hEditValueData = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 150, 100, 300, 20, hwnd, NULL,
                                          NULL, NULL);

            // Кнопки для взаимодействия с реестром
            hButtonCreateKey = CreateWindow("BUTTON", "Create key", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 50, 150, 150,
                                            30, hwnd, (HMENU)1, NULL, NULL);
            hButtonSetValue = CreateWindow("BUTTON", "Set value", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 50, 200, 150,
                                           30, hwnd, (HMENU)2, NULL, NULL);
            hButtonQueryValue = CreateWindow("BUTTON", "Get value", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 50, 250, 150,
                                             30, hwnd, (HMENU)3, NULL, NULL);
            hButtonDeleteKey = CreateWindow("BUTTON", "Delete key", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 50, 300, 150,
                                            30, hwnd, (HMENU)4, NULL, NULL);
            break;
        }

        case WM_COMMAND: {
            char keyBuffer[256], valueNameBuffer[256], valueDataBuffer[256];
            std::string subKey, valueName;
            DWORD valueData;

            switch (LOWORD(wParam)) {
                case 1: {
                    GetWindowText(hEditKey, keyBuffer, 256);
                    subKey = keyBuffer;
                    CreateRegistryKey(subKey);
                    break;
                }
                case 2: {
                    GetWindowText(hEditValueName, valueNameBuffer, 256);
                    GetWindowText(hEditValueData, valueDataBuffer, 256);
                    valueName = valueNameBuffer;
                    valueData = atoi(valueDataBuffer);
                    SetRegistryValue(valueName, valueData);
                    break;
                }
                case 3: {
                    GetWindowText(hEditValueName, valueNameBuffer, 256);
                    valueName = valueNameBuffer;
                    QueryRegistryValue(valueName);
                    break;
                }
                case 4: {
                    GetWindowText(hEditKey, keyBuffer, 256);
                    subKey = keyBuffer;
                    DeleteRegistryKey(subKey);
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
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = "lapa3";
    wcex.hIconSm = wcex.hIcon;

    RegisterClassEx(&wcex);
    hWnd = CreateWindow("lapa3", "lapa-3",
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
