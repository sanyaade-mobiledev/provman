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
 *   was begun but it failed because provman was killed.
 * \exception com.intel.provman.Error.Died Provman was killed before
 *   the #Start command could be initiated.
*/

void Start(string imsi);

/*!
 * \brief Assigns a value to a given key.
 *
 * If the key does not already exist it is
 * created.  If any of the key's ancestors do not exist they will also be
 * created.
 *
 * A successful call to #Set
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
 * \exception com.intel.provman.Error.NotFound The specified key
 *   is not associated with any plugin.
 * \exception com.intel.provman.Error.BadArgs The key is not valid
 * \exception com.intel.provman.Error.Cancelled The call to #Set
 *   was begun but it failed because provman was killed.
 * \exception com.intel.provman.Error.Died Provman was killed before
 *   the #Set command could be initiated.
*/

void Set(string key, string value);

/*!
 * \brief Sets multiple keys in a single command
 *
 * #SetMultiple should be used to set multiple settings in a single command.
 * If you need to provision multiple settings it is more efficient to
 * call #SetMultiple once instead of invoking #Set multiple times as doing so
 * reduces the IPC overhead.
 *
 * The failure to set an individual key does not cause the entire #SetMultiple
 * command to fail.  It will continue to set the remaining keys.  Once
 * the comamnd has finished a list of failed keys will be returned to
 * the caller.
 *
 * As with #Set, if the key does not already exist it is
 * created.  If any of the key's ancestors do not exist they will also be
 * created.

 * @param dict A dictionary of key value pairs to provision, type \a a{ss}
 * @return An array of keys that could not be set, of type \a as.  If this
 * array is empty then all keys were correctly set.
 *
 * \exception com.intel.provman.Error.Unexpected #SetMultiple is invoked
 * before #Start.
 * \exception com.intel.provman.Error.Cancelled The call to #SetMultiple
 *   was begun but it failed because provman was killed.
 * \exception com.intel.provman.Error.Died Provman was killed before
 *   the #SetMultiple command could be initiated.
*/

array SetMultiple(dictionary dict);

/*!
 * \brief Retrieves the value associated with a key.
 *
 * If the key represents a setting the value of that setting
 * is returned.  If, on the other hand, the key represents a
 * directory, a '/' separated list of the names of the key's
 * children is returned.  Calling #Get on /applications might
 * return "email/sync/browser", for example.
 *
 * @param key the key whose value you wish to retrieve
 * @return the value associated with the specified key.
 *
 * \exception com.intel.provman.Error.Unexpected #Get is invoked
 * before #Start.
 * \exception com.intel.provman.Error.NotFound The specified key
 *   does not exist.
 * \exception com.intel.provman.Error.BadArgs The key is not valid
 * \exception com.intel.provman.Error.Cancelled The call to #Get
 *   was begun but it failed because provman was killed.
 * \exception com.intel.provman.Error.Died Provman was killed before
 *   the #Get command could be initiated.
*/

string Get(string key);

/*!
 * \brief Retrieves the set of key/value pairs associated with a
 * given key.
 *
 * If #GetAll is invoked on a directory, it returns all the keys
 * and values contained in that directory and its decendants.
 *
 * @param key the key whose value(s) you wish to retrieve
 * @return a dictionary of key value settings of type \a a{ss}
 *
 * \exception com.intel.provman.Error.Unexpected #GetAll is invoked
 * before #Start.
 * \exception com.intel.provman.Error.NotFound The specified key
 *   does not exist.
 * \exception com.intel.provman.Error.BadArgs The key is not valid
 * \exception com.intel.provman.Error.Cancelled The call to #GetAll
 *   was begun but it failed because provman was killed.
 * \exception com.intel.provman.Error.Died Provman was killed before
 *   the #GetAll command could be initiated.
*/

dictionary GetAll(string key);

