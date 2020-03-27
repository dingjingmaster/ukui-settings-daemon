#include "mediakey-manager.h"

#include "global.h"

#include <gio/gio.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XIproto.h>

#include <QDebug>
#include <QDBusError>
#include <QDBusConnectionInterface>

#define NO_SCORE                0
#define SCORE_CAN_EJECT         50
#define SCORE_HAS_MEDIA         100

typedef struct {
    guint       keysym;
    guint       state;
    guint*      keycodes;
} Key;

struct _media_player {
    char   *application;
    guint32 time;
};

enum {
        TOUCHPAD_KEY,
        MUTE_KEY,
        VOLUME_DOWN_KEY,
        VOLUME_UP_KEY,
        POWER_KEY,
        EJECT_KEY,
        HOME_KEY,
        MEDIA_KEY,
        CALCULATOR_KEY,
        SEARCH_KEY,
        EMAIL_KEY,
        SCREENSAVER_KEY,
        HELP_KEY,
        WWW_KEY,
        PLAY_KEY,
        PAUSE_KEY,
        STOP_KEY,
        PREVIOUS_KEY,
        NEXT_KEY,
        REWIND_KEY,
        FORWARD_KEY,
        REPEAT_KEY,
        RANDOM_KEY,
        MAGNIFIER_KEY,
        SCREENREADER_KEY,
        SETTINGS_KEY,
        FILE_MANAGER_KEY,
        ON_SCREEN_KEYBOARD_KEY,
        LOGOUT_KEY,
        TERMINAL_KEY,
        SCREENSHOT_KEY,
        WINDOW_SCREENSHOT_KEY,
        AREA_SCREENSHOT_KEY,
        HANDLED_KEYS,
};

static struct {
        int key_type;
        const char *settings_key;
        const char *hard_coded;
        Key *key;
} keys[HANDLED_KEYS] = {
        { TOUCHPAD_KEY, "touchpad", NULL, NULL },
        { MUTE_KEY, "volume-mute", NULL, NULL },
        { VOLUME_DOWN_KEY, "volume-down", NULL, NULL },
        { VOLUME_UP_KEY, "volume-up", NULL, NULL },
        { POWER_KEY, "power", NULL, NULL },
        { EJECT_KEY, "eject", NULL, NULL },
        { HOME_KEY, "home", NULL, NULL },
        { MEDIA_KEY, "media", NULL, NULL },
        { CALCULATOR_KEY, "calculator", NULL, NULL },
        { SEARCH_KEY, "search", NULL, NULL },
        { EMAIL_KEY, "email", NULL, NULL },
        { SCREENSAVER_KEY, "screensaver", NULL, NULL },
        { SETTINGS_KEY, "ukui-control-center", NULL, NULL},
        { FILE_MANAGER_KEY, "peony-qt", NULL, NULL},
        { HELP_KEY, "help", NULL, NULL },
        { WWW_KEY, "www", NULL, NULL },
        { PLAY_KEY, "play", NULL, NULL },
        { PAUSE_KEY, "pause", NULL, NULL },
        { STOP_KEY, "stop", NULL, NULL },
        { PREVIOUS_KEY, "previous", NULL, NULL },
        { NEXT_KEY, "next", NULL, NULL },
        /* Those are not configurable in the UI */
        { REWIND_KEY, NULL, "XF86AudioRewind", NULL },
        { FORWARD_KEY, NULL, "XF86AudioForward", NULL },
        { REPEAT_KEY, NULL, "XF86AudioRepeat", NULL },
        { RANDOM_KEY, NULL, "XF86AudioRandomPlay", NULL },
        { MAGNIFIER_KEY, "magnifier", NULL, NULL },
        { SCREENREADER_KEY, "screenreader", NULL, NULL },
        { ON_SCREEN_KEYBOARD_KEY, "on-screen-keyboard", NULL, NULL },
        { LOGOUT_KEY, "logout", NULL, NULL },
        { TERMINAL_KEY, "terminal", NULL, NULL },
        { SCREENSHOT_KEY, "screenshot", NULL, NULL },
        { WINDOW_SCREENSHOT_KEY, "window-screenshot", NULL, NULL },
        { AREA_SCREENSHOT_KEY, "area-screenshot", NULL, NULL },
};

