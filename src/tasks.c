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
 * @file tasks.c
 *
 * @brief Main file for the provisioning process's tasks.
 *
 ******************************************************************************/

#include "config.h"

#include "log.h"
#include "error.h"

#include "tasks.h"

#define PROV_ERROR_NOT_FOUND PROVMAN_SERVICE".Error.NotFound"
#define PROV_ERROR_BAD_KEY PROVMAN_SERVICE".Error.BadKey"
#define PROV_ERROR_UNKNOWN PROVMAN_SERVICE".Error.Unknown"


typedef struct provman_task_context_t_ provman_task_context_t;
struct provman_task_context_t_ {
	provman_task_sync_cb finished;
	void *finished_data;
	GDBusMethodInvocation *invocation;
};

void provman_task_new(provman_task_type type, GDBusMethodInvocation *invocation,
		      provman_task **task)
{
	provman_task *new_task = g_new0(provman_task, 1);

	new_task->type = type;
	new_task->invocation = invocation;

	*task = new_task;
}

void provman_task_delete(provman_task *task)
{
	if (task) {
		g_free(task->key);
		g_free(task->value);
		g_free(task->prop);
		if (task->variant)
			g_variant_unref(task->variant);
		if (task->invocation)
			g_dbus_method_invocation_return_dbus_error(
				task->invocation,
				PROVMAN_DBUS_ERR_DIED, "");
		g_free(task->imsi);
		g_free(task);
	}
}

static void prv_task_failed(int result, provman_task_context_t *task_context)
{
	PROVMAN_LOGF("%s called with error %u", __FUNCTION__, result);

	if (task_context->invocation)
		g_dbus_method_invocation_return_dbus_error(
			task_context->invocation, provman_err_to_dbus(result),
			"");

	g_free(task_context);
}

static void prv_task_finished(int result, void *user_data)
{
	provman_task_context_t *task_context = user_data;

	PROVMAN_LOGF("%s called with error %u", __FUNCTION__, result);

	if (task_context->invocation) {
		if (result == PROVMAN_ERR_NONE) {
			g_dbus_method_invocation_return_value(
				task_context->invocation, NULL);
		} else {
			g_dbus_method_invocation_return_dbus_error(
				task_context->invocation,
				provman_err_to_dbus(result), "");
		}
	}
	task_context->finished(result, task_context->finished_data);

	g_free(task_context);
}

static void prv_value_task_finished(int result, gchar *value, void *user_data)
{
	provman_task_context_t *task_context = user_data;

	PROVMAN_LOGF("%s called with error %u", __FUNCTION__, result);

	if (task_context->invocation) {
		if (result == PROVMAN_ERR_NONE) {
			g_dbus_method_invocation_return_value(
				task_context->invocation,
				g_variant_new("(s)", value));
			g_free(value);
		} else {
			g_dbus_method_invocation_return_dbus_error(
				task_context->invocation,
				provman_err_to_dbus(result), "");
		}
	}

	task_context->finished(result, task_context->finished_data);

	g_free(task_context);
}

static void prv_variant_task_finished(const char * type, int result,
				      GVariant *values, void *user_data)
{
	provman_task_context_t *task_context = user_data;

	PROVMAN_LOGF("%s called with error %u", __FUNCTION__, result);

	if (task_context->invocation) {
		if (result == PROVMAN_ERR_NONE) {
			g_dbus_method_invocation_return_value(
				task_context->invocation,
				g_variant_new(type, values));
			g_variant_unref(values);
		} else {
			g_dbus_method_invocation_return_dbus_error(
				task_context->invocation,
				provman_err_to_dbus(result), "");
		}
	}

	task_context->finished(result, task_context->finished_data);

	g_free(task_context);
}

static void prv_variant_sss_task_finished(int result, GVariant *values,
					  void *user_data)
{
	prv_variant_task_finished("(@a(sss))", result, values, user_data);
}

static void prv_variant_ss_task_finished(int result, GVariant *values,
					 void *user_data)
{
	prv_variant_task_finished("(@a{ss})", result, values, user_data);
}

static void prv_variant_ass_task_finished(int result, GVariant *values,
					  void *user_data)
{
	prv_variant_task_finished("(@a(ss))", result, values, user_data);
}

static void prv_variant_s_task_finished(int result, GVariant *values,
					void *user_data)
{
	prv_variant_task_finished("(@as)", result, values, user_data);
}

void provman_task_sync_in(plugin_manager_t *plugin_manager, provman_task *task)
{
	(void) plugin_manager_sync_in(plugin_manager, task->imsi);
}

bool provman_task_async_cancel(plugin_manager_t *plugin_manager)
{
	return plugin_manager_cancel(plugin_manager);
}

