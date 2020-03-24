/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2008 Michael J. Chudobiak <mjc@avtechpulse.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"

#include <glib/gi18n-lib.h>
#include <gmodule.h>

#include "ukui-settings-plugin.h"
#include "usd-housekeeping-plugin.h"
#include "usd-housekeeping-manager.h"

struct UsdHousekeepingPluginPrivate {
        UsdHousekeepingManager *manager;
};

#define USD_HOUSEKEEPING_PLUGIN_GET_PRIVATE(object) (G_TYPE_INSTANCE_GET_PRIVATE ((object), USD_TYPE_HOUSEKEEPING_PLUGIN, UsdHousekeepingPluginPrivate))

UKUI_SETTINGS_PLUGIN_REGISTER (UsdHousekeepingPlugin, usd_housekeeping_plugin)

static void
usd_housekeeping_plugin_init (UsdHousekeepingPlugin *plugin)
{
        plugin->priv = USD_HOUSEKEEPING_PLUGIN_GET_PRIVATE (plugin);

        g_debug ("UsdHousekeepingPlugin initializing");

        plugin->priv->manager = usd_housekeeping_manager_new ();
}

static void
usd_housekeeping_plugin_finalize (GObject *object)
{
        UsdHousekeepingPlugin *plugin;

        g_return_if_fail (object != NULL);
        g_return_if_fail (USD_IS_HOUSEKEEPING_PLUGIN (object));

        g_debug ("UsdHousekeepingPlugin finalizing");

        plugin = USD_HOUSEKEEPING_PLUGIN (object);

        g_return_if_fail (plugin->priv != NULL);

        if (plugin->priv->manager != NULL) {
                g_object_unref (plugin->priv->manager);
        }

        G_OBJECT_CLASS (usd_housekeeping_plugin_parent_class)->finalize (object);
}

static void
impl_activate (UkuiSettingsPlugin *plugin)
{
        gboolean res;
        GError  *error;

        g_debug ("Activating housekeeping plugin");

        error = NULL;
        res = usd_housekeeping_manager_start (USD_HOUSEKEEPING_PLUGIN (plugin)->priv->manager, &error);
        if (! res) {
                g_warning ("Unable to start housekeeping manager: %s", error->message);
                g_error_free (error);
        }
}

static void
impl_deactivate (UkuiSettingsPlugin *plugin)
{
        g_debug ("Deactivating housekeeping plugin");
        usd_housekeeping_manager_stop (USD_HOUSEKEEPING_PLUGIN (plugin)->priv->manager);
}

static void
usd_housekeeping_plugin_class_init (UsdHousekeepingPluginClass *klass)
{
        GObjectClass             *object_class = G_OBJECT_CLASS (klass);
        UkuiSettingsPluginClass *plugin_class = UKUI_SETTINGS_PLUGIN_CLASS (klass);

        object_class->finalize = usd_housekeeping_plugin_finalize;

        plugin_class->activate = impl_activate;
        plugin_class->deactivate = impl_deactivate;

        g_type_class_add_private (klass, sizeof (UsdHousekeepingPluginPrivate));
}

static void
usd_housekeeping_plugin_class_finalize (UsdHousekeepingPluginClass *klass)
{
}