gboolean touchpad_is_present (void);
void do_media_action (MediakeyManager*);
gboolean supports_xinput_devices (void);
bool register_manager (MediakeyManager&);
char* get_term_command (MediakeyManager*);
void do_magnifier_action (MediakeyManager*);
gboolean match_key (Key* key, XEvent* event);
void do_screenreader_action (MediakeyManager*);
void do_eject_action (MediakeyManager* manager);
void do_logout_action (MediakeyManager* manager);
void do_toggle_accessibility_key (const char *key);
void do_terminal_action (MediakeyManager* manager);
void do_shutdown_action (MediakeyManager* manager);
void do_touchpad_action (MediakeyManager* manager);
void do_screenshot_action (MediakeyManager* manager);
void do_on_screen_keyboard_action (MediakeyManager*);
XDevice* device_is_touchpad (XDeviceInfo *deviceinfo);
gboolean do_action (MediakeyManager* manager, int type);
void do_area_screenshot_action (MediakeyManager* manager);
void do_url_action (MediakeyManager*, const gchar* scheme);
void do_window_screenshot_action (MediakeyManager* manager);
gboolean device_has_property (XDevice* device, const char* property_name);
void do_eject_action_cb (GDrive* drive, GAsyncResult* res, MediakeyManager*);
gboolean usd_media_player_key_pressed (MediakeyManager* manager, const char*);
gboolean do_multimedia_player_action (MediakeyManager* manager, const char* key);
void execute (MediakeyManager* manager, const char* cmd, gboolean sync, gboolean need_term);
GdkFilterReturn acme_filter_events (GdkXEvent* xevent, GdkEvent* event, MediakeyManager* manager);

MediakeyManager* MediakeyManager::mMediaManager = nullptr;

MediakeyManager* MediakeyManager::mediakeyNew()
{
    bool flag = true;

    // beginning .... new

    // 只有一次机会调用成功,那就是第一次调用
    if (nullptr == mMediaManager) {
        mMediaManager = new MediakeyManager(nullptr);
        if (nullptr == mMediaManager || !register_manager(*mMediaManager)) {
            flag = false;
            CT_SYSLOG(LOG_ERR, "create media manager failed!");
        }
    }

    if (!flag) { delete mMediaManager; mMediaManager = nullptr;}

    return mMediaManager;
}

void MediakeyManager::run()
{
    // init
    mVolumeMonitor = g_volume_monitor_get();

    // ...
    mSettings = new QGSettings(MEDIAKEY_SCHEMA);
//    init_screens();
//    init_kdb();
    while (!mExit) {
        for (QList<GdkScreen*>::ConstIterator l = mScreens->constBegin(); l != mScreens->constEnd(); ++l) {
            gdk_window_add_filter(gdk_screen_get_root_window(*l), (GdkFilterFunc)acme_filter_events, this);
        }
    }
}

bool MediakeyManager::mediakeyStart()
{
    start(QThread::LowestPriority);
    return false;
}

bool MediakeyManager::mediakeyStop()
{
    return true;
}

MediakeyManager::MediakeyManager(QObject *parent) : QThread(parent)
{
    mExit = false;
}

bool MediakeyManager::grabMediaPlayerKeys()
{

}

bool MediakeyManager::releaseMediaPlayerKeys()
{

}

bool register_manager (MediakeyManager& mm)
{
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(UKUI_SETTINGS_DAEMON_DBUS_NAME)) {
        CT_SYSLOG(LOG_ERR, "ukui-settings-daemon is not running!");
        return false;
    }

    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerService(UKUI_SETTINGS_DAEMON_DBUS_NAME)) {
        CT_SYSLOG(LOG_ERR, "error getting session bus: '%s'", bus.lastError().message().toUtf8().data());
        return false;
    }

    if (!bus.registerObject(MEDIA_KEYS_DBUS_PATH, MEDIA_KEYS_DBUS_NAME,
                            (QObject*)&mm, QDBusConnection::ExportAllSlots|QDBusConnection::ExportAllSignals)) {
        CT_SYSLOG(LOG_ERR, "regist media key error: '%s'", bus.lastError().message().toUtf8().data());
        return false;
    }

    CT_SYSLOG(LOG_DEBUG, "regist media key successful!");

    return true;
}

