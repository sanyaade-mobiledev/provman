/******************************************************************************
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/
/*!
 * @file dbus.h
 * \brief Documentation for the D-Bus interface exported by provman instances
 *
 * All the methods documented below are part of the 
 * \a com.intel.provman.Settings interface implemented by the 
 * \a /com/intel/provman object
 */

/*!
 * \brief Initiates a management session with the provman.
 * 
 * If a sesison
 * is already in progress with another management client this method will not
 * return until that management client has completed its session by calling
 * either #End or #Abort or has died unexepectedly.  This method must be called
 * before any other com.intel.provman.Settings methods can be invoked on a
 * given provman instance
 *	
 * @param imsi The IMSI number with which the settings should be associated.  If
 * the caller does not care or is not intending to provision any SIM specific
 * settings he can simply pass an empty string.  Provman will
 * then associate any SIM specific settings with the SIM card of the first
 * modem it discovers in the device.
 *
 * \exception com.intel.provman.Error.Unexpected A call to #Start is
 *   outstanding or has completed and a device management session is already
 *   in process with this client.  #Start cannot be called again on this client
 *   until #End has been called.
 * \exception com.intel.provman.Error.Cancelled The call to #Start
 *   has failed because provman has been killed.
*/

void Start(string imsi);

/*!
 * \brief Assigns a value to a given key.
 *
 * If the key does not already exist it is
 * created.  If any of the key's ancestors do not exist they will also be
 * created.
 *
 * A successful call to set
 * means that the setting is supported by provman and will be
 * be passed to the relevant plugin for storage in the appropriate
 * application/middleware data store when the management session
 * completes.  It does not indicate that the setting has been 
 * provisioned.
 * 
 * @param key the key to create or whose value you wish to change.
 * @param value the value to assign to the key.
 *
 * \exception com.intel.provman.Error.Unexpected #Set is invoked
 * before #Start.
 * \exception com.intel.provman.Error.Cancelled The call to #Set
 *   has failed because provman has been killed.
 * \exception com.intel.provman.Error.NotFound The specified key
 *   is not associated with any plugin.
 * \exception com.intel.provman.Error.BadArgs The key is not valid
*/

void Set(string key, string value);

/*!
 * \brief Sets multiple keys in a single command
 *
 * SetAll should be used to set multiple settings in a single command.
 * If you need to provision multiple settings it is more efficient to
 * call SetAll once instead of invoking Set multiple times as doing so
 * reduces the IPC overhead.
 *
 * The failure to set an individual key does not cause the entire SetAll
 * command to fail.  It will continue to set the remaining keys.  Once
 * the comamnd has finished a list of failed keys will be returned to
 * the caller.
 *
 * As with Set, if the key does not already exist it is
 * created.  If any of the key's ancestors do not exist they will also be
 * created.

 * @param dict A dictionary of key value pairs to provision, type \a a{ss}
 * @return An array of keys that could not be set, of type \a as.  If this
 * array is empty then all keys were correctly set.
 *
 * \exception com.intel.provman.Error.Unexpected #SetAll is invoked
 * before #Start.
 * \exception com.intel.provman.Error.Cancelled The call to #SetAll
 *   has failed because provman has been killed.
*/

array SetAll(dictionary dict);

/*!
 * \brief Retrieves the value associated with a key.
 *
 * If the key represents a setting the value of that setting
 * is returned.  If, on the other hand, the key represents a
 * directory, a '/' separated list of the names of the key's 
 * children is returned.  Calling Get on /applications might
 * return "email/sync/browser", for example.
 * 
 * @param key the key whose value you wish to retrieve
 * @return the value associated with the specified key.
 *
 * \exception com.intel.provman.Error.Unexpected #Get is invoked
 * before #Start.
 * \exception com.intel.provman.Error.Cancelled The call to #Get
 *   has failed because provman has been killed.
 * \exception com.intel.provman.Error.NotFound The specified key
 *   does not exist.
 * \exception com.intel.provman.Error.BadArgs The key is not valid
*/

string Get(string key);

/*!
 * \brief Retrieves the set of key/value pairs associated with a
 * given key.
 *
 * If GetAll is invoked on a directory, it returns all the keys
 * and values contained in that directory and its decendants.
 *
 * @param key the key whose value(s) you wish to retrieve
 * @return a dictionary of key value settings of type \a a{ss}
 *
 * \exception com.intel.provman.Error.Unexpected #Get is invoked
 * before #Start.
 * \exception com.intel.provman.Error.Cancelled The call to #Get
 *   has failed because provman has been killed.
 * \exception com.intel.provman.Error.NotFound The specified key
 *   does not exist.
 * \exception com.intel.provman.Error.BadArgs The key is not valid
*/

