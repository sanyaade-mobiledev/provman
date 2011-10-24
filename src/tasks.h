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
 * @file tasks.h
 *
 * @brief contains definitions for the provisiong process tasks
 *
 *****************************************************************************/

#ifndef PROVMAN_TASKS_H
#define PROVMAN_TASKS_H

#include <stdbool.h>
#include <gio/gio.h>

#include "plugin_manager.h"

enum provman_task_type_ {
	PROVMAN_TASK_SYNC_IN,
	PROVMAN_TASK_SYNC_OUT,
	PROVMAN_TASK_SET,
	PROVMAN_TASK_GET,
	PROVMAN_TASK_GET_MULTIPLE,
	PROVMAN_TASK_SET_MULTIPLE,
	PROVMAN_TASK_SET_MULTIPLE_META,
	PROVMAN_TASK_GET_ALL,
	PROVMAN_TASK_GET_ALL_META,
	PROVMAN_TASK_DELETE,
	PROVMAN_TASK_DELETE_MULTIPLE,
	PROVMAN_TASK_ABORT,
	PROVMAN_TASK_GET_CHILDREN_TYPE_INFO,
	PROVMAN_TASK_GET_TYPE_INFO,
	PROVMAN_TASK_SET_META,
	PROVMAN_TASK_GET_META
};

typedef enum provman_task_type_ provman_task_type;

typedef struct provman_task_ provman_task;
struct provman_task_ {
	provman_task_type type;
	GDBusMethodInvocation *invocation;
	gchar *imsi;
	gchar *key;
	gchar *value;
	gchar *prop;
	GVariant *variant;
};

typedef void (*provman_task_sync_in_cb)(
	int result, void *user_data);

typedef void (*provman_task_sync_out_cb)(
	int result, void *user_data);

void provman_task_new(provman_task_type type, GDBusMethodInvocation *invocation,
		      provman_task **task);
void provman_task_delete(provman_task *task);

bool provman_task_sync_in(plugin_manager_t *plugin_manager,
			       provman_task *task,
			       provman_task_sync_in_cb finished,
			       void *finished_data);
void provman_task_set(plugin_manager_t *manager, provman_task *task);
void provman_task_set_multiple(plugin_manager_t *manager, provman_task *task);
void provman_task_set_multiple_meta(plugin_manager_t *manager,
				    provman_task *task);
void provman_task_get_all(plugin_manager_t *manager, provman_task *task);
void provman_task_get_all_meta(plugin_manager_t *manager, provman_task *task);
void provman_task_get(plugin_manager_t *manager, provman_task *task);
void provman_task_get_multiple(plugin_manager_t *manager, provman_task *task);
void provman_task_remove(plugin_manager_t *manager, provman_task *task);
void provman_task_remove_multiple(plugin_manager_t *manager,
				  provman_task *task);
bool provman_task_sync_out(plugin_manager_t *plugin_manager,
				provman_task *task,
				provman_task_sync_out_cb finished,
				void *finished_data);
bool provman_task_async_cancel(plugin_manager_t *plugin_manager);
void provman_task_abort(plugin_manager_t *plugin_manager, provman_task *task);
void provman_task_get_children_type_info(plugin_manager_t *manager,
					 provman_task *task);
void provman_task_get_type_info(plugin_manager_t *manager,
				provman_task *task);
void provman_task_set_meta(plugin_manager_t *manager, provman_task *task);
void provman_task_get_meta(plugin_manager_t *manager, provman_task *task);


#endif