GdkFilterReturn acme_filter_events (GdkXEvent* xevent, GdkEvent*, MediakeyManager* manager)
{
    int             i;
    XEvent          *xev = (XEvent *) xevent;
    XAnyEvent       *xany = (XAnyEvent *) xevent;

    /* verify we have a key event */
    if (xev->type != KeyPress && xev->type != KeyRelease) {
        return GDK_FILTER_CONTINUE;
    }

    for (i = 0; i < HANDLED_KEYS; i++) {
        if (match_key (keys[i].key, xev)) {
            switch (keys[i].key_type) {
            case VOLUME_DOWN_KEY:
            case VOLUME_UP_KEY:
                if (xev->type != KeyPress) {
                    return GDK_FILTER_CONTINUE;
                }
                break;
            default:
                if (xev->type != KeyRelease) {
                    return GDK_FILTER_CONTINUE;
                }
            }

            manager->mCurrentScreen = acme_get_screen_from_event (manager, xany);
            if (do_action (manager, keys[i].key_type) == FALSE) {
                return GDK_FILTER_REMOVE;
            } else {
                return GDK_FILTER_CONTINUE;
            }
        }
    }

    return GDK_FILTER_CONTINUE;
}

GdkScreen* acme_get_screen_from_event (MediakeyManager *manager, XAnyEvent* xanyev)
{
    GdkWindow *window;
    GdkScreen *screen;

    /* Look for which screen we're receiving events */
    for (QList<GdkScreen*>::ConstIterator l = manager->mScreens->constBegin();
         l != manager->mScreens->constEnd(); ++l) {
        screen = (GdkScreen *) *l;
        window = gdk_screen_get_root_window (screen);
        if (GDK_WINDOW_XID (window) == xanyev->window) {
            return screen;
        }
    }

    return nullptr;
}

