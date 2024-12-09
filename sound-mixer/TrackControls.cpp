//
// Created by madam on 07.12.2024.
//

#include "TrackControls.h"
#include <algorithm>

TrackControls::TrackControls(HWND parent, int index, AudioSource *source)
    : hParent(parent), source(source) {
    int yOffset = 50 + index * 50;

    HINSTANCE hInst = (HINSTANCE) GetWindowLongPtr(parent, GWLP_HINSTANCE);

    playButtonId = 1000 + index * 10 + 0;
    pauseButtonId = 1000 + index * 10 + 1;
    stopButtonId = 1000 + index * 10 + 2;
    progressBarId = 1000 + index * 10 + 3;
    volumeBarId = 1000 + index * 10 + 4;
    speedBarId = 1000 + index * 10 + 5;
    panBarId = 1000 + index * 10 + 6;
    loopButtonId = 1000 + index * 10 + 7;

    // Track title
    hTrackTitle = CreateWindowW(L"STATIC", source->getName(), WS_CHILD | WS_VISIBLE,
                                10, yOffset, 90, 30, hParent, NULL, hInst, NULL);
    // Buttons
    hPlayButton = CreateWindowW(L"BUTTON", L"Play", WS_CHILD | WS_VISIBLE,
                                110, yOffset, 50, 30, hParent, (HMENU)playButtonId, hInst, NULL);
    hStopButton = CreateWindowW(L"BUTTON", L"Stop", WS_CHILD | WS_VISIBLE,
                                160, yOffset, 50, 30, hParent, (HMENU)stopButtonId, hInst, NULL);
    hPauseButton = CreateWindowW(L"BUTTON", L"Pause", WS_CHILD | WS_VISIBLE,
                                 210, yOffset, 50, 30, hParent, (HMENU)pauseButtonId, hInst, NULL);
    hLoopButton = CreateWindowW(L"BUTTON", L"LOOP", WS_CHILD | WS_VISIBLE,
                                260, yOffset, 50, 30, hParent, (HMENU)loopButtonId, hInst, NULL);

    // Progress bar with label
    CreateWindowW(L"STATIC", L"Position:", WS_CHILD | WS_VISIBLE,
                  320, yOffset - 20, 70, 20, hParent, NULL, hInst, NULL);
    hProgressBar = CreateWindowW(L"msctls_trackbar32", NULL, WS_CHILD | WS_VISIBLE,
                                 320, yOffset, 150, 30, hParent, (HMENU)progressBarId, hInst, NULL);
    SendMessage(hProgressBar, TBM_SETRANGE, TRUE, MAKELPARAM(0, 1000));
    SendMessage(hProgressBar, TBM_SETPOS, TRUE, source->getPosition());

    // Volume bar with label
    CreateWindowW(L"STATIC", L"Volume:", WS_CHILD | WS_VISIBLE,
                  480, yOffset - 20, 70, 20, hParent, NULL, hInst, NULL);
    hVolumeBar = CreateWindowW(L"msctls_trackbar32", NULL, WS_CHILD | WS_VISIBLE,
                               480, yOffset, 150, 30, hParent, (HMENU)volumeBarId, hInst, NULL);
    SendMessage(hVolumeBar, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));
    SendMessage(hVolumeBar, TBM_SETPOS, TRUE, static_cast<int>(source->getVolume() * 100));

    // Speed bar with label
    CreateWindowW(L"STATIC", L"Speed:", WS_CHILD | WS_VISIBLE,
                  640, yOffset - 20, 70, 20, hParent, NULL, hInst, NULL);
    hSpeedBar = CreateWindowW(L"msctls_trackbar32", NULL, WS_CHILD | WS_VISIBLE,
                              640, yOffset, 150, 30, hParent, (HMENU)speedBarId, hInst, NULL);
    SendMessage(hSpeedBar, TBM_SETRANGE, TRUE, MAKELPARAM(500, 2000));
    SendMessage(hSpeedBar, TBM_SETPOS, TRUE, static_cast<int>(source->getSpeed() * 1000));

    // Pan bar with label
    CreateWindowW(L"STATIC", L"Pan:", WS_CHILD | WS_VISIBLE,
                  800, yOffset - 20, 70, 20, hParent, NULL, hInst, NULL);
    hPanBar = CreateWindowW(L"msctls_trackbar32", NULL, WS_CHILD | WS_VISIBLE,
                            800, yOffset, 150, 30, hParent, (HMENU)panBarId, hInst, NULL);
    SendMessage(hPanBar, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));
    SendMessage(hPanBar, TBM_SETPOS, TRUE, static_cast<int>(source->getPan() * 100));
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
}

void TrackControls::HandleCommand(WPARAM wParam) {
    if (LOWORD(wParam) == playButtonId) {
        source->play();

        timerId = SetTimer(hParent, progressBarId, 1000 / 30, NULL);
    } else if (LOWORD(wParam) == stopButtonId) {
        source->stop();

        KillTimer(hParent, timerId);
        timerId = 0;
        SendMessage(hProgressBar, TBM_SETPOS, TRUE, 0);
    } else if (LOWORD(wParam) == pauseButtonId) {
        source->pause();

        KillTimer(hParent, timerId);
        timerId = 0;
    } else if (LOWORD(wParam) == loopButtonId) {
        source->setLoop(!source->getLoop());
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
           || commandId == speedBarId || commandId == panBarId;
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
