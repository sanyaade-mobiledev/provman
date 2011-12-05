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
 * @file meta-data.c
 *
 * @brief Main file meta data.  Contains methods to associate meta data with
 * existing settings.  Meta data is store persistently and survives the end
 * of a session.
 *
 ******************************************************************************/

#include "config.h"

#include <string.h>
#include <stdbool.h>

#include "meta-data.h"
#include "log.h"

struct provman_meta_data_t_ {
	GKeyFile *key_file;
	gchar *fname;
};

static void prv_unref_ht(gpointer ht)
{
	if (ht)
		g_hash_table_unref((GHashTable *) ht);
}

void provman_meta_data_new(const gchar *fname, provman_meta_data_t **meta_data)
{
	provman_meta_data_t *md = g_new0(provman_meta_data_t, 1);
	md->key_file = g_key_file_new();
	(void) g_key_file_load_from_file(md->key_file, fname,
					 G_KEY_FILE_NONE, NULL);
	md->fname = g_strdup(fname);
	*meta_data = md;
}

void provman_meta_data_delete(provman_meta_data_t *meta_data)
{
	if (meta_data) {
		g_free(meta_data->fname);
		g_key_file_free(meta_data->key_file);
		g_free(meta_data);
	}
}

GHashTable *provman_meta_data_get_all(provman_meta_data_t *meta_data)
{
	GHashTable *md_settings;
	GHashTable *props;
	gchar **groups;
	unsigned int i;
	unsigned int j;
	gchar **keys;
	gchar *value;

	md_settings = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
					    prv_unref_ht);

	groups = g_key_file_get_groups(meta_data->key_file, NULL);

	for (i = 0; groups[i]; ++i) {
		keys = g_key_file_get_keys(meta_data->key_file, groups[i], NULL,
					   NULL);
		if (keys) {
			props = g_hash_table_new_full(g_str_hash, g_str_equal,
						      g_free, g_free);
			for (j = 0; keys[j]; ++j) {
				value = g_key_file_get_value(
					meta_data->key_file, groups[i], keys[j],
					NULL);
				if (value)
					g_hash_table_insert(props, keys[j],
							    value);
				else
					g_free(keys[j]);
			}
			g_free(keys);
			g_hash_table_insert(md_settings, groups[i], props);
		} else {
			g_free(groups[i]);
		}
	}

	g_free(groups);

	return md_settings;
}

static void prv_insert_new_group(provman_meta_data_t *meta_data,
				 const gchar* group_name, GHashTable *props)
{
	GHashTableIter iter;
	gpointer key;
	gpointer value;

	g_hash_table_iter_init(&iter, props);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		g_key_file_set_value(meta_data->key_file, group_name,
				     key, value);
	}
}

static bool prv_update_group(provman_meta_data_t *meta_data,
			     const gchar* group_name, GHashTable *props)
{
	GHashTableIter iter;
	gpointer key;
	gpointer value;
	gchar *old_value;
	bool dirty = false;

	g_hash_table_iter_init(&iter, props);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		old_value = g_key_file_get_value(meta_data->key_file,
						 group_name, key, NULL);
		if (!old_value || strcmp(old_value, value)) {
			g_key_file_set_value(meta_data->key_file, group_name,
					     key, value);
			dirty = true;
		}
		g_free(old_value);
	}

	return dirty;
}

void provman_meta_data_update(provman_meta_data_t *meta_data,
			      GHashTable *md_settings)
{
	bool dirty = false;
	gchar **groups;
	unsigned int i;
	GHashTableIter iter;
	gpointer key;
	gpointer value;
	gsize length;
	gchar *data;
#ifdef PROVMAN_LOGGING
	bool saved = false;
#endif

	groups = g_key_file_get_groups(meta_data->key_file, NULL);

	for (i = 0; groups[i]; ++i)
		if (!g_hash_table_lookup(md_settings, groups[i]))
			if (g_key_file_remove_group(meta_data->key_file,
						    groups[i], NULL))
				dirty = true;
	g_strfreev(groups);

	g_hash_table_iter_init(&iter, md_settings);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		if (!g_key_file_has_group(meta_data->key_file, key)) {
			prv_insert_new_group(meta_data, key, value);
			dirty = true;
		} else if (prv_update_group(meta_data, key, value)) {
			dirty = true;
		}
	}

	if (dirty) {
		data = g_key_file_to_data(meta_data->key_file, &length, NULL);
		if (data) {
#ifdef PROVMAN_LOGGING
			saved = g_file_set_contents(meta_data->fname, data,
						    length, NULL);
#else
			(void) g_file_set_contents(meta_data->fname, data,
						   length, NULL);
#endif
			g_free(data);
		}

#ifdef PROVMAN_LOGGING
		if (!saved)
			PROVMAN_LOGF("Unable to write meta file %s",
				     meta_data->fname);
#endif
	}

}
