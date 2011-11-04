#! /usr/bin/python


# Provman
#
# Copyright (C) 2011 Intel Corporation. All rights reserved.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License version
# 2 as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.
#
#
# Jerome Blin <jerome.blin@intel.com>

import testprovman, unittest, commands, time, os, sys


#check Python's version >= 2.7
if sys.version_info < (2, 7):
    print "ERROR. This script requires newer version of Python."
    raise SystemExit(1)


#global constants
BUS_TYPE_SESSION = "session"
BUS_TYPE_SYSTEM  = "system"
IMSI_DEFAULT_SIM = ""

PROVMAN_EXCEPT_UNEXPECTED  = "com.intel.provman.Error.Unexpected"
PROVMAN_EXCEPT_CANCELLED   = "com.intel.provman.Error.Cancelled"
PROVMAN_EXCEPT_UNKNOWN     = "com.intel.provman.Error.Unknown"
PROVMAN_EXCEPT_OOM         = "com.intel.provman.Error.Oom"
PROVMAN_EXCEPT_NOT_FOUND   = "com.intel.provman.Error.NotFound"
PROVMAN_EXCEPT_BAD_ARGS    = "com.intel.provman.Error.BadArgs"
PROVMAN_EXCEPT_IN_PROGRESS = "com.intel.provman.Error.TransactionInProgress"
PROVMAN_EXCEPT_NO_TRANSAC  = "com.intel.provman.Error.NotInTransaction"
PROVMAN_EXCEPT_DENIED      = "com.intel.provman.Error.Denied"

PROVMAN_PROCESS_TIMEOUT_SEC    = 30

PROVMAN_PROCESS_SESSION    = "provman-session"
PROVMAN_PROCESS_SYSTEM     = "provman-system"


#returns a dictionary: {'/path/key001': 'val001, ... '/path/key100': 'val100}
def many_keys(path):
    many_keys = {}
    for i in range(1,101):
        i_leading_zero = str(i)
        while len(i_leading_zero) < 3:
            i_leading_zero = "0" + i_leading_zero
        many_keys[path + "key" + i_leading_zero] = "val" + i_leading_zero
    return many_keys


