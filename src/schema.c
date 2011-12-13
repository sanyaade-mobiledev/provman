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
 * @file schema.c
 *
 * @brief Functions for creating and consulting schemas
 *
 *****************************************************************************/

#include "config.h"

#include <string.h>

#include "log.h"
#include "error.h"
#include "schema.h"

typedef struct schema_context_t_ schema_context_t;
struct schema_context_t_ {
	GPtrArray *stack;
	provman_schema_t *root;
};

#define PROVMAN_SCHEMA_ERROR prv_schema_error_quark()
static GQuark prv_schema_error_quark (void)
{
	return g_quark_from_static_string("g-schema-error-quark");
}

static void prv_provman_schema_delete(gpointer data)
{
	provman_schema_delete(data);
}

static provman_schema_t *prv_provman_schema_dir_new(const gchar *name,
						    gboolean can_delete)
{
	provman_schema_t *dir = g_slice_new0(provman_schema_t);

	dir->name = g_strdup(name);
	dir->type = PROVMAN_SCHEMA_TYPE_DIR;
	dir->can_delete = can_delete;
	dir->dir.children =
		g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
				      prv_provman_schema_delete);

	return dir;
}

static provman_schema_t *prv_provman_schema_key_new(
	const gchar *name,
	gboolean can_delete,
	gboolean can_write,
	provman_schema_value_type_t
	type,
	const gchar *allowed_values_str)
{
	provman_schema_t *key = g_slice_new0(provman_schema_t);
	gchar **allowed_values;
	unsigned int i;

	key->name = g_strdup(name);
	key->type = PROVMAN_SCHEMA_TYPE_KEY;
	key->can_delete = can_delete;
	key->key.type = type;
	key->key.can_write = can_write;

	if ((type == PROVMAN_SCHEMA_VALUE_TYPE_ENUM) && allowed_values_str) {
		key->key.allowed_values =
			g_hash_table_new_full(g_str_hash, g_str_equal,
					      g_free, NULL);
		allowed_values = g_strsplit(allowed_values_str, ",", 0);
		for (i = 0; allowed_values[i]; ++i) {
			g_strstrip(allowed_values[i]);
			g_hash_table_insert(key->key.allowed_values,
					    allowed_values[i], NULL);
		}
		g_free(allowed_values);
	}

	return key;
}

static void prv_parse_schema(const gchar *element_name,
			     const gchar **attribute_names,
			     const gchar **attribute_values,
			     gpointer user_data, GError **error)
{
	schema_context_t *context = user_data;
	const gchar *root;
	unsigned int root_len;

	if (context->root) {
		g_set_error(error, PROVMAN_SCHEMA_ERROR, 0,
			    "Schema tag must be root");
		goto on_error;
	}

	if (!g_markup_collect_attributes(element_name, attribute_names,
					 attribute_values, error,
					 G_MARKUP_COLLECT_STRING,
					 "root", &root,
					 G_MARKUP_COLLECT_INVALID))
		goto on_error;

	root_len = strlen(root);

	if ((root_len == 0) || !strcmp(root, "/") ||
	    root[root_len - 1] != '/') {
		g_set_error(error, PROVMAN_SCHEMA_ERROR, 0,
			    "Invalid plugin root: %s", root);
		goto on_error;
	}

	context->root = prv_provman_schema_dir_new(root, TRUE);
	g_ptr_array_add(context->stack, context->root);

on_error:

	return;
}

static gboolean prv_parse_bool_att(const gchar *value, gboolean default_val,
				   gboolean *outval, GError **error)
{
	gboolean flag = default_val;
	gboolean retval = TRUE;

	if (value) {
		if (!strcmp(value, "yes")) {
			flag = TRUE;
		} else if (!strcmp(value, "no")) {
			flag = FALSE;
		} else {
			retval = FALSE;
			g_set_error(error, PROVMAN_SCHEMA_ERROR, 0,
				    "Unrecognised value %s", value);
			goto on_error;
		}
	}

	*outval = flag;

on_error:

	return retval;
}

static gboolean prv_parse_type_att(const gchar *value,
				   provman_schema_value_type_t *type,
				   GError **error)
{
	gboolean retval = TRUE;

	if (!strcmp(value, "string")) {
		*type = PROVMAN_SCHEMA_VALUE_TYPE_STRING;
	} else if (!strcmp(value, "int")) {
		*type = PROVMAN_SCHEMA_VALUE_TYPE_INT;
	}  else if (!strcmp(value, "enum")) {
		*type = PROVMAN_SCHEMA_VALUE_TYPE_ENUM;
	} else {
		retval = FALSE;
		g_set_error(error, PROVMAN_SCHEMA_ERROR, 0,
			    "Unrecognised type %s", value);
	}

	return retval;
}