static void prv_provman_task_context_new(provman_task *task,
					 provman_task_sync_out_cb finished,
					 void *finished_data,
					 provman_task_context_t **task_context)
{
	provman_task_context_t *tc = g_new0(provman_task_context_t, 1);
	tc->finished = finished;
	tc->finished_data = finished_data;
	tc->invocation = task->invocation;
	*task_context = tc;
}

bool provman_task_sync_out(plugin_manager_t *plugin_manager,
			   provman_task *task,
			   provman_task_sync_out_cb finished,
			   void *finished_data)
{
	provman_task_context_t *task_context;
	int err = PROVMAN_ERR_NONE;

	prv_provman_task_context_new(task, finished, finished_data,
				     &task_context);
	task->invocation = NULL;

	err = plugin_manager_sync_out(plugin_manager, prv_task_finished,
				      task_context);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	return true;

on_error:

	prv_task_failed(err, task_context);

	return false;
}

bool provman_task_set(plugin_manager_t *manager, provman_task *task,
		      provman_task_sync_cb finished, void *finished_data)
{
	int err;
	provman_task_context_t *task_context;

	PROVMAN_LOGF("Processing Set task: %s=%s", task->key, task->value);

	prv_provman_task_context_new(task, finished, finished_data,
				     &task_context);
	task->invocation = NULL;

	err = plugin_manager_set(manager, task->key, task->value,
				 prv_task_finished, task_context);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	return true;

on_error:

	prv_task_failed(err, task_context);

	return false;
}

bool provman_task_set_multiple(plugin_manager_t *manager, provman_task *task,
			       provman_task_sync_cb finished,
			       void *finished_data)
{
	int err;
	provman_task_context_t *task_context;

	PROVMAN_LOG("Processing Set Multiple task");

	prv_provman_task_context_new(task, finished, finished_data,
				     &task_context);

	task->invocation = NULL;

	err = plugin_manager_set_multiple(manager, task->variant,
					  prv_variant_s_task_finished,
					  task_context);

	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	return true;

on_error:

	prv_task_failed(err, task_context);

	return false;
}

bool provman_task_set_multiple_meta(plugin_manager_t *manager,
				    provman_task *task,
				    provman_task_sync_cb finished,
				    void *finished_data)
{
	int err;
	provman_task_context_t *task_context;

	PROVMAN_LOG("Processing Set Meta Multiple task");

	prv_provman_task_context_new(task, finished, finished_data,
				     &task_context);

	task->invocation = NULL;

	err = plugin_manager_set_multiple_meta(manager, task->variant,
					       prv_variant_ass_task_finished,
					       task_context);

	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	return true;

on_error:

	prv_task_failed(err, task_context);

	return false;
}

bool provman_task_get(plugin_manager_t *manager, provman_task *task,
		      provman_task_sync_cb finished, void *finished_data)
{
	int err;
	provman_task_context_t *task_context;

	PROVMAN_LOGF("Processing Get task: %s", task->key);

	prv_provman_task_context_new(task, finished, finished_data,
				     &task_context);
	task->invocation = NULL;

	err = plugin_manager_get(manager, task->key, prv_value_task_finished,
				 task_context);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	return true;

on_error:

	prv_task_failed(err, task_context);

	return false;
}

bool provman_task_get_multiple(plugin_manager_t *manager, provman_task *task,
			       provman_task_sync_cb finished,
			       void *finished_data)
{
	int err;
	provman_task_context_t *task_context;

	PROVMAN_LOG("Processing Get Multiple task");

	prv_provman_task_context_new(task, finished, finished_data,
				     &task_context);

	task->invocation = NULL;

	err = plugin_manager_get_multiple(manager, task->variant,
					  prv_variant_ss_task_finished,
					  task_context);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	return true;

on_error:

	prv_task_failed(err, task_context);

	return false;
}

bool provman_task_get_all(plugin_manager_t *manager, provman_task *task,
			  provman_task_sync_cb finished, void *finished_data)
{
	int err;
	provman_task_context_t *task_context;

	PROVMAN_LOGF("Processing Get All task on key %s", task->key);

	prv_provman_task_context_new(task, finished, finished_data,
				     &task_context);

	task->invocation = NULL;
	err = plugin_manager_get_all(manager, task->key,
				     prv_variant_ss_task_finished,
				     task_context);

	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	return true;

on_error:

	prv_task_failed(err, task_context);

	return false;
}

bool provman_task_get_all_meta(plugin_manager_t *manager, provman_task *task,
			       provman_task_sync_cb finished,
			       void *finished_data)
{
	int err;
	provman_task_context_t *task_context;

	PROVMAN_LOGF("Processing Get All Meta task on key %s", task->key);

	prv_provman_task_context_new(task, finished, finished_data,
				     &task_context);

	task->invocation = NULL;

	err = plugin_manager_get_all_meta(manager, task->key,
					  prv_variant_sss_task_finished,
					  task_context);

	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	return true;

on_error:

	prv_task_failed(err, task_context);

	return false;
}


