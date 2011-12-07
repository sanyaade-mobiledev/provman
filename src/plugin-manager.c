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
 * @file plugin-manager.c
 *
 * @brief contains functions for managing plugins
 *
 *****************************************************************************/

#include "config.h"

#include <string.h>
#include <glib.h>

#include "error.h"
#include "log.h"
#include "schema.h"

#include "plugin-manager.h"
#include "plugin.h"
#include "cache.h"
#include "utils.h"
#include "meta-data.h"

enum plugin_manager_state_t_ {
	PLUGIN_MANAGER_STATE_IDLE,
	PLUGIN_MANAGER_STATE_SYNC_IN,
	PLUGIN_MANAGER_STATE_SYNC_OUT,
};
typedef enum plugin_manager_state_t_ plugin_manager_state_t;

#define PLUGIN_MANAGER_TYPE_STRING "string"
#define PLUGIN_MANAGER_TYPE_INT "int"
#define PLUGIN_MANAGER_TYPE_DIR "dir"
#define PLUGIN_MANAGER_TYPE_ENUM "enum"

#define PLUGIN_MANAGER_UNNAMED_DIR "<X>"

#define PROVMAN_META_DATA_NAME "metadata.ini"

enum plugin_manager_cmd_type_t_ {
	PLUGIN_MANAGER_CMD_TYPE_VOID,
	PLUGIN_MANAGER_CMD_TYPE_VALUE,
	PLUGIN_MANAGER_CMD_TYPE_VARIANT
};

typedef enum plugin_manager_cmd_type_t_ plugin_manager_cmd_type_t;

typedef struct plugin_manager_cmd_t_ plugin_manager_cmd_t;
struct plugin_manager_cmd_t_ {
	plugin_manager_cmd_type_t type;
	union {
		plugin_manager_cb_t cb_void;
		plugin_manager_cb_value_t cb_value;
		plugin_manager_cb_variant_t cb_variant;
	};

	gchar *key;
	gchar *prop;
	gchar *value;
	GVariant *keys;
	GArray *indicies;

	unsigned int current_index;
	plugin_manager_cb_t sync_finished;
	void *user_data;
	int err;
	gchar *ret_value;
	GVariant *ret_variant;
};

struct plugin_manager_t_ {
	bool system;
	plugin_manager_state_t state;
	provman_plugin_instance *plugin_instances;
	provman_schema_t **plugin_schemas;
	GHashTable **plugin_meta_data;
	provman_cache_t *cache;
	bool *plugin_synced;
	unsigned int synced;
	guint completion_source;
	gchar *imsi;
	plugin_manager_cmd_t cb;
	plugin_manager_cb_t sync_in_cb;
};

static void prv_sync_out_next_plugin(plugin_manager_t *manager);
static bool prv_sync_plugins(plugin_manager_t *manager);

static void prv_plugin_manager_cmd_free(plugin_manager_cmd_t *cmd)
{
	if (cmd->key)
		g_free(cmd->key);

	if (cmd->prop)
		g_free(cmd->prop);

	if (cmd->value)
		g_free(cmd->value);

	if (cmd->keys)
		g_variant_unref(cmd->keys);

	if (cmd->indicies)
		(void) g_array_free(cmd->indicies, TRUE);
}

static void prv_free_meta_data(gpointer md)
{
	if (md)
		provman_meta_data_delete(md);
}

int plugin_manager_new(plugin_manager_t **manager, bool system)
{
	int err = PROVMAN_ERR_NONE;

	unsigned int count = provman_plugin_get_count();
	unsigned int i;
	const provman_plugin *plugin;
	plugin_manager_t *retval = g_new0(plugin_manager_t, 1);

	PROVMAN_LOGF("%s called system %d", __FUNCTION__, system);

	err = provman_plugin_check();
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	retval->system = system;
	retval->state = PLUGIN_MANAGER_STATE_IDLE;
	retval->plugin_instances = g_new0(provman_plugin_instance, count);
	retval->plugin_schemas = g_new0(provman_schema_t*, count);
	retval->plugin_meta_data = g_new0(GHashTable*, count);

	for (i = 0; i < count; ++i) {
		plugin = provman_plugin_get(i);

		err = provman_schema_new(plugin->schema, strlen(plugin->schema),
					 &retval->plugin_schemas[i]);
		if (err != PROVMAN_ERR_NONE) {
			PROVMAN_LOGF("Unable to instantiate schema for plugin"
				     " %s", plugin->name);
			goto on_error;
		}

		err = plugin->new_fn(&retval->plugin_instances[i], system);
		if (err != PROVMAN_ERR_NONE) {
			PROVMAN_LOGF("Unable to instantiate plugin %s",
				      plugin->name);
			goto on_error;
		}

		retval->plugin_meta_data[i] =
			g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
					      prv_free_meta_data);
	}

	provman_cache_new(&retval->cache);
	retval->plugin_synced = g_new0(bool, count);
	*manager = retval;

	return err;

on_error:

	PROVMAN_LOGF("%s failed %d", __FUNCTION__, err);

	plugin_manager_delete(retval);

	return err;
}

static void prv_clear_cache(plugin_manager_t *manager)
{
	unsigned int i;
	unsigned int count = provman_plugin_get_count();

	for (i = 0; i < count; ++i)
		manager->plugin_synced[i] = false;

	(void) provman_cache_remove(manager->cache, "/");
}

void plugin_manager_delete(plugin_manager_t *manager)
{
	unsigned int count;
	unsigned int i;
	const provman_plugin *plugin;

	PROVMAN_LOGF("%s called", __FUNCTION__);

	if (manager) {
		count = provman_plugin_get_count();
		for (i = 0; i < count; ++i) {
			provman_schema_delete(manager->plugin_schemas[i]);
			plugin = provman_plugin_get(i);
			plugin->delete_fn(manager->plugin_instances[i]);
			if (manager->plugin_meta_data[i])
				g_hash_table_unref(
					manager->plugin_meta_data[i]);
		}
		g_free(manager->plugin_meta_data);
		g_free(manager->plugin_schemas);
		g_free(manager->plugin_instances);
		provman_cache_delete(manager->cache);
		g_free(manager->plugin_synced);
		g_free(manager->imsi);
		g_free(manager);
	}
}

