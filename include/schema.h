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
 * @file schema.h
 *
 * @brief Functions for creating and consulting schemas
 *
 *****************************************************************************/

#ifndef PROVMAN_SCHEMA_H
#define PROVMAN_SCHEMA_H

#include <glib.h>

enum provman_schema_type_t_ {
	PROVMAN_SCHEMA_TYPE_DIR,
	PROVMAN_SCHEMA_TYPE_KEY
};
typedef enum provman_schema_type_t_ provman_schema_type_t;

enum provman_schema_value_type_t_ {
	PROVMAN_SCHEMA_VALUE_TYPE_STRING,
	PROVMAN_SCHEMA_VALUE_TYPE_INT,
	PROVMAN_SCHEMA_VALUE_TYPE_ENUM
};
typedef enum provman_schema_value_type_t_ provman_schema_value_type_t;

typedef struct provman_schema_t_ provman_schema_t;

typedef struct provman_schema_dir_t_ provman_schema_dir_t;
struct provman_schema_dir_t_ {
	GHashTable *children;
};

typedef struct provman_schema_key_t_ provman_schema_key_t;
struct provman_schema_key_t_ {
	provman_schema_value_type_t type;
	GHashTable *allowed_values;
	gboolean can_write;
};

struct provman_schema_t_ {
	provman_schema_type_t type;
	gchar *name;
	gboolean can_delete;
	union {
		provman_schema_dir_t dir;
		provman_schema_key_t key;
	};
};

int provman_schema_new(const gchar *schema_xml, guint schema_xml_len,
		       provman_schema_t **schema);
void provman_schema_delete(provman_schema_t *schema);

int provman_schema_locate(provman_schema_t *root, const gchar *path,
			  provman_schema_t **schema);
int provman_schema_check_value(provman_schema_t *schema, const gchar *value);


#ifdef PROVMAN_LOGGING
void provman_schema_dump(provman_schema_t *schema);
#endif

#endif