bool provman_task_remove(plugin_manager_t *manager, provman_task *task,
			 provman_task_sync_cb finished, void *finished_data)
{
	int err;
	provman_task_context_t *task_context;

	PROVMAN_LOGF("Processing Delete task: %s", task->key);

	prv_provman_task_context_new(task, finished, finished_data,
				     &task_context);
	task->invocation = NULL;

	err = plugin_manager_remove(manager, task->key, prv_task_finished,
				    task_context);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	return true;

on_error:

	prv_task_failed(err, task_context);

	return false;
}

bool provman_task_remove_multiple(plugin_manager_t *manager, provman_task *task,
				  provman_task_sync_cb finished,
				  void *finished_data)
{
	int err;
	provman_task_context_t *task_context;

	PROVMAN_LOG("Processing Delete Multiple task:");

	prv_provman_task_context_new(task, finished, finished_data,
				     &task_context);

	task->invocation = NULL;

	err = plugin_manager_remove_multiple(manager, task->variant,
					     prv_variant_s_task_finished,
					     task_context);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	return true;

on_error:

	prv_task_failed(err, task_context);

	return false;
}

void provman_task_abort(plugin_manager_t *plugin_manager, provman_task *task)
{
	int err;

	PROVMAN_LOG("Processing Abort task");

	err = plugin_manager_abort(plugin_manager);

	if (err == PROVMAN_ERR_NONE)
		g_dbus_method_invocation_return_value(task->invocation, NULL);
	else
		g_dbus_method_invocation_return_dbus_error(
			task->invocation, provman_err_to_dbus(err), "");

	task->invocation = NULL;
}

void provman_task_get_children_type_info(plugin_manager_t *manager,
					 provman_task *task)
{
	int err;
	GVariant *array;

	PROVMAN_LOG("Processing Get Children Type Info task");

	err = plugin_manager_get_children_type_info(manager, task->key,
						    &array);

	if (err == PROVMAN_ERR_NONE)
		g_dbus_method_invocation_return_value(
			task->invocation,
			g_variant_new("(@a{ss})",array));
	else
		g_dbus_method_invocation_return_dbus_error(
			task->invocation, provman_err_to_dbus(err), "");

	task->invocation = NULL;
}

void provman_task_get_type_info(plugin_manager_t *manager,
				provman_task *task)
{
	int err;
	gchar *type_info;

	PROVMAN_LOG("Processing Get Type Info task");

	err = plugin_manager_get_type_info(manager, task->key, &type_info);

	if (err == PROVMAN_ERR_NONE) {
		g_dbus_method_invocation_return_value(
			task->invocation,
			g_variant_new("(s)",type_info));
		g_free(type_info);
	}
	else
		g_dbus_method_invocation_return_dbus_error(
			task->invocation, provman_err_to_dbus(err), "");

	task->invocation = NULL;
}

bool provman_task_set_meta(plugin_manager_t *manager, provman_task *task,
			   provman_task_sync_cb finished, void *finished_data)
{
	int err;
	provman_task_context_t *task_context;

	PROVMAN_LOGF("Processing Set Meta task: %s?%s=%s", task->key,
		     task->prop, task->value);

	prv_provman_task_context_new(task, finished, finished_data,
				     &task_context);
	task->invocation = NULL;

	err = plugin_manager_set_meta(manager, task->key, task->prop,
				      task->value, prv_task_finished,
				      task_context);

	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	return true;

on_error:

	prv_task_failed(err, task_context);

	return false;
}

bool provman_task_get_meta(plugin_manager_t *manager, provman_task *task,
			   provman_task_sync_cb finished, void *finished_data)
{
	int err;
	provman_task_context_t *task_context;

	PROVMAN_LOGF("Processing Get Meta task: %s?%s", task->key, task->prop);

	prv_provman_task_context_new(task, finished, finished_data,
				     &task_context);
	task->invocation = NULL;

	err = plugin_manager_get_meta(manager, task->key, task->prop,
				      prv_value_task_finished,
				      task_context);
	if (err != PROVMAN_ERR_NONE)
		goto on_error;

	return true;

on_error:

	prv_task_failed(err, task_context);

	return false;
}

void provman_task_get_version(plugin_manager_t *manager, provman_task *task)
{
	PROVMAN_LOGF("Retrieving Provman Version %s", PACKAGE_VERSION);

	g_dbus_method_invocation_return_value(task->invocation,
					      g_variant_new("(s)",
							    PACKAGE_VERSION));
	task->invocation = NULL;
}