static gboolean prv_complete_callback(gpointer user_data)
{
	plugin_manager_t *manager = user_data;
	plugin_manager_cmd_t *cmd = &manager->cb;

	/* Ownership of ret_value or ret_variant is transferred to calback */

	switch (cmd->type) {
	case PLUGIN_MANAGER_CMD_TYPE_VOID:
		cmd->cb_void(cmd->err, cmd->user_data);
		break;
	case PLUGIN_MANAGER_CMD_TYPE_VALUE:
		cmd->cb_value(cmd->err, cmd->ret_value, cmd->user_data);
		cmd->ret_value = NULL;
		break;
	case PLUGIN_MANAGER_CMD_TYPE_VARIANT:
		cmd->cb_variant(cmd->err, cmd->ret_variant, cmd->user_data);
		cmd->ret_variant = NULL;
		break;
	}
	prv_plugin_manager_cmd_free(cmd);
	manager->completion_source = 0;
	manager->state = PLUGIN_MANAGER_STATE_IDLE;

	return FALSE;
}

static void prv_schedule_completion(plugin_manager_t *manager, int err)
{
	if (!manager->completion_source) {
		manager->cb.err = err;
		manager->completion_source =
			g_idle_add(prv_complete_callback, manager);
	}
}

static provman_meta_data_t* prv_get_plugin_md(plugin_manager_t *manager,
					      unsigned int pindex)
{
	const provman_plugin *plugin;
	const gchar *imsi;
	provman_meta_data_t* md = NULL;
	gchar *meta_data_path = NULL;
	GString *fname = NULL;
	GHashTable *ht = manager->plugin_meta_data[pindex];

	plugin = provman_plugin_get(pindex);
	imsi = plugin->sim_id_fn ?
		plugin->sim_id_fn(manager->plugin_instances[pindex]) :
		"";
	md = g_hash_table_lookup(ht, imsi);
	if (!md) {
		fname = g_string_new(plugin->name);
		if (imsi[0]) {
			g_string_append_c(fname, '-');
			g_string_append(fname, imsi);
		}
		g_string_append_c(fname, '-');
		g_string_append(fname, PROVMAN_META_DATA_NAME);
		if (provman_utils_make_file_path(fname->str, manager->system,
						 &meta_data_path) !=
		    PROVMAN_ERR_NONE)
			goto on_error;

		PROVMAN_LOGF("Loading meta data file %s", meta_data_path);

		provman_meta_data_new(meta_data_path, &md);
		g_hash_table_insert(ht, g_strdup(imsi), md);
	}

on_error:

	if (fname)
		g_string_free(fname, TRUE);
	g_free(meta_data_path);

	return md;
}

static void prv_plugin_sync_cb(int err, GHashTable *settings, void *user_data)
{
	plugin_manager_t *manager = user_data;
	provman_meta_data_t* md;
	GHashTable *ht;

	PROVMAN_LOGF("Plugin %s sync_in completed with error %d",
		      provman_plugin_get(manager->synced)->name, err);

	if (err == PROVMAN_ERR_NONE) {
		provman_cache_add_settings(manager->cache, settings);

		md = prv_get_plugin_md(manager, manager->synced);
		if (md) {
			ht = provman_meta_data_get_all(md);
			provman_cache_add_meta_data(manager->cache, ht);
			g_hash_table_unref(ht);
		}
		manager->plugin_synced[manager->synced] = true;
	}

	manager->sync_in_cb(err, manager);
}

static int prv_sync_plugin(plugin_manager_t *manager, unsigned int pindex,
			   plugin_manager_cb_t callback)
{
	int err;
	const provman_plugin *plugin;
	const char *imsi = (const char*) manager->imsi;

	plugin = provman_plugin_get(pindex);
	err = plugin->sync_in_fn(manager->plugin_instances[pindex],
				 imsi, prv_plugin_sync_cb, manager);

	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	manager->synced = pindex;
	manager->sync_in_cb = callback;
	manager->state = PLUGIN_MANAGER_STATE_SYNC_IN;

	return PROVMAN_ERR_NONE;

on_error:

	PROVMAN_LOGF("Unable to instantiate plugin %s", plugin->name);

	return err;
}

int plugin_manager_sync_in(plugin_manager_t *manager, const char *imsi)
{
	int err = PROVMAN_ERR_NONE;

	PROVMAN_LOGF("%s called with imsi %s", __FUNCTION__, imsi);

	if (manager->state != PLUGIN_MANAGER_STATE_IDLE) {
		err = PROVMAN_ERR_DENIED;
		goto on_error;
	}

	manager->imsi = g_strdup(imsi);

on_error:

	PROVMAN_LOGF("%s exit with err %d", __FUNCTION__, err);

	return err;
}

static void prv_sync_in_cancel(plugin_manager_t *manager)
{
	const provman_plugin *plugin = provman_plugin_get(manager->synced);

	PROVMAN_LOGF("%s called ", __FUNCTION__);
	PROVMAN_LOGF("Cancelling %s ", plugin->root);

	plugin->sync_in_cancel_fn(manager->plugin_instances[manager->synced]);
}

static void prv_plugin_sync_out_cb(int err, void *user_data)
{
	plugin_manager_t *manager = user_data;

	PROVMAN_LOGF("Plugin %s sync_out completed with error %d",
		 provman_plugin_get(manager->synced)->name, err);

	if (err == PROVMAN_ERR_CANCELLED) {
		prv_clear_cache(manager);
		g_free(manager->imsi);
		manager->imsi = NULL;
		prv_schedule_completion(manager, err);
	} else {
		++manager->synced;
		prv_sync_out_next_plugin(manager);
	}
}

