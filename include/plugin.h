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
 * @file plugin.h
 *
 * @brief contains definitions for provman plugins
 *
 *****************************************************************************/

#ifndef PROVMAN_PLUGIN_H
#define PROVMAN_PLUGIN_H

#include <stdbool.h>
#include <glib.h>

/*!
 * @brief Handle to a provman plugin instance.
 */

typedef void *provman_plugin_instance;

/*!
 * @brief Typedef for the callback function that plugins invoke when they
 *        want to complete a call to #provman_plugin_sync_in.
 *
 * @param result an error code indicating whether the call to
 * #provman_plugin_sync_in could be successfully completed.
 * @param settings A GHashTable containing all the settings obtained
 *        by the plugin from the middleware during the call to
 *        #provman_plugin_sync_in.  If result indicates an
 *        error this parameter should be NULL.
 * @param user_data This parameter should contain the data that
 *        provman passed to the #provman_plugin_sync_in
 *        in the user_data parameter.
 *
 */
typedef void (*provman_plugin_sync_in_cb)(int result, GHashTable* settings,
					  void *user_data);
/*!
 * @brief Typedef for the callback function that plugins invoke when they
 *        want to complete a call to #provman_plugin_sync_out.
 *
 * @param result an error code indicating whether the call to
 * #provman_plugin_sync_out could be successfully completed.
 * @param user_data This parameter should contain the data that
 *        provman passed to the #provman_plugin_sync_in
 *        in the user_data parameter.
 *
 */

typedef void (*provman_plugin_sync_out_cb)(int result, void *user_data);

/*!
 * @brief Typedef for a function pointer used to construct a new plugin
 *        instance.
 *
 * Each plugin needs to implement a function matching this
 * prototype.  It will be called when provman
 * first starts.  The function is synchronous so the plugin
 * should avoid performing any time consuming task in this
 * function.
 * @param instance A pointer to the new plugin instance is returned via
 *                 parameter upon the successful execution of the function.
 *        in the user_data parameter.
 * @param system Indicates whether the provman instance is running on
 *        the system d-Bus
 * @returns result an error code indicating whether or not the plugin
 *        instance could be created.
 *
 */

typedef int (*provman_plugin_new)(provman_plugin_instance *instance,
				  bool system);

/*!
 * @brief Typedef for a function pointer used to destroy a plugin
 *        instance.
 *
 * Each plugin needs to implement a function matching this
 * prototype.  It will be called when provman
 * is shutting down.
 *
 * @param instance A pointer to the plugin instance.
 *
 */

typedef void (*provman_plugin_delete)(provman_plugin_instance instance);

/*!
 * @brief Typedef for a function pointer that is called when a device
 *         management client initiates a new management session.
 *
 * Each plugin needs to implement a function matching this
 * prototype.  It will be called when provman
 * initiates a new session in response to a client's invocation of
 * the #Start D-Bus method.  This method is asynchronous as it may
 * take the plugin a substantial amount of time to retrieve the
 * data it needs from the middleware and we do not want it to block
 * provman while it does so.  At some time in the future provman will
 * inform the plugin that the session has finished by calling
 * either #provman_plugin_sync_out or #provman_plugin_abort.
 *
 * @param instance A pointer to the plugin instance.
 * @param imsi The imsi number specified by the client in its call to #Start.
 *        If the plugin does not support SIM specific settings it can
 *        ignore this parameter.  A special value of "", i.e., the
 *        empty string, is defined to mean the IMSI of the first available
 *        modem.  If this paramater is set to "" and the plugin supports SIM
 *        specific settings it must associate all settings in
 *        the management session with the SIM card of the first modem
 *        discovered in the device.
 * @param callback A function pointer that must be invoked by the plugin when
 *        it has completed the #provman_plugin_sync_in task.  If the plugin
 *        returns PROVMAN_ERR_NONE for the call to #provman_plugin_sync_in it
 *        must invoke this function to inform provman whether
 *        the request has been successfully completed.
 * @param user_data A pointer to some data specific to provman.
 *        The plugin must store this data somewhere and pass it back to
 *        provman when it calls callback.
 *
 * @return PROVMAN_ERROR_NONE The plugin has successfully initiated the sync
 *         in request. It will invoke callback at some point in the future to
 *         indicate whether
 *         or not the request succeeded.
 * @return PROVMAN_ERROR_* The plugin instance could not initiate the sync in
 *         request. The request has failed and the callback will not be invoked.
 */

typedef int (*provman_plugin_sync_in)(
	provman_plugin_instance instance, const char *imsi,
	provman_plugin_sync_in_cb callback,
	void *user_data);

/*!
 * @brief Typedef for a function pointer that is called when provman
 *  wishes to cancel a previous call to  #provman_plugin_sync_in.
 *
 * This may happen because the client has cancelled its Start Request or
 * because provman has been asked to shutdown.
 *
 * All plugin instances must implement this function.
 *
 * @param instance A pointer to the plugin instance.
 */

typedef void (*provman_plugin_sync_in_cancel)(
	provman_plugin_instance instance);