class TestProvmanTestCases(testprovman.TestProvman):

    """Each method in this class represents an isolated test case.

    TODO:
        -   Separate test cases into test suites. We may use the method's name
            prefix to determine the tested feature and the type of the test
            case (i.e.: "test_sim_posi_xxxxxxx" means tested feature is SIM and
            test case is of type functional positive).
    """
    
    def test_sim_posi_set_key_specific_to_sim(self):

        """test_sim_posi_set_key_specific_to_sim"""
        
        #Set key specific to a SIM card

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_specific_sim)
        self.reset()
        self.get_auto(key1, "", PROVMAN_EXCEPT_NOT_FOUND)
        self.set_auto(key1, key1_val)
        self.get_auto(key1, key1_val)

    def test_sim_posi_set_key_not_specific_to_sim(self):
    
        """test_sim_posi_set_key_not_specific_to_sim"""
        
        #Set key not specific to a SIM card

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_default_sim)
        self.reset()
        self.get_auto(key1, "", PROVMAN_EXCEPT_NOT_FOUND)
        self.set_auto(key1, key1_val)
        self.get_auto(key1, key1_val)

    def test_sim_posi_delete_key_dir_specific_to_sim(self):

        """test_sim_posi_delete_key_dir_specific_to_sim"""
        
        #Delete key/directory specific to a SIM card

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_specific_sim)

        #delete key
        self.reset()
        self.set_auto(key_del, key_del_val)
        self.get_auto(key_del, key_del_val)
        self.delete_auto(key_del)
        self.get_auto(key_del, "", PROVMAN_EXCEPT_NOT_FOUND)

        #delete directory
        self.reset()
        self.set_auto(key_dir_del, key_dir_del_val)
        self.get_auto(key_dir_del, key_dir_del_val)
        self.delete_auto(dir_del)
        self.get_auto(dir_del, "", PROVMAN_EXCEPT_NOT_FOUND)
        self.get_auto(key_dir_del, "", PROVMAN_EXCEPT_NOT_FOUND)

    def test_sim_posi_delete_key_dir_not_specific_to_sim(self):
    
        """test_sim_posi_delete_key_dir_not_specific_to_sim"""
        
        #Delete key/directory not specific to a SIM card

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_default_sim)
        
        #delete key
        self.reset()
        self.set_auto(key_del, key_del_val)
        self.get_auto(key_del, key_del_val)
        self.delete_auto(key_del)
        self.get_auto(key_del, "", PROVMAN_EXCEPT_NOT_FOUND)

        #delete directory
        self.reset()
        self.set_auto(key_dir_del, key_dir_del_val)
        self.get_auto(key_dir_del, key_dir_del_val)
        self.delete_auto(dir_del)
        self.get_auto(dir_del, "", PROVMAN_EXCEPT_NOT_FOUND)
        self.get_auto(key_dir_del, "", PROVMAN_EXCEPT_NOT_FOUND)

    def test_sim_posi_set_identical_key_multiple_sims(self):

        """test_sim_posi_set_identical_key_multiple_sims"""

        #Set identical key to multiple SIM cards

        self.set_bus_type(bus_type_any)

        #initialize data trees
        self.set_imsi("001")
        self.reset()
        self.set_imsi("002")
        self.reset()
        self.set_imsi("003")
        self.reset()

        #set/get key
        self.set_imsi("001")
        self.set_auto(key1, key1_val)
        self.set_imsi("002")
        self.set_auto(key1, key1_val)
        self.set_imsi("003")
        self.set_auto(key1, key1_val)
        self.set_imsi("001")
        self.get_auto(key1, key1_val)
        self.set_imsi("002")
        self.get_auto(key1, key1_val)
        self.set_imsi("003")
        self.get_auto(key1, key1_val)

    def test_keys_posi_set_key_in_subdir(self):

        """test_keys_posi_set_key_in_subdir"""

        #Set key (located in a subdirectory)

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()

        #set new key
        self.get_auto(key1, "", PROVMAN_EXCEPT_NOT_FOUND)
        self.set_auto(key1, key1_val)
        self.get_auto(key1, key1_val)

        #set already existing key
        self.set_auto(key1, key1_val)
        self.set_auto(key1, "new value - test_keys_posi_set_key_in_subdir")
        self.get_auto(key1, "new value - test_keys_posi_set_key_in_subdir")

    def test_keys_posi_delete_key_dir(self):
    
        """test_keys_posi_delete_key_dir"""
        
        #Delete key/directory

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)

        #delete key
        self.reset()
        self.set_auto(key_del, key_del_val)
        self.get_auto(key_del, key_del_val)
        self.delete_auto(key_del)
        self.get_auto(key_del, "", PROVMAN_EXCEPT_NOT_FOUND)

        #delete directory
        self.reset()
        self.set_auto(key_dir_del, key_dir_del_val)
        self.get_auto(key_dir_del, key_dir_del_val)
        self.delete_auto(dir_del)
        self.get_auto(dir_del, "", PROVMAN_EXCEPT_NOT_FOUND)
        self.get_auto(key_dir_del, "", PROVMAN_EXCEPT_NOT_FOUND)

    def test_keys_posi_set_same_key_twice(self):

        """test_keys_posi_set_same_key_twice"""

        #Set same key twice

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)

        #set in separate DM sessions
        self.reset()
        self.get_auto(key1, "", PROVMAN_EXCEPT_NOT_FOUND)
        self.set_auto(key1, key1_val)
        self.set_auto(key1, key1_val)   #set twice
        self.get_auto(key1, key1_val)

        #set in same DM session
        self.reset()
        self.get_auto(key1, "", PROVMAN_EXCEPT_NOT_FOUND)
        self.connect_dbus()
        self.start()
        self.set(key1, key1_val)
        self.set(key1, key1_val)    #set twice
        self.end()
        self.get_auto(key1, key1_val)

    def test_keys_posi_create_implicit_dir(self):

        """test_keys_posi_create_implicit_dir"""

        #Set key and create implicit directory structure

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        self.get_auto(key1, "", PROVMAN_EXCEPT_NOT_FOUND)
        self.set_auto(key1, key1_val)
        self.get_auto(key1, key1_val)

    def test_keys_posi_get(self):

        """test_keys_posi_get"""

        #Get key with Get

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        self.set_auto(key1, key1_val)
        self.get_auto(key1, key1_val)

    def test_keys_posi_get_all_key(self):

        """test_keys_posi_get_all_key"""

        #Get a single key with GetAll

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        self.set_auto(key1, key1_val)
        self.get_all_auto(key1, {key1 : key1_val})

    def test_keys_posi_get_all_dir(self):

        """test_keys_posi_get_all_dir"""

        #Get directory with GetAll

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        self.set_mult_auto(keys)
        self.get_all_auto(keys_subdir, keys)

    def test_keys_posi_get_session(self):

        """test_keys_posi_get_session"""

        #Get key from session bus

        self.set_bus_type(BUS_TYPE_SESSION)
        self.set_imsi(imsi_any)
        self.reset()
        self.set_auto(key1, key1_val)
        self.get_auto(key1, key1_val)

    def test_keys_posi_get_system(self):

        """test_keys_posi_get_system"""

        #Get key from system bus

        self.set_bus_type(BUS_TYPE_SYSTEM)
        self.set_imsi(imsi_any)
        self.reset()
        self.set_auto(key1, key1_val)
        self.get_auto(key1, key1_val)

    def test_keys_posi_get_all_session(self):

        """test_keys_posi_get_all_session"""

        #GetAll from session bus

        self.set_bus_type(BUS_TYPE_SESSION)
        self.set_imsi(imsi_any)
        self.reset()
        self.set_mult_auto(keys)
        self.get_all_auto(keys_subdir, keys)

    def test_keys_posi_get_all_system(self):

        """test_keys_posi_get_all_system"""

        #GetAll from system bus

        self.set_bus_type(BUS_TYPE_SYSTEM)
        self.set_imsi(imsi_any)
        self.reset()
        self.set_mult_auto(keys)
        self.get_all_auto(keys_subdir, keys)

    def test_keys_posi_set_session(self):

        """test_keys_posi_set_session"""

        #Set key from session bus

        self.set_bus_type(BUS_TYPE_SESSION)
        self.set_imsi(imsi_any)
        self.reset()
        self.get_auto(key1, "", PROVMAN_EXCEPT_NOT_FOUND)
        self.set_auto(key1, key1_val)
        self.get_auto(key1, key1_val)

    def test_keys_posi_set_system(self):

        """test_keys_posi_set_system"""

        #Set key from system bus

        self.set_bus_type(BUS_TYPE_SYSTEM)
        self.set_imsi(imsi_any)
        self.reset()
        self.get_auto(key1, "", PROVMAN_EXCEPT_NOT_FOUND)
        self.set_auto(key1, key1_val)
        self.get_auto(key1, key1_val)

    def test_keys_posi_set_mult_session(self):

        """test_keys_posi_set_mult_session"""

        #SetMultiple from session bus

        self.set_bus_type(BUS_TYPE_SESSION)
        self.set_imsi(imsi_any)
        self.reset()
        self.get_all_auto(keys_subdir, {}, PROVMAN_EXCEPT_NOT_FOUND)
        self.set_mult_auto(keys)
        self.get_all_auto(keys_subdir, keys)

    def test_keys_posi_set_mult_system(self):

        """test_keys_posi_set_mult_system"""

        #SetMultiple from system bus

        self.set_bus_type(BUS_TYPE_SYSTEM)
        self.set_imsi(imsi_any)
        self.reset()
        self.get_all_auto(keys_subdir, {}, PROVMAN_EXCEPT_NOT_FOUND)
        self.set_mult_auto(keys)
        self.get_all_auto(keys_subdir, keys)

    def test_keys_posi_set_mult_implicit_dir(self):

        """test_keys_posi_set_mult_implicit_dir"""

        #SetMultiple keys and create implicit directory structure

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        self.get_all_auto(keys_subdir, {}, PROVMAN_EXCEPT_NOT_FOUND)
        self.set_mult_auto(keys)
        self.get_all_auto(keys_subdir, keys)

    def test_keys_posi_set_mult_one_key(self):

        """test_keys_posi_set_mult_one_key"""

        #SetMultiple one single key

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        self.get_auto(key1, "", PROVMAN_EXCEPT_NOT_FOUND)
        self.set_mult_auto({key1 : key1_val})
        self.get_all_auto(key1, {key1 : key1_val})

    def test_keys_posi_delete_session(self):
    
        """test_keys_posi_delete_session"""
        
        #Delete key from session bus

        self.set_bus_type(BUS_TYPE_SESSION)
        self.set_imsi(imsi_any)
        self.reset()
        self.set_auto(key1, key1_val)
        self.get_auto(key1, key1_val)
        self.delete_auto(key1)
        self.get_auto(key1, "", PROVMAN_EXCEPT_NOT_FOUND)

    def test_keys_posi_delete_system(self):
    
        """test_keys_posi_delete_system"""
        
        #Delete key from system bus

        self.set_bus_type(BUS_TYPE_SYSTEM)
        self.set_imsi(imsi_any)
        self.reset()
        self.set_auto(key1, key1_val)
        self.get_auto(key1, key1_val)
        self.delete_auto(key1)
        self.get_auto(key1, "", PROVMAN_EXCEPT_NOT_FOUND)

    def test_keys_posi_get_all_path_ending_slash(self):
    
        """test_keys_posi_get_all_path_ending_slash"""
        
        #GetAll with path ending with/without '/'

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        self.set_mult_auto(keys)

        path_no_slash = keys_subdir.rstrip("/")
        
        if keys_subdir.endswith("/"):
            path_with_slash = keys_subdir
        else:
            path_with_slash = keys_subdir + "/"

        self.get_all_auto(path_with_slash, keys)
        self.get_all_auto(path_no_slash, keys)

    def test_keys_posi_delete_path_ending_slash(self):
    
        """test_keys_posi_delete_path_ending_slash"""
        
        #Delete with path ending with/without '/'

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)

        path_no_slash = keys_subdir.rstrip("/")
        
        if keys_subdir.endswith("/"):
            path_with_slash = keys_subdir
        else:
            path_with_slash = keys_subdir + "/"

        #path with ending '/'
        self.reset()
        self.set_mult_auto(keys)
        self.delete_auto(path_with_slash)
        self.get_all_auto(keys_subdir, {}, PROVMAN_EXCEPT_NOT_FOUND)

        #path without ending '/'
        self.reset()
        self.set_mult_auto(keys)
        self.delete_auto(path_no_slash)
        self.get_all_auto(keys_subdir, {}, PROVMAN_EXCEPT_NOT_FOUND)

    def test_keys_posi_get_dir_return_subdir(self):
    
        """test_keys_posi_get_dir_return_subdir"""
        
        #Get on a directory -> should return list of subdirectories

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        
        #create implicitely a list of directories
        self.set_auto(key1, key1_val)
        self.set_auto(key_del, key_del_val)
        self.set_auto(key_dir_undel, key_dir_undel_val)

        #extract the last directory level
        dir1 = os.path.split(subdir.rstrip("/"))[1]
        dir2 = os.path.split(dir_del.rstrip("/"))[1]
        dir3 = os.path.split(dir_undel.rstrip("/"))[1]

        dir_list = dir2 + "/" + dir3 + "/" + dir1

        self.get_auto(root, dir_list)

    def test_keys_posi_del_key_key_same_name_begin_not_affect(self):
    
        """test_keys_posi_del_key_key_same_name_begin_not_affect"""

        #Delete key, key with same beginning name is not affected

        #initialization
        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)

        #case where key points to a setting
        self.reset()
        #set the keys with same beginning names (1 short, 1 long name)
        self.set_auto(key_dir_same, key_dir_same_val)
        self.set_auto(key_dir_same_suffix, key_dir_same_suffix_val)
        #check that keys were successfully set
        self.get_auto(key_dir_same, key_dir_same_val)
        self.get_auto(key_dir_same_suffix, key_dir_same_suffix_val)
        #delete key (with short name)
        self.delete_auto(key_dir_same)
        #check key was successully deleted
        self.get_auto(key_dir_same, "", PROVMAN_EXCEPT_NOT_FOUND)
        #check that other key (with long name) was not deleted
        self.get_auto(key_dir_same_suffix, key_dir_same_suffix_val)

        #case where key points to a directory
        self.reset()
        #set keys with same beginning names (1 short, 2 long names)
        self.set_auto(key_dir_same, key_dir_same_val)
        self.set_auto(key_dir_same_suffix2, key_dir_same_suffix2_val)
        self.set_auto(key_same_suffix, key_same_suffix_val)
        #check that keys were successfully set
        self.get_auto(key_dir_same, key_dir_same_val)
        self.get_auto(key_dir_same_suffix2, key_dir_same_suffix2_val)
        self.get_auto(key_same_suffix, key_same_suffix_val)
        #delete key (with short name)
        self.delete_auto(dir_same)
        #check key was successully deleted
        self.get_auto(key_dir_same, "", PROVMAN_EXCEPT_NOT_FOUND)
        self.get_auto(dir_same, "", PROVMAN_EXCEPT_NOT_FOUND)
        #extract name of key
        key_name = os.path.split(key_dir_same_suffix2)[1]
        #check that other keys (with long names) were not deleted
        self.get_auto(dir_same_suffix, key_name)
        self.get_auto(key_same_suffix, key_same_suffix_val)

    def test_keys_posi_get_all_result_same_name_begin_not_returned(self):
    
        """test_keys_posi_get_all_result_same_name_begin_not_returned"""
        
        #GetAll key, result of key with same beginning name is not returned

        #initialization
        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)

        #case where key points to a setting
        
        self.reset()
        #set the keys with same beginning names (1 short, 1 long name)
        self.set_auto(key_dir_same, key_dir_same_val)
        self.set_auto(key_dir_same_suffix, key_dir_same_suffix_val)
        #check that keys were successfully set
        self.get_auto(key_dir_same, key_dir_same_val)
        self.get_auto(key_dir_same_suffix, key_dir_same_suffix_val)
        #GetAll key (with short name)
        self.get_all_auto(key_dir_same, {key_dir_same : key_dir_same_val})

        #case where key points to a directory
        
        self.reset()
        #set keys with same beginning names (1 short, 2 long names)
        self.set_auto(key_dir_same, key_dir_same_val)
        self.set_auto(key_dir_same_suffix2, key_dir_same_suffix2_val)
        self.set_auto(key_same_suffix, key_same_suffix_val)
        #check that keys were successfully set
        self.get_auto(key_dir_same, key_dir_same_val)
        self.get_auto(key_dir_same_suffix2, key_dir_same_suffix2_val)
        self.get_auto(key_same_suffix, key_same_suffix_val)
        #GetAll key (with short name)
        self.get_all_auto(dir_same, {key_dir_same : key_dir_same_val})

    def test_keys_neg_set_key_root_dir(self):
    
        """test_keys_neg_set_key_root_dir"""
        
        #Set key at root directory level

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        
        self.set_auto("/", "val", PROVMAN_EXCEPT_BAD_ARGS)

    def test_keys_neg_set_mult_some_keys_not_existing(self):
    
        """test_keys_neg_set_mult_some_keys_not_existing"""
        
        #SetMultiple with some keys not existing

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        
        key_not_existing = key3 + "not_existing"
        self.set_mult_auto({key1: key1_val, key2: key2_val, key3: key3_val,
                                key_not_existing: key3_val}, 
                          {key_not_existing: key3_val})
                           
    def test_keys_neg_get_non_existing_key(self):
    
        """test_keys_neg_get_non_existing_key"""
        
        #Get non-existing key

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        
        key_not_existing = key1 + "not_existing"
        self.get_auto(key_not_existing, "", PROVMAN_EXCEPT_NOT_FOUND)
                           
    def test_keys_neg_get_all_non_existing_key(self):
    
        """test_keys_neg_get_all_non_existing_key"""
        
        #GetAll non-existing key

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        
        key_not_existing = key1 + "not_existing"
        self.get_all_auto(key_not_existing, {}, PROVMAN_EXCEPT_NOT_FOUND)
                           
    def test_keys_neg_set_non_existing_key(self):
    
        """test_keys_neg_set_non_existing_key"""
        
        #Set non-existing key

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        
        key_not_existing = key1 + "not_existing"
        self.set_auto(key_not_existing, "", PROVMAN_EXCEPT_NOT_FOUND)
                           
    def test_keys_neg_set_mult_non_existing_key(self):
    
        """test_keys_neg_set_mult_non_existing_key"""
        
        #SetMultiple non-existing key

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        
        key_not_existing = key1 + "not_existing"
        self.set_mult_auto({key_not_existing: ""}, [key_not_existing])
        
        self.get_auto(key_not_existing, "", PROVMAN_EXCEPT_NOT_FOUND)

    def test_keys_neg_delete_non_existing_key(self):
    
        """test_keys_neg_delete_non_existing_key"""
        
        #Delete non-existing key

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        
        key_not_existing = key1 + "not_existing"
        self.delete_auto(key_not_existing, PROVMAN_EXCEPT_NOT_FOUND)
                           
    def test_keys_neg_delete_undeletable_key(self):
    
        """test_keys_neg_delete_undeletable_key"""
        
        #Delete undeletable key

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        
        self.set_auto(key_undel, key_undel_val)
        self.get_auto(key_undel, key_undel_val)
        
        self.delete_auto(key_undel, PROVMAN_EXCEPT_DENIED)

        self.get_auto(key_undel, key_undel_val)

    def test_keys_neg_delete_undeletable_directory(self):
    
        """test_keys_neg_delete_undeletable_directory"""
        
        #Delete undeletable directory

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        
        self.set_auto(key_dir_undel, key_dir_undel_val)
        self.get_auto(key_dir_undel, key_dir_undel_val)
        
        self.delete_auto(dir_undel, PROVMAN_EXCEPT_DENIED)

        self.get_auto(key_dir_undel, key_dir_undel_val)

    def test_keys_neg_call_methods_key_empty_string(self):
    
        """test_keys_neg_call_methods_key_empty_string"""
        
        #Call all methods with key parameter set to empty string

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        
        self.set_auto("", "val", PROVMAN_EXCEPT_BAD_ARGS)
        self.set_mult_auto("", [])
        self.get_auto("", "val", PROVMAN_EXCEPT_BAD_ARGS)
        self.get_all_auto("", {}, PROVMAN_EXCEPT_BAD_ARGS)
        self.delete_auto("", PROVMAN_EXCEPT_BAD_ARGS)

    def test_keys_neg_call_methods_key_invalid_string(self):
    
        """test_keys_neg_call_methods_key_invalid_string"""
        
        #Call all methods with key parameter set to invalid string

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        
        invalid_string = "test"
        self.set_auto(invalid_string, "val", PROVMAN_EXCEPT_BAD_ARGS)
        self.set_mult_auto({invalid_string: "val"}, [invalid_string])
        self.get_auto(invalid_string, "val", PROVMAN_EXCEPT_BAD_ARGS)
        self.get_all_auto(invalid_string, {}, PROVMAN_EXCEPT_BAD_ARGS)
        self.delete_auto(invalid_string, PROVMAN_EXCEPT_BAD_ARGS)

        invalid_string = "test/"
        self.set_auto(invalid_string, "val", PROVMAN_EXCEPT_BAD_ARGS)
        self.set_mult_auto({invalid_string: "val"}, [invalid_string])
        self.get_auto(invalid_string, "val", PROVMAN_EXCEPT_BAD_ARGS)
        self.get_all_auto(invalid_string, {}, PROVMAN_EXCEPT_BAD_ARGS)
        self.delete_auto(invalid_string, PROVMAN_EXCEPT_BAD_ARGS)

        invalid_string = "\\"
        self.set_auto(invalid_string, "val", PROVMAN_EXCEPT_BAD_ARGS)
        self.set_mult_auto({invalid_string: "val"}, [invalid_string])
        self.get_auto(invalid_string, "val", PROVMAN_EXCEPT_BAD_ARGS)
        self.get_all_auto(invalid_string, {}, PROVMAN_EXCEPT_BAD_ARGS)
        self.delete_auto(invalid_string, PROVMAN_EXCEPT_BAD_ARGS)

        invalid_string = ",;:!?./&(-_)=+~#{[|`^@]}$*%"
        self.set_auto(invalid_string, "val", PROVMAN_EXCEPT_BAD_ARGS)
        self.set_mult_auto({invalid_string: "val"}, [invalid_string])
        self.get_auto(invalid_string, "val", PROVMAN_EXCEPT_BAD_ARGS)
        self.get_all_auto(invalid_string, {}, PROVMAN_EXCEPT_BAD_ARGS)
        self.delete_auto(invalid_string, PROVMAN_EXCEPT_BAD_ARGS)

    def test_keys_neg_call_methods_key_2_slash(self):
    
        """test_keys_neg_call_methods_key_2_slash"""
        
        #Call all methods with key parameter set to string containing 2 slash
        #characters

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        
        invalid_string = "/" + key1
        self.set_auto(invalid_string, "val", PROVMAN_EXCEPT_BAD_ARGS)
        self.set_mult_auto({invalid_string: "val"}, [invalid_string])
        self.get_auto(invalid_string, "val", PROVMAN_EXCEPT_BAD_ARGS)
        self.get_all_auto(invalid_string, {}, PROVMAN_EXCEPT_BAD_ARGS)
        self.delete_auto(invalid_string, PROVMAN_EXCEPT_BAD_ARGS)

        invalid_string = subdir + "/"
        self.set_auto(invalid_string, "val", PROVMAN_EXCEPT_BAD_ARGS)
        self.set_mult_auto({invalid_string: "val"}, [invalid_string])
        self.get_auto(invalid_string, "val", PROVMAN_EXCEPT_BAD_ARGS)
        self.get_all_auto(invalid_string, {}, PROVMAN_EXCEPT_BAD_ARGS)
        self.delete_auto(invalid_string, PROVMAN_EXCEPT_BAD_ARGS)

    def test_keys_stress_set_get_many_implicit_dir_levels(self):
    
        """test_keys_stress_set_get_many_implicit_dir_levels"""
        
        #Set/Get with many implicit directory levels

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        
        self.get_auto(key_dir_many_levels, "", PROVMAN_EXCEPT_NOT_FOUND)
        self.set_auto(key_dir_many_levels, key_dir_many_levels_val)
        self.get_auto(key_dir_many_levels, key_dir_many_levels_val)

    def test_keys_stress_set_get_key_long_path(self):
    
        """test_keys_stress_set_get_key_long_path"""
        
        #Set/Get key with a long path

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        
        long_path = subdir + "loooong_path_" * 20
        
        self.set_auto(long_path, "val", PROVMAN_EXCEPT_NOT_FOUND)
        self.get_auto(long_path, "val", PROVMAN_EXCEPT_NOT_FOUND)

    def test_keys_stress_set_mult_get_all_many_keys(self):
    
        """test_keys_stress_set_mult_get_all_many_keys"""
        
        #SetMultiple/GetAll many keys

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        
        self.set_mult_auto(many_keys)
        self.get_all_auto(dir_many_keys, many_keys)

    def test_keys_stress_set_many_times(self):
    
        """test_keys_stress_set_many_times"""
        
        #Call Set many times

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        
        self.connect_dbus()
        self.start()
        for i in range(250):
            self.set(key1, key1_val)
        self.end()

        self.get_auto(key1, key1_val)

    def test_keys_stress_get_many_times(self):
    
        """test_keys_stress_get_many_times"""
        
        #Call Get many times

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        
        self.set_auto(key1, key1_val)
        
        self.connect_dbus()
        self.start()
        for i in range(250):
            self.get(key1, key1_val)
        self.end()

    def test_keys_stress_set_mult_many_times(self):
    
        """test_keys_stress_set_mult_many_times"""
        
        #Call SetMultiple many times

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        
        self.connect_dbus()
        self.start()
        for i in range(100):
            self.set_mult(many_keys)
        self.end()

        self.get_all_auto(dir_many_keys, many_keys)

    def test_keys_stress_get_all_many_times(self):
    
        """test_keys_stress_get_all_many_times"""
        
        #Call GetAll many times

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        
        self.set_mult_auto(many_keys)

        self.connect_dbus()
        self.start()
        for i in range(100):
            self.get_all(dir_many_keys, many_keys)
        self.end()

    def test_sess_posi_abort_session(self):
    
        """test_sess_posi_abort_session"""
        
        #Abort DM session conducted by session provman instance

        self.set_bus_type(BUS_TYPE_SESSION)
        self.set_imsi(imsi_any)
        self.reset()

        self.connect_dbus()
        self.start()
        self.set(key1, key1_val)
        self.get(key1, key1_val)
        self.abort()
        
        self.get_auto(key1, "", PROVMAN_EXCEPT_NOT_FOUND)

    def test_sess_posi_abort_system(self):

        """test_sess_posi_abort_system"""
        
        #Abort DM session conducted by system provman instance

        self.set_bus_type(BUS_TYPE_SYSTEM)
        self.set_imsi(imsi_any)
        self.reset()

        self.connect_dbus()
        self.start()
        self.set(key1, key1_val)
        self.get(key1, key1_val)
        self.abort()
        
        self.get_auto(key1, "", PROVMAN_EXCEPT_NOT_FOUND)

    def test_sess_posi_no_op(self):
    
        """test_sess_posi_no_op"""
        
        #DM session with no operations

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()

        self.connect_dbus()
        self.start()
        self.end()

    def test_sess_posi_set_get_same_session(self):

        """test_sess_posi_set_get_same_session"""

        #Set then Get in same DM session

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()

        self.get_auto(key1, "", PROVMAN_EXCEPT_NOT_FOUND)
        self.connect_dbus()
        self.start()
        self.set(key1, key1_val)
        self.get(key1, key1_val)
        self.end()

    def test_sess_posi_set_get_separate_session(self):

        """test_sess_posi_set_get_separate_session"""

        #Set then Get in separate DM sessions

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()
        
        self.get_auto(key1, "", PROVMAN_EXCEPT_NOT_FOUND)
        self.set_auto(key1, key1_val)
        self.get_auto(key1, key1_val)

    def test_sess_posi_start_session_no_session_running(self):

        """test_sess_posi_start_session_no_session_running"""

        #Start a session and provman was not previously running -> test timeout

        #test case initialization
        self.set_imsi(imsi_any)
        self.set_bus_type(BUS_TYPE_SESSION)
        self.reset()
        self.set_bus_type(BUS_TYPE_SYSTEM)
        self.reset()

        kill_status_succeed = 0
        
        #kill any running provman process on session bus
        commands.getstatusoutput("killall -e %s" % PROVMAN_PROCESS_SESSION)
        
        #check that provman proces is not running anymore
        kill_status_sess = commands.getstatusoutput(
                "killall -e %s" % PROVMAN_PROCESS_SESSION[0])
        self.assertNotEqual(kill_status_sess, kill_status_succeed)
        
        #start session on session bus
        self.set_bus_type(BUS_TYPE_SESSION)
        self.set_auto(key1, key1_val)

        #kill any running provman process on system bus
        commands.getstatusoutput("killall -e %s" % PROVMAN_PROCESS_SYSTEM)

        #check that provman processes are not running anymore
        kill_status_sys = commands.getstatusoutput(
                "killall -e %s" % PROVMAN_PROCESS_SYSTEM[0])
        self.assertNotEqual(kill_status_sys, kill_status_succeed)

        #start session on system bus
        self.set_bus_type(BUS_TYPE_SYSTEM)
        self.set_auto(key1, key1_val)

    def test_sess_posi_start_session_prev_session_running(self):

        """test_sess_posi_start_session_prev_session_running"""

        #Start a session and provman was already running -> testing timeout

        #test case initialization
        self.set_imsi(imsi_any)
        self.set_bus_type(BUS_TYPE_SESSION)
        self.reset()
        self.set_bus_type(BUS_TYPE_SYSTEM)
        self.reset()

        #start session on session bus
        self.set_bus_type(BUS_TYPE_SESSION)
        self.set_auto(key1, key1_val)

        #making sure provman process is running
        proc_sess = len(commands.getoutput(
                "ps -A|grep -i '%s'|grep -v 'grep'" % PROVMAN_PROCESS_SESSION))
        self.assertNotEqual(proc_sess, 0)
        
        self.set_auto(key1, key1_val)

        #start session on system bus
        self.set_bus_type(BUS_TYPE_SYSTEM)
        self.set_auto(key1, key1_val)

        #making sure provman process is running
        proc_sys = len(commands.getoutput(
                "ps -A|grep -i '%s'|grep -v 'grep'" % PROVMAN_PROCESS_SYSTEM))
        self.assertNotEqual(proc_sys, 0)
        
        self.set_auto(key1, key1_val)

    def test_sess_neg_call_methods_no_session_initiated(self):

        """test_sess_neg_call_methods_no_session_initiated"""

        #Call all methods without having initiated a DM session

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi_any)
        self.reset()

        self.get(key1, "", PROVMAN_EXCEPT_UNEXPECTED)
        self.set(key1, "", PROVMAN_EXCEPT_UNEXPECTED)
        self.get_all(keys_subdir, {}, PROVMAN_EXCEPT_UNEXPECTED)
        self.set_mult(keys, [], PROVMAN_EXCEPT_UNEXPECTED)
        self.delete(key1, PROVMAN_EXCEPT_UNEXPECTED)
        self.end(PROVMAN_EXCEPT_UNEXPECTED)
        self.abort(PROVMAN_EXCEPT_UNEXPECTED)

    def test_sim_neg_handle_key_assoc_to_other_sim(self):
    
        """test_sim_neg_handle_key_assoc_to_other_sim"""
        
        #Handle a key that is associated to another SIM card

        imsi1 = "1"
        imsi2 = "2"

        self.set_bus_type(bus_type_any)
        self.set_imsi(imsi1)
        self.reset()
        self.set_imsi(imsi2)
        self.reset()

        #Set key for SIM#1
        self.set_imsi(imsi1)
        self.set_auto(key1, key1_val)
        self.get_auto(key1, key1_val)

        #Get/GetAll/Delete same key but for SIM#2
        self.set_imsi(imsi2)
        self.get_auto(key1, "", PROVMAN_EXCEPT_NOT_FOUND)
        self.get_all_auto(key1, {}, PROVMAN_EXCEPT_NOT_FOUND)
        self.delete_auto(key1, PROVMAN_EXCEPT_NOT_FOUND)