static void prv_sync_out_next_plugin(plugin_manager_t *manager)
{
	const provman_plugin *plugin;
	unsigned int count = provman_plugin_get_count();
	int err;
	GHashTable *settings;
	GHashTable *ht;
	provman_meta_data_t* md;

	while (manager->synced < count) {
		plugin = provman_plugin_get(manager->synced);
		if (manager->plugin_synced[manager->synced]) {
			settings = provman_cache_get_settings(manager->cache,
							      plugin->root);
			err = plugin->sync_out_fn(
				manager->plugin_instances[manager->synced],
				settings, prv_plugin_sync_out_cb,
				manager);
			md = prv_get_plugin_md(manager, manager->synced);
			if (md) {
				ht = provman_cache_get_meta_data(manager->cache,
								 plugin->root);
				provman_meta_data_update(md, ht);
				g_hash_table_unref(ht);
			}
			g_hash_table_unref(settings);

			if (err == PROVMAN_ERR_NONE)
				break;
		}

		PROVMAN_LOGF("No sync out for plugin %s", plugin->name);

		++manager->synced;
	}

	if (manager->synced == count) {
		prv_clear_cache(manager);
		prv_schedule_completion(manager, PROVMAN_ERR_NONE);
		g_free(manager->imsi);
		manager->imsi = NULL;
	}
}

int plugin_manager_sync_out(plugin_manager_t *manager,
			    plugin_manager_cb_t callback, void *user_data)
{
	int err = PROVMAN_ERR_NONE;
	plugin_manager_cmd_t *cmd = &manager->cb;

	PROVMAN_LOGF("%s called", __FUNCTION__);

	if (manager->state != PLUGIN_MANAGER_STATE_IDLE) {
		err = PROVMAN_ERR_DENIED;
		goto on_error;
	}

	memset(cmd, 0, sizeof(*cmd));

	manager->synced = 0;
	manager->state = PLUGIN_MANAGER_STATE_SYNC_OUT;

	cmd->type = PLUGIN_MANAGER_CMD_TYPE_VOID;
	cmd->cb_void = callback;
	cmd->user_data = user_data;

	prv_sync_out_next_plugin(manager);

on_error:

	PROVMAN_LOGF("%s exit with err %d", __FUNCTION__, err);

	return err;
}

static void prv_sync_out_cancel(plugin_manager_t *manager)
{
	const provman_plugin *plugin;
	unsigned int count;

	PROVMAN_LOGF("%s called ", __FUNCTION__);

	count = provman_plugin_get_count();
	if (manager->synced < count) {
		plugin = provman_plugin_get(manager->synced);
		PROVMAN_LOGF("Cancelling %s ", plugin->root);
		plugin->sync_out_cancel_fn(
			manager->plugin_instances[manager->synced]);
	}
}

bool plugin_manager_cancel(plugin_manager_t *manager)
{
	bool retval = false;

	PROVMAN_LOGF("%s called", __FUNCTION__);

	if (!manager->completion_source) {
		if (manager->state == PLUGIN_MANAGER_STATE_SYNC_IN) {
			prv_sync_in_cancel(manager);
			retval = true;
		} else if (manager->state == PLUGIN_MANAGER_STATE_SYNC_OUT) {
			prv_sync_out_cancel(manager);
			retval = true;
		}
	} else {
		retval = true;
	}

	PROVMAN_LOGF("Can quit straight away: %d", !retval);

	return retval;
}

bool plugin_manager_busy(plugin_manager_t *manager)
{
	bool busy = manager->state != PLUGIN_MANAGER_STATE_IDLE;

	PROVMAN_LOGF("%s called.  Busy %d", __FUNCTION__, busy);

	return busy;
}

static void prv_add_plugin_index(GArray *indicies, const char *key)
{
	unsigned int index;

	/* We allow duplicate instances in this array.  The plugins
	 will only be synchronised once.  We will end up with a larger
	 array full of redundant indicies but its not worth the effort
	 to elimate the duplicates. */

	if (provman_plugin_find_index(key, &index) == PROVMAN_ERR_NONE)
		g_array_append_val(indicies, index);
	else
		provman_plugin_find_plugins(key, indicies);
}

static GArray *prv_indicies_from_array(GVariant *variant)
{
	GVariantIter *iter;
	gchar *key;
	GArray *indicies = g_array_new (FALSE, FALSE, sizeof(guint));

	iter = g_variant_iter_new(variant);
	while (g_variant_iter_next(iter, "s", &key)) {
		g_strstrip(key);
		prv_add_plugin_index(indicies, key);
		g_free(key);
	}
	g_variant_iter_free(iter);
	return indicies;
}

static GArray *prv_indicies_from_dict(GVariant *variant)
{
	GVariantIter *iter;
	gchar *key;
	gchar *value;
	GArray *indicies = g_array_new (FALSE, FALSE, sizeof(guint));

	iter = g_variant_iter_new(variant);
	while (g_variant_iter_next(iter, "{s&s}", &key, &value)) {
		g_strstrip(key);
		prv_add_plugin_index(indicies, key);
		g_free(key);
	}
	g_variant_iter_free(iter);
	return indicies;
}

static GArray *prv_indicies_from_prop_array(GVariant *variant)
{
	GVariantIter *iter;
	gchar *key;
	gchar *value;
	gchar *prop;
	GArray *indicies = g_array_new (FALSE, FALSE, sizeof(guint));

	iter = g_variant_iter_new(variant);
	while (g_variant_iter_next(iter, "(s&s&s)", &key, &value,
		       &prop)) {
		g_strstrip(key);
		prv_add_plugin_index(indicies, key);
		g_free(key);
	}
	g_variant_iter_free(iter);
	return indicies;
}

static void prv_sync_plugins_cb(int result, void *user_data)
{
	plugin_manager_t *manager = user_data;
	plugin_manager_cmd_t *cmd = &manager->cb;

	if (result == PROVMAN_ERR_CANCELLED) {
		cmd->sync_finished(result, manager);
	} else {
		++cmd->current_index;
		if (prv_sync_plugins(manager))
			cmd->sync_finished(PROVMAN_ERR_NONE, manager);
	}
}

static bool prv_sync_plugins(plugin_manager_t *manager)
{
	plugin_manager_cmd_t *cmd = &manager->cb;
	guint index;
	int err;

	for (; cmd->current_index < cmd->indicies->len; ++cmd->current_index) {
		index = g_array_index(cmd->indicies, guint,
				      cmd->current_index);
		if (!manager->plugin_synced[index]) {
			err = prv_sync_plugin(manager, index,
					      prv_sync_plugins_cb);
			if (err == PROVMAN_ERR_NONE)
				break;
		}
	}

	return cmd->current_index == cmd->indicies->len;
}

