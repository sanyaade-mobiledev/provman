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
 * @file cache.h
 *
 * @brief Contains function declarations for managing a tree of settings
 *
 *****************************************************************************/

#ifndef PROVMAN_CACHE_H
#define PROVMAN_CACHE_H

#include <glib.h>
#include <stdbool.h>

typedef struct provman_cache_t_ provman_cache_t;

void provman_cache_new(provman_cache_t **cache);
int provman_cache_exists(provman_cache_t *cache, const gchar *key, bool *leaf);
int provman_cache_set(provman_cache_t *cache, const gchar *key,
		      const gchar *value);
int provman_cache_set_meta(provman_cache_t *cache, const gchar *key,
			   const gchar *prop, const gchar *value);
int provman_cache_get(provman_cache_t *cache, const gchar *key, gchar **value);
int provman_cache_get_meta(provman_cache_t *cache, const gchar *key,
			   const gchar *prop, gchar **value);
int provman_cache_remove(provman_cache_t *cache, const gchar *key);
void provman_cache_add_settings(provman_cache_t *cache, GHashTable *settings);
void provman_cache_add_meta_data(provman_cache_t *cache, GHashTable *meta_data);
GHashTable *provman_cache_get_settings(provman_cache_t *cache,
				       const gchar *root);
GHashTable *provman_cache_get_meta_data(provman_cache_t *cache,
					const gchar *root);
void provman_cache_delete(provman_cache_t *cache);
int provman_cache_get_all(provman_cache_t *cache, const gchar *root,
			  GVariant **variant);
int provman_cache_get_all_meta(provman_cache_t *cache, const gchar *root,
			       GVariant **variant);
#ifdef PROVMAN_LOGGING
void provman_cache_dump_settings(provman_cache_t *cache, const gchar *key);
#endif

#endif
