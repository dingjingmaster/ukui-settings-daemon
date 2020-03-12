#include "plugin-manager.h"
#include "plugin-info.h"
//#include "plugin-manager-adaptor.h"

#include "global.h"
#include "clib-syslog.h"

#include <glib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gio/gio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <glib/gstdio.h>
#include <dbus/dbus-glib.h>

#include <QDebug>
#include <QDBusError>

QList<PluginInfo*>* PluginManager::mPlugin = NULL;
PluginManager* PluginManager::mPluginManager = NULL;

static bool is_schema (QString& schema);
static bool register_manager(PluginManager& pm);

PluginManager::PluginManager()
{
    mPlugin = new QList<PluginInfo*>();
}

PluginManager::~PluginManager()
{
    delete[] mPlugin;

}

// DD-OK!
PluginManager* PluginManager::getInstance()
{
    if (nullptr == mPluginManager) {
        CT_SYSLOG(LOG_DEBUG, "ukui settings manager will be created!")
        mPluginManager = new PluginManager;
        if (!register_manager(*mPluginManager)) {
            delete mPluginManager;
            return nullptr;
        }
    }

    return mPluginManager;
}

bool PluginManager::managerStart()
{
    CT_SYSLOG(LOG_DEBUG, "Starting settings manager");

    loadAll();

    return true;
}

void PluginManager::managerStop()
{
    CT_SYSLOG(LOG_DEBUG, "Stopping settings manager");
     unloadAll();
}

bool PluginManager::managerAwake()
{
    CT_SYSLOG(LOG_DEBUG, "Awake called")
    return managerStart();
}

void PluginManager::onPluginActivated(QString &name)
{
    CT_SYSLOG(LOG_DEBUG, "emitting plugin-activated '%s'", name.toUtf8().data());
    emit pluginActivated(name);
}

void PluginManager::onPluginDeactivated(QString &name)
{
    CT_SYSLOG(LOG_DEBUG, "emitting plugin-deactivated '%s'", name.toUtf8().data());
    emit pluginDeactivated(name);
}

void PluginManager::loadAll()
{
    PluginInfo* info = NULL;

    QString p(UKUI_SETTINGS_PLUGINDIR);
    loadDir (p);

    // FIXME:// sort plugin

    CT_SYSLOG(LOG_DEBUG, "Now Activity plugins ...");
    for (int i = 0; i < mPlugin->size(); ++i) {
        info = mPlugin->at(i);
        CT_SYSLOG(LOG_DEBUG, "activity plugin: %s", info->getPluginName().toUtf8().data());
        info->pluginActivate();
    }
}

void PluginManager::loadDir(QString &path)
{
    GError*     error = NULL;
    GDir*       dir = NULL;
    const char* name = NULL;

    CT_SYSLOG(LOG_DEBUG, "Loading settings plugins from dir: %s", (char*)path.toLatin1().data());

    dir = g_dir_open ((char*)path.toLatin1().data(), 0, &error);
    if (NULL == dir) {
        CT_SYSLOG(LOG_ERR, "%s", error->message);
        g_error_free(error);
        return;
    }

    while ((name = g_dir_read_name(dir))) {
        char* filename = NULL;
        if (!g_str_has_suffix(name, PLUGIN_EXT)) {
            continue;
        }
        filename = g_build_filename((char*)path.toLatin1().data(), name, NULL);
        if (g_file_test(filename, G_FILE_TEST_IS_REGULAR)) {
            QString ftmp(filename);
            loadFile(ftmp);
        }
        g_free(filename);
    }
    g_dir_close(dir);
}

void PluginManager::loadFile(QString &fileName)
{
    PluginInfo* info = NULL;
    QString                 schema;

    CT_SYSLOG(LOG_DEBUG, "Loading plugin: %s", fileName.toLatin1().data());

    // FIXME://
    info = new PluginInfo(fileName);
    if (info == NULL) {
        goto out;
    }

    if (mPlugin->contains(info)) {
        CT_SYSLOG(LOG_DEBUG, "The list has contain this plugin, '%s'", fileName.toLatin1().data());
        goto out;
    }

    // check plugin's schema
    schema = QString("%1.plugins.%2").arg(DEFAULT_SETTINGS_PREFIX).arg(info->getPluginLocation().toUtf8().data());
    if (is_schema (schema)) {
        CT_SYSLOG(LOG_DEBUG, "right schema '%s'", schema.toUtf8().data());
       QObject::connect(info, SIGNAL(activated), this, SLOT(onPluginActivated));
       QObject::connect(info, SIGNAL(deactivated), this, SLOT(onPluginDeactivated));
       info->setPluginSchema(schema);
       mPlugin->insert(0, info);
    } else {
        CT_SYSLOG(LOG_ERR, "Ignoring unknown schema '%s'", schema.toUtf8().data());
    }

    return;

out:
    if (info != NULL) {
        delete info;
    }
}

void PluginManager::unloadAll()
{
    while (!mPlugin->isEmpty()) delete mPlugin->takeFirst();
}

static bool is_item_in_schema (const char* const* items, QString& item)
{
    while (*items) {
       if (g_strcmp0 (*items++, item.toLatin1().data()) == 0) return true;
    }
    return false;
}

bool is_schema (QString& schema)
{
    return is_item_in_schema (g_settings_list_schemas(), schema);
}

static bool register_manager(PluginManager& pm)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerService(UKUI_SETTINGS_DAEMON_DBUS_NAME)) {
        CT_SYSLOG(LOG_ERR, "error getting system bus: '%s'", bus.lastError().message().toUtf8().data());
        return false;
    }

    if (!bus.registerObject(UKUI_SETTINGS_DAEMON_DBUS_PATH, (QObject*)&pm, QDBusConnection::ExportAllContents)) {
        CT_SYSLOG(LOG_ERR, "regist settings manager error: '%s'", bus.lastError().message().toUtf8().data());
        return false;
    }

    CT_SYSLOG(LOG_DEBUG, "regist settings manager successful!");

    return true;
}