static void prv_sync_plugins_and_run(plugin_manager_t *manager,
				     plugin_manager_cb_t cb)
{
	plugin_manager_cmd_t *cmd = &manager->cb;

	cmd->sync_finished = cb;
	if (prv_sync_plugins(manager))
		cb(PROVMAN_ERR_NONE, manager);
}

static int prv_get_common(plugin_manager_t *manager, const gchar *key,
			  plugin_manager_cb_value_t callback, void *user_data)
{
	int err;
	plugin_manager_cmd_t *cmd;
	GArray *indicies = NULL;

	if (manager->state != PLUGIN_MANAGER_STATE_IDLE) {
		err = PROVMAN_ERR_DENIED;
		goto on_error;
	}

	err = provman_utils_validate_key(key);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	indicies = g_array_new (FALSE, FALSE, sizeof(guint));
	prv_add_plugin_index(indicies, key);
	if (indicies->len == 0) {
		err = PROVMAN_ERR_NOT_FOUND;
		goto on_error;
	}

	cmd = &manager->cb;
	memset(cmd, 0, sizeof(*cmd));
	cmd->type = PLUGIN_MANAGER_CMD_TYPE_VALUE;
	cmd->cb_value = callback;
	cmd->user_data = user_data;
	cmd->indicies = indicies;
	cmd->current_index = 0;
	cmd->key = g_strdup(key);

	return PROVMAN_ERR_NONE;

on_error:

	if (indicies)
		(void) g_array_free(indicies, TRUE);

	return err;

}

static void prv_get_cb(int result, void *user_data)
{
	plugin_manager_t *manager = user_data;
	plugin_manager_cmd_t *cmd = &manager->cb;

	if (result == PROVMAN_ERR_NONE)
		result = provman_cache_get(manager->cache, cmd->key,
					   &cmd->ret_value);
	prv_schedule_completion(manager, result);
}

int plugin_manager_get(plugin_manager_t *manager, const gchar *key,
		       plugin_manager_cb_value_t callback, void *user_data)
{
	int err;
	PROVMAN_LOGF("%s called on key %s", __FUNCTION__, key);

	err = prv_get_common(manager, key, callback, user_data);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	prv_sync_plugins_and_run(manager, prv_get_cb);

on_error:

	PROVMAN_LOGF("%s exit with err %d", __FUNCTION__, err);

	return err;
}

static void prv_get_next_key(plugin_manager_t *manager, const char *key,
			     GVariantBuilder *vb)
{
	int err;
	gchar *value;

	err = provman_utils_validate_key(key);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	/* No need to check if plugin synced successfully.  If it did
	   not it won't have any entries in the cache and we will return
	   PROVMAN_ERR_NOT_FOUND. */

	err = provman_cache_get(manager->cache, key, &value);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	g_variant_builder_add(vb, "{ss}", key, value);
	PROVMAN_LOGF("Retrieved %s = %s", key, value);
	g_free(value);

	return;

on_error:

	PROVMAN_LOGF("Unable to retrieve %s", key);

	return;
}

static void prv_get_multiple_cb(int result, void *user_data)
{
	plugin_manager_t *manager = user_data;
	plugin_manager_cmd_t *cmd = &manager->cb;
	GVariantIter *iter;
	gchar *key;
	GVariantBuilder vb;

	if (result == PROVMAN_ERR_NONE) {
		g_variant_builder_init(&vb, G_VARIANT_TYPE("a{ss}"));
		iter = g_variant_iter_new(cmd->keys);
		while (g_variant_iter_next(iter, "s", &key)) {
			g_strstrip(key);
			prv_get_next_key(manager, key, &vb);
			g_free(key);
		}
		g_variant_iter_free(iter);
		cmd->ret_variant =
			g_variant_ref_sink(g_variant_builder_end(&vb));
	}

	prv_schedule_completion(manager, PROVMAN_ERR_NONE);
}

int plugin_manager_get_multiple(plugin_manager_t *manager, GVariant *keys,
				plugin_manager_cb_variant_t callback,
				void *user_data)
{
	int err = PROVMAN_ERR_NONE;
	plugin_manager_cmd_t *cmd;

	PROVMAN_LOGF("%s called", __FUNCTION__);

	if (manager->state != PLUGIN_MANAGER_STATE_IDLE) {
		err = PROVMAN_ERR_DENIED;
		goto on_error;
	}

	cmd = &manager->cb;
	memset(cmd, 0, sizeof(*cmd));
	cmd->type = PLUGIN_MANAGER_CMD_TYPE_VARIANT;
	cmd->cb_variant = callback;
	cmd->user_data = user_data;
	cmd->keys = g_variant_ref_sink(keys);
	cmd->indicies = prv_indicies_from_array(keys);
	cmd->current_index = 0;

	prv_sync_plugins_and_run(manager, prv_get_multiple_cb);

on_error:

	PROVMAN_LOGF("%s exit with err %d", __FUNCTION__, err);

	return err;
}

static int prv_get_all_common(plugin_manager_t *manager,
			      const gchar *search_key,
			      plugin_manager_cb_variant_t callback,
			      void *user_data)
{
	int err = PROVMAN_ERR_NONE;
	plugin_manager_cmd_t *cmd;
	GArray *indicies = NULL;

	if (manager->state != PLUGIN_MANAGER_STATE_IDLE) {
		err = PROVMAN_ERR_DENIED;
		goto on_error;
	}

	err = provman_utils_validate_key(search_key);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	indicies = g_array_new (FALSE, FALSE, sizeof(guint));
	prv_add_plugin_index(indicies, search_key);
	if (indicies->len == 0) {
		err = PROVMAN_ERR_NOT_FOUND;
		goto on_error;
	}

	cmd = &manager->cb;
	memset(cmd, 0, sizeof(*cmd));
	cmd->type = PLUGIN_MANAGER_CMD_TYPE_VARIANT;
	cmd->key = g_strdup(search_key);
	cmd->cb_variant = callback;
	cmd->user_data = user_data;
	cmd->indicies = indicies;
	cmd->current_index = 0;

	return PROVMAN_ERR_NONE;

on_error:

	if (indicies)
		(void) g_array_free(indicies, TRUE);

	return err;
}