static void prv_parse_dir(const gchar *element_name,
			  const gchar **attribute_names,
			  const gchar **attribute_values,
			  gpointer user_data,
			  GError **error)
{
	schema_context_t *context = user_data;
	const gchar *name;
	const gchar *delete;
	gboolean can_delete;
	provman_schema_t *schema;
	provman_schema_t *parent;
	GHashTable *children;

	if (!g_markup_collect_attributes(element_name, attribute_names,
					 attribute_values, error,
					 G_MARKUP_COLLECT_STRING |
					 G_MARKUP_COLLECT_OPTIONAL,
					 "name", &name,
					 G_MARKUP_COLLECT_STRING  |
					 G_MARKUP_COLLECT_OPTIONAL,
					 "delete", &delete,
					 G_MARKUP_COLLECT_INVALID))
		goto on_error;


	if (!prv_parse_bool_att(delete, TRUE, &can_delete, error))
		goto on_error;

	parent = g_ptr_array_index(context->stack, context->stack->len - 1);
	children = parent->dir.children;

	if (!name)
		name = "";

	if (!name[0] && g_hash_table_size(children) > 0) {
		g_set_error(error, PROVMAN_SCHEMA_ERROR, 0,
			    "Unnamed directories must be only children");
		goto on_error;
	}

	if (g_hash_table_lookup_extended(children, name, NULL, NULL)) {
		g_set_error(error, PROVMAN_SCHEMA_ERROR, 0,
			    "Entry %s already exists", name[0] ? name : "<X>");
		goto on_error;
	}

	schema = prv_provman_schema_dir_new(name, can_delete);
	g_hash_table_insert(children, g_strdup(name), schema);
	g_ptr_array_add(context->stack, schema);

on_error:

	return;
}

static void prv_parse_key(const gchar *element_name,
			  const gchar **attribute_names,
			  const gchar **attribute_values,
			  gpointer user_data,
			  GError **error)
{
	schema_context_t *context = user_data;
	const gchar *name;
	const gchar *type;
	const gchar *delete;
	const gchar *write;
	const gchar *values;
	gboolean can_delete;
	gboolean can_write;
	provman_schema_value_type_t value_type;
	provman_schema_t *schema;
	provman_schema_t *parent;
	GHashTable *children;

	if (!g_markup_collect_attributes(element_name, attribute_names,
					 attribute_values, error,
					 G_MARKUP_COLLECT_STRING,
					 "name", &name,
					 G_MARKUP_COLLECT_STRING,
					 "type", &type,
					 G_MARKUP_COLLECT_STRING  |
					 G_MARKUP_COLLECT_OPTIONAL,
					 "delete", &delete,
					 G_MARKUP_COLLECT_STRING  |
					 G_MARKUP_COLLECT_OPTIONAL,
					 "write", &write,
					 G_MARKUP_COLLECT_STRING  |
					 G_MARKUP_COLLECT_OPTIONAL,
					 "values", &values,
					 G_MARKUP_COLLECT_INVALID))
		goto on_error;

	if (!prv_parse_type_att(type, &value_type, error))
		goto on_error;

	if (!prv_parse_bool_att(delete, FALSE, &can_delete, error))
		goto on_error;

	if (!prv_parse_bool_att(write, TRUE, &can_write, error))
		goto on_error;

	parent = g_ptr_array_index(context->stack, context->stack->len - 1);
	children = parent->dir.children;

	if (g_hash_table_lookup(children, name)) {
		g_set_error(error, PROVMAN_SCHEMA_ERROR, 0,
			    "Entry %s already exists", name ? name : "<X>");
		goto on_error;
	}

	schema = prv_provman_schema_key_new(name, can_delete, can_write,
					    value_type, values);

	g_hash_table_insert(children, g_strdup(name), schema);

on_error:

	return;
}


static void prv_start_element(GMarkupParseContext *pcontext,
			      const gchar *element_name,
			      const gchar **attribute_names,
			      const gchar **attribute_values,
			      gpointer user_data,
			      GError **error)
{
	schema_context_t *context = user_data;
	provman_schema_t *parent;
	GHashTable *children;

	if (!strcmp(element_name, "schema"))
		prv_parse_schema(element_name, attribute_names,
				 attribute_values, user_data, error);
	else {
		if (!context->root) {
			g_set_error(error, PROVMAN_SCHEMA_ERROR, 0,
				    "Schema must be the first tag");
			goto on_error;
		}

		parent = g_ptr_array_index(context->stack,
					   context->stack->len - 1);
		children = parent->dir.children;
		if (g_hash_table_lookup_extended(children, "", NULL, NULL)) {
			g_set_error(error, PROVMAN_SCHEMA_ERROR, 0,
				    "Unnamed directory exists at "
				    "this level");
			goto on_error;
		}

		if (!strcmp(element_name, "dir"))
			prv_parse_dir(element_name, attribute_names,
				      attribute_values, user_data, error);
		else if (!strcmp(element_name, "key"))
			prv_parse_key(element_name, attribute_names,
				      attribute_values, user_data, error);
		else
			g_set_error(error, PROVMAN_SCHEMA_ERROR, 0,
				    "Unrecognised tag %s", element_name);
	}

on_error:

	return;
}