gboolean do_action (MediakeyManager* manager, int type)
{
    char*       cmd;
    char*       path;

    switch (type) {
    case TOUCHPAD_KEY:
        do_touchpad_action (manager);
        break;
    case MUTE_KEY:
    case VOLUME_DOWN_KEY:
    case VOLUME_UP_KEY:
        break;
    case POWER_KEY:
        do_shutdown_action (manager);
        break;
    case LOGOUT_KEY:
        do_logout_action (manager);
        break;
    case EJECT_KEY:
        do_eject_action (manager);
        break;
    case HOME_KEY:
        path = g_shell_quote (g_get_home_dir ());
        cmd = g_strconcat ("caja --no-desktop ", path, NULL);
        g_free (path);
        execute (manager, cmd, FALSE, FALSE);
        g_free (cmd);
        break;
    case SEARCH_KEY:
        cmd = NULL;
        if ((cmd = g_find_program_in_path ("beagle-search"))) {
            execute (manager, "beagle-search", FALSE, FALSE);
        } else if ((cmd = g_find_program_in_path ("tracker-search-tool"))) {
            execute (manager, "tracker-search-tool", FALSE, FALSE);
        } else {
            execute (manager, "mate-search-tool", FALSE, FALSE);
        }
        g_free (cmd);
        break;
    case EMAIL_KEY:
        do_url_action (manager, "mailto");
        break;
    case SCREENSAVER_KEY:
        if ((cmd = g_find_program_in_path ("ukui-screensaver-command"))) {
            execute (manager, "ukui-screensaver-command --lock", FALSE, FALSE);
        } else {
            execute (manager, "xscreensaver-command -lock", FALSE, FALSE);
        }
        g_free (cmd);
        break;
    case SETTINGS_KEY:
        execute(manager, "ukui-control-center", FALSE, FALSE);
        break;
    case FILE_MANAGER_KEY:
        //execute(manager, "peony", FALSE, FALSE);
        system("peony");
        break;
    case HELP_KEY:
        do_url_action (manager, "help");
        break;
    case WWW_KEY:
        do_url_action (manager, "http");
        break;
    case MEDIA_KEY:
        do_media_action (manager);
        break;
    case CALCULATOR_KEY:
        if ((cmd = g_find_program_in_path ("galculator"))) {
            execute (manager, "galculator", FALSE, FALSE);
        } else if ((cmd = g_find_program_in_path ("ukui-calc"))) {
            execute (manager, "ukui-calc", FALSE, FALSE);
        } else {
            execute (manager, "gnome-calculator", FALSE, FALSE);
        }
        g_free (cmd);
        break;
    case PLAY_KEY:
        return do_multimedia_player_action (manager, "Play");
    case PAUSE_KEY:
        return do_multimedia_player_action (manager, "Pause");
    case STOP_KEY:
        return do_multimedia_player_action (manager, "Stop");
    case PREVIOUS_KEY:
        return do_multimedia_player_action (manager, "Previous");
    case NEXT_KEY:
        return do_multimedia_player_action (manager, "Next");
    case REWIND_KEY:
        return do_multimedia_player_action (manager, "Rewind");
    case FORWARD_KEY:
        return do_multimedia_player_action (manager, "FastForward");
    case REPEAT_KEY:
        return do_multimedia_player_action (manager, "Repeat");
    case RANDOM_KEY:
        return do_multimedia_player_action (manager, "Shuffle");
    case MAGNIFIER_KEY:
        do_magnifier_action (manager);
        break;
    case SCREENREADER_KEY:
        do_screenreader_action (manager);
        break;
    case ON_SCREEN_KEYBOARD_KEY:
        do_on_screen_keyboard_action (manager);
        break;
    case TERMINAL_KEY:
        do_terminal_action (manager);
        break;
    case SCREENSHOT_KEY:
        do_screenshot_action (manager);
        break;
    case AREA_SCREENSHOT_KEY:
        do_area_screenshot_action (manager);
        break;
    case WINDOW_SCREENSHOT_KEY:
        do_window_screenshot_action (manager);
        break;
    default:
        g_assert_not_reached ();
    }

    return FALSE;
}

// FIXME:// windows
void do_touchpad_action (MediakeyManager* manager)
{
    GSettings *settings = g_settings_new (TOUCHPAD_SCHEMA);
    gboolean state = g_settings_get_boolean (settings, TOUCHPAD_ENABLED_KEY);

    if (touchpad_is_present () == FALSE) {
        dialog_init (manager);
        usd_media_keys_window_set_action_custom (manager->mDialog, "touchpad-disabled", FALSE);
        return;
    }

    dialog_init (manager);
    usd_media_keys_window_set_action_custom (manager->mDialog,
                      (!state) ? "touchpad-enabled" : "touchpad-disabled", FALSE);
    dialog_show (manager);

    g_settings_set_boolean (settings, TOUCHPAD_ENABLED_KEY, !state);
    g_object_unref (settings);
}

void do_shutdown_action (MediakeyManager* manager)
{
    execute (manager, "ukui-session-tools", FALSE, FALSE);
}

void do_logout_action (MediakeyManager* manager)
{
    execute (manager, "ukui-session-tools", FALSE, FALSE);
}