static void prv_get_all_cb(int result, void *user_data)

{
	plugin_manager_t *manager = user_data;
	plugin_manager_cmd_t *cmd = &manager->cb;

	if (result == PROVMAN_ERR_NONE)
		result = provman_cache_get_all(manager->cache, cmd->key,
					       &cmd->ret_variant);
	prv_schedule_completion(manager, result);
}

int plugin_manager_get_all(plugin_manager_t *manager, const gchar *search_key,
			   plugin_manager_cb_variant_t callback,
			   void *user_data)
{
	int err;

	PROVMAN_LOGF("%s called on key %s", __FUNCTION__, search_key);

	err = prv_get_all_common(manager, search_key, callback, user_data);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	prv_sync_plugins_and_run(manager, prv_get_all_cb);

on_error:

	PROVMAN_LOGF("%s exit with err %d", __FUNCTION__, err);

	return err;
}

static void prv_get_all_meta_cb(int result, void *user_data)
{
	plugin_manager_t *manager = user_data;
	plugin_manager_cmd_t *cmd = &manager->cb;

	if (result == PROVMAN_ERR_NONE)
		result = provman_cache_get_all_meta(manager->cache, cmd->key,
						    &cmd->ret_variant);
	prv_schedule_completion(manager, result);
}

int plugin_manager_get_all_meta(plugin_manager_t *manager,
				const gchar *search_key,
				plugin_manager_cb_variant_t callback,
				void *user_data)
{
	int err;

	PROVMAN_LOGF("%s called on key %s", __FUNCTION__, search_key);

	err = prv_get_all_common(manager, search_key, callback, user_data);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	prv_sync_plugins_and_run(manager, prv_get_all_meta_cb);

on_error:

	PROVMAN_LOGF("%s exit with err %d", __FUNCTION__, err);

	return err;
}

static int prv_validate_set(provman_schema_t *root, const char *key,
			    const char *value)
{
	int err;
	provman_schema_t *schema;

	err = provman_schema_locate(root, key, &schema);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	if ((schema->type == PROVMAN_SCHEMA_TYPE_DIR) ||
	    !schema->key.can_write) {
		err = PROVMAN_ERR_BAD_KEY;
		goto on_error;
	}

	err = provman_schema_check_value(schema, value);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

on_error:

	return err;
}

static int prv_get_plugin_index(plugin_manager_t *manager, const gchar *key,
				unsigned int *pindex)
{
	int err;

	err = provman_utils_validate_key(key);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	err = provman_plugin_find_index(key, pindex);
	if (err != PROVMAN_ERR_NONE) {
		err = PROVMAN_ERR_BAD_ARGS;
		goto on_error;
	}

on_error:

	return err;
}

static int prv_set_common(plugin_manager_t *manager, const gchar *key,
			  const gchar *value, unsigned int *pindex)
{
	int err;
	unsigned int index;
	provman_schema_t *root;

	err = prv_get_plugin_index(manager, key, &index);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	root = manager->plugin_schemas[index];

	err = prv_validate_set(root, key, value);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	*pindex = index;

on_error:

	return err;
}

static void prv_set_cb(int result, void *user_data)
{
	plugin_manager_t *manager = user_data;
	plugin_manager_cmd_t *cmd = &manager->cb;

	if (result == PROVMAN_ERR_NONE)
		result = provman_cache_set(manager->cache, cmd->key,
					   cmd->value);
	prv_schedule_completion(manager, result);
}

int plugin_manager_set(plugin_manager_t *manager, const gchar *key,
		       const gchar *value, plugin_manager_cb_t callback,
		       void *user_data)
{
	int err = PROVMAN_ERR_NONE;
	plugin_manager_cmd_t *cmd;
	GArray *indicies;
	unsigned int index;

	PROVMAN_LOGF("%s called with key %s value %s", __FUNCTION__, key,
		value);

	if (manager->state != PLUGIN_MANAGER_STATE_IDLE) {
		err = PROVMAN_ERR_DENIED;
		goto on_error;
	}

	err = prv_set_common(manager, key, value, &index);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	indicies = g_array_new(FALSE, FALSE, sizeof(guint));
	g_array_append_val(indicies, index);

	cmd = &manager->cb;
	memset(cmd, 0, sizeof(*cmd));

	cmd->value = g_strdup(value);
	cmd->key = g_strdup(key);
	cmd->type = PLUGIN_MANAGER_CMD_TYPE_VOID;
	cmd->cb_void = callback;
	cmd->user_data = user_data;
	cmd->indicies = indicies;
	cmd->current_index = 0;

	prv_sync_plugins_and_run(manager, prv_set_cb);

on_error:

	PROVMAN_LOGF("%s exit with err %d", __FUNCTION__, err);

	return err;
}

static void prv_set_multiple_cb(int result, void *user_data)
{
	plugin_manager_t *manager = user_data;
	plugin_manager_cmd_t *cmd = &manager->cb;
	GVariantIter *iter;
	gchar *key;
	gchar *value;
	GVariantBuilder vb;
	unsigned int index;
	int err;

	if (result != PROVMAN_ERR_NONE)
		goto on_error;

	g_variant_builder_init(&vb, G_VARIANT_TYPE("as"));

	iter = g_variant_iter_new(cmd->keys);
	while (g_variant_iter_next(iter,"{s&s}", &key, &value)) {
		g_strstrip(key);
		err = prv_set_common(manager, key, value, &index);
		if (err == PROVMAN_ERR_NONE) {
			if (!manager->plugin_synced[index])
				err = PROVMAN_ERR_UNKNOWN;
			else
				err = provman_cache_set(manager->cache,
							key, value);
		}
		if (err != PROVMAN_ERR_NONE)
			g_variant_builder_add(&vb, "s", key);
#ifdef PROVMAN_LOGGING
		if (err != PROVMAN_ERR_NONE)
			PROVMAN_LOGF("Unable to set %s = %s", key, value);
		else
			PROVMAN_LOGF("Set %s = %s", key, value);
#endif
		g_free(key);
	}
	g_variant_iter_free(iter);
	cmd->ret_variant =
		g_variant_ref_sink(g_variant_builder_end(&vb));

on_error:

	prv_schedule_completion(manager, result);
}

