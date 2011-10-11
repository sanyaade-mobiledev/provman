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
 * @file test_schemas.c
 *
 * @brief contains schemas test plugins
 *
 *****************************************************************************/

#include "config.h"

#include <glib.h>

/* We use two spaces to indent here otherwise we would not be able to fit
   this schema into 80 chars. */

#ifdef PROVMAN_TEST_PLUGIN
const gchar g_provman_test_schema[] =
	"<schema root='/applications/test_plugin/'>"
	"  <dir delete='yes'>"
	"    <key name='key_deletable' delete='yes' type='string'/>"
	"    <key name='key_undeletable' delete='no' type='string'/>"
	"    <dir name='subdir' delete='yes'>"
	"       <key name='key1' delete='yes' type='string'/>"
	"       <key name='key2' delete='yes' type='string'/>"
	"       <key name='key3' delete='yes' type='string'/>"
	"    </dir>"
	"    <dir name='subdir_deletable' delete='yes'>"
	"      <key name='string_deletable' delete='yes' type='string'/>"
	"      <key name='int_deletable' delete='yes' type='int'/>"
	"      <key name='enum_deletable' delete='yes' type='enum' "
	"           values='val1, val2, val3'/>"
	"      <key name='string_undeletable' delete='no' type='string'/>"
	"      <key name='int_undeletable' delete='no' type='int'/>"
	"      <key name='enum_undeletable' delete='no' type='enum' "
	"           values='val1, val2, val3'/>"
	"    </dir>"
	"    <dir name='subdir_undeletable' delete='no'>"
	"      <key name='string_deletable' delete='yes' type='string'/>"
	"      <key name='int_deletable' delete='yes' type='int'/>"
	"      <key name='enum_deletable' delete='yes' type='enum' "
	"           values='val1, val2, val3'/>"
	"      <key name='string_undeletable' delete='no' type='string'/>"
	"      <key name='int_undeletable' delete='no' type='int'/>"
	"      <key name='enum_undeletable' delete='no' type='enum' "
	"           values='val1, val2, val3'/>"
	"    </dir>"
	"    <dir name='subdir_key_rw' delete='yes'>"
	"      <key name='key1_ro_undel' delete='no' write='no' type='string'/>"
	"      <key name='key1_rw_undel' delete='no' write='yes' type='string'/>"
	"      <key name='key2_ro_undel' delete='no' write='no' type='string'/>"
	"      <key name='key2_rw_undel' delete='no' write='yes' type='string'/>"
	"      <key name='key1_ro_del' delete='yes' write='no' type='string'/>"
	"      <key name='key1_rw_del' delete='yes' write='yes' type='string'/>"
	"      <key name='key2_ro_del' delete='yes' write='no' type='string'/>"
	"      <key name='key2_rw_del' delete='yes' write='yes' type='string'/>"
	"      <key name='key_rw_string' delete='yes' write='yes' type='string'/>"
	"      <key name='key_rw_int' delete='yes' write='yes' type='int'/>"
	"      <key name='key_rw_enum' delete='yes' write='yes' type='enum' "
	"           values='val1, val2, val3'/>"
	"      <key name='key_ro_string' delete='yes' write='no' type='string'/>"
	"      <key name='key_ro_int' delete='yes' write='no' type='int'/>"
	"      <key name='key_ro_enum' delete='yes' write='no' type='enum' "
	"           values='val1, val2, val3'/>"
	"    </dir>"
	"    <dir name='subdir_big_enum' delete='yes'>"
	"      <key name='enum' delete='yes' type='enum' "
	"           values='val001, val002, val003, val004, val005, val006, "
	"                   val007, val008, val009, val010, val011, val012, "
	"                   val013, val014, val015, val016, val017, val018, "
	"                   val019, val020, val021, val022, val023, val024, "
	"                   val025, val026, val027, val028, val029, val030, "
	"                   val031, val032, val033, val034, val035, val036, "
	"                   val037, val038, val039, val040, val041, val042, "
	"                   val043, val044, val045, val046, val047, val048, "
	"                   val049, val050, val051, val052, val053, val054, "
	"                   val055, val056, val057, val058, val059, val060, "
	"                   val061, val062, val063, val064, val065, val066, "
	"                   val067, val068, val069, val070, val071, val072, "
	"                   val073, val074, val075, val076, val077, val078, "
	"                   val079, val080, val081, val082, val083, val084, "
	"                   val085, val086, val087, val088, val089, val090, "
	"                   val091, val092, val093, val094, val095, val096, "
	"                   val097, val098, val099, val100'/>"
	"    </dir>"
	"    <dir name='subdir_many_keys' delete='yes'>"
	"      <key name='key001' delete='yes' type='string'/>"
	"      <key name='key002' delete='yes' type='string'/>"
	"      <key name='key003' delete='yes' type='string'/>"
	"      <key name='key004' delete='yes' type='string'/>"
	"      <key name='key005' delete='yes' type='string'/>"
	"      <key name='key006' delete='yes' type='string'/>"
	"      <key name='key007' delete='yes' type='string'/>"
	"      <key name='key008' delete='yes' type='string'/>"
	"      <key name='key009' delete='yes' type='string'/>"
	"      <key name='key010' delete='yes' type='string'/>"
	"      <key name='key011' delete='yes' type='string'/>"
	"      <key name='key012' delete='yes' type='string'/>"
	"      <key name='key013' delete='yes' type='string'/>"
	"      <key name='key014' delete='yes' type='string'/>"
	"      <key name='key015' delete='yes' type='string'/>"
	"      <key name='key016' delete='yes' type='string'/>"
	"      <key name='key017' delete='yes' type='string'/>"
	"      <key name='key018' delete='yes' type='string'/>"
	"      <key name='key019' delete='yes' type='string'/>"
	"      <key name='key020' delete='yes' type='string'/>"
	"      <key name='key021' delete='yes' type='string'/>"
	"      <key name='key022' delete='yes' type='string'/>"
	"      <key name='key023' delete='yes' type='string'/>"
	"      <key name='key024' delete='yes' type='string'/>"
	"      <key name='key025' delete='yes' type='string'/>"
	"      <key name='key026' delete='yes' type='string'/>"
	"      <key name='key027' delete='yes' type='string'/>"
	"      <key name='key028' delete='yes' type='string'/>"
	"      <key name='key029' delete='yes' type='string'/>"
	"      <key name='key030' delete='yes' type='string'/>"
	"      <key name='key031' delete='yes' type='string'/>"
	"      <key name='key032' delete='yes' type='string'/>"
	"      <key name='key033' delete='yes' type='string'/>"
	"      <key name='key034' delete='yes' type='string'/>"
	"      <key name='key035' delete='yes' type='string'/>"
	"      <key name='key036' delete='yes' type='string'/>"
	"      <key name='key037' delete='yes' type='string'/>"
	"      <key name='key038' delete='yes' type='string'/>"
	"      <key name='key039' delete='yes' type='string'/>"
	"      <key name='key040' delete='yes' type='string'/>"
	"      <key name='key041' delete='yes' type='string'/>"
	"      <key name='key042' delete='yes' type='string'/>"
	"      <key name='key043' delete='yes' type='string'/>"
	"      <key name='key044' delete='yes' type='string'/>"
	"      <key name='key045' delete='yes' type='string'/>"
	"      <key name='key046' delete='yes' type='string'/>"
	"      <key name='key047' delete='yes' type='string'/>"
	"      <key name='key048' delete='yes' type='string'/>"
	"      <key name='key049' delete='yes' type='string'/>"
	"      <key name='key050' delete='yes' type='string'/>"
	"      <key name='key051' delete='yes' type='string'/>"
	"      <key name='key052' delete='yes' type='string'/>"
	"      <key name='key053' delete='yes' type='string'/>"
	"      <key name='key054' delete='yes' type='string'/>"
	"      <key name='key055' delete='yes' type='string'/>"
	"      <key name='key056' delete='yes' type='string'/>"
	"      <key name='key057' delete='yes' type='string'/>"
	"      <key name='key058' delete='yes' type='string'/>"
	"      <key name='key059' delete='yes' type='string'/>"
	"      <key name='key060' delete='yes' type='string'/>"
	"      <key name='key061' delete='yes' type='string'/>"
	"      <key name='key062' delete='yes' type='string'/>"
	"      <key name='key063' delete='yes' type='string'/>"
	"      <key name='key064' delete='yes' type='string'/>"
	"      <key name='key065' delete='yes' type='string'/>"
	"      <key name='key066' delete='yes' type='string'/>"
	"      <key name='key067' delete='yes' type='string'/>"
	"      <key name='key068' delete='yes' type='string'/>"
	"      <key name='key069' delete='yes' type='string'/>"
	"      <key name='key070' delete='yes' type='string'/>"
	"      <key name='key071' delete='yes' type='string'/>"
	"      <key name='key072' delete='yes' type='string'/>"
	"      <key name='key073' delete='yes' type='string'/>"
	"      <key name='key074' delete='yes' type='string'/>"
	"      <key name='key075' delete='yes' type='string'/>"
	"      <key name='key076' delete='yes' type='string'/>"
	"      <key name='key077' delete='yes' type='string'/>"
	"      <key name='key078' delete='yes' type='string'/>"
	"      <key name='key079' delete='yes' type='string'/>"
	"      <key name='key080' delete='yes' type='string'/>"
	"      <key name='key081' delete='yes' type='string'/>"
	"      <key name='key082' delete='yes' type='string'/>"
	"      <key name='key083' delete='yes' type='string'/>"
	"      <key name='key084' delete='yes' type='string'/>"
	"      <key name='key085' delete='yes' type='string'/>"
	"      <key name='key086' delete='yes' type='string'/>"
	"      <key name='key087' delete='yes' type='string'/>"
	"      <key name='key088' delete='yes' type='string'/>"
	"      <key name='key089' delete='yes' type='string'/>"
	"      <key name='key090' delete='yes' type='string'/>"
	"      <key name='key091' delete='yes' type='string'/>"
	"      <key name='key092' delete='yes' type='string'/>"
	"      <key name='key093' delete='yes' type='string'/>"
	"      <key name='key094' delete='yes' type='string'/>"
	"      <key name='key095' delete='yes' type='string'/>"
	"      <key name='key096' delete='yes' type='string'/>"
	"      <key name='key097' delete='yes' type='string'/>"
	"      <key name='key098' delete='yes' type='string'/>"
	"      <key name='key099' delete='yes' type='string'/>"
	"      <key name='key100' delete='yes' type='string'/>"
	"    </dir>"
	"    <dir name='subdir_many_levels' delete='yes'>"
	"      <dir name='subdir' delete='yes'>"
	"        <dir name='subdir' delete='yes'>"
	"          <dir name='subdir' delete='yes'>"
	"            <dir name='subdir' delete='yes'>"
	"              <dir name='subdir' delete='yes'>"
	"                <dir name='subdir' delete='yes'>"
	"                  <dir name='subdir' delete='yes'>"
	"                    <dir name='subdir' delete='yes'>"
	"                      <dir name='subdir' delete='yes'>"
	"                        <dir name='subdir' delete='yes'>"
	"                          <dir name='subdir' delete='yes'>"
	"                            <dir name='subdir' delete='yes'>"
	"                              <dir name='subdir' delete='yes'>"
	"                                <dir name='subdir' delete='yes'>"
	"                                  <dir name='subdir' delete='yes'>"
	"                                    <dir name='subdir' delete='yes'>"
	"                                      <dir name='subdir' delete='yes'>"
	"                                        <dir name='subdir' "
	"                                          delete='yes'>"
	"                                          <dir name='subdir' "
	"                                            delete='yes'>"
	"                                            <key name='key' "
	"                                              delete='yes' "
	"                                              type='string'/>"
	"                                          </dir>"
	"                                        </dir>"
	"                                      </dir>"
	"                                    </dir>"
	"                                  </dir>"
	"                                </dir>"
	"                              </dir>"
	"                            </dir>"
	"                          </dir>"
	"                        </dir>"
	"                      </dir>"
	"                    </dir>"
	"                  </dir>"
	"                </dir>"
	"              </dir>"
	"            </dir>"
	"          </dir>"
	"        </dir>"
	"      </dir>"
	"    </dir>"
	"  </dir>"
	"</schema>";
#endif
