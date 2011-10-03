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
 * @file meta_data.h
 *
 * @brief contains definitions for the storage and retrieval of meta data
 *
 * Any number of meta data properties can be assigned to an existing keys
 * (settings or directories), by clients via the #SetMeta function.  Each
 * meta property has a single string value assoicated with it.  For example,
 * the key \a /applications/email/test may have a property called \a ACL with
 * a value \a Add=*&Get=* associated with it.
 *
 * Provman supports meta data as a means for separate device management clients
 * to communicate with each other.  It does not currently define or reserve any
 * property names, such as \a ACL, nor do any of the property names have any
 * special meaning to provman.  Thus management clients are currently free to
 * use whatever property names they choose.  Needless to say, clients that
 * wish to communicate with each other will need to agree on a naming protocol.
 *
 *****************************************************************************/

#ifndef PROVMAN_META_DATA_H
#define PROVMAN_META_DATA_H

#include <glib.h>

typedef struct provman_meta_data_t_ provman_meta_data_t;

void provman_meta_data_new(const gchar *fname, provman_meta_data_t **meta_data);
void provman_meta_data_delete(provman_meta_data_t *meta_data);
GHashTable *provman_meta_data_get_all(provman_meta_data_t *meta_data);
void provman_meta_data_update(provman_meta_data_t *meta_data,
			      GHashTable *md_settings);
#endif
