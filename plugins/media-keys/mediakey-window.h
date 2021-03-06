#ifndef MEDIAKEYWINDOW_H
#define MEDIAKEYWINDOW_H

#include <QWidget>
#include "base-window.h"

class MediakeyWindow : public BaseWindow
{
    Q_OBJECT

public:
    typedef enum {
        MWACTION_VOLUME,
        MWACTION_CUSTOM
    } MWAction;
    Q_ENUM(MWAction);
public:
    explicit MediakeyWindow(QWidget *parent = nullptr);

    void setAction (MWAction&);
    void setActionCustom (QString iconName, bool showLevel);
    void setVolumeMuted (bool muted);
    void setVolumeLevel (int level);
    void setIconName (QString name);

    static void windowIsValid ();

Q_SIGNALS:

private:
    friend void volume_controls_set_visible (MediakeyWindow& window, bool visible);

private:
    friend void action_changed (MediakeyWindow& window);

private:
    MWAction                    mAction;
    QString                     mIconName;
    bool                        mShowLevel;
    uint                        mVolumeMuted;
    int                         mVolumeLevel;

    QImage*                     mImage;
    QWidget*                    mProgress;
};

#endif // MEDIAKEYWINDOW_H
