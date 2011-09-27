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
 * @file cache.c
 *
 * @brief Contains function definitions for managing a tree of settings
 *
 *****************************************************************************/

#include "config.h"

#include <string.h>

#include "cache.h"
#include "error.h"
#include "log.h"

struct provman_cache_t_ {
	GHashTable *children;
	gchar *value;
	provman_cache_t *parent;
};

typedef struct provman_cache_key_t_ provman_cache_key_t;
struct provman_cache_key_t_ {
	const gchar *key;
	gchar *key_copy;
};

typedef void (*provman_cache_visit_cb_t)(const gchar* path, const gchar *value,
					 gpointer user_data);


static void prv_del_node(void *node)
{	
	provman_cache_delete((provman_cache_t *) node);
}

void provman_cache_new(provman_cache_t **cache)
{
	provman_cache_t *node = g_slice_new0(provman_cache_t);
	node->children = g_hash_table_new_full(g_str_hash, g_str_equal,
					       g_free, prv_del_node);
	*cache = node;
}

static int prv_find_node(provman_cache_t *cache, const gchar *key,
			 provman_cache_t **node)
{
	int err = 0;
	gchar** child_names = NULL;
	provman_cache_t *child;
	GHashTable *children;
	unsigned int i;

	if (key[0] != '/') {
		err = PROVMAN_ERR_BAD_ARGS;
		goto on_error;
	}
	
	if (!strcmp(key,"/")) {
		*node = cache;
	} else {
		child_names = g_strsplit(key + 1, "/", 0);
		children = cache->children;
		child = cache;
		for (i = 0; child_names[i]; ++i) {
			if (!children) {
				err = PROVMAN_ERR_NOT_FOUND;
				goto on_error;
			}			

			child = g_hash_table_lookup(children, child_names[i]);
			if (!child) {
				err = PROVMAN_ERR_NOT_FOUND;
				goto on_error;
			}
			children = child->children;
		}
		
		*node = child;
	}

on_error:

	if (child_names)
		g_strfreev(child_names);

	return err;
}

			 
int provman_cache_exists(provman_cache_t *cache, const gchar *key, bool *leaf)
{
	provman_cache_t *node;
	int err;

	err = prv_find_node(cache, key, &node);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	*leaf = node->children == NULL;	

on_error:

	return err;
}

static int prv_create_and_add_node(provman_cache_t *cache, const gchar *key,
				   provman_cache_t **node, gchar **node_name)
{
	int err = 0;
	gchar **child_names = NULL;
	provman_cache_t *child;
	provman_cache_t *next_child;
	unsigned int i;

	if (key[0] != '/') {
		err = PROVMAN_ERR_BAD_ARGS;
		goto on_error;
	}
	
	child_names = g_strsplit(key + 1, "/", 0);

	if (!child_names[0]) {
		err = PROVMAN_ERR_BAD_ARGS;
		goto on_error;
	}
	
	/* Find deepest existing ancestor */

	child = cache;
	next_child = cache;
	for (i = 0; child_names[i] && child_names[i + 1]; ++i) {
		if (!child->children) {
			err = PROVMAN_ERR_BAD_ARGS;
			goto on_error;			
		}

		next_child = g_hash_table_lookup(child->children,
						 child_names[i]);			
		if (!next_child)
			break;

		child = next_child;
	}

	for  (; child_names[i] && child_names[i + 1]; ++i) {
		next_child = g_slice_new0(provman_cache_t);
		next_child->parent = child;
		if (!child->children)
			child->children = 
				g_hash_table_new_full(g_str_hash, g_str_equal,
						      g_free, prv_del_node);
		g_hash_table_insert(child->children, g_strdup(child_names[i]),
				    next_child);
		child = next_child;
	}

	*node = next_child;
	*node_name = g_strdup(child_names[i]);

on_error:

	if (child_names)
		g_strfreev(child_names);

	return err;       	
}

static void prv_provman_cache_key_init(provman_cache_key_t *cache_key, 
				       const gchar *key)
{
	unsigned int key_len;

	key_len = strlen(key) - 1;
	if (key_len > 1 && key[key_len] == '/') {
		cache_key->key_copy = g_strdup(key);	
		cache_key->key_copy[key_len] = 0;
		cache_key->key = cache_key->key_copy;
	} else {
		cache_key->key = key;
		cache_key->key_copy = NULL;
	}
}

static void prv_provman_cache_key_free(provman_cache_key_t *cache_key)
{
	g_free(cache_key->key_copy);
}

int provman_cache_set(provman_cache_t *cache, const gchar *key, 
		      const gchar *value)
{
	int err;
	provman_cache_t *node;
	provman_cache_t *child;
	gchar *node_name;
	provman_cache_key_t cache_key;

	prv_provman_cache_key_init(&cache_key, key);	
	err = prv_create_and_add_node(cache, cache_key.key, &node, &node_name);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	child = g_slice_new0(provman_cache_t);
	child->parent = node;
	child->value = g_strdup(value);
	
	if (!node->children)
		node->children = 
			g_hash_table_new_full(g_str_hash, g_str_equal,
					      g_free, prv_del_node);
	
	g_hash_table_insert(node->children, node_name, child);

on_error:

	prv_provman_cache_key_free(&cache_key);
	
	return err;
}