#-------------------------------------------------------------------

#global variables

#test cases that must be run with a non-default SIM card will use the
#following variable
imsi_specific_sim  = "1234567890"

#test cases that must be run with the default SIM card will use the
#following variable
imsi_default_sim   = IMSI_DEFAULT_SIM

#test cases that may be run with any SIM card (either default or non/default)
#will use following variable
imsi_any           = IMSI_DEFAULT_SIM

#test cases that may be run on any bus (either session or system) will use the
#following variable
bus_type_any       = BUS_TYPE_SESSION


#data tree used in test cases
#root/                           <-- root of plug-in
#    subdir/                     <-- subdir
#        key1                    <-- key
#        key2                    <-- key
#        key3                    <-- key
#    dir_del/                    <-- subdir, deletable
#        key_dir_del             <-- key
#    dir_undel/                  <-- subdir, undeletable
#        key_dir_undel           <-- key
#    dir_key_del_undel/          <-- subdir
#        key_del                 <-- key, deletable
#        key_undel               <-- key, undeletable
#    dir_key_type/               <-- subdir
#        key_string              <-- key, string type
#        key_enum                <-- key, enum type
#        key_int                 <-- key, int type
#    dir_many_levels             <-- subdir (20 levels)
#        ...
#            ...
#                key_dir_many_levels    <--key
#    dir_many_keys               <-- subdir
#        ...
#        keyxxx                  <-- key (100 total)
#    dir_same_name_begin/        <-- subdir
#        dir_same/               <-- subdir
#            key_dir_same        <-- key
#            key_dir_same_suffix <-- key, name starts as 'key_dir_same'
#        dir_same_suffix/        <-- subdir, name starts as 'dir_same'
#            key_dir_same_suffix2 <-- key
#        key_same_suffix         <-- key, name starts as 'dir_same'