int plugin_manager_set_multiple(plugin_manager_t *manager, GVariant *settings,
				plugin_manager_cb_variant_t callback,
				void *user_data)
{
	int err = PROVMAN_ERR_NONE;
	plugin_manager_cmd_t *cmd;

	PROVMAN_LOGF("%s called", __FUNCTION__);

	if (manager->state != PLUGIN_MANAGER_STATE_IDLE) {
		err = PROVMAN_ERR_DENIED;
		goto on_error;
	}

	cmd = &manager->cb;
	memset(cmd, 0, sizeof(*cmd));
	cmd->type = PLUGIN_MANAGER_CMD_TYPE_VARIANT;
	cmd->cb_variant = callback;
	cmd->user_data = user_data;
	cmd->keys = g_variant_ref_sink(settings);
	cmd->indicies = prv_indicies_from_dict(settings);
	cmd->current_index = 0;

	prv_sync_plugins_and_run(manager, prv_set_multiple_cb);

on_error:

	PROVMAN_LOGF("%s exit with err %d", __FUNCTION__, err);

	return err;
}

static void prv_set_multiple_meta_cb(int result, void *user_data)
{
	plugin_manager_t *manager = user_data;
	plugin_manager_cmd_t *cmd = &manager->cb;
	GVariantIter *iter;
	gchar *key;
	gchar *value;
	gchar *prop;
	GVariantBuilder vb;
	unsigned int index;
	int err;

	if (result != PROVMAN_ERR_NONE)
		goto on_error;

	g_variant_builder_init(&vb, G_VARIANT_TYPE("a(ss)"));

	iter = g_variant_iter_new(cmd->keys);
	while (g_variant_iter_next(iter,"(s&s&s)", &key, &prop, &value)) {
		g_strstrip(key);
		err = prv_get_plugin_index(manager, key, &index);
		if (err == PROVMAN_ERR_NONE) {
			if (!manager->plugin_synced[index])
				err = PROVMAN_ERR_UNKNOWN;
			else
				err = provman_cache_set_meta(manager->cache,
							     key, prop, value);
		}
		if (err != PROVMAN_ERR_NONE)
			g_variant_builder_add(&vb, "(ss)", key, prop);
#ifdef PROVMAN_LOGGING
		if (err != PROVMAN_ERR_NONE)
			PROVMAN_LOGF("Unable to set %s?%s = %s", key,
				     prop, value);
		else
			PROVMAN_LOGF("Set %s?%s = %s", key, prop, value);
#endif
		g_free(key);
	}
	g_variant_iter_free(iter);
	cmd->ret_variant = g_variant_ref_sink(g_variant_builder_end(&vb));

on_error:

	prv_schedule_completion(manager, result);
}

int plugin_manager_set_multiple_meta(plugin_manager_t *manager,
				     GVariant *settings,
				     plugin_manager_cb_variant_t callback,
				     void *user_data)
{
	int err = PROVMAN_ERR_NONE;
	plugin_manager_cmd_t *cmd;

	PROVMAN_LOGF("%s called", __FUNCTION__);

	if (manager->state != PLUGIN_MANAGER_STATE_IDLE) {
		err = PROVMAN_ERR_DENIED;
		goto on_error;
	}

	cmd = &manager->cb;
	memset(cmd, 0, sizeof(*cmd));
	cmd->type = PLUGIN_MANAGER_CMD_TYPE_VARIANT;
	cmd->cb_variant = callback;
	cmd->user_data = user_data;
	cmd->keys = g_variant_ref_sink(settings);
	cmd->indicies = prv_indicies_from_prop_array(settings);
	cmd->current_index = 0;

	prv_sync_plugins_and_run(manager, prv_set_multiple_meta_cb);

on_error:

	PROVMAN_LOGF("%s exit with err %d", __FUNCTION__, err);

	return err;
}

static int prv_remove_common(plugin_manager_t *manager, const gchar *key)
{
	int err = PROVMAN_ERR_NONE;
	provman_schema_t *root;
	provman_schema_t *schema;
	unsigned int index;

	err = provman_utils_validate_key(key);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	if (provman_plugin_find_index(key, &index) == PROVMAN_ERR_NONE) {
		root = manager->plugin_schemas[index];

		err = provman_schema_locate(root, key, &schema);
		if (err != PROVMAN_ERR_NONE)
			goto on_error;

		if (!schema->can_delete) {
			err = PROVMAN_ERR_DENIED;
			goto on_error;
		}
	}

on_error:

	return err;
}

static void prv_remove_cb(int result, void *user_data)
{
	plugin_manager_t *manager = user_data;
	plugin_manager_cmd_t *cmd = &manager->cb;

	if (result == PROVMAN_ERR_NONE)
		result = provman_cache_remove(manager->cache, cmd->key);

	prv_schedule_completion(manager, result);
}

int plugin_manager_remove(plugin_manager_t *manager, const gchar *key,
			  plugin_manager_cb_t callback, void *user_data)
{
	int err = PROVMAN_ERR_NONE;
	GArray *indicies = NULL;
	plugin_manager_cmd_t *cmd = &manager->cb;

	PROVMAN_LOGF("%s called on key %s", __FUNCTION__, key);

	if (manager->state != PLUGIN_MANAGER_STATE_IDLE) {
		err = PROVMAN_ERR_DENIED;
		goto on_error;
	}

	err = prv_remove_common(manager, key);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	indicies = g_array_new (FALSE, FALSE, sizeof(guint));
	prv_add_plugin_index(indicies, key);
	if (indicies->len == 0) {
		err = PROVMAN_ERR_NOT_FOUND;
		goto on_error;
	}

	cmd = &manager->cb;
	memset(cmd, 0, sizeof(*cmd));
	cmd->type = PLUGIN_MANAGER_CMD_TYPE_VOID;
	cmd->cb_void = callback;
	cmd->user_data = user_data;
	cmd->indicies = indicies;
	indicies = NULL;
	cmd->current_index = 0;
	cmd->key = g_strdup(key);

	prv_sync_plugins_and_run(manager, prv_remove_cb);

on_error:

	PROVMAN_LOGF("%s %s returned with err %d", __FUNCTION__, key, err);

	if (indicies)
		(void) g_array_free(indicies, TRUE);

	return err;
}

