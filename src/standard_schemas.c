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
 * @file standard_schemas.c
 *
 * @brief contains schemas for support interfaces
 *
 *****************************************************************************/

#include <glib.h>

const gchar g_provman_email_schema[] =
	"<schema root='/applications/email/'>"
	"    <dir delete='yes'>"
	"        <key name='address' delete='no' type='string'/>"
	"        <key name='name' delete='no' type='string'/>"
	"        <dir name='incoming' delete='no'>"
	"            <key name='host' delete='no' type='string'/>"
	"            <key name='password' delete='no' type='string'/>"
	"            <key name='port' delete='no' type='int'/>"
	"            <key name='type' delete='no' type='enum'"
        "                values='pop, imap, imapx, exchange, ews, groupwise,"
	"                       nntp, mbox, mh, maildir, spooldir, spool'/>"
	"            <key name='authtype' delete='no' type='enum'"
        "                values='+APOP, CRAM-MD5, DIGEST-MD5, GSSAPI, PLAIN,"
	"                        POPB4SMTP, NTLM'/>"
	"            <key name='username' delete='no' type='string'/>"
	"            <key name='usessl' delete='no' type='enum'"
        "                values='always, never, when-possible'/>"
        "        </dir>"
	"        <dir name='outgoing' delete='no'>"
	"            <key name='host' delete='no' type='string'/>"
	"            <key name='password' delete='no' type='string'/>"
	"            <key name='port' delete='no' type='int'/>"
	"            <key name='type' delete='no' type='enum'"
        "                values='smtp, sendmail, ews'/>"
	"            <key name='authtype' delete='no' type='enum'"
        "                values='PLAIN, NTLM, GSSAPI, CRAM-MD5, DIGEST-MD5,"
	"                        POPB4SMTP, LOGIN'/>"
	"            <key name='username' delete='no' type='string'/>"
	"            <key name='usessl' delete='no' type='enum'"
        "                values='always, never, when-possible'/>"
        "        </dir>"
	"    </dir>"
	"</schema>";

const gchar g_provman_telephony_schema[] =
	"<schema root='/telephony/'>"
	"    <dir name='contexts' delete='yes'>"
	"        <dir delete='yes'>"
	"            <key name='apn' delete='no' type='string'/>"
	"            <key name='name' delete='no' type='string'/>"
	"            <key name='password' delete='no' type='string'/>"
	"            <key name='username' delete='no' type='string'/>"
        "        </dir>"
        "    </dir>"
	"    <dir name='mms' delete='yes'>"
	"        <key name='apn' delete='no' type='string'/>"
	"        <key name='name' delete='no' type='string'/>"
	"        <key name='password' delete='no' type='string'/>"
	"        <key name='username' delete='no' type='string'/>"
	"        <key name='mmsc' delete='no' type='string'/>"
	"        <key name='proxy' delete='no' type='string'/>"
        "    </dir>"
	"    <key name='imsis' delete='no' write='no' type='string'/>"
	"</schema>";

const gchar g_provman_sync_schema[] =
	"<schema root='/applications/sync/'>"
	"    <dir delete='yes'>"
	"        <key name='name' delete='no' type='string'/>"
	"        <key name='password' delete='no' type='string'/>"
	"        <key name='username' delete='no' type='string'/>"
	"        <key name='url' delete='no' type='string'/>"
	"        <key name='client' delete='no' type='enum' values='0, 1'/>"
	"        <dir name='contacts' delete='no'>"
	"            <key name='format' delete='no' type='string'/>"
	"            <key name='sync' delete='no' type='enum'"
        "                values='disabled, two-way, slow, one-way-from-client,"
	"                        refresh-from-client, refresh-from-server,"
	"                        restore-from-backup'/>"
	"            <key name='uri' delete='no' type='string'/>"
        "        </dir>"
	"        <dir name='calendar' delete='no'>"
	"            <key name='format' delete='no' type='string'/>"
	"            <key name='sync' delete='no' type='enum'"
        "                values='disabled, two-way, slow, one-way-from-client,"
	"                        refresh-from-client, refresh-from-server,"
	"                        restore-from-backup'/>"
	"            <key name='uri' delete='no' type='string'/>"
        "        </dir>"
	"        <dir name='todo' delete='no'>"
	"            <key name='format' delete='no' type='string'/>"
	"            <key name='sync' delete='no' type='enum'"
        "                values='disabled, two-way, slow, one-way-from-client,"
	"                        refresh-from-client, refresh-from-server,"
	"                        restore-from-backup'/>"
	"            <key name='uri' delete='no' type='string'/>"
        "        </dir>"
	"        <dir name='memo' delete='no'>"
	"            <key name='format' delete='no' type='string'/>"
	"            <key name='sync' delete='no' type='enum'"
        "                values='disabled, two-way, slow, one-way-from-client,"
	"                        refresh-from-client, refresh-from-server,"
	"                        restore-from-backup'/>"
	"            <key name='uri' delete='no' type='string'/>"
        "        </dir>"
	"        <dir name='eas-contacts' delete='no'>"
	"            <key name='format' delete='no' type='string'/>"
	"            <key name='sync' delete='no' type='enum'"
        "                values='disabled, two-way, slow, one-way-from-client,"
	"                        refresh-from-client, refresh-from-server,"
	"                        restore-from-backup'/>"
	"            <key name='uri' delete='no' type='string'/>"
        "        </dir>"
	"        <dir name='eas-calendar' delete='no'>"
	"            <key name='format' delete='no' type='string'/>"
	"            <key name='sync' delete='no' type='enum'"
        "                values='disabled, two-way, slow, one-way-from-client,"
	"                        refresh-from-client, refresh-from-server,"
	"                        restore-from-backup'/>"
	"            <key name='uri' delete='no' type='string'/>"
        "        </dir>"
	"        <dir name='eas-todo' delete='no'>"
	"            <key name='format' delete='no' type='string'/>"
	"            <key name='sync' delete='no' type='enum'"
        "                values='disabled, two-way, slow, one-way-from-client,"
	"                        refresh-from-client, refresh-from-server,"
	"                        restore-from-backup'/>"
	"            <key name='uri' delete='no' type='string'/>"
        "        </dir>"
	"        <dir name='eas-memo' delete='no'>"
	"            <key name='format' delete='no' type='string'/>"
	"            <key name='sync' delete='no' type='enum'"
        "                values='disabled, two-way, slow, one-way-from-client,"
	"                        refresh-from-client, refresh-from-server,"
	"                        restore-from-backup'/>"
	"            <key name='uri' delete='no' type='string'/>"
        "        </dir>"
	"    </dir>"
	"</schema>";