int provman_cache_remove(provman_cache_t *cache, const gchar *key)
{
	provman_cache_t *node;
	provman_cache_t *target_node;
	provman_cache_t *parent;
	int err;
	GHashTableIter iter;
	gpointer hash_key;
	gpointer value;
	provman_cache_key_t cache_key;

	prv_provman_cache_key_init(&cache_key, key);

	err = prv_find_node(cache, cache_key.key, &target_node);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	node = target_node;
	parent = node->parent;
	while (parent && g_hash_table_size(parent->children) == 1) {
		node = parent;
		parent = node->parent;
	}

	if (!parent) {
		/* We have deleted the last setting in the tree.
		   We don't want to delete the root node so we
		   just free its only child. */

		g_hash_table_remove_all(node->children);
	} else if (target_node == node) {
		g_hash_table_remove(parent->children,
				    strrchr(cache_key.key,'/') + 1);
	} else {
		g_hash_table_iter_init(&iter, parent->children);
		while (g_hash_table_iter_next(&iter, &hash_key, &value) && 
		       (value != node));
		g_hash_table_remove(parent->children, hash_key);
	}

on_error:

	prv_provman_cache_key_free(&cache_key);

	return err;
}

int provman_cache_get(provman_cache_t *cache, const gchar *key, 
		      gchar **value)
{
	provman_cache_t *node;
	int err;
	GHashTableIter iter;
	GString *str;
	int i;
	int num_children;
	gpointer child;
	provman_cache_key_t cache_key;

	prv_provman_cache_key_init(&cache_key, key);
	
	err = prv_find_node(cache, cache_key.key, &node);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	if (node->children == NULL)
		*value = g_strdup(node->value);
	else {
		str = g_string_new("");
		num_children = g_hash_table_size(node->children);
		g_hash_table_iter_init(&iter, node->children);
		for (i = 0; i < num_children - 1 &&
			     g_hash_table_iter_next(&iter, &child, NULL); ++i) {
			g_string_append(str, (const gchar*) child);
			g_string_append(str, "/");
		}
		
		if (g_hash_table_iter_next(&iter, &child, NULL))
			g_string_append(str, (const gchar*) child);
		*value = g_string_free(str, FALSE);
	}
	
on_error:

	prv_provman_cache_key_free(&cache_key);

	return err;
}

static void prv_add_node_to_vb(const gchar* path, const gchar *value,
			       gpointer user_data)
{
	GVariantBuilder *vb = user_data;
	g_variant_builder_add(vb, "{ss}", path, value);
}

static void prv_add_node_to_ht(const gchar* path, const gchar *value,
			       gpointer user_data)
{
	GHashTable *settings = user_data;
	g_hash_table_insert(settings, g_strdup(path), g_strdup(value));
}

static void prv_visit_leaves_r(provman_cache_t *node, GString *path,
			       provman_cache_visit_cb_t cb, gpointer user_data)
{
	GHashTableIter iter;
	gpointer node_name;
	gpointer value;
	gsize path_len;
	
	if (node->children) {
		g_hash_table_iter_init(&iter, node->children);
		while (g_hash_table_iter_next(&iter, &node_name, &value)) {
			path_len = path->len;
			g_string_append_c(path, '/');
			g_string_append(path, node_name);
			prv_visit_leaves_r(value, path, cb, user_data);
			g_string_set_size(path, path_len);
		}		
	} else {
		cb(path->str, node->value, user_data);
	}
}

static int prv_visit_leaves(provman_cache_t *cache, const gchar *root,
			    provman_cache_visit_cb_t cb, gpointer user_data)
{
	provman_cache_key_t cache_key;
	GString* path;
	provman_cache_t *node;
	int err;

	prv_provman_cache_key_init(&cache_key, root);

	err = prv_find_node(cache, cache_key.key, &node);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	path = g_string_new(!cache_key.key[1] ? "" : cache_key.key);	
	prv_visit_leaves_r(node, path, cb, user_data);
	(void) g_string_free(path, TRUE);

on_error:

	prv_provman_cache_key_free(&cache_key);

	return err;
}

int provman_cache_get_all(provman_cache_t *cache, const gchar *root,
			  GVariant **variant)
{
	GVariantBuilder *vb;
	int err;

	vb = g_variant_builder_new(G_VARIANT_TYPE("a{ss}"));
	err = prv_visit_leaves(cache, root, prv_add_node_to_vb, vb);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	*variant = g_variant_builder_end(vb);
		
on_error:

	g_variant_builder_unref(vb);

	return err;
}

void provman_cache_delete(provman_cache_t *cache)
{
	if (cache) {
		if (cache->children)
			g_hash_table_unref(cache->children);
		else		
			g_free(cache->value);
		g_slice_free(provman_cache_t, cache);
	}
}

void provman_cache_add_settings(provman_cache_t *cache, GHashTable *settings)
{
	GHashTableIter iter;
	gpointer key;
	gpointer value;
		
	g_hash_table_iter_init(&iter, settings);
	while (g_hash_table_iter_next(&iter, &key, &value))
		(void) provman_cache_set(cache, key, value); 
}

GHashTable *provman_cache_get_settings(provman_cache_t *cache, const gchar *root)
{
	GHashTable *settings;

	settings = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
					 g_free);
	(void) prv_visit_leaves(cache, root, prv_add_node_to_ht, settings);

	return settings;
}

#ifdef PROVMAN_LOGGING
void provman_cache_dump_settings(provman_cache_t *cache, const gchar *key)
{
	GHashTableIter iter;
	gpointer node_name;
	gpointer value;
	gchar *key_name;

	if (cache->children) {
		g_hash_table_iter_init(&iter, cache->children);
		while (g_hash_table_iter_next(&iter, &node_name, &value)) {
			key_name = g_strdup_printf("%s/%s", key,
						   (const gchar*) node_name);
			provman_cache_dump_settings(value, key_name);
			g_free(key_name);
		}
	} else {
		PROVMAN_LOGF("%s = %s", key, cache->value);
	}
}
#endif