/*!
 * @brief Typedef for a function pointer that is called when a device
 *        management client completes a management session.
 *
 * Each plugin needs to implement a function matching this
 * prototype.  It will be called when provman
 * completes a session in response to a client's invocation of
 * the #End D-Bus method.  This method is asynchronous as it may
 * take the plugin a substantial amount of time to make the appropriate
 * modifications to the middleware.
 *
 * The plugin needs to compare the set of settings it receives in the settings
 * parameter to the current state of the data store of the middleware that it
 * manages.  Once the comparison is done it needs to modify the middleware data
 * store so that it corresponds to the settings contained with the settings
 * parameter.  Doing so may involve adding and deleting accounts or changing the
 * values of various account parameters.
 *
 * @param instance A pointer to the plugin instance.
 * @param settings A GHashTable that reflects the state of the plugin's
 *        settings at the end of the management session.
 * @param callback A function pointer that must be invoked by the plugin when
 *        it has completed the #provman_plugin_sync_out task.  If the
 *        plugin returns PROVMAN_ERR_NONE for the call to
 *        #provman_plugin_sync_out it must invoke this function
 *        to inform provman whether the request has been
 *        successfully completed.
 * @param user_data A pointer to some data specific to provman.
 *        The plugin must store this data somewhere and pass it back
 *        to provman when it calls callback.
 *
 * @return PROVMAN_ERROR_NONE The plugin has successfully initiated the sync out
 *         request it will invoke callback at some point in the future to
 *         indicate whether or not the request succeeded.
 * @return PROVMAN_ERROR_* The plugin instance could not initiate the sync out
 *         request. The request has failed and the callback will not be invoked.
 */

typedef int (*provman_plugin_sync_out)(
	provman_plugin_instance instance, GHashTable* settings,
	provman_plugin_sync_out_cb callback, void *user_data);

/*!
 * @brief Typedef for a function pointer that is called when provman
 *        wishes to cancel a previous call to  #provman_plugin_sync_out.
 *
 * This may happen because the client has cancelled its End Request or
 * because provman has been asked to shutdown.
 *
 * All plugin instances must implement this function.
 *
 * @param instance A pointer to the plugin instance.
 */

typedef void (*provman_plugin_sync_out_cancel)(
	provman_plugin_instance instance);

/*!
 * @brief Typedef for a function pointer that is called when provman
 *        is aborting a management session.
 *
 * The plugin should free any data that it has allocated for the
 * current session.
 *
 * Implementation of this method is optional.  If the plugin does
 * not maintain any session specific state or resources it does
 * not need to implement this function.
 *
 * @param instance A pointer to the plugin instance.
 */

typedef void (*provman_plugin_abort)(
	provman_plugin_instance instance);

/*!
 * @brief Typedef for a function pointer that informs provman of the
 *  SIM idenitifer currently being used by the plugin instance.
 *
 * Provman does not know the SIM identifier or IMSI number of the
 * current management session.  Although it receives a SIM identifier from the
 * management client as a parameter to the #Start method this parameter may
 * be an empty string, indicating that the default SIM identifier should be
 * used.  The problem is that Provman has no way of transforming an empty string
 * into the default identifier as it does not itself use the facilities of the
 * underlying telephony system.  Therefore, it needs to ask the plugins that
 * support SIM specific data to do this for it.  Provman uses this information
 * when storing and retrieving meta data for SIM specific plugins.
 *
 * Plugins that do not support SIM specific data do not need to implement this
 * function.
 *
 * @param instance A pointer to the plugin instance.
 *
 * @return a string containing the SIM identifier the plugin is using for the
 *   current session.  Ownership of this string is retained by the plugin.
 */

typedef const gchar *(*provman_plugin_sim_id)(
	provman_plugin_instance instance);


/*! \brief Typedef for struct provman_plugin_ */
typedef struct provman_plugin_ provman_plugin;

/*! \brief Provisioning plugin structure.
 *
 * One instance of this structure must be created for each plugin.  Instances of
 * this structure are all located in an array called #g_provman_plugins.
 * To create a new plugin you need to add a new instance of this structure to
 * the #g_provman_plugins array.  This structure contains
 * some information about the plugin, such as its name and its root, its root
 * being the directory that is managed by the plugin.  The plugin owns this
 * directory and all the sub-directories and settings that fall under this
 * directory.  Provman uses the plugins' roots to determine which key should be
 * managed by which plugin.
 */

struct provman_plugin_
{
        /*! \brief The name of the plugin. */
	const char *name;
        /*! \brief The root of the plugin. */
	const char *root;
        /*! \brief The XML schema of the plugin. */
	const char *schema;
        /*! \brief Pointer to the plugin's creation function. */
	provman_plugin_new new_fn;
        /*! \brief Pointer to the plugin's destructor. */
	provman_plugin_delete delete_fn;
        /*! \brief Pointer to the plugin's sync in function. */
	provman_plugin_sync_in sync_in_fn;
        /*! \brief Pointer to the plugin's sync in cancel function. */
	provman_plugin_sync_in_cancel sync_in_cancel_fn;
        /*! \brief Pointer to the plugin's sync out function. */
	provman_plugin_sync_out sync_out_fn;
        /*! \brief Pointer to the plugin's sync out cancel function. */
	provman_plugin_sync_out_cancel sync_out_cancel_fn;
        /*! \brief Pointer to the plugin's abort function. */
	provman_plugin_abort abort_fn;
        /*! \brief Pointer to the plugin's sim_id function. */
	provman_plugin_sim_id sim_id_fn;
};

/*! \cond */

int provman_plugin_check();
unsigned int provman_plugin_get_count();
const provman_plugin *provman_plugin_get(unsigned int i);
int provman_plugin_find_index(const char *uri, unsigned int *index);
GPtrArray *provman_plugin_find_children(const char *uri);
GPtrArray *provman_plugin_find_direct_children(const char *uri);
bool provman_plugin_uri_exists(const char *uri);
void provman_plugin_find_plugins(const char *uri, GArray *indicies);
/*! \endcond */

#endif
