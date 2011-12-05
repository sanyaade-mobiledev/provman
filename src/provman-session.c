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
 * @file provman-session.c
 *
 * @brief Main file for user instance of provman
 *
 ******************************************************************************/

#include "config.h"

#include <glib.h>

#include "provman.h"

int main(int argc, char *argv[])
{
	int retval;
	const char *log_name = NULL;

#ifdef PROVMAN_LOGGING
	GString *log_name_str;

	log_name_str = g_string_new(PROVMAN_SESSION_LOG);
	g_string_append(log_name_str, g_get_user_name());
	g_string_append(log_name_str, ".log");
	log_name = log_name_str->str;
#endif

	retval = provman_run(G_BUS_TYPE_SESSION, log_name);

#ifdef PROVMAN_LOGGING
	(void) g_string_free(log_name_str, TRUE);
#endif

	return retval;
}
