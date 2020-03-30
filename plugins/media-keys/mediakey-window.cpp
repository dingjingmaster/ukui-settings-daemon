#include "mediakey-window.h"

void action_changed (MediakeyWindow& window);
void volume_controls_set_visible (MediakeyWindow& window, bool visible);

MediakeyWindow::MediakeyWindow(QWidget *parent) : BaseWindow(parent)
{

}

void MediakeyWindow::setAction(MediakeyWindow::MWAction& action)
{
    if (action != mAction) {
        mAction = action;
        action_changed (*this);
    } else {
        updateAndHide();
    }
}

void MediakeyWindow::setActionCustom(QString iconName, bool showLevel)
{

}

void action_changed (MediakeyWindow& window)
{
    if (!window.isComposited()) {
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

void volume_controls_set_visible (MediakeyWindow& window, bool visible)
{
    if (nullptr == window.mProgress) {
        return;
    }

    if (visible) {
        window.mProgress->show();
    } else {
        window.mProgress->hide();
    }
}
