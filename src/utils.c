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
 * @file utils.c
 *
 * @brief contains general utility functions 
 *
 *****************************************************************************/
#include "config.h"

#include <string.h>
#include <unistd.h>

#include <glib.h>

#include "error.h"
#include "log.h"

#include "utils.h"

int provman_utils_validate_key(const char *key)
{
	int err = PROVMAN_ERR_NONE;
	const char *start;
	const char *end;

	if (!key) {
		err = PROVMAN_ERR_BAD_ARGS;
		goto on_error;
	}

	if (key[0] != '/') {
		err = PROVMAN_ERR_BAD_ARGS;
		goto on_error;
	}
	
	start = end = key + 1;
	while (*start) {	
		while (*end && *end != '/') {
			if (!g_ascii_isprint(*end) || g_ascii_isspace(*end)) {
				err = PROVMAN_ERR_BAD_ARGS;
				goto on_error;
			}
			++end;
		}
		
		if (start == end) {
			err = PROVMAN_ERR_BAD_ARGS;
			goto on_error;		
		}
		
		if (*end)
			++end;
		start = end;
	}

on_error:

	return err;
}

int provman_utils_make_file_path(const char* fname, gchar **path)
{
	int err = PROVMAN_ERR_NONE;

	const char *home_dir;
	GString *db_path = NULL;

	if (getuid()) {
		home_dir = g_get_home_dir();
		if (!home_dir) {
			err = PROVMAN_ERR_NOT_FOUND;
			goto on_error;
		}
		db_path = g_string_new(home_dir);
		db_path = g_string_append(db_path,"/");
		db_path = g_string_append(db_path, PROVMAN_SESSION_DB_PATH);		
	} else {
		db_path = g_string_new(PROVMAN_SYSTEM_DB_PATH);
	}

	(void) g_mkdir_with_parents(db_path->str, 0700);
	db_path = g_string_append(db_path,"/");
	db_path = g_string_append(db_path, fname);

	*path = db_path->str;
	g_string_free(db_path, FALSE);

on_error:

	return err;
}

gchar *provman_utils_get_context_from_key(const gchar *key, const char *root,
					  unsigned int root_len)
{
	gchar *context = NULL;
	gchar *ptr;

	if (!strncmp(root, key, root_len)) {
		context = g_strdup(key + root_len); 
		ptr = strchr(context, '/');
		if (ptr)
			*ptr = 0;
	}
	
	return context;		
}

GHashTable *provman_utils_get_contexts(GHashTable *settings, const char *root,
				       unsigned int root_len)
{
	gpointer key;
	gchar *context;
	GHashTableIter iter;
	GHashTable *contexts = 
		g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

	g_hash_table_iter_init(&iter, settings);
	while (g_hash_table_iter_next(&iter, &key, NULL)) {
		context = provman_utils_get_context_from_key(key, root,
							       root_len);
		if (context)
			g_hash_table_insert(contexts, context, NULL);
	}
	
	return contexts;
}

static gboolean prv_can_remove_settings(gpointer key, gpointer value,
					gpointer user_data)
{
	GString *account_key = user_data;

	return ((strlen((const char*) key) > account_key->len) &&
		!strncmp(account_key->str, (const char*) key, 
			 account_key->len));		
}

void provman_utils_remove_account(GHashTable *settings, const gchar *root,
				  const gchar *account)
{
	GString *account_key;

	account_key = g_string_new(root);
	g_string_append(account_key, account);
	g_string_append(account_key, "/");
	g_hash_table_foreach_remove(settings, prv_can_remove_settings,
				    account_key);
	(void) g_string_free(account_key, TRUE);
}				  

#ifdef PROVMAN_LOGGING
void provman_utils_dump_hash_table(GHashTable* hash_table)
{
	gpointer key, value;
	GList *list;
	GList *ptr;

	list = g_hash_table_get_keys(hash_table);
	list = g_list_sort(list, (GCompareFunc) strcmp);
	ptr = list;
	while (ptr)
	{
		key = ptr->data;
		value = g_hash_table_lookup(hash_table, key);
		PROVMAN_LOGF("%s = %s", key, value);
		ptr = ptr->next;
	}
	g_list_free(list);
}
#endif
