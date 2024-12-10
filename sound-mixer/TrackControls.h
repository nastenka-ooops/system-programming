#ifndef TRACKCONTROLS_H
#define TRACKCONTROLS_H

#include "AudioSource.h"

#include <vector>
#include <Windows.h>
#include <commctrl.h>

class TrackControls {
    HWND hParent;
    HWND hTrackTitle, hPlayButton, hStopButton, hPauseButton, hLoopButton, hDeleteButton,
            hProgressBar, hVolumeBar, hSpeedBar, hPanBar;
    std::vector<HWND> staticLabels;

    AudioSource *source;
    int playButtonId, pauseButtonId, stopButtonId, loopButtonId, deleteButtonId,
            progressBarId, volumeBarId, speedBarId, panBarId;
    UINT_PTR timerId;

public:
    int index;

    TrackControls(HWND parent, int yOffset, AudioSource *source);

    ~TrackControls();

    AudioSource* getSource();

    void UpdatePosition(int newIndex);

    void HandleCommand(WPARAM wParam, int i);

    void HandleScroll(HWND lParam);

    void HandlePlayCommand();

    void HandleStopCommand();

    void HandlePauseCommand();

    bool IsControlRelevant(WPARAM wParam);

    void UpdateProgressBar();

    void HandleProgressBarChange();

    void HandleVolumeBarChange();

    void HandleSpeedBarChange();

    void HandlePanBarChange();
};

#endif //TRACKCONTROLS_H
