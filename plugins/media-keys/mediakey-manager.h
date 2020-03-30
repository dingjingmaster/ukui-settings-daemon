#ifndef MEDIAKEYMANAGER_H
#define MEDIAKEYMANAGER_H

#include "mediakey-window.h"

#include <QObject>
#include <QThread>
#include <QDBusConnection>

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gio/gio.h>
#include <gdk/gdkx.h>

#include <QGSettings/QGSettings>

#define TOUCHPAD_ENABLED_KEY        "touchpad-enabled"
#define TOUCHPAD_SCHEMA             "org.ukui.peripherals-touchpad"

#define MEDIA_KEYS_DBUS_NAME        "org.ukui.SettingsDaemon.MediaKeys"
#define MEDIA_KEYS_DBUS_PATH        "/org/ukui/SettingsDaemon/MediaKeys"

#define MEDIAKEY_SCHEMA             "org.ukui.SettingsDaemon.plugins.media-keys"

#define VOLUME_STEP 6

typedef struct _media_player MediaPlayer;

class MediakeyManager : public QThread
{
    Q_OBJECT
    Q_CLASSINFO ("D-Bus Interface", MEDIA_KEYS_DBUS_NAME)

public:
    bool mediakeyStart ();
    bool mediakeyStop ();
    static MediakeyManager* mediakeyNew();

private:
    void run() override;
    explicit MediakeyManager(QObject *parent = nullptr);
    explicit MediakeyManager(MediakeyManager&) = delete;

public Q_SLOTS:
    int grabMediaPlayerKeys();              // return code true or false
    int releaseMediaPlayerKeys();           // return code true or false

Q_SIGNALS:
    void mediaPlayerKeyPressed(QString application, QString key);

private:
    friend bool register_manager (MediakeyManager&);
    friend void dialog_init (MediakeyManager* manager);
    friend void dialog_show (MediakeyManager* manager);
    friend void do_eject_action (MediakeyManager* manager);
    friend void do_touchpad_action (MediakeyManager* manager);
    friend gboolean do_action (MediakeyManager* manager, int type);
    friend gboolean usd_media_player_key_pressed (MediakeyManager* manager, const char*);
    friend GdkScreen* acme_get_screen_from_event (MediakeyManager *manager, XAnyEvent* xanyev);
    friend GdkFilterReturn acme_filter_events (GdkXEvent* xevent, GdkEvent* event, MediakeyManager* manager);

private:
    bool                            mExit;
    MediakeyWindow*                 mDialog;
    QGSettings*                     mSettings;
    GVolumeMonitor*                 mVolumeMonitor;             // FIXME://qt version

    GdkScreen*                      mCurrentScreen;
    QList<GdkScreen*>*              mScreens;

    QList<MediaPlayer*>*            mMediaPlayers;
    QDBusConnection*                mConnection;
//    guint                           mNotify[HANDLED_KEYS];

    static MediakeyManager*         mMediaManager;
};

#endif // MEDIAKEYMANAGER_H