static void prv_end_element(GMarkupParseContext *pcontext,
			    const gchar *element_name,
			    gpointer user_data,
			    GError **error)
{
	schema_context_t *context = user_data;

	if (!strcmp(element_name, "schema")) {
		if (context->stack->len != 1) {
			g_set_error(error, PROVMAN_SCHEMA_ERROR, 0,
				    "Unexpected </schema> tag");
			goto on_error;
		}
		(void) g_ptr_array_remove_index(context->stack,
						context->stack->len - 1);
	} else if (!strcmp(element_name, "dir")) {
		if (context->stack->len < 2) {
			g_set_error(error, PROVMAN_SCHEMA_ERROR, 0,
				    "Unexpected </dir> tag");
			goto on_error;
		}
		(void) g_ptr_array_remove_index(context->stack,
						context->stack->len - 1);
	} else if (strcmp(element_name, "key")) {
		g_set_error(error, PROVMAN_SCHEMA_ERROR, 0,
			    "Unknown end tag %s", element_name);
		goto on_error;
	}

on_error:

	return;
}

int provman_schema_new(const gchar *schema_xml, guint schema_xml_len,
		       provman_schema_t **schema)
{
	int err = PROVMAN_ERR_NONE;
	GMarkupParser parser;
	GMarkupParseContext *parse_context;
	schema_context_t context;
	GError *error = NULL;

	memset(&parser, 0, sizeof(parser));

	parser.start_element = prv_start_element;
	parser.end_element = prv_end_element;
	context.stack = g_ptr_array_new();
	context.root = NULL;

	parse_context = g_markup_parse_context_new(
		&parser,
		G_MARKUP_PREFIX_ERROR_POSITION,
		&context, NULL);

	if (!g_markup_parse_context_parse(parse_context, schema_xml,
					  schema_xml_len,
					  &error)) {
		PROVMAN_LOGF("Unable to parse schema: %s ",
			     error ? error->message : "Unknown");
		if (error)
			g_error_free(error);

		err = PROVMAN_ERR_CORRUPT;
		goto on_error;
	}

	if (context.stack->len != 0) {
		PROVMAN_LOG("Unbalanced Schema");
		err = PROVMAN_ERR_CORRUPT;
		goto on_error;
	}

#ifdef PROVMAN_LOGGING
	provman_schema_dump(context.root);
#endif

	*schema = context.root;
	context.root = NULL;

on_error:
	g_markup_parse_context_free(parse_context);
	provman_schema_delete(context.root);
	g_ptr_array_free(context.stack, TRUE);

	return err;
}

void provman_schema_delete(provman_schema_t *schema)
{
	if (schema) {
		g_free(schema->name);
		if (schema->type == PROVMAN_SCHEMA_TYPE_DIR) {
			g_hash_table_unref(schema->dir.children);
		} else {
			if (schema->key.allowed_values)
				g_hash_table_unref(schema->key.allowed_values);
		}
		g_slice_free(provman_schema_t, schema);
	}
}

static int provman_find_schema(provman_schema_t *parent, const gchar *path,
			       provman_schema_t **schema)
{
	int err = PROVMAN_ERR_NONE;
	const gchar *name;
	gchar *name_cpy = NULL;
	GHashTable *children = parent->dir.children;
	const gchar *slash = strchr(path, '/');
	gpointer value;

	if (!slash) {
		name = path;
	} else {
		name_cpy = g_strndup(path, slash - path);
		name = name_cpy;
	}

	if (!g_hash_table_lookup_extended(children, name, NULL, &value)) {
		if (!g_hash_table_lookup_extended(children, "", NULL, &value)) {
			err = PROVMAN_ERR_NOT_FOUND;
			goto on_error;
		}
	}

	if (!slash || !slash[1]) {
		*schema = value;
	} else {
		err = provman_find_schema(value, slash + 1, schema);
		if (err != PROVMAN_ERR_NONE)
			goto on_error;
	}

on_error:

	g_free(name_cpy);

	return err;
}

