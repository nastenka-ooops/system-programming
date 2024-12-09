//
// Created by madam on 07.12.2024.
//

#ifndef TRACKCONTROLS_H
#define TRACKCONTROLS_H

#include "AudioSource.h"
#include <Windows.h>

#include "commctrl.h"

class TrackControls {
    HWND hParent;
    HWND hTrackTitle, hPlayButton, hStopButton, hPauseButton, hLoopButton,
            hProgressBar, hVolumeBar, hSpeedBar, hPanBar;
    AudioSource *source;
    int playButtonId, pauseButtonId, stopButtonId, loopButtonId,
            progressBarId, volumeBarId, speedBarId, panBarId;
    UINT_PTR timerId;

public:
    TrackControls(HWND parent, int yOffset, AudioSource *source);

    ~TrackControls();

    void HandleCommand(WPARAM wParam);

    void HandleScroll(HWND lParam);

    bool IsControlRelevant(WPARAM wParam);

    void UpdateProgressBar();

    void HandleProgressBarChange();

    void HandleVolumeBarChange();

    void HandleSpeedBarChange();

    void HandlePanBarChange();
};

#endif //TRACKCONTROLS_H
