#include "mediakey-window.h"

void action_changed (MediakeyWindow& window);

MediakeyWindow::MediakeyWindow(QWidget *parent) : QWidget(parent)
{

}

void MediakeyWindow::setAction(MediakeyWindow::MWAction& action)
{
    if (action != mAction) {
        mAction = action;
        action_changed (*this);
    } else {
        window_update_and_hide (*this);
    }
}

void action_changed (MediakeyWindow& window)
{
    if (!window_is_composited(window)) {
        switch (window.mAction) {
        case MediakeyWindow::MWACTION_VOLUME:
            volume_controls_set_visible (window, true);
            if (window.mVolumeMuted) {
                window.setIconName("audio-volume-muted");
            } else {
                window.setIconName("audio-volume-high");
            }
            break;
        case MediakeyWindow::MWACTION_CUSTOM:
            volume_controls_set_visible (window, window.mShowLevel);
            window.setIconName(window.mIconName);
            break;
        default:
            break;
        }
    }
}
