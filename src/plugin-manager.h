/*
 * Provman
 *
 * Copyright (C) 2011 Intel Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 *
 * Mark Ryan <mark.d.ryan@intel.com>
 *
 */

/*!
 * @file plugin-manager.h
 *
 * @brief contains functions for managing plugins
 *
 *****************************************************************************/

#ifndef PROVMAN_PLUGIN_MANAGER_H
#define PROVMAN_PLUGIN_MANAGER_H

#include <stdbool.h>

#include <glib.h>

typedef struct plugin_manager_t_ plugin_manager_t;

typedef void (*plugin_manager_cb_t)(int result, void *user_data);

int plugin_manager_new(plugin_manager_t **manager, bool system);
int plugin_manager_sync_in(plugin_manager_t *manager, const char *imsi,
			   plugin_manager_cb_t callback, void *user_data);
int plugin_manager_sync_out(plugin_manager_t *manager,
			    plugin_manager_cb_t callback, void *user_data);
bool plugin_manager_cancel(plugin_manager_t *manager);
int plugin_manager_get(plugin_manager_t* manager, const gchar* key,
		       gchar** value);
int plugin_manager_get_multiple(plugin_manager_t* manager, GVariant* keys,
				GVariant **settings);
int plugin_manager_get_all(plugin_manager_t* manager, const gchar* key,
			   GVariant** values);
int plugin_manager_get_all_meta(plugin_manager_t* manager, const gchar* key,
				GVariant** values);
int plugin_manager_set(plugin_manager_t* manager, const gchar* key,
		       const gchar* value);
int plugin_manager_set_multiple(plugin_manager_t* manager, GVariant* settings,
				GVariant** errors);
int plugin_manager_set_multiple_meta(plugin_manager_t* manager,
				     GVariant* settings, GVariant** errors);
int plugin_manager_remove(plugin_manager_t* manager, const gchar* key);
int plugin_manager_remove_multiple(plugin_manager_t* manager, GVariant* keys,
				   GVariant **errors);
void plugin_manager_delete(plugin_manager_t *manager);
int plugin_manager_abort(plugin_manager_t *manager);
int plugin_manager_get_children_type_info(plugin_manager_t* manager,
					  const gchar* search_key,
					  GVariant** values);
int plugin_manager_get_type_info(plugin_manager_t* manager,
				 const gchar* search_key, gchar **type_info);
bool plugin_manager_busy(plugin_manager_t *manager);
int plugin_manager_get_meta(plugin_manager_t* manager, const gchar* key,
			    const gchar *prop, gchar** value);
int plugin_manager_set_meta(plugin_manager_t* manager, const gchar* key,
			    const gchar *prop, const gchar* value);
#endif
