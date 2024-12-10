#include "TrackControls.h"
#include <algorithm>


TrackControls::TrackControls(HWND parent, int id, int index, AudioSource *source)
    : hParent(parent), source(source) {
    this->index = id;

    int yOffset = 70 + index * 75;

    HINSTANCE hInst = (HINSTANCE) GetWindowLongPtr(parent, GWLP_HINSTANCE);

    playButtonId = 1000 + id * 10 + 0;
    pauseButtonId = 1000 + id * 10 + 1;
    stopButtonId = 1000 + id * 10 + 2;
    progressBarId = 1000 + id * 10 + 3;
    volumeBarId = 1000 + id * 10 + 4;
    speedBarId = 1000 + id * 10 + 5;
    panBarId = 1000 + id * 10 + 6;
    loopButtonId = 1000 + id * 10 + 7;
    deleteButtonId = 1000 + id * 10 + 8;

    // Track title
    hTrackTitle = CreateWindowW(L"STATIC", source->getName(), WS_CHILD | WS_VISIBLE,
                                10, yOffset, 90, 30, hParent, NULL, hInst, NULL);
    staticLabels.push_back(hTrackTitle);
    // Buttons
    hPlayButton = CreateWindowW(L"BUTTON", L"Play", WS_CHILD | WS_VISIBLE,
                                110, yOffset, 50, 30, hParent, (HMENU)playButtonId, hInst, NULL);
    hStopButton = CreateWindowW(L"BUTTON", L"Stop", WS_CHILD | WS_VISIBLE,
                                160, yOffset, 50, 30, hParent, (HMENU)stopButtonId, hInst, NULL);
    hPauseButton = CreateWindowW(L"BUTTON", L"Pause", WS_CHILD | WS_VISIBLE,
                                 210, yOffset, 50, 30, hParent, (HMENU)pauseButtonId, hInst, NULL);
    hLoopButton = CreateWindowW(L"BUTTON", L"LOOP", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE,
                                260, yOffset, 50, 30, hParent, (HMENU)loopButtonId, hInst, NULL);

    // Progress bar with label
    staticLabels.push_back(CreateWindowW(L"STATIC", L"Position:", WS_CHILD | WS_VISIBLE,
                                         320, yOffset - 20, 70, 20, hParent, NULL, hInst, NULL));
    hProgressBar = CreateWindowW(L"msctls_trackbar32", NULL, WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
                                 320, yOffset, 200, 30, hParent, (HMENU)progressBarId, hInst, NULL);
    staticLabels.push_back(CreateWindowW(L"STATIC", L"00:00", WS_CHILD | WS_VISIBLE,
                                         320, yOffset + 30, 40, 20, hParent, NULL, hInst, NULL));

    wchar_t durationText[50];
    swprintf(durationText, 50, L"%.2d:%.2d",
             (int) source->getTotalSeconds() / 60,
             (int) source->getTotalSeconds() % 60);

    staticLabels.push_back(CreateWindowW(L"STATIC", durationText, WS_CHILD | WS_VISIBLE,
                                         480, yOffset + 30, 40, 20, hParent, NULL, hInst, NULL));
    SendMessage(hProgressBar, TBM_SETRANGE, TRUE, MAKELPARAM(0, 1000));
    SendMessage(hProgressBar, TBM_SETPOS, TRUE, source->getPosition());
    SendMessage(hProgressBar, TBM_SETTICFREQ, 100, 0);

    // Volume bar with label
    staticLabels.push_back(CreateWindowW(L"STATIC", L"Volume:", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
                                         530, yOffset - 20, 70, 20, hParent, NULL, hInst, NULL));
    hVolumeBar = CreateWindowW(L"msctls_trackbar32", NULL, WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
                               530, yOffset, 200, 30, hParent, (HMENU)volumeBarId, hInst, NULL);
    staticLabels.push_back(CreateWindowW(L"STATIC", L"0", WS_CHILD | WS_VISIBLE,
                                         530, yOffset + 30, 10, 20, hParent, NULL, hInst, NULL));
    staticLabels.push_back(CreateWindowW(L"STATIC", L"10", WS_CHILD | WS_VISIBLE,
                                         710, yOffset + 30, 20, 20, hParent, NULL, hInst, NULL));
    SendMessage(hVolumeBar, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));
    SendMessage(hVolumeBar, TBM_SETPOS, TRUE, static_cast<int>(source->getVolume() * 100));
    SendMessage(hVolumeBar, TBM_SETTICFREQ, 10, 0);

    // Speed bar with label
    staticLabels.push_back(CreateWindowW(L"STATIC", L"Speed:", WS_CHILD | WS_VISIBLE,
                                         740, yOffset - 20, 70, 20, hParent, NULL, hInst, NULL));
    hSpeedBar = CreateWindowW(L"msctls_trackbar32", NULL, WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS| TBS_AUTOTICKS,
                              740, yOffset, 200, 30, hParent, (HMENU)speedBarId, hInst, NULL);
    staticLabels.push_back(CreateWindowW(L"STATIC", L"0.5", WS_CHILD | WS_VISIBLE,
                                         740, yOffset + 30, 20, 20, hParent, NULL, hInst, NULL));
    staticLabels.push_back(CreateWindowW(L"STATIC", L"2.0", WS_CHILD | WS_VISIBLE,
                                         920, yOffset + 30, 20, 20, hParent, NULL, hInst, NULL));
    SendMessage(hSpeedBar, TBM_SETRANGE, TRUE, MAKELPARAM(500, 2000));
    SendMessage(hSpeedBar, TBM_SETPOS, TRUE, static_cast<int>(source->getSpeed() * 1000));
    SendMessage(hSpeedBar, TBM_SETTICFREQ, 150, 0);

    // Pan bar with label
    staticLabels.push_back(CreateWindowW(L"STATIC", L"Pan:", WS_CHILD | WS_VISIBLE,
                                         950, yOffset - 20, 70, 20, hParent, NULL, hInst, NULL));
    hPanBar = CreateWindowW(L"msctls_trackbar32", NULL, WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
                            950, yOffset, 200, 30, hParent, (HMENU)panBarId, hInst, NULL);
    staticLabels.push_back(CreateWindowW(L"STATIC", L"Left", WS_CHILD | WS_VISIBLE,
                                         950, yOffset + 30, 30, 20, hParent, NULL, hInst, NULL));
    staticLabels.push_back(CreateWindowW(L"STATIC", L"Right", WS_CHILD | WS_VISIBLE,
                                         1110, yOffset + 30, 40, 20, hParent, NULL, hInst, NULL));
    SendMessage(hPanBar, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));
    SendMessage(hPanBar, TBM_SETPOS, TRUE, static_cast<int>(source->getPan() * 100));
    SendMessage(hPanBar, TBM_SETTICFREQ, 10, 0);

    hDeleteButton = CreateWindowW(L"BUTTON", L"Delete", WS_CHILD | WS_VISIBLE, // Кнопка удаления
                                  1160, yOffset, 50, 30, hParent, (HMENU)deleteButtonId, hInst, NULL);
}

