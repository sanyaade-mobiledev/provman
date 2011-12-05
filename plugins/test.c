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
 * @file test.c
 *
 * @brief contains function definitions for the test plugin
 *
 *****************************************************************************/

#include "config.h"

#include <string.h>

#include <glib.h>
#include <gio/gio.h>

#include "error.h"
#include "log.h"
#include "utils.h"

#include "plugin.h"
#include "test.h"

#define TEST_KEY_FILE_NAME "test-plugin-storage.ini"
#define TEST_GROUP_NAME "GROUP"
#define TEST_DEFAULT_IMSI "012345678987654321"

typedef struct test_plugin_t_ test_plugin_t;
struct test_plugin_t_ {
	bool system;
	gchar *fname;
	gchar *imsi;
	GHashTable *settings;
	provman_plugin_sync_in_cb sync_in_cb;
	void *sync_in_user_data;
	provman_plugin_sync_out_cb sync_out_cb;
	void *sync_out_user_data;
};

int test_plugin_new(provman_plugin_instance *instance, bool system)
{
	test_plugin_t *retval;

	retval = g_new0(test_plugin_t, 1);
	retval->system = system;

	*instance = retval;

	return PROVMAN_ERR_NONE;
}

void test_plugin_delete(provman_plugin_instance instance)
{
	test_plugin_t *plugin_instance;

	if (instance) {
		plugin_instance = instance;
		g_free(plugin_instance->imsi);
		g_free(plugin_instance->fname);
		g_free(instance);
	}
}

static gboolean prv_complete_sync_in(gpointer user_data)
{
	test_plugin_t *plugin_instance = user_data;

#ifdef PROVMAN_LOGGING
	provman_utils_dump_hash_table(plugin_instance->settings);
#endif
	plugin_instance->sync_in_cb(PROVMAN_ERR_NONE, plugin_instance->settings,
				    plugin_instance->sync_in_user_data);

	g_hash_table_unref(plugin_instance->settings);
	plugin_instance->settings = NULL;

	return FALSE;
}

int test_plugin_sync_in(provman_plugin_instance instance,
			const char* imsi,
			provman_plugin_sync_in_cb callback,
			void *user_data)
{
	int err = PROVMAN_ERR_NONE;
	test_plugin_t *plugin_instance = instance;
	gchar *value;
	gchar **keys;
	unsigned int i;
	GHashTable *settings;
	GString *fname = NULL;
	GKeyFile *key_file;
	gchar *test_imsi;

	if (imsi[0])
		test_imsi = g_strdup(imsi);
	else
		test_imsi = g_strdup(TEST_DEFAULT_IMSI);

	fname = g_string_new(test_imsi);
	g_string_append_c(fname, '-');
	g_string_append(fname, TEST_KEY_FILE_NAME);

	err = provman_utils_make_file_path(fname->str, plugin_instance->system,
					   &plugin_instance->fname);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	key_file = g_key_file_new();
	(void) g_key_file_load_from_file(key_file, plugin_instance->fname,
					 G_KEY_FILE_NONE, NULL);
	settings = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
					 g_free);

	keys = g_key_file_get_keys(key_file, TEST_GROUP_NAME,
				   NULL, NULL);
	if (keys)
		for (i = 0; keys[i]; ++i) {
			value = g_key_file_get_value(key_file,
						     TEST_GROUP_NAME, keys[i],
						     NULL);
			if (!value)
				g_free(keys[i]);
			else
				g_hash_table_insert(settings, keys[i], value);
		}

	g_key_file_free(key_file);

	plugin_instance->imsi = test_imsi;
	test_imsi = NULL;
	plugin_instance->settings = settings;
	plugin_instance->sync_in_cb = callback;
	plugin_instance->sync_in_user_data = user_data;
	(void) g_idle_add(prv_complete_sync_in, plugin_instance);

	g_free(keys);

on_error:

	g_string_free(fname, TRUE);
	g_free(test_imsi);

	return PROVMAN_ERR_NONE;
}

void test_plugin_sync_in_cancel(provman_plugin_instance instance)
{

}

static gboolean prv_complete_sync_out(gpointer user_data)
{
	test_plugin_t *plugin_instance = user_data;

	plugin_instance->sync_out_cb(PROVMAN_ERR_NONE,
				     plugin_instance->sync_out_user_data);

	g_free(plugin_instance->imsi);
	plugin_instance->imsi = NULL;
	g_free(plugin_instance->fname);
	plugin_instance->fname = NULL;

	return FALSE;
}


int test_plugin_sync_out(provman_plugin_instance instance,
			  GHashTable* settings,
			  provman_plugin_sync_out_cb callback,
			  void *user_data)
{
	int err = PROVMAN_ERR_NONE;
	GHashTableIter iter;
	gpointer key;
	gpointer value;
	gsize length;
	GKeyFile *key_file;
	gchar *data = NULL;
	test_plugin_t *plugin_instance = instance;

	key_file = g_key_file_new();
	g_hash_table_iter_init(&iter, settings);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		g_key_file_set_value(key_file,
				     TEST_GROUP_NAME, key, value);
	}

	data = g_key_file_to_data(key_file, &length, NULL);

	if (!data) {
		err = PROVMAN_ERR_OOM;
		goto on_error;
	}

	if (!g_file_set_contents(plugin_instance->fname, data, length, NULL))  {
		err = PROVMAN_ERR_IO;
		goto on_error;
	}

	plugin_instance->sync_out_cb = callback;
	plugin_instance->sync_out_user_data = user_data;
	(void) g_idle_add(prv_complete_sync_out, plugin_instance);

on_error:

	g_free(data);
	g_key_file_free(key_file);

#ifdef PROVMAN_LOGGING
	if (err != PROVMAN_ERR_NONE)
		PROVMAN_LOGF("Unable to write key file %s",
			     plugin_instance->fname);
#endif

	return err;
}

void test_plugin_sync_out_cancel(provman_plugin_instance instance)
{

}

void test_plugin_abort(provman_plugin_instance instance)
{
	test_plugin_t *plugin_instance = instance;

	g_free(plugin_instance->imsi);
	plugin_instance->imsi = NULL;
	g_free(plugin_instance->fname);
	plugin_instance->fname = NULL;
}

const gchar* test_plugin_sim_id(provman_plugin_instance instance)
{
	test_plugin_t *plugin_instance = instance;

	return plugin_instance->imsi;
}
