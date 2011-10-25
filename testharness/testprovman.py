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

import dbus, unittest, time, inspect, sys

DBUS_PROVMAN_BUS_NAME  = "com.intel.provman.server"
DBUS_PROVMAN_OBJ_PATH  = "/com/intel/provman"
DBUS_PROVMAN_INTERFACE = "com.intel.provman.Settings"

TEST_PROVMAN_LOG_FILE = "/tmp/testprovman.log"

IMSI_DEFAULT_SIM = ""


#check Python's version >= 2.7
if sys.version_info < (2, 7):
    print "ERROR. This script requires newer version of Python."
    raise SystemExit(1)


class TestProvman(unittest.TestCase):

    """Methods in this class may be called to create a test case scenario.
    
    Requires Python version >= 2.7.
    
    Typical use case:
        
        set_dbus_type = "session"       #D-Bus bus type to connect to
        set_imsi = "12345"              #IMSI
        reset()                         #reset provman database
        connect_dbus()                  #connect to D-Bus
        start()                         #start DM session
        get(key=           "/path/key", #get key, check returned value
            expect_val=    "value",
            expect_except= False)
        end()                           #end DM session
        
    TODO:
        -   Modify code to remove sleep instruction that prevents D-Bus
            from timing out.
        -   Dictionaries returned by D-Bus are written in log file as strings.
            We may format these strings to make them look nicer.
    """


    def setUp(self):
    
        """This method is called before each test case execution"""
        
        self.log()
        self.log("-------------------------------------------------------")
        self.log(self.shortDescription())
        self.log("setUp")
        
        #do not automatically call 'end' method in 'tearDown' method until
        #'start' method was called
        self.__force_call_end = False
        
        self.imsi = IMSI_DEFAULT_SIM

    def set_bus_type(self, bus_type):
        """set D-Bus type

        parameters:
            bus_type (string)
                either "session" or "system"
        """

        if bus_type == "session" or bus_type == "system":
            self.bus_type = bus_type
            self.log("Set bus type = %s" % bus_type)
        else:
            self.assertTrue(False, "Wrong parameter: %s" % bus_type)

    def set_imsi(self, imsi):
        """set IMSI

        parameters:
            imsi (string)
                IMSI
        """
        self.log("Set IMSI = '%s'" % imsi)
        self.imsi = imsi

    def connect_dbus(self):
    
        """Connect to Provman's Interface over D-Bus."""

        #TODO: remove the sleep that prevents D-Bus from timing out
        time.sleep(0.15)
        
        if (self.bus_type == "session"):
            bus = dbus.SessionBus()
        elif (self.bus_type == "system"):
            bus = dbus.SystemBus()
        else:
            self.assertTrue(False, "Wrong parameter: %s" % self.bus_type)

        bus_obj = bus.get_object(DBUS_PROVMAN_BUS_NAME, DBUS_PROVMAN_OBJ_PATH)

        self.__dbus = dbus.Interface(bus_obj, DBUS_PROVMAN_INTERFACE)
        
    def set(self, key, val="", expect_except=""):
    
        """Sets value to a key via D-Bus Set method. Checks also raised
        exception, if any.
        
        parameters:
            key (string)
                full path and name of key to set.
            val (string)
                value to set to key.
            expect_except (string)
                Type of exception expected to be raised.
        """
        
        self.log("Set: %s <- %s" % (key, val))

        if expect_except == "":
            self.__dbus.Set(key, val)

        else:
            with self.assertRaises(dbus.exceptions.DBusException) as cm:
                self.__dbus.Set(key, val)
            self.assertEquals(cm.exception.get_dbus_name(), expect_except)

    def set_mult(self, keys, expect_keys_fail=[], expect_except=""):
    
        """Sets key/value pairs via D-Bus SetMultiple method. Checks keys that
        were not set. Checks also raised exception, if any.
        
        parameters:
            keys (dictionary)
                key/value pairs.
            expect_keys_fail (array)
                array of keys that have not been set
            expect_except (string)
                Type of exception expected to be raised.
        """
        
        self.log("SetMultiple: %s" % keys)
        
        if expect_except == "":
            ret = self.__dbus.SetMultiple(keys)
            
            #check if list of keys that were not set is expected
            ret_sort = sorted(ret)
            expect_keys_fail_sort = sorted(expect_keys_fail)
            self.log("Returned list of keys not set: %s" % ret_sort)
            compare = cmp(ret_sort, expect_keys_fail_sort)
            self.assertEqual(compare, 0)
            
        else:
            with self.assertRaises(dbus.exceptions.DBusException) as cm:
                self.__dbus.SetMultiple(keys)
            self.assertEquals(cm.exception.get_dbus_name(), expect_except)

    def get_auto(self, key, expect_val="", expect_except=""):
    
        """starts a DM session automatically and calls the Get method.

        parameters:
            key (string)
                full path and name of key to get its value.
            expect_val (string)
                expected key's value to be returned by Get method.
            expect_except (string)
                Type of exception expected to be raised.
        """
        
        self.connect_dbus()
        self.start()
        self.get(key, expect_val, expect_except)
        self.end()

    def get_all_auto(self, keys, expect_keys={}, expect_except=""):
    
        """starts a DM session automatically and calls the GetAll method.

        parameters:
            keys (string)
                full path and name of key/directory to get.
            expect_keys (dictionary)
                dictionary compared with dictionary returned by GetAll method.
            expect_except (string)
                Type of exception expected to be raised.
        """
        
        self.connect_dbus()
        self.start()
        self.get_all(keys, expect_keys, expect_except)
        self.end()

    def set_auto(self, key, val="", expect_except=""):
    
        """starts a DM session automatically and calls the Set method.

        parameters:
            key (string)
                full path and name of key to set.
            val (string)
                value to set to key.
            expect_except (string)
                Type of exception expected to be raised.
        """
        
        self.connect_dbus()
        self.start()
        self.set(key, val, expect_except)
        self.end()

    def set_mult_auto(self, keys, expect_keys_fail=[], expect_except=""):
    
        """starts a DM session automatically and calls the SetMultiple method.

        parameters:
            keys (dictionary)
                key/value pairs.
            expect_keys_fail (array)
                array of keys that have not been set
            expect_except (string)
                Type of exception expected to be raised.
        """
        
        self.connect_dbus()
        self.start()
        self.set_mult(keys, expect_keys_fail, expect_except)
        self.end()

    def delete_auto(self, key, expect_except=""):
    
        """starts a DM session automatically and calls the Delete method.

        parameters:
            key (string)
                full path and name of key/directory to delete.
            expect_except (string)
                Type of exception expected to be raised.
        """
        
        self.connect_dbus()
        self.start()
        self.delete(key, expect_except)
        self.end()

    def get(self, key, expect_val="", expect_except=""):
    
        """Tests a key's value returned by D-Bus Get method. 
        Checks also raised exception, if any.
        
        parameters:
            key (string)
                full path and name of key to get its value.
            expect_val (string)
                expected key's value to be returned by Get method.
            expect_except (string)
                Type of exception expected to be raised.
        """
        
        self.log("Get: %s" % key)

        if expect_except == "":
            ret = self.__dbus.Get(key)
            self.log("returned value: %s" % ret)
            self.log("expected value: %s" % expect_val)
            self.assertEqual(ret, expect_val)
            
        else:
            with self.assertRaises(dbus.exceptions.DBusException) as cm:
                self.__dbus.Get(key)
            self.assertEquals(cm.exception.get_dbus_name(), expect_except)
            
    def get_all(self, path, expect_keys={}, expect_except=""):
    
        """Checks key/value pairs returned by D-Bus GetAll method. Checks
        also raised exception, if any.
        
        parameters:
            path (string)
                full path and name of key/directory to get.
            expect_keys (dictionary)
                dictionary compared with dictionary returned by GetAll method.
            expect_except (string)
                Type of exception expected to be raised.
        """
        
        self.log("GetAll: %s" % path)

        if expect_except == "":
            ret = self.__dbus.GetAll(path)
            self.log("returned keys: %s" % ret.items())
            self.log("expected keys: %s" % expect_keys.items())
            self.assertDictEqual(ret, expect_keys)
            
        else:
            with self.assertRaises(dbus.exceptions.DBusException) as cm:
                self.__dbus.GetAll(path)
            self.assertEquals(cm.exception.get_dbus_name(), expect_except)
    
    def delete(self, key, expect_except=""):
    
        """Deletes a key/directory via D-Bus Delete method. Checks also raised
        exception, if any.
        
        parameters:
            key (string)
                full path and name of key/directory to delete.
            expect_except (string)
                Type of exception expected to be raised.
        """
        
        self.log("Delete: %s" % key)

        if expect_except == "":
            self.__dbus.Delete(key)
            
        else:
            with self.assertRaises(dbus.exceptions.DBusException) as cm:
                self.__dbus.Delete(key)
            self.assertEquals(cm.exception.get_dbus_name(), expect_except)

    def start(self, expect_except=""):
    
        """Start a DM session. Check raised exception,
        if any.
        
        parameters:
            expect_except (string)
                Type of exception expected to be raised.
        """

        self.log("Start(imsi='%s')" % self.imsi)

        if expect_except == "":
            self.__dbus.Start(self.imsi)
            
            #helps 'tearDown' method decide if 'end' method should be
            #automatically called at end of test case scenario
            self.__force_call_end = True
            
        else:
            with self.assertRaises(dbus.exceptions.DBusException) as cm:
                self.__dbus.Start(self.imsi)
            self.assertEquals(cm.exception.get_dbus_name(), expect_except)

    def abort(self, expect_except=""):
    
        """Aborts a DM session and checks raised exception.
        
        parameters:
            expect_except (string)
                Type of exception expected to be raised.
         """

        self.log("Abort")

        if expect_except == "":
            self.__dbus.Abort()
            
            #prevents from calling 'end' method in tearDown method
            #it is not necessary to end an aborted session
            self.__force_call_end = False
            
        else:
            with self.assertRaises(dbus.exceptions.DBusException) as cm:
                self.__dbus.Abort()
            self.assertEquals(cm.exception.get_dbus_name(), expect_except)

    def end(self, expect_except=""):
    
        """Ends a DM session and checks raised exception.
        
        parameters:
            expect_except (string)
                Type of exception expected to be raised.
        """

        self.log("End")

        if expect_except == "":
            self.__dbus.End()
            
            #prevents from automatically calling 'end' method again in
            #tearDown method
            self.__force_call_end = False
            
        else:
            with self.assertRaises(dbus.exceptions.DBusException) as cm:
                self.__dbus.End()
            self.assertEquals(cm.exception.get_dbus_name(), expect_except)

    def reset(self):

        """Resets all data in provman data tree through a DM session"""

        self.log("Reset")
        
        self.connect_dbus()
        self.start()
        self.delete("/")
        self.end()

    def log(self, string=""):
    
        """write string to log file. New line is automatically added.
        
        parameters:
            string
                string to write to log file
        """
        
        string_string = str(string)
        f = open(TEST_PROVMAN_LOG_FILE, 'a')
        f.write(string_string)
        f.write("\n")
        f.close()

    def tearDown(self):
    
        """This method is called at end of each test case scenario"""
        
        self.log("tearDown")
        
        if self.__force_call_end == True:
            self.log("Calling 'End' method automatically")
        
            #'end' method may not have been called in test case scenario or
            #an error may have prevented it from being called.
            self.end()


def getLogFile():

    """Returns path and name of log file."""

    return TEST_PROVMAN_LOG_FILE