static void prv_remove_multiple_cb(int result, void *user_data)
{
	plugin_manager_t *manager = user_data;
	plugin_manager_cmd_t *cmd = &manager->cb;
	GVariantIter *iter;
	gchar *key;
	GVariantBuilder vb;
	int err;

	if (result != PROVMAN_ERR_NONE)
		goto on_error;

	g_variant_builder_init(&vb, G_VARIANT_TYPE("as"));

	iter = g_variant_iter_new(cmd->keys);
	while (g_variant_iter_next(iter,"s", &key)) {
		g_strstrip(key);
		err = prv_remove_common(manager, key);
		if (err == PROVMAN_ERR_NONE)
			err = provman_cache_remove(manager->cache, key);
		if (err != PROVMAN_ERR_NONE)
			g_variant_builder_add(&vb, "s", key);

#ifdef PROVMAN_LOGGING
		if (err != PROVMAN_ERR_NONE)
			PROVMAN_LOGF("Unable to remove %s", key);
		else
			PROVMAN_LOGF("Deleted %s", key);
#endif
		g_free(key);
	}
	g_variant_iter_free(iter);
	cmd->ret_variant = g_variant_ref_sink(g_variant_builder_end(&vb));

on_error:

	prv_schedule_completion(manager, result);
}

int plugin_manager_remove_multiple(plugin_manager_t *manager, GVariant *keys,
				   plugin_manager_cb_variant_t callback,
				   void *user_data)
{
	int err = PROVMAN_ERR_NONE;
	plugin_manager_cmd_t *cmd;

	PROVMAN_LOGF("%s called", __FUNCTION__);

	if (manager->state != PLUGIN_MANAGER_STATE_IDLE) {
		err = PROVMAN_ERR_DENIED;
		goto on_error;
	}

	cmd = &manager->cb;
	memset(cmd, 0, sizeof(*cmd));
	cmd->type = PLUGIN_MANAGER_CMD_TYPE_VARIANT;
	cmd->cb_variant = callback;
	cmd->user_data = user_data;
	cmd->keys = g_variant_ref_sink(keys);
	cmd->indicies = prv_indicies_from_array(keys);
	cmd->current_index = 0;

	prv_sync_plugins_and_run(manager, prv_remove_multiple_cb);

on_error:

	PROVMAN_LOGF("%s exit with err %d", __FUNCTION__, err);

	return err;
}

int plugin_manager_abort(plugin_manager_t *manager)
{
	int err = PROVMAN_ERR_NONE;
	unsigned int i;
	const provman_plugin *plugin;
	provman_plugin_instance pi;
	unsigned int count = provman_plugin_get_count();

	PROVMAN_LOGF("%s called", __FUNCTION__);

	if (manager->state != PLUGIN_MANAGER_STATE_IDLE) {
		err = PROVMAN_ERR_DENIED;
		goto on_error;
	}

	for (i = 0; i < count; ++i) {
		plugin = provman_plugin_get(i);

		if (plugin->abort_fn) {
			pi = manager->plugin_instances[i];
			plugin->abort_fn(pi);
		}
	}
	prv_clear_cache(manager);
	g_free(manager->imsi);
	manager->imsi = NULL;

on_error:

	PROVMAN_LOGF("%s exit with err %d", __FUNCTION__, err);

	return err;
}

static gchar *prv_get_schema_type(provman_schema_t *schema)
{
	gchar *retval = NULL;
	const gchar *type = NULL;
	GString *str;
	GHashTableIter iter;
	unsigned int enum_count;
	unsigned int i = 0;
	gpointer key;

	if (schema->key.type == PROVMAN_SCHEMA_VALUE_TYPE_ENUM) {
		str = g_string_new(PLUGIN_MANAGER_TYPE_ENUM);
		g_string_append(str, ": ");
		g_hash_table_iter_init(&iter, schema->key.allowed_values);
		enum_count = g_hash_table_size(schema->key.allowed_values);
		while ((i < enum_count - 1) &&
		       g_hash_table_iter_next(&iter, &key, NULL)) {
			g_string_append(str, (gchar*) key);
			g_string_append(str, ", ");
			++i;
		}

		if (g_hash_table_iter_next(&iter, &key, NULL))
			g_string_append(str, (gchar*) key);

		retval = g_string_free(str, FALSE);
	} else {
		if (schema->type == PROVMAN_SCHEMA_TYPE_DIR)
			type = PLUGIN_MANAGER_TYPE_DIR;
		else if (schema->key.type == PROVMAN_SCHEMA_VALUE_TYPE_STRING)
			type = PLUGIN_MANAGER_TYPE_STRING;
		else if (schema->key.type == PROVMAN_SCHEMA_VALUE_TYPE_INT)
			type = PLUGIN_MANAGER_TYPE_INT;
		if (type)
			retval = g_strdup(type);
	}

	return retval;
}

static int prv_get_schema_dir_type_info(plugin_manager_t *manager,
					const gchar *search_key,
					unsigned int index,
					GVariantBuilder *vb)
{
	int err = PROVMAN_ERR_NONE;
	provman_schema_t *root;
	provman_schema_t *parent;
	provman_schema_t *child;
	GHashTableIter iter;
	gpointer key;
	gpointer value;
	gchar *type;
	gchar *key_name;

	root = manager->plugin_schemas[index];

	err = provman_schema_locate(root, search_key, &parent);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	if (parent->type != PROVMAN_SCHEMA_TYPE_DIR) {
		err = PROVMAN_ERR_BAD_ARGS;
		goto on_error;
	}

	g_hash_table_iter_init(&iter, parent->dir.children);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		child = value;
		type = prv_get_schema_type(child);
		if (type) {
			key_name = key;
			if (!key_name[0])
				key_name = PLUGIN_MANAGER_UNNAMED_DIR;

			g_variant_builder_add(vb, "{ss}", key_name, type);
			PROVMAN_LOGF("Get Supported %s=%u", key_name, type);
			g_free(type);
		}
	}