TrackControls::~TrackControls() {
    DestroyWindow(hTrackTitle);
    DestroyWindow(hPlayButton);
    DestroyWindow(hPauseButton);
    DestroyWindow(hStopButton);
    DestroyWindow(hProgressBar);
    DestroyWindow(hVolumeBar);
    DestroyWindow(hSpeedBar);
    DestroyWindow(hPanBar);
    DestroyWindow(hLoopButton);
    DestroyWindow(hDeleteButton);

    for (auto label: staticLabels) {
        DestroyWindow(label);
    }
}

void TrackControls::UpdatePosition(int newIndex) {
    int yOffset = 70 + newIndex * 75;

    SetWindowPos(hTrackTitle, NULL, 10, yOffset, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
    SetWindowPos(hPlayButton, NULL, 110, yOffset, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
    SetWindowPos(hStopButton, NULL, 160, yOffset, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
    SetWindowPos(hPauseButton, NULL, 210, yOffset, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
    SetWindowPos(hLoopButton, NULL, 260, yOffset, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
    SetWindowPos(hProgressBar, NULL, 320, yOffset, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
    SetWindowPos(hVolumeBar, NULL, 530, yOffset, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
    SetWindowPos(hSpeedBar, NULL, 740, yOffset, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
    SetWindowPos(hPanBar, NULL, 950, yOffset, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
    SetWindowPos(hDeleteButton, NULL, 1160, yOffset, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);

    int staticIndex = 1;

    SetWindowPos(staticLabels[staticIndex++], NULL, 320, yOffset - 20, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
    SetWindowPos(staticLabels[staticIndex++], NULL, 320, yOffset + 30, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
    SetWindowPos(staticLabels[staticIndex++], NULL, 480, yOffset + 30, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);

    SetWindowPos(staticLabels[staticIndex++], NULL, 530, yOffset - 20, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
    SetWindowPos(staticLabels[staticIndex++], NULL, 530, yOffset + 30, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
    SetWindowPos(staticLabels[staticIndex++], NULL, 710, yOffset + 30, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);

    SetWindowPos(staticLabels[staticIndex++], NULL, 740, yOffset - 20, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
    SetWindowPos(staticLabels[staticIndex++], NULL, 740, yOffset + 30, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
    SetWindowPos(staticLabels[staticIndex++], NULL, 920, yOffset + 30, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);

    SetWindowPos(staticLabels[staticIndex++], NULL, 950, yOffset - 20, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
    SetWindowPos(staticLabels[staticIndex++], NULL, 950, yOffset + 30, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
    SetWindowPos(staticLabels[staticIndex++], NULL, 1110, yOffset + 30, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
}


void TrackControls::HandlePlayCommand() {
    source->play();
    timerId = SetTimer(hParent, progressBarId, 1000 / 30, NULL);
}

void TrackControls::HandleStopCommand() {
    source->stop();
    KillTimer(hParent, timerId);
    timerId = 0;
    SendMessage(hProgressBar, TBM_SETPOS, TRUE, 0);
}

void TrackControls::HandlePauseCommand() {
    source->pause();
    KillTimer(hParent, timerId);
    timerId = 0;
}

void TrackControls::HandleCommand(WPARAM wParam, int i) {
    if (LOWORD(wParam) == playButtonId) {
        HandlePlayCommand();
    } else if (LOWORD(wParam) == stopButtonId) {
        HandleStopCommand();
    } else if (LOWORD(wParam) == pauseButtonId) {
        HandlePauseCommand();
    } else if (LOWORD(wParam) == loopButtonId) {
        source->setLoop(!source->getLoop());
    } else if (LOWORD(wParam) == deleteButtonId) {
        PostMessage(hParent, WM_USER + 1, i, 0);
    }
}

void TrackControls::HandleScroll(HWND lParam) {
    int controlId = GetDlgCtrlID(lParam);

    if (controlId == progressBarId) {
        HandleProgressBarChange();
    } else if (controlId == volumeBarId) {
        HandleVolumeBarChange();
    } else if (controlId == speedBarId) {
        HandleSpeedBarChange();
    } else if (controlId == panBarId) {
        HandlePanBarChange();
    }
}

bool TrackControls::IsControlRelevant(WPARAM wParam) {
    int commandId = LOWORD(wParam);
    return commandId == playButtonId || commandId == pauseButtonId || commandId == stopButtonId
           || commandId == loopButtonId || commandId == progressBarId || commandId == volumeBarId
           || commandId == speedBarId || commandId == panBarId || commandId == deleteButtonId;
}

void TrackControls::UpdateProgressBar() {
    if (source != NULL && source->getStatus() == AudioSource::PLAY) {
        double progress = source->getElapsedSeconds() / source->getTotalSeconds();
        int position = static_cast<int>(progress * 1000);

        SendMessage(hProgressBar, TBM_SETPOS, TRUE, position);
    }
}

void TrackControls::HandleProgressBarChange() {
    if (source == NULL)
        return;

    int position = SendMessage(hProgressBar, TBM_GETPOS, 0, 0);
    double newPosition = position / 1000.0 * source->getBuffer()->getLength();
    source->setPosition(newPosition);
}

void TrackControls::HandleVolumeBarChange() {
    if (source == NULL)
        return;

    int position = SendMessage(hVolumeBar, TBM_GETPOS, 0, 0);
    double volume = position / 100.0f;
    source->setVolume(volume);
}

void TrackControls::HandleSpeedBarChange() {
    if (source == NULL) return;

    int position = SendMessage(hSpeedBar, TBM_GETPOS, 0, 0);
    double speedFactor = position / 1000.0;

    source->setSpeed(speedFactor);
}

void TrackControls::HandlePanBarChange() {
    if (source == NULL) return;

    int position = SendMessage(hPanBar, TBM_GETPOS, 0, 0);
    double pan = position / 100.0;

    source->setPan(pan);
}

AudioSource* TrackControls::getSource() {
    return source;
}