/*!
 * \brief Retrieves the individual values associated with one or more keys
 *
 * #GetMultiple executes the #Get command on each key passed to
 * it by the caller.  It returns a dictionary that maps the input keys
 * to their values.  If a requested key is a setting, the setting's
 * value is returned.  If it is a directory, the '/' separated list
 * of that directory's children is returned.
 *
 * @param keys an array of keys whose values you wish to retrieve
 * @return a dictionary of key value settings of type \a a{ss}
 *
 * \exception com.intel.provman.Error.Unexpected #GetMultiple is invoked
 * before #Start.
 * \exception com.intel.provman.Error.Cancelled The call to #GetMultiple
 *   was begun but it failed because provman was killed.
 * \exception com.intel.provman.Error.Died Provman was killed before
 *   the #GetMultiple command could be initiated.
*/

dictionary GetMultiple(array keys);

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
 * \exception com.intel.provman.Error.NotFound The specified key
 *   does not exist.
 * \exception com.intel.provman.Error.BadArgs The key is not valid
 * \exception com.intel.provman.Error.Cancelled The call to #Delete
 *   was begun but it failed because provman was killed.
 * \exception com.intel.provman.Error.Died Provman was killed before
 *   the #Delete command could be initiated.
*/

void Delete(string key);

/*!
 * \brief Deletes one or more specified keys.
 *
 * This function essentially just calls delete multiple times,
 * once for every key in the keys array parameter.
 *
 * As with delete, if the key to be deleted is a subdirectory, that
 * key and all its chilren (and their meta data) will be deleted.
 *
 * The keys array should not contain any overlapping keys, e.g.,
 * '/applications', '/applications/email'.  If this occurs errors
 * may be raised as multiple attempts may be made to delete the same key,
 * depending on the order in which the keys are specified.
 *
 * Failure to delete a key does not cause #DeleteMultiple to abort.  Rather
 * it attempts to delete the remaining keys and returns an array of the
 * keys that could not be deleted when it is finished.
 *
 * @param keys an array of keys to delete
 * @return an array of keys that could not be deleted.  If this array is
 * empty then everything has been successfully deleted.
 *
 * \exception com.intel.provman.Error.Unexpected #DeleteMultiple is invoked
 * before #Start.
 * \exception com.intel.provman.Error.Cancelled The call to #DeleteMultiple
 *   was begun but it failed because provman was killed.
 * \exception com.intel.provman.Error.Died Provman was killed before
 *   the #DeleteMultiple command could be initiated.
*/

array DeleteMultiple(array keys);

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
 *   was begun but it failed because provman was killed.
 * \exception com.intel.provman.Error.Died Provman was killed before
 *   the #End command could be initiated.
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
 *   was begun but it failed because provman was killed.
 * \exception com.intel.provman.Error.Died Provman was killed before
 *   the #Abort command could be initiated.
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
 * \exception com.intel.provman.Error.NotFound The key is not supported by
 * provman or its plugins.
 * \exception com.intel.provman.Error.BadArgs The key is not valid
 * \exception com.intel.provman.Error.Cancelled The call to #GetTypeInfo
 *   was begun but it failed because provman was killed.
 * \exception com.intel.provman.Error.Died Provman was killed before
 *   the #GetTypeInfo command could be initiated.
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
 * \exception com.intel.provman.Error.Cancelled The call to #GetChildrenTypeInfo
 *   was begun but it failed because provman was killed.
 * \exception com.intel.provman.Error.Died Provman was killed before
 *   the #GetChildrenTypeInfo command could be initiated.
*/

dictionary GetChildrenTypeInfo(string key);

/*!
 * \brief Assigns a meta data property value to a given key.
 *
 * The key can be a setting or a directory.
 *
 * It is an error to assign a meta data property value to a
 * a non-existent key.  If the meta data property does not
 * exist it will be created.  If the property does exists
 * its old value will be overwritten.
 *
 * Meta data can be assigned to read only leaf nodes.
 *
 * Finally, meta data can only be assigned to keys that are
 * maintained by plugins.   Therefore meta data cannot be
 * assoicated with '/' or '/applications'
 *
 * @param key the key with which you would like to associate meta data
 * @param prop the name of the meta data property.
 * @param value the value to assign to the meta data property.
 *
 * \exception com.intel.provman.Error.Unexpected #SetMeta is invoked
 * before #Start.
 * \exception com.intel.provman.Error.NotFound The specified key
 *   does not exist.
 * \exception com.intel.provman.Error.BadArgs The key is not well formed
 *   or is not managed by a plugin.
 * \exception com.intel.provman.Error.Cancelled The call to #SetMeta
 *   was begun but it failed because provman was killed.
 * \exception com.intel.provman.Error.Died Provman was killed before
 *   the #SetMeta command could be initiated.
*/