void do_eject_action (MediakeyManager* manager)
{
    GList *drives, *l;
    GDrive *fav_drive;
    guint score;

    /* Find the best drive to eject */
    fav_drive = NULL;
    score = NO_SCORE;
    drives = g_volume_monitor_get_connected_drives (manager->mVolumeMonitor);
    for (l = drives; l != NULL; l = l->next) {
        GDrive *drive = (GDrive*) l->data;

        if (g_drive_can_eject (drive) == FALSE) continue;
        if (g_drive_is_media_removable (drive) == FALSE) continue;
        if (score < SCORE_CAN_EJECT) {
            fav_drive = drive;
            score = SCORE_CAN_EJECT;
        }
        if (g_drive_has_media (drive) == FALSE) continue;
        if (score < SCORE_HAS_MEDIA) {
            fav_drive = drive;
            score = SCORE_HAS_MEDIA;
            break;
        }
    }

    /* Show the dialogue */
    dialog_init (manager);
    usd_media_keys_window_set_action_custom (USD_MEDIA_KEYS_WINDOW (manager->mDialog), "media-eject", FALSE);
    dialog_show (manager);

    /* Clean up the drive selection and exit if no suitable drives are found */
    if (fav_drive != NULL) fav_drive = (GDrive*)g_object_ref (fav_drive);

    g_list_foreach (drives, (GFunc) g_object_unref, NULL);
    if (fav_drive == NULL) return;

    /* Eject! */
    g_drive_eject_with_operation (fav_drive, G_MOUNT_UNMOUNT_FORCE, NULL, NULL,
                                  (GAsyncReadyCallback) do_eject_action_cb, manager);
    g_object_unref (fav_drive);
}

void execute (MediakeyManager* manager, const char* cmd, gboolean sync, gboolean need_term)
{
    int                 argc;
    char                *exec;
    char                **argv;
    char                *term = NULL;
    gboolean            retval;

    retval = FALSE;

    if (need_term) {
        term = get_term_command (manager);
        if (term == NULL) {
            CT_SYSLOG(LOG_ERR, ("Could not get default terminal. Verify that your default \
                    terminal command is set and points to a valid application."));
            return;
        }
    }

    if (term) {
        exec = g_strdup_printf ("%s %s", term, cmd);
        g_free (term);
    } else {
        exec = g_strdup (cmd);
    }

    if (g_shell_parse_argv (exec, &argc, &argv, NULL)) {
        if (sync != FALSE) {
            retval = g_spawn_sync (g_get_home_dir (), argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL, NULL, NULL);
        } else {
            retval = g_spawn_async (g_get_home_dir (), argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL);
        }
        g_strfreev (argv);
    }

    if (retval == FALSE) {
        CT_SYSLOG(LOG_ERR, "Couldn't execute command: %s\n Verify that this is a valid command.", exec);
    }
    g_free (exec);
}

void do_url_action (MediakeyManager*, const gchar* scheme)
{
    GError *error = NULL;
    GAppInfo *app_info;

    app_info = g_app_info_get_default_for_uri_scheme (scheme);

    if (app_info != NULL) {
        if (!g_app_info_launch (app_info, NULL, NULL, &error)) {
            CT_SYSLOG(LOG_WARNING, "Could not launch '%s': %s", g_app_info_get_commandline (app_info), error->message);
            g_object_unref (app_info);
            g_error_free (error);
        }
    } else {
        CT_SYSLOG(LOG_WARNING, "Could not find default application for '%s' scheme", scheme);
    }
}

void do_media_action (MediakeyManager*)
{
    GError *error = NULL;
    GAppInfo *app_info;

    app_info = g_app_info_get_default_for_type ("audio/x-vorbis+ogg", FALSE);

    if (app_info != NULL) {
        if (!g_app_info_launch (app_info, NULL, NULL, &error)) {
            CT_SYSLOG(LOG_WARNING, "Could not launch '%s': %s", g_app_info_get_commandline (app_info), error->message);
            g_error_free (error);
        }
    } else {
        CT_SYSLOG(LOG_WARNING, "Could not find default application for '%s' mime-type", "audio/x-vorbis+ogg");
    }
}

gboolean do_multimedia_player_action (MediakeyManager* manager, const char* key)
{
    return usd_media_player_key_pressed (manager, key);
}

gboolean usd_media_player_key_pressed (MediakeyManager* manager, const char*)
{
    const char *application = NULL;
    gboolean    have_listeners;

    have_listeners = (manager->mMediaPlayers != NULL);
    if (have_listeners) {
        application = ((MediaPlayer *)manager->mMediaPlayers->first())->application;
    }
    //FIXME://
    //g_signal_emit (manager, signals[MEDIA_PLAYER_KEY_PRESSED], 0, application, key);

    return !have_listeners;
}

void do_magnifier_action (MediakeyManager*)
{
    do_toggle_accessibility_key ("screen-magnifier-enabled");
}