int provman_schema_locate(provman_schema_t *root, const gchar *path,
			  provman_schema_t **schema)
{
	int err = PROVMAN_ERR_NONE;
	unsigned int path_len = strlen(path);
	unsigned int root_len = strlen(root->name) - 1;
	unsigned int i;

	if (path_len < root_len) {
		err = PROVMAN_ERR_NOT_FOUND;
		goto on_error;
	}

	if (strncmp(path, root->name, root_len)) {
		err = PROVMAN_ERR_NOT_FOUND;
		goto on_error;
	}

	i = root_len;

	if (i < path_len) {
		if (path[i] != '/') {
			err = PROVMAN_ERR_NOT_FOUND;
			goto on_error;
		}

		++i;

		if (i < path_len) {
			err = provman_find_schema(root, &path[i], schema);
			if (err != PROVMAN_ERR_NONE)
				goto on_error;
		} else {
			*schema = root;
		}
	} else {
		*schema = root;
	}

on_error:

	return err;
}

int provman_schema_check_value(provman_schema_t *schema, const gchar *value)
{
	int err = PROVMAN_ERR_NONE;
	provman_schema_value_type_t vt;

	if (schema->type != PROVMAN_SCHEMA_TYPE_KEY) {
		err = PROVMAN_ERR_BAD_KEY;
		goto on_error;
	}

	vt = schema->key.type;

	if (vt == PROVMAN_SCHEMA_VALUE_TYPE_INT) {
		if (!g_regex_match_simple("^[0-9]+$", value, 0, 0)) {
			err = PROVMAN_ERR_BAD_ARGS;
			goto on_error;
		}
	} else if (vt == PROVMAN_SCHEMA_VALUE_TYPE_ENUM) {
		if (!g_hash_table_lookup_extended(schema->key.allowed_values,
						  value, NULL, NULL))
		{
			err = PROVMAN_ERR_BAD_ARGS;
			goto on_error;
		}
	}

on_error:

	return err;
}

#ifdef PROVMAN_LOGGING

static void prv_provman_dump_schema(provman_schema_t *schema,
				    unsigned int depth)
{
	gchar *indent = g_strnfill(depth, '\t');
	GHashTable *children;
	GHashTableIter iter;
	unsigned int i;
	const char *type;
	GString *enum_values;
	gpointer value;
	GHashTable *allowed_values;
	gpointer key;

	if (schema->type == PROVMAN_SCHEMA_TYPE_DIR) {
		PROVMAN_LOGUF("%sdir name='%s' delete='%s'", indent,
			      schema->name[0] ? schema->name : "<X>",
			      schema->can_delete ? "yes" : "no");
		children = schema->dir.children;
		g_hash_table_iter_init(&iter, children);
		while (g_hash_table_iter_next(&iter, NULL, &value))
			prv_provman_dump_schema(value, depth + 1);
	} else {
		if (schema->key.type == PROVMAN_SCHEMA_VALUE_TYPE_STRING)
			type = "string";
		else if (schema->key.type == PROVMAN_SCHEMA_VALUE_TYPE_INT)
			type = "int";
		else
			type = "enum";

		PROVMAN_LOGUF(
			"%skey name='%s' delete='%s' write='%s' type='%s'",
			indent, schema->name,
			schema->can_delete ? "yes" : "no",
			schema->key.can_write ? "yes" : "no",
			type);
		if (schema->key.type == PROVMAN_SCHEMA_VALUE_TYPE_ENUM) {
			enum_values = g_string_new("");
			allowed_values = schema->key.allowed_values;
			g_hash_table_iter_init(&iter, allowed_values);
			for (i = 0; i < g_hash_table_size(allowed_values) - 1;
			     ++i) {
				if (g_hash_table_iter_next(&iter, &key, NULL)) {
					g_string_append(enum_values, key);
					g_string_append(enum_values, ", ");
				}
			}

			if (g_hash_table_iter_next(&iter, &key, NULL))
				g_string_append(enum_values, key);

			PROVMAN_LOGUF("%s\t(%s)", indent, enum_values->str);
			g_string_free(enum_values, TRUE);
		}
	}

	g_free(indent);
}

void provman_schema_dump(provman_schema_t *schema)
{
	GHashTable *children;
	GHashTableIter iter;
	gpointer value;

	PROVMAN_LOGUF("Schema: name='%s' delete='%s'", schema->name,
		      schema->can_delete ? "yes" : "no");

	children = schema->dir.children;
	g_hash_table_iter_init(&iter, children);
	while (g_hash_table_iter_next(&iter, NULL, &value))
		prv_provman_dump_schema(value, 1);
}
#endif

