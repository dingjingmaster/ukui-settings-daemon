#include "mediakey-plugin.h"
#include "clib-syslog.h"

MediakeyManager* MediakeyPlugin::mManager = nullptr;
PluginInterface* MediakeyPlugin::mInstance = nullptr;

MediakeyPlugin::MediakeyPlugin()
{
    CT_SYSLOG(LOG_DEBUG, "mediakey plugin init...");
}

MediakeyPlugin::~MediakeyPlugin()
{

}

PluginInterface *MediakeyPlugin::getInstance()
{
    bool flag = true;
    if (nullptr == mInstance) {
        mInstance = new MediakeyPlugin();
        if (nullptr != mInstance) {
            mManager = MediakeyManager::mediakeyNew();
            if (nullptr == mManager) {
                flag = false;
                CT_SYSLOG(LOG_ERR, "new media-key manager failed!");
            }
        } else {
            flag = false;
            CT_SYSLOG(LOG_ERR, "new media-key plugin failed!");
        }
    }

    if (!flag) {
        if (nullptr != mManager) {delete mManager; mManager = nullptr;}
        if (nullptr != mInstance) {delete mInstance; mInstance = nullptr;}
    }

    return mInstance;
}

void MediakeyPlugin::activate()
{
    CT_SYSLOG(LOG_DEBUG, "activating mediakey plugin ...");
//    if (!)
}

void MediakeyPlugin::deactivate()
{
    CT_SYSLOG(LOG_DEBUG, "deactivating mediakey plugin ...");
}

PluginInterface* createSettingsPlugin()
{
    return MediakeyPlugin::getInstance();
}