dictionary GetAll(string key);


/*!
 * \brief Deletes a key or directory.
 *
 * If the key represents a sub-directory, the entire
 * sub-directory and all its keys are deleted.
 *
 * @param key the key to delete
 *
 * \exception com.intel.provman.Error.Unexpected #Delete is invoked
 * before #Start.
 * \exception com.intel.provman.Error.Cancelled The call to #Delete
 *   has failed because provman has been killed.
 * \exception com.intel.provman.Error.NotFound The specified key
 *   does not exist.
 * \exception com.intel.provman.Error.BadArgs The key is not valid
*/

void Delete(string key);

/*!
 * \brief Ends the device management session begun by #Start
 *
 * All changes made during the session will be push to the plugins who
 * will reflect the changes by invoking the appropriate middleware APIs.
 * If one or more other device management client are blocked by a call to
 * #Start, the #Start method will complete for one of these clients and
 * its device management session will begin.
 *
 * \exception com.intel.provman.Error.Unexpected #End is invoked
 * before #Start.
 * \exception com.intel.provman.Error.Cancelled The call to #End
 *   has failed because provman has been killed.
*/

void End();

/*!
 * \brief Ends the device management session begun by #Start, abandoning
 *        all changes made during that session.
 *
 * All changes made during the session will be discarded.
 * If one or more other device management client are blocked by a call to
 * #Start, the #Start method will complete for one of these clients and
 * its device management session will begin.
 *
 * \exception com.intel.provman.Error.Unexpected #Abort is invoked
 * before #Start.
 * \exception com.intel.provman.Error.Cancelled The call to #Abort
 *   has failed because provman has been killed.
*/

void Abort();

/*!
 * \brief Retrieves type information about a given key.
 *
 * The key does not actually have to exist for this function to succeed.
 * It just needs to be supported by one of the plugins or be a parent
 * directory of one or more plugins.  So for example, we can use this
 * function to query the type of the port number setting for incoming
 * email accounts, even if no email accounts are currently defined 
 * in the system.  As long as the provman instance contains an email
 * plugin this method should return the correct value.
 *
 * #GetTypeInfo is able to return information for non-existent keys as it
 * reads the type information from the schema.  As plugin schemas are available
 * as soon as the plugins are created, #GetTypeInfo can be called outside
 * of a device management session, that is before #Start or after #End.
 *
 * One use of this function would be a client that receives its provisioning
 * data from a user via a UI.  The UI might need to know which authentication
 * mechanisms are supported by the email application so it can display a list
 * of options to the user before it creates the setting.  It can find this
 * information by calling #GetTypeInfo with the key 
 * "/applications/email/<X>/incoming/authtype"
 *
 * @param key Should be a uri of the key in which you are interested. Any
 * non empty string can be used as a place holder for unnamed directories.
 * In the example above "<X>" is used, but "<X>" could be replaced with
 * "account" or "x", etc.
 *
 * \return "dir" the key represents a directory
 * \return "int" the key is used to store an integer value
 * \return "string" the key is used to store a string value
 * \return "enum: val1 [,val2]*" the key is used to store an enumerated type.
 *   A comma separated list of permissable values is provided after the string,
 *   e.g., "enum: never, when-possible, always"
 *   
 * \exception com.intel.provman.Error.NotFound The key is not supported by provman
 *    or its plugins.
*/

string GetTypeInfo(string key);

/*!
 * \brief Retrieves type information about all the children of a given key.
 *
 * This function is similar to #GetTypeInfo.  The key does not need to exist
 * for this function to succeed and it can be called outside a management
 * session.  It returns the names of all the possible children of the specified
 * key, whether they currently exist or not, together with their type
 * information.  All this information is returned in a dictionary.  The
 * function is not recursive.
 *
 * It is an error to call this function on a key that represents a possible
 * setting rather than a possible directory.
 *
 * This function reports unamed directories as "<X>".
 *
 * @param key Should be a uri of the key in which you are interested. Any
 * non empty string can be used as a place holder for unnamed directories.
 * In the example about "<X>" is used, but "<X>" could be replaced with
 * "account" or "x", etc.
 *
 * \return a dictionary that maps child names to their types.  The child names
 * are simply names and not complete uris.
 *   
 * \exception com.intel.provman.Error.NotFound The key is not supported by
 *    provman or its plugins.
 * \exception com.intel.provman.Error.BadArgs The key represents a setting and
 *    not a directory.
*/

dictionary GetChildrenTypeInfo(string key);