void SetMeta(string key, string prop, string value);

/*!
 * \brief Retrieves a meta data property value from a given key.
 *
 * @param key the key whose meta data you would like to retrieve
 * @param prop the name of the meta data property.
 *
 * @return the value of the meta data property.
 *
 * \exception com.intel.provman.Error.Unexpected #GetMeta is invoked
 * before #Start.
 * \exception com.intel.provman.Error.NotFound The specified key
 *   or meta data property does not exist.
 * \exception com.intel.provman.Error.BadArgs The key is not well formed
 *   or is not managed by a plugin.
 * \exception com.intel.provman.Error.Cancelled The call to #GetMeta
 *   was begun but it failed because provman was killed.
 * \exception com.intel.provman.Error.Died Provman was killed before
 *   the #GetMeta command could be initiated.
*/

string GetMeta(string key, string prop);

/*!
 * \brief Sets multiple meta data entries in a single command
 *
 * #SetMultipleMeta should be used to set multiple meta data values in a single
 * command.  If you need to provision multiple meta data values it is more
 * efficient to call #SetMultipleMeta once instead of invoking #SetMeta multiple
 * times as doing so reduces the IPC overhead.
 *
 * The failure to set an individual piece of meta data  does not cause the
 * entire #SetMultipleMeta command to fail.  It will continue to set the
 * remaining pieces of meta data.  Once the command has finished, an array of
 * structures, is returned.  Each structure in this list contains the key/
 * a property name pair to which provman failed to assign meta data.
 *
 * As with #SetMeta it is an error to attempt to associate meta data with a
 * non-existent key.
 *
 * @param meta A dictionary of meta data structures, type \a a(sss).  Each
 * structure contains, in order, the following values, key, meta data property
 * name, meta data value, e.g., ("/telephony/mms", "ACL,"Get='*'").
 * @return An array of key/property name pairs that could not be set,
 * type \a a(ss).  If this array is empty then all pieces of meta data
 * were correctly set.
 *
 * \exception com.intel.provman.Error.Unexpected #SetMultipleMeta is invoked
 * before #Start.
 * \exception com.intel.provman.Error.Cancelled The call to #SetMultipleMeta
 *   was begun but it failed because provman was killed.
 * \exception com.intel.provman.Error.Died Provman was killed before
 *   the #SetMultipleMeta command could be initiated.
*/

array SetMultipleMeta(array meta);

/*!
 * \brief Retrieves all the meta data associated with a given key
 *
 *
 * If #GetAllMeta is invoked on a directory, it returns all meta
 * data properties associated with that key and its descendants.
 *
 * @param key the key whose value(s) you wish to retrieve
 * @return an array of meta data structures of type \a a(sss).
 * Each structure contains, in order, the key, the meta data property
 * name, and the meta data value, e.g., ("/telephony/mms", "ACL,"Get='*'").
 * The returned array contains no entries for keys that do not define any
 * meta data.  Therefore, if you were to execute this command on a sub-tree
 * that contained 1000 nodes, but none of these nodes defined any meta data,
 * the returned array would be empty.
 *
 * \exception com.intel.provman.Error.Unexpected #GetAllMeta is invoked
 * before #Start.
 * \exception com.intel.provman.Error.NotFound The specified key
 *   does not exist.
 * \exception com.intel.provman.Error.BadArgs The key is not valid
 * \exception com.intel.provman.Error.Cancelled The call to #GetAllMeta
 *   was begun but it failed because provman was killed.
 * \exception com.intel.provman.Error.Died Provman was killed before
 *   the #GetAllMeta command could be initiated.
*/

array GetAllMeta(string key);
