/******************************************************************************
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/
/*!
 * @file mainpage.h
 * @mainpage Provman
 *
 * @section introduction Introduction
 *
 * Device management refers to technologies that allow a trusted third
 * party to perform remote management of an end user's device. There are many
 * different device management protocols, such as OMA CP, OMA DM, Active
 * Sync, etc.  Today's devices are often required to support more than one
 * of these protocols. Smart phones need to support OMA CP and
 * OMA DM to permit remote management of the device by network operators in
 * addition to various enterprise protocols such as ActiveSync to allow
 * remote management by the user's IT department.  Although these
 * competing protocols differ substantially from one another they are all
 * fundamentally trying to solve the same sorts of problems. One of the most
 * common of these problems is provisioning, that is the remote configuration
 * of user and system settings.
 *
 * The primary goal of provman is to avoid code duplication
 * between the various device management clients on a device.  The intention
 * is to place all the code that actually performs the provisioning
 * of the device into provman and to expose this functionality
 * via a simple to use D-Bus API.  For example, to provision the MMS application
 * a device management client needs to install several settings in various
 * locations
 * in the operating system.  It may need to create a new 3G APN in the telephony
 * subsystem and to configure various application level settings in the MMS
 * application itself.  Device management clients that use provman can simply
 * pass all of the MMS settings to the provman
 * and it will take care of the complicated procedure of storing these
 * settings in the appropriate locations on the device.
 * This simplifies greatly the task of creating the various device management
 * clients and it allows their creators to focus on what really differentiates
 * the clients from one another, i.e., the implementation of their protocols.
 * Provman also isolates device management clients from any changes to
 * underlying middleware.  For example, they would be unaffected by the
 * change of a distribution's email application or telephony subsystem, as the
 * interface exposed by provman would remain the same
 *
 * In addition to implementing the basic provisioning operations,
 * provman process provides device management clients with some additional
 * services:
 * <ul>
 * <li> Allows device management clients to associate meta data with the
 *      various provisioned settings. </li>
 * <li> Implements synchronisation between device management clients, i.e.,
 *      ensures that multiple clients cannot provision the device at the same
 *      time. </li>
 * <li> Is aware that certain settings and their associated meta data are SIM
 *      specific. Clients can specify a SIM identifier to identify the modem
 *      whose settings they wish to modify/access.
 * </ul>
 *
 * @section quick-start Getting Started
 *
 * A description of how to install and test provman on MeeGo Netbook 1.2 can
 * be found in the getting-started.h page.
 *
 * @section architecture Architecture
 *
 * Provisioned settings can be split into two separate groups, user and system
 * settings.  A user setting might be the reply-to address of an email account.
 * A system setting might be the name of a 3G access point or a parameter
 * that controls the maximum number of failed login attempts a user is permitted
 * to make.  This creates a problem for provman as APIs for
 * modifying user settings are published on the D-Bus session bus and
 * APIs for system settings are published on the system bus.  For this reason
 * two separate instances of the provman will be required,
 * one accessible via the session D-Bus that exposes user settings and
 * another accessible via the system D-Bus that exposes system settings.  Both
 * provman instances will expose the same API.  The instance that exposes
 * system settings would need to run as root or be granted permission by the OS
 * to access the protected resources that it needs.
 *
 * Each provman instance defines a set of keys that it supports.  The
 * device management clients manage the device by inspecting, creating and
 * modifying these keys.  There are two types of keys; settings that associate
 * a single string value with a key and directories that can contain other keys.
 * Setting and directory names are separated by '/'s.  The root directory is
 * also represented by a '/'.  So for example, application settings may be
 * stored under the \a /applications directory, email settings could be stored
 * under the \a /applications/email directory and an email server may be changed
 * by modifying the value of the setting
 * \a /applications/email/account1/incoming/address.
 *
 * A single string value is associated with each key.  Provman
 * provides D-Bus methods that allow keys to be read, created, set, and deleted.
 * There are also methods for setting and retrieving keys and their associated
 * values in bulk to cut down on the IPC overhead.
 *
 * Each provman instance consists of some core code and a set of plugins.  The
 * core code manages the D-Bus interface and provides the infrastructure in
 * which the plugins exist.  Each plugin defines and owns a subdirectory of
 * keys. For example, an email plugin might own \a /applications/email and all
 * its descendents. When a provman instance receives a request to read or
 * modify a given key, it identifies the plugin that owns the key and passes the
 * request to that plugin.  When the plugin receives a new key/value pair it
 * must somehow map that setting to the appropriate setting in the underlying
 * operating system.  Currently, all plugins are hardcoded into the provman
 * instances.  Each provman instance supports a different set of plugins.
 *
 * The provman instances are launched by D-Bus.  They run until all of their
 * tasks have been completed and then exit.
 *
 * @section api-overview API Overview
 *
 * Each provman instance registers the name
 * \a com.intel.provman.server on the relevant D-Bus bus.  Each instance exposes
 * a single D-Bus object, \a /com/intel/provman that implements a single
 * interface, \a com.intel.provman.Settings.  Device management clients manage
 * the device by invoking methods on this interface.  A client must always
 * initiate a management session by calling the #Start method.  Once the #Start
 * method has returned successfully it can call additional methods such as #Get,
 * #Set and #Delete to get, set and delete various settings.  When a client has
 * finished managing the device it needs to call the #End method.  Only one
 * device management client can manage the device via a provman instance at any
 * one time.  This constraint is enforced by the #Start method.  If a device
 * management client has successfully invoked the #Start method, no other device
 * management clients will be able to manage the device via the same provman
 * instance until the first client calls #End.  If a second client attempts to
 * do so, its call to #Start will not return until the first client has called
 * #End.  Attempts to call any of the other methods, such as #Get or
 * #Set, by a client before it has successfully called #Start will fail.
 *
 * Some settings, such as the telephony and MMS settings, are SIM specific.  The
 * operating system maintains separate sets of such settings for each SIM card.
 * When a device management client wishes to provision such a setting it must
 * inform provman of the SIM card to which the settings are
 * to be associated.  It does this by passing the IMSI
 * number of the target SIM card to the Start method.  All settings provisioned
 * during a session, delimeted by the call to #Start and #End, are therefore
 * associated with a given SIM card.  Provman passes the
 * IMSI number to the plugins when setting and retrieving settings. Some plugins
 * whose data is not SIM specific simply ignore this information.  These
 * plugins manipulate the same set of settings, regardless of the IMSI number
 * passed to the #Start method.  If a device management client does not care
 * about which SIM card the settings are to be associated with or is not
 * planning to access or manipulate any SIM specific settings it can simply
 * pass an empty string to the #Start method.  Provman will then associate
 * any SIM specific settings with the SIM card of the first modem detetected
 * in the device.
 *
 * Provman has support for introspection.  The methods #GetTypeInfo and
 * #GetChildrenTypeInfo allow clients to identify the settings that are
 * supported by the device and the types and possible values that these
 * settings can hold.
 *
 * Finally, provman allows meta data to be associated with provman keys
 * (settings or directories).  Each key can support an unlimited number
 * of meta data key/value pairs.  Meta data keys and values are strings.
 * For example, the key \a /applications/email/test might have a meta
 * data key/value pair of 'Type' and 'com.intel.com.email' associated
 * with it.  Meta data properties are bound to a key and are deleted if
 * the key is deleted.  Provman supports meta data as a means to allow
 * separate device management clients to communicate with each other.
 * For example, an OMA CP client that processes an OMA CP document
 * containing bootstrap information for a DM server must grant sufficient
 * ACL rights to that server so that it can modify any of the settings
 * provisioned in the OMA CP document.  The OMA CP client can satisify
 * this requirement by associating an ACL property with the root node
 * of each newly provisioned account.  This property can be read by the
 * DM client at a later stage.  Two methods are provided for setting
 * and retrieving meta data.  These are #SetMeta and #GetMeta, respectively.
 *
 * A brief description of each D-Bus method is given below.  Click on the links
 * for more detailed information:
 *
 * \par
 * <table>
 * <tr><th>Method</th><th>Description</th></tr>
 * <tr><td>#GetVersion</td><td>\copybrief GetVersion</td></tr>
 * <tr><td>#Start</td><td>\copybrief Start</td></tr>
 * <tr><td>#Set</td><td>\copybrief Set</td></tr>
 * <tr><td>#SetMultiple</td><td>\copybrief SetMultiple</td></tr>
 * <tr><td>#Get</td><td>\copybrief Get</td></tr>
 * <tr><td>#GetMultiple</td><td>\copybrief GetMultiple</td></tr>
 * <tr><td>#GetAll</td><td>\copybrief GetAll</td></tr>
 * <tr><td>#Delete</td><td>\copybrief Delete</td></tr>
 * <tr><td>#DeleteMultiple</td><td>\copybrief DeleteMultiple</td></tr>
 * <tr><td>#End</td><td>\copybrief End</td></tr>
 * <tr><td>#Abort</td><td>\copybrief Abort</td></tr>
 * <tr><td>#GetTypeInfo</td><td>\copybrief GetTypeInfo</td></tr>
 * <tr><td>#GetChildrenTypeInfo</td><td>\copybrief GetChildrenTypeInfo</td></tr>
 * <tr><td>#GetMeta</td><td>\copybrief GetMeta</td></tr>
 * <tr><td>#SetMeta</td><td>\copybrief SetMeta</td></tr>
 * <tr><td>#GetAllMeta</td><td>\copybrief GetAllMeta</td></tr>
 * <tr><td>#SetMultipleMeta</td><td>\copybrief SetMultipleMeta</td></tr>
 * </table>
 *
 * A simple python script demonstrating how these methods can be used is shown
 * below.
 * \code
#!/usr/bin/python

import dbus
import sys

bus = dbus.SessionBus()

manager = dbus.Interface(bus.get_object('com.intel.provman.server',
					'/com/intel/provman'),
					'com.intel.provman.Settings')
manager.Start("")
manager.Set("/telephony/contexts/test/apn","test-apn")
manager.Set("/telephony/contexts/test/name","Test APN")
manager.Set("/telephony/contexts/test/username","markus")
manager.Set("/telephony/contexts/test/password","tulius")
manager.End()
\endcode

 *
 * @section plugin-overview Plugin Overview
 *
 * Each plugin must provide an XML schema describing the settings that it
 * supports.  The schema DTD is show below:
\code
<!ELEMENT schema (dir)* >
<!ELEMENT dir (dir | key)*>
<!ELEMENT key EMPTY>
<!ATTLIST schema
   root   CDATA             #REQUIRED
>
<!ATTLIST dir
   name   CDATA             #IMPLIED ""
   delete (yes|no)          "yes"
>
<!ATTLIST dir
   name   CDATA             #REQUIRED
   delete (yes|no)          "no"
   write  (yes|no)          "yes"
   type   (int|string|enum) #REQUIRED
   values CDATA             #IMPLIED
>
\endcode
 *  The first element must be called \a schema.  This element must have a single
 * attribute called \a root whose value is the root path of the plugin, e.g.,
 * /applications/email/.  The root path must end in a '/'.
 *
 * The \a schema element can contain one or more \a dir elements, which
 * represent directories.  Each \a dir element should have a \a name attribute
 * that indicates the name of the directory.  The name attribute
 * should only contain the directory's name, e.g., \a incoming, and not a
 * full path such as
 * \a /applications/email/incoming.  It should not end in a '/'.  The \a dir
 * element supports an additional attribute called \a delete which determines
 * whether or not a directory can be deleted.  The \a delete attribute is
 * optional and if not specified defaults to \a yes.  The \a name element is
 * actually optional as well. If it is not specified or is set to the empty
 * string the directory is said to be unnamed.  Unnamed directories allow
 * clients to specify the name of the directory themselves.  This is useful if
 * the name of the directory corresponds to an account name.  Unammed
 * directories are represented by the notation \<X\> in the \ref settings
 * section below.  An unnamed directory cannot have any siblings.
 *
 * A directory can contain zero or more \a dir or \a key elements.  A \a key
 * element corresponds to an individual setting.  \a key elements cannot have
 * any children.  Like \a dir elements, they support a \a name attribute but
 * unlike \a dir elements this attribute is not optional and must be provided.
 * \a key elements also support a \a delete attribute.  The \a delete attribute
 * is optional but, unlike the \a dir's delete attribute, it defaults to \a no.
 * The \a key element
 * also supports an attribute called \a write which indicates whether or not
 * the setting it represents can be modified by clients.  \a write is optional
 * and defaults to \a yes.  Finally, the \a key element supports two additional
 * attributes \a type and \a values.  \a type indicates the type of the setting.
 * Three values are currently supported, \a string, \a int and \a enum.  If the
 * value of \a enum is specified for type, the \a values attribute must also be
 * present.  \a values should be set to a comma separated list of values
 * supported by the setting.
 *
 * An example schema is presented below:
 *
\code
<schema root='/telephony/'>
    <dir name='contexts' delete='yes'>
        <dir delete='yes'>
            <key name='apn' delete='no' type='string'/>
            <key name='name' delete='no' type='string'/>
            <key name='password' delete='no' type='string'/>
            <key name='username' delete='no' type='string'/>
       </dir>
    </dir>
    <dir name='mms' delete='yes'>
        <key name='apn' delete='no' type='string'/>
        <key name='name' delete='no' type='string'/>
        <key name='password' delete='no' type='string'/>
        <key name='username' delete='no' type='string'/>
        <key name='mmsc' delete='no' type='string'/>
        <key name='proxy' delete='no' type='string'/>
    </dir>
    <key name='imsis' delete='no' write='no' type='string'/>
</schema>
\endcode
 *
 * Note the first grandchild of the schema.  This is an unnamed directory
 * that allows the clients to provide their own names for telephony contexts.
 *
 * In addition to the schema plugins must also implement a number of methods.
 * The most important of these
 * are called #provman_plugin_sync_in and #provman_plugin_sync_out.
 * When a client initiates a new management session by calling the #Start method
 * provman iterates through all of the plugins invoking their
 * #provman_plugin_sync_in methods.  When a plugin's  #provman_plugin_sync_in
 * method is called it must create a set of settings (key/value pairs) that
 * represent the current state of the data managed by the plugin.  For example,
 * if the plugin were responsible for managing telephony settings it might ask
 * the telephony sub-system for information about all of the 3G and MMS access
 * points.  Once received, the plugin would transform this information into one
 * or more settings.  These settings are then returned to provman.
 *
 * Provman caches all settings it receives from the plugins for
 * the duration of the management session.  If the client attempts to modify,
 * delete or add any settings the changes are only reflected in provman's
 * cache and are not propagated to the plugins.  It is not until
 * the client ends the session by calling #End that changes to the settings
 * are pushed to the plugins.  Provman does this by calling
 * each plugin's #provman_plugin_sync_out method.  During the call to
 * this method the plugin is passed an updated group of settings.  It must
 * compare this updated group of settings to the current state of the middleware
 * which it manages and identify what changes need to be made to the middleware,
 * to reflect the changes made by the client.  It makes the appropriate changes
 * and returns.
 *
 * There are three main reasons that provman caches changes to
 * settings and passes them in bulk to the plugins at the end of the session
 * rather than passing the settings immediately to the plugin as they are
 * altered by the clients.
 * <ol>
 * <li>It is not always clear what needs to be done when a new setting is added.
 *     Provisioning a new email account or 3G access point requires the creation
 *     of a number of new settings.  The plugin may need to have all of these
 *     settings available before it can create the account.  It may not be able
 *     to create the email account upon the reception of the first setting
 *     corresponding to that account.  You cannot invoke a constructor if you do
 *     not have all the paramaters it requires.  The problem is that clients are
 *     free to create settings in any order they chose and are not obliged to
 *     pass all the settings corresponding to a new account to provman in
 *     bulk.  This is flexibility is necessary to support OMA DM.  Therefore,
 *     the only way to ensure that the plugins have all the information they
 *     need to create the accounts they manage, is to cache the data in provman
 *     and pass it to the client in bulk at the end of the session when we
 *     are sure that we have received all the information from the client. </li>
 * <li>It is potentially a lot faster to do things this way.  For example, when
 *     you want to modify a single setting in evolution you need to create a new
 *     xml document containing all of the settings for that email account and
 *     then store the XML document in GConf.  Passing all the new settings to
 *     the plugin at the end of the session is a lot more efficient, as only
 *     one XML document is created, regardless of the number of settings that
 *     have changed, and only one call is made to GConf.</li>
 * <li>It allows us to implement basic transactions, although atomicity is not
 *     and cannot be guaranteed. </li>
 * </ol>
 *
 * Of course the problem with this design is that even though provman
 * indicates to a client that a call to #Set has succeeded, it is possible that
 * this setting is never actually propagated to the relevant piece of
 * middleware.  This could happen because the operation is invalid, or because
 * some unexpected error occurs during the call to the relevant plugin's
 * #provman_plugin_sync_out method such as an out of memory or out of disk space
 * error. Provman attempts to minimise these sorts of descrepancies by
 * validating a client's attempts
 * to write and delete settings against the plugins' schemas.  The client will
 * be notified if they try to create an unsupported setting, delete a read only
 * setting or directory or specify an invalid value for a setting.  Therefore,
 * if the #Set or #Delete methods succeed only a really exceptional error such
 * as a lack of memory or disk space or a device shutdown will prevent
 * modifications being propagated to the middleware.
 *
 * \subsection lifecycle Plugin Lifecycle
 * Plugins are created when the provman instance that hosts them is launched
 * and are destroyed when it exits.  Provman creates and
 * destroys plugins by calling their #provman_plugin_new and
 * #provman_plugin_delete methods respectively.  Plugins are not destroyed
 * when a management session ends.  It is therefore possible for
 * #provman_plugin_sync_in and #provman_plugin_sync_out to be called
 * multiple times on the same plugin instance.  This could happen if two clients
 * try to manage the device via the same provman instance at the same
 * time.  The second client will block until the first client has finished. When
 * the first client calls the #End function, provman will call
 * #provman_plugin_sync_out for each plugin to complete the first session,
 * followed by #provman_plugin_sync_in on each client to begin the second
 * session.  Once all the calls to #provman_plugin_sync_out have completed,
 * the Start method invoked by the second client will complete.  Note that
 * the Start method currently completes before all the calls to
 * #provman_plugin_sync_in have returned.  If a call to #provman_plugin_sync_in
 * fails for some reason, the management session can continue but not settings
 * for that plugin will be available.
 *
 * When provman invokes a plugin's #provman_plugin_sync_out
 * method, the plugin is not obliged to delete any cached data that it may have
 * obtained from the middleware during the previous call to
 * #provman_plugin_sync_in.  In fact if a second device management session
 * follows the first, it is more efficient if the plugin does not delete this
 * data, as it would only have to retrieve the data when the next session
 * starts.  Plugins that cache their data between settings are encouraged to
 * register for the relevant notifications to ensure that their cached data
 * is always up to date.
 *
 * A device management client may chose to discard any changes made during
 * a session by invoking the #Abort function.  #Abort discards any changes
 * made during the device management session and then ends the session.
 * When a session is ended by calling #Abort, the plugins'
 * #provman_plugin_sync_out methods are not invoked and no changes are pushed
 * to the applications or middleware.  Instead #provman_plugin_abort is called
 * to give plugins a chance to delete any data they have cached for the session
 * such as the IMSI number.
 *
 * \subsection sim-id SIM ID and Meta Data
 * All settings are stored by the plugins.  However, to ease the
 * burden on plugin development, all meta data is stored by provman.  This
 * creates difficulties when plugins exist that maintain SIM specific settings.
 * SIM specific settings are assoicated with a given SIM identifier.  To
 * associate the correct meta data with the correct set of SIM specific settings
 * both provman and the plugin that manages these settings needs to use the
 * same SIM identifier.  The problem is that provman does not know the SIM
 * identifier or IMSI number of the current management session.  Although it
 * receives a SIM identifier from the management client as a parameter to the
 * #Start method this parameter may be an empty string, indicating that the
 * default SIM identifier should be used.  Provman has no
 * way of transforming this empty string into the default identifier as it does
 * not itself use the facilities of the underlying telephony system.  Therefore,
 * it needs to ask the plugins that support SIM specific data to do this for
 * it.  It does this by calling the plugins' #provman_plugin_sim_id methods.
 *
 * \subsection client-ids Mapping between Client and OS IDs
 *
 * The operating system middleware generally assigns unique identifiers to the
 * various objects that it maintains.  For example, an identifier for an
 * evolution email account might look something like
 * \a 1310570935.30366.0\@hostname.
 * An identifier for an oFono APN might look like /phonesim/context1.  Now when
 * clients create new accounts they generally specify an ID that they will use
 * to identify that account, the identifier being the name of a directory, e.g.,
 *  \a /applications/email/\b my-email-account.  The problem is that many of the
 * middleware APIs generate identifiers automatically and do not allow the
 * provman plugins to associate client identifiers with new accounts, apns, etc.
 * For this reason the plugins need to map between the client supplied IDs and
 * IDs generated by the middleware.  This is only a problem for new accounts
 * created by provman.  For other accounts the client ID and the operating
 * system ID will be the same.
 *
 * As the mapping of IDs is a common task for plugins a help class,
 * #provman_map_file_t, has been created to facilitate this task.  Refer to
 * the documentation for this structure for more information.
 *
 * \subsection utility-functions Utility functions
 * Provman also provides some utility functions that make a
 * plugin writer's life a little easier.  For more information the reader is
 * referred to the utils.h page.
 *
 * @section settings Supported Settings
 *
 * @subsection email Email Settings
 * A provman instance supprting an email plugin can be used to read, create
 * and modify email accounts.  All email account settings are stored under the
 * directory \a /applications/email.  \a /applications/email itself can contain
 * 0 or more sub-directory, each sub-directory representing a unique email
 * account.  To create a new email account you simply need add some new settings
 * under /applications/email/\<X\> where \<X\> is the client side identifier
 * chosen to represent the new email account.  The following email settings are
 * defined
 *
 * <table>
 * <tr><th>Key</th><th>Description</th><th>Permissable Values</th></tr>
 * <tr><td colspan="3" align="center"><i>Generic email settings, all stored
 *     under /applications/email/\<X\>/</i></td></tr>
 * <tr><td>name</td><td>The name of the email account
 *     </td><td>Any string</td></tr>
 * <tr><td>address</td><td>The email address associated
 *      with this account. </td><td>A valid RFC 2822 email address.  The email
 *      can contain a nick name.  If it does this nick name will be registered
 *      with the email account. </td></tr>
 * <tr><td colspan="3" align="center"><i>Settings for the incoming email server
 *      all stored under /applications/email/\<X\>/incoming/</i></td></tr>
 * <tr><td>type</td><td>The type of the
 *     email server.</td><td>pop, imap, imapx, exchange, ews, groupwise, nntp,
 *     mbox, mh, maildir, spooldir, spool.</td></tr>
 * <tr><td>host</td><td>The address of the
 *     incoming email server.</td><td>An IP address or a domain name.</td></tr>
 * <tr><td>port</td><td>The port number of the
 *         incoming email server.  If not specified the default port number
 *         for the specified protocol will be used.</td><td>An integer
 *     </td></tr>
 * <tr><td>authtype</td><td>The
 *     authentication method to use when logging onto the incoming email
 *     server. </td><td>+APOP, CRAM-MD5, DIGEST-MD5, GSSAPI, PLAIN,
 *     POPB4SMTP, NTLM</td></tr>
 * <tr><td>username</td><td>The user name
 *     used to access the incoming email server.</td><td>A string</td></tr>
 * <tr><td>password</td><td>The password
 *     used to access the incoming email server.</td><td>A string</td></tr>
 * <tr><td>usessl</td><td>Indicates
 *     whether or not ssl should be used to communicate with the incoming
 *     email server.</td><td>always, never, when-possible</td></tr>
 * <tr><td colspan="3" align="center"><i>Settings for the outgoing email server
 *      all stored under /applications/email/\<X\>/outgoing/</i></td></tr>
 * <tr><td>type</td><td>The type of the
 *     outgoing email server.</td><td>smtp, sendmail.</td></tr>
 * <tr><td>host</td><td>The address of the
 *     outgoing email server.</td><td>An IP address or a domain name.</td></tr>
 * <tr><td>port</td><td>The port number of the
 *         incoming email server.  If not specified the default port number
 *         for the specified protocol will be used.</td><td>An integer
 *     </td></tr>
 * <tr><td>authtype</td><td>The
 *     authentication method to use when logging onto the outgoing email
 *     server. </td><td>PLAIN, NTLM, GSSAPI, CRAM-MD5, DIGEST-MD5,
 *     POPB4SMTP, LOGIN</td></tr>
 * <tr><td>username</td><td>The user name
 *     used to access the outgoing email server.</td><td>A string</td></tr>
 * <tr><td>password</td><td>The password
 *     used to access the outgoing email server.</td><td>A string</td></tr>
 * <tr><td>usessl</td><td>Indicates
 *     whether or not ssl should be used to communicate with the outgoing
 *     email server.</td><td>always, never, when-possible</td></tr>
 * </table>
 *
 * @subsection telephony Telephony Settings
 * Telephony settings are grouped together into contexts. Two different types of
 * contexts are currently supported.  Contexts for 3G access points and contexts
 * for MMS settings.  Provman supports multiple separate 3G
 * contexts but only a single MMS context. If the underlying telephony
 * middleware supports multiple MMS contexts, the telephony plugins should
 * export the first
 * MMS context received from the middleware.  When MMS settings are written via
 * provman and no MMS context exists in the telephony
 * middleware, a new middleware MMS context should be created.  If the first MMS
 * context is deleted, provman will automatically export the
 * second MMS context maintained by the middleware, if such a context exists.
 * The following settings are supported by the provman.
 * <table>
 * <tr><th>Key</th><th>Description</th><th>Permissable Values</th></tr>
 * <tr><td colspan="3" align="center"><i>Settings for 3G contexts all stored
 * under /telephony/contexts/\<X\>/</i></td></tr>
 * <tr><td>name</td><td>The name of the 3G context</td><td>Any string</td></tr>
 * <tr><td>apn</td><td>The address of the access point</td>
 * <td>Any string</td></tr>
 * <tr><td>username</td><td>The user name
 *     used to access the access point.</td><td>A string</td></tr>
 * <tr><td>password</td><td>The password
 *     used to access the access point.</td><td>A string</td></tr>
 * <tr><td>http_proxy</td><td>The address of an HTTP proxy</td><td>An IP address
 *         or a domain name.  The port number, if specified, should be separated
 *         from the proxy address with a ':', e.g., 192.168.0.1:8080
 * </td></tr>
 * <tr><td>https_proxy</td><td>The address of an HTTPS proxy</td>
 *     <td>An IP address
 *         or a domain name.  The port number, if specified, should be separated
 *         from the proxy address with a ':', e.g., 192.168.0.1:8080
 * </td></tr>
 * <tr><td>ftp_proxy</td><td>The address of an FTP proxy</td><td>An IP address
 *         or a domain name.  The port number, if specified, should be separated
 *         from the proxy address with a ':', e.g., 192.168.0.1:8080
 * </td></tr>
 * <tr><td colspan="3" align="center"><i>Settings for MMS context stored under
 *   /telephony/mms</i></td></tr>
 * <tr><td>name</td><td>The name of the MMS context</td><td>Any string</td></tr>
 * <tr><td>apn</td><td>The address of the MMS access point</td>
 *     <td>Any string</td></tr>
 * <tr><td>mmsc</td><td>The address of the MMS service centre</td>
 *     <td>A URL</td></tr>
 * <tr><td>username</td><td>The user name
 *     used to access the MMS access point.</td><td>A string</td></tr>
 * <tr><td>password</td><td>The password
 *     used to access the MMS access point.</td><td>A string</td></tr>
 * <tr><td>proxy</td><td>The address of the MMS proxy</td><td>An IP address
 *         or a domain name.  The port number, if specified, should be separated
 *         from the proxy address with a ':', e.g., 192.168.0.1:8080
 * </td></tr>
 * <tr><td colspan="3" align="center"><i>General telephony settings stored under
 *   /telephony</i></td></tr>
 * <tr><td>imsis</td><td>Contains the IMSI number of each modem enabled on the
 *  device.  If no modems are enabled this setting will not exist.  The first
 *  number listed is the default IMSI number, i.e., the number used if a session
 *  is started by passing an empty string to Start</td>
 *  <td>A comma separated list of imsi numbers</td></tr>
 * </td></tr>
 * </table>
 *
 * @subsection sync Data Synchronisation Settings
 * Synchronisation accounts, e.g., SyncML accounts, can be provisioned via
 * provman.
 *
 * The following settings are supported by the provman.
 * <table>
 * <tr><th>Key</th><th>Description</th><th>Permissable Values</th></tr>
 * <tr><td colspan="3" align="center"><i>Settings for data synchronisation
 * accounts are all stored under /applications/sync/\<X\>/</i></td></tr>
 * <tr><td>name</td><td>The friendly name of the synchronisation account</td>
 * <td>Any string</td></tr><tr><td>username</td><td>The user name
 *     used to access the entity with which you are synchronising.</td>
 * <td>A string</td></tr>
 * <tr><td>password</td><td>The password
 *     used to access the entity with which you are synchronising.</td>
 * <td>A string</td></tr>
 * <tr><td>url</td><td>The URL of the entity with which you are synchronising.
 * </td><td>A string</td></tr>
 * <tr><td>client</td><td>Indicates whether or not the account is for a remote
 * server or a client</td><td>0 (server) or 1 (client) </td></tr>
 * <tr><td colspan="3" align="center"><i>Settings for synchronising contacts
 *   are stored under /applications/sync/\<X\>/contacts/</i></td></tr>
 * <tr><td>format</td><td>Type of the contacts data</td><td>A MIME type
 * </td></tr>
 * <tr><td>sync</td><td>Type of synchronisation to be performed</td>
 *     <td>disabled, two-way, slow, one-way-from-local, refresh-from-local,
 *         one-way-from-remote, refresh-from-remote</td></tr>
 * <tr><td>uri</td><td>Identifies the contacts database on the entity to
 *   which you are synchronising</td>
 *<td>A string</td></tr>
 *<tr><td colspan="3" align="center"><i>Settings for synchronising calendar
 *   entries are stored under /applications/sync/\<X\>/calendar/</i></td></tr>
 * <tr><td>format</td><td>Type of the calendar data</td><td>A MIME type
 * </td></tr>
 * <tr><td>sync</td><td>Type of synchronisation to be performed</td>
 *     <td>disabled, two-way, slow, one-way-from-local, refresh-from-local,
 *         one-way-from-remote, refresh-from-remote</td></tr>
 * <tr><td>uri</td><td>Identifies the calendar database on the entity to
 *   which you are synchronising</td>
 *<td>A string</td></tr>
 *<tr><td colspan="3" align="center"><i>Settings for synchronising task
 *   entries are stored under /applications/sync/\<X\>/todo/</i></td></tr>
 * <tr><td>format</td><td>Type of the todo data</td><td>A MIME type</td></tr>
 * <tr><td>sync</td><td>Type of synchronisation to be performed</td>
 *     <td>disabled, two-way, slow, one-way-from-local, refresh-from-local,
 *         one-way-from-remote, refresh-from-remote</td></tr>
 * <tr><td>uri</td><td>Identifies the todo database on the entity to
 *   which you are synchronising</td>
 *<td>A string</td></tr>
 *<tr><td colspan="3" align="center"><i>Settings for synchronising
 *   memos are stored under /applications/sync/\<X\>/memo/</i></td></tr>
 * <tr><td>format</td><td>Type of the memo data</td><td>A MIME type</td></tr>
 * <tr><td>sync</td><td>Type of synchronisation to be performed</td>
 *     <td>disabled, two-way, slow, one-way-from-local, refresh-from-local,
 *         one-way-from-remote, refresh-from-remote</td></tr>
 * <tr><td>uri</td><td>Identifies the memo database on the entity to
 *   which you are synchronising</td>
 *<td>A string</td></tr>
 * <tr><td colspan="3" align="center"><i>Settings for synchronising
 *   active sync contacts are stored under
 *   /applications/sync/\<X\>/eas-contacts/</i></td></tr>
 * <tr><td>format</td><td>Type of the contacts data</td><td>A MIME type
 * </td></tr>
 * <tr><td>sync</td><td>Type of synchronisation to be performed</td>
 *     <td>disabled, two-way, slow, one-way-from-local, refresh-from-local,
 *         one-way-from-remote, refresh-from-remote</td></tr>
 * <tr><td>uri</td><td>Identifies the contacts database on the entity to
 *   which you are synchronising</td>
 *<td>A string</td></tr>
 *<tr><td colspan="3" align="center"><i>Settings for synchronising active sync
 *   calendar entries are stored under /applications/sync/\<X\>/eas-calendar/
 *   </i></td></tr>
 * <tr><td>format</td><td>Type of the calendar data</td><td>A MIME type
 * </td></tr>
 * <tr><td>sync</td><td>Type of synchronisation to be performed</td>
 *     <td>disabled, two-way, slow, one-way-from-local, refresh-from-local,
 *         one-way-from-remote, refresh-from-remote</td></tr>
 * <tr><td>uri</td><td>Identifies the calendar database on the entity to
 *   which you are synchronising</td>
 *<td>A string</td></tr>
 *<tr><td colspan="3" align="center"><i>Settings for synchronising active sync
 *   task entries are stored under /applications/sync/\<X\>/eas-todo/
 *   </i></td></tr>
 * <tr><td>format</td><td>Type of the todo data</td><td>A MIME type</td></tr>
 * <tr><td>sync</td><td>Type of synchronisation to be performed</td>
 *     <td>disabled, two-way, slow, one-way-from-local, refresh-from-local,
 *         one-way-from-remote, refresh-from-remote</td></tr>
 * <tr><td>uri</td><td>Identifies the todo database on the entity to
 *   which you are synchronising</td>
 *<td>A string</td></tr>
 *<tr><td colspan="3" align="center"><i>Settings for synchronising
 *   active sync memos are stored under /applications/sync/\<X\>/eas-memo/
 *   </i></td></tr>
 * <tr><td>format</td><td>Type of the memo data</td><td>A MIME type</td></tr>
 * <tr><td>sync</td><td>Type of synchronisation to be performed</td>
 *     <td>disabled, two-way, slow, one-way-from-local, refresh-from-local,
 *         one-way-from-remote, refresh-from-remote</td></tr>
 * <tr><td>uri</td><td>Identifies the memo database on the entity to
 *   which you are synchronising</td>
 *<td>A string</td></tr>
 * </table>
 *
 * @subsection browser Browser Settings
 * A single set of brower settings can be provisioned.  These consist of a start
 * page and one or more bookmarks.
 *
 * The following settings are supported by the provman.
 * <table>
 * <tr><th>Key</th><th>Description</th><th>Permissable Values</th></tr>
 * <tr><td colspan="3" align="center"><i>Settings for the browser startpage
 * are stored under /applications/browser/startpage</i></td></tr>
 * <tr><td>name</td><td>The friendly name of the startpage</td>
 * <td>Any string</td></tr>
 * <tr><td>url</td><td>The URL of the startpage.</td><td>A URL</td></tr>
 * <tr><td colspan="3" align="center"><i>Settings for browser bookmarks
 * are stored under /applications/browser/bookmarks/\<X\></i></td></tr>
 * <tr><td>name</td><td>The friendly name of the bookmark</td>
 * <td>Any string</td></tr>
 * <tr><td>url</td><td>The URL of the bookmark</td><td>A URL</td></tr>
 * </table>
 *
 * @subsection omadm Device Management Settings
 * OMA Device Management accounts can be provisioned via provman.
 *
 * The following settings are supported by the provman.
 * <table>
 * <tr><th>Key</th><th>Description</th><th>Permissable Values</th></tr>
 * <tr><td colspan="3" align="center"><i>Settings for OMA DM
 * accounts are all stored under /applications/omadm/\<X\>/</i></td></tr>
 * <tr><td>name</td><td>The friendly name of the DM account</td>
 *  <td>A string</td></tr>
 * <tr><td>server_id</td><td>The DM server identifier</td>
 *  <td>A string</td></tr>
 * <tr><td>url</td><td>The URL of the DM server.  The port number, if specified,
 * should be separated from the server address with a ':', e.g.,
 * 192.168.0.1:8080 containing an optioanl port.</td><td>A URL</td></tr>
 * <tr><td colspan="3" align="center"><i>Settings for OMA DM
 * server credentials are all stored under
 * /applications/omadm/\<X\>/server_creds</i></td></tr>
 * <tr><td>username</td><td>The user name
 *     the client should use to authenticate with the server.</td>
 * <td>A string</td></tr>
 * <tr><td>password</td><td>The password the client should use to
 *     to authenticate with the server.</td>
 * <td>A string</td></tr>
 * <tr><td>nonce</td><td>The nonce a client should use to authenticate
 * with the server.</td><td>A base 64 encoded string</td></tr>
 * <tr><td>authtype</td><td>The authentication method to use.</td>
 * <td>http-basic, http-digest, basic,digest, x509, secure-id, safeword,
 * digipass</td></tr>
 * <tr><td colspan="3" align="center"><i>Settings for OMA DM
 * client credentials are all stored under
 * /applications/omadm/\<X\>/client_creds</i></td></tr>
 * <tr><td>username</td><td>The user name
 *     the server should use to authenticate with the client.</td>
 * <td>A string</td></tr>
 * <tr><td>password</td><td>The password the server should use to
 *     to authenticate with the client.</td>
 * <td>A string</td></tr>
 * <tr><td>nonce</td><td>The nonce a server should use to authenticate
 * with the client.</td>
 * <td>A base 64 encoded string</td></tr>
 * <tr><td>authtype</td><td>The authentication method to use.</td>
 * <td>http-basic, http-digest, basic,digest, x509, secure-id, safeword,
 * digipass</td></tr>
 * <tr><td colspan="3" align="center"><i>Settings for HTTP
 * authentication are all stored under
 * /applications/omadm/\<X\>/http_creds</i></td></tr>
 * <tr><td>username</td><td>The user name
 *     used in http authentication.</td>
 * <td>A string</td></tr>
 * <tr><td>password</td><td>The password used in http authentication.</td>
 * <td>A string</td></tr>
 * <tr><td>authtype</td><td>The authentication method to use.</td>
 * <td>http-basic, http-digest, basic,digest, x509, secure-id, safeword,
 * digipass</td></tr>
 * </table>
 *
 *
 * @section development Developing for Provman
 *
 * Please read the coding-style.h page before submitting any patches to
 * provman.
 ******************************************************************************/