void do_screenreader_action (MediakeyManager*)
{
    do_toggle_accessibility_key ("screen-reader-enabled");
}

void do_on_screen_keyboard_action (MediakeyManager*)
{
    do_toggle_accessibility_key ("screen-keyboard-enabled");
}

void do_terminal_action (MediakeyManager* manager)
{
    execute (manager, "mate-terminal",FALSE,FALSE);
}

void do_screenshot_action (MediakeyManager* manager)
{
    execute (manager, "gnome-screenshot",FALSE,FALSE);
}

void do_area_screenshot_action (MediakeyManager* manager)
{
    execute (manager, "gnome-screenshot -a",FALSE,FALSE);
}

void do_eject_action_cb (GDrive* drive, GAsyncResult* res, MediakeyManager*)
{
    g_drive_eject_with_operation_finish (drive, res, NULL);
}

void do_window_screenshot_action (MediakeyManager* manager)
{
    execute (manager, "gnome-screenshot -w",FALSE,FALSE);
}

char* get_term_command (MediakeyManager*)
{
    char            *cmd_term, *cmd_args;
    char            *cmd = NULL;
    GSettings       *settings;

    settings = g_settings_new ("org.mate.applications-terminal");
    cmd_term = g_settings_get_string (settings, "exec");
    cmd_args = g_settings_get_string (settings, "exec-arg");

    if (cmd_term[0] != '\0') {
        cmd = g_strdup_printf ("%s %s -e", cmd_term, cmd_args);
    } else {
        cmd = g_strdup_printf ("ukui-terminal -e");
    }

    g_free (cmd_args);
    g_free (cmd_term);
    g_object_unref (settings);

    return cmd;
}

void do_toggle_accessibility_key (const char *key)
{
    GSettings*          settings;
    gboolean            state;

    settings = g_settings_new ("org.gnome.desktop.a11y.applications");
    state = g_settings_get_boolean (settings, key);
    g_settings_set_boolean (settings, key, !state);
    g_object_unref (settings);
}

gboolean touchpad_is_present (void)
{
    XDeviceInfo *device_info;
    gint n_devices;
    gint i;
    gboolean retval;

    if (supports_xinput_devices () == FALSE) return TRUE;

    retval = FALSE;

    device_info = XListInputDevices (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()), &n_devices);
    if (device_info == NULL) return FALSE;

    for (i = 0; i < n_devices; i++) {
        XDevice *device;
        device = device_is_touchpad (&device_info[i]);
        if (device != NULL) {
            retval = TRUE;
            break;
        }
    }

    if (device_info != NULL) XFreeDeviceList (device_info);

    return retval;
}

gboolean supports_xinput_devices (void)
{
    gint op_code, event, error;

    return XQueryExtension (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()),
                            "XInputExtension", &op_code, &event, &error);
}

XDevice* device_is_touchpad (XDeviceInfo *deviceinfo)
{
    XDevice *device;

    if (deviceinfo->type != XInternAtom (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()), XI_TOUCHPAD, True))
        return NULL;

    gdk_error_trap_push ();
    device = XOpenDevice (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()), deviceinfo->id);
    if (gdk_error_trap_pop () || (device == NULL))
        return NULL;

    if (device_has_property (device, "libinput Tapping Enabled") || device_has_property (device, "Synaptics Off")) {
        return device;
    }

    XCloseDevice (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()), device);
    return NULL;
}

gboolean device_has_property (XDevice* device, const char* property_name)
{
    Atom realtype, prop;
    int realformat;
    unsigned long nitems, bytes_after;
    unsigned char *data;

    prop = XInternAtom (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()), property_name, True);
    if (!prop) return FALSE;

    gdk_error_trap_push ();
    if ((XGetDeviceProperty (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()), device, prop, 0, 1, False,
                             XA_INTEGER, &realtype, &realformat, &nitems, &bytes_after, &data) == Success)
            && (realtype != None)) {
        gdk_error_trap_pop_ignored ();
        XFree (data);
        return TRUE;
    }

    gdk_error_trap_pop_ignored ();
    return FALSE;
}