root                    = "/applications/test_plugin/test/"
subdir                  = root + "subdir/"
key1                    = subdir + "key1"
key1_val                = "value of key1"
key2                    = subdir + "key2"
key2_val                = "value of key2"
key3                    = subdir + "key3"
key3_val                = "value of key3"
dir_del                 = root + "subdir_deletable/"
key_dir_del             = dir_del + "string_deletable"
key_dir_del_val         = "key_dir_del"
dir_undel               = root + "subdir_undeletable/"
key_dir_undel           = dir_undel + "string_deletable"
key_dir_undel_val       = "value of key_dir_undel"
dir_key_del_undel       = root + "subdir_deletable/"
key_del                 = dir_key_del_undel + "string_deletable"
key_del_val             = "value of key_del"
key_undel               = dir_key_del_undel + "string_undeletable"
key_undel_val           = "value of key_undel"
dir_key_type            = root + "subdir_undeletable/"
key_string              = dir_key_type + "string_deletable"
key_string_val          = "value of key_string"
key_enum                = dir_key_type + "enum_deletable"
key_enum_val            = "value of key_enum"
key_int                 = dir_key_type + "int_deletable"
key_int_val             = "value of key_int"
dir_many_levels         = root + "subdir_many_levels/" + "subdir/" * 19
key_dir_many_levels     = dir_many_levels + "key"
key_dir_many_levels_val = "value of key_dir_many_levels_val"
dir_many_keys           = root + "subdir_many_keys/"
dir_same_name_begin     = root + "subdir_same_name_begin/"
dir_same                = dir_same_name_begin + "subdir/"
key_dir_same            = dir_same + "key"
key_dir_same_val        = "value of key_dir_same"
key_dir_same_suffix     = key_dir_same + "_suffix"
key_dir_same_suffix_val = "value of key_dir_same_suffix"
dir_same_suffix         = dir_same[:-1] + "_suffix/"
key_dir_same_suffix2    = dir_same_suffix + "key"
key_dir_same_suffix2_val= "value of key_dir_same_suffix2"
key_same_suffix         = dir_same[:-1] + "_suffix_but_i_am_a_key"
key_same_suffix_val     = "value of key_same_suffix"

keys                = {key1 : key1_val, key2: key2_val, key3: key3_val}
keys_subdir         = subdir

many_keys           = many_keys(dir_many_keys)


#-------------------------------------------------------------------
#run test cases

#delete log file
log_file = testprovman.getLogFile()
try:
    os.remove(log_file)
except:
    pass

#create a test suite containing all test cases represented in class
suite = unittest.TestLoader().loadTestsFromTestCase(TestProvmanTestCases)

#execute test cases from test suite
unittest.TextTestRunner(verbosity=1).run(suite)