on_error:

	return err;
}

int plugin_manager_get_children_type_info(plugin_manager_t *manager,
					  const gchar *search_key,
					  GVariant **values)
{
	int err = PROVMAN_ERR_NONE;
	unsigned int index;
	GVariantBuilder *vb = NULL;
	GPtrArray *children = NULL;
	unsigned int i;
	const gchar *root;

	PROVMAN_LOGF("%s called on key %s", __FUNCTION__, search_key);

	if (manager->state != PLUGIN_MANAGER_STATE_IDLE) {
		err = PROVMAN_ERR_DENIED;
		goto on_error;
	}

	err = provman_utils_validate_key(search_key);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	vb = g_variant_builder_new(G_VARIANT_TYPE("a{ss}"));

	err = provman_plugin_find_index(search_key, &index);
	if (err == PROVMAN_ERR_NONE) {
		err = prv_get_schema_dir_type_info(manager, search_key, index,
						   vb);
		if (err != PROVMAN_ERR_NONE)
			goto on_error;
	} else {
		children = provman_plugin_find_direct_children(search_key);
		if (children->len == 0) {
			err = PROVMAN_ERR_NOT_FOUND;
			goto on_error;
		}

		err = PROVMAN_ERR_NONE;
		for (i = 0; i < children->len; ++i) {
			root = g_ptr_array_index(children, i);
			g_variant_builder_add(vb, "{ss}", root,
				PLUGIN_MANAGER_TYPE_DIR);
		}
	}

	*values = g_variant_ref_sink(g_variant_builder_end(vb));

on_error:

	if (children)
		g_ptr_array_unref(children);

	if (vb)
		g_variant_builder_unref(vb);

	PROVMAN_LOGF("Get Children Type Info of %s returned with %d",
		     search_key, err);

	return err;
}

int plugin_manager_get_type_info(plugin_manager_t *manager,
				 const gchar *search_key, gchar **type_info)
{
	int err = PROVMAN_ERR_NONE;
	unsigned int index;
	provman_schema_t *schema_root;
	provman_schema_t *schema;
	gchar *type;

	PROVMAN_LOGF("%s called on key %s", __FUNCTION__, search_key);

	if (manager->state != PLUGIN_MANAGER_STATE_IDLE) {
		err = PROVMAN_ERR_DENIED;
		goto on_error;
	}

	err = provman_utils_validate_key(search_key);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	err = provman_plugin_find_index(search_key, &index);
	if (err == PROVMAN_ERR_NONE) {
		schema_root = manager->plugin_schemas[index];

		err = provman_schema_locate(schema_root, search_key, &schema);
		if (err != PROVMAN_ERR_NONE)
			goto on_error;

		type = prv_get_schema_type(schema);
		if (!type) {
			err = PROVMAN_ERR_CORRUPT;
			goto on_error;
		}
		*type_info = type;
	} else {
		err = PROVMAN_ERR_NONE;
		if (!provman_plugin_uri_exists(search_key)) {
			err = PROVMAN_ERR_NOT_FOUND;
			goto on_error;
		}
		*type_info = g_strdup(PLUGIN_MANAGER_TYPE_DIR);
	}

on_error:

	PROVMAN_LOGF("Get Type Info of %s returned with %d", search_key, err);

	return err;
}

static void prv_get_meta_cb(int result, void *user_data)
{
	plugin_manager_t *manager = user_data;
	plugin_manager_cmd_t *cmd = &manager->cb;

	if (result == PROVMAN_ERR_NONE)
		result = provman_cache_get_meta(manager->cache,
						cmd->key, cmd->prop,
						&cmd->ret_value);
	prv_schedule_completion(manager, result);
}

int plugin_manager_get_meta(plugin_manager_t *manager, const gchar *key,
			    const gchar *prop,
			    plugin_manager_cb_value_t callback, void *user_data)
{
	int err;
	plugin_manager_cmd_t *cmd = &manager->cb;

	PROVMAN_LOGF("%s called on key %s", __FUNCTION__, key);

	err = prv_get_common(manager, key, callback, user_data);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	cmd->prop = g_strdup(prop);
	prv_sync_plugins_and_run(manager, prv_get_meta_cb);

on_error:

	PROVMAN_LOGF("%s exit with err %d", __FUNCTION__, err);

	return err;
}

static void prv_set_meta_cb(int result, void *user_data)
{
	plugin_manager_t *manager = user_data;
	plugin_manager_cmd_t *cmd = &manager->cb;

	if (result == PROVMAN_ERR_NONE)
		result = provman_cache_set_meta(manager->cache, cmd->key,
						cmd->value, cmd->prop);
	prv_schedule_completion(manager, result);
}

int plugin_manager_set_meta(plugin_manager_t *manager, const gchar *key,
			    const gchar *value, const gchar *prop,
			    plugin_manager_cb_t callback, void *user_data)
{
	int err;
	plugin_manager_cmd_t *cmd;
	GArray *indicies;
	unsigned int index;

	PROVMAN_LOGF("%s called with key %s prop %s value %s", __FUNCTION__,
		     key, prop, value);

	if (manager->state != PLUGIN_MANAGER_STATE_IDLE) {
		err = PROVMAN_ERR_DENIED;
		goto on_error;
	}

	err = prv_get_plugin_index(manager, key, &index);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	indicies = g_array_new(FALSE, FALSE, sizeof(guint));
	g_array_append_val(indicies, index);

	cmd = &manager->cb;
	memset(cmd, 0, sizeof(*cmd));

	cmd->value = g_strdup(value);
	cmd->key = g_strdup(key);
	cmd->prop = g_strdup(prop);
	cmd->type = PLUGIN_MANAGER_CMD_TYPE_VOID;
	cmd->cb_void = callback;
	cmd->user_data = user_data;
	cmd->indicies = indicies;
	cmd->current_index = 0;

	prv_sync_plugins_and_run(manager, prv_set_meta_cb);

on_error:

	PROVMAN_LOGF("Set Meta (%s, %s, %p) returned with error: %u", key, prop,
		     value, err);

	return err;
}
