#!/usr/bin/env python

from gi.repository import GLib
import dbus
import dbus.service
import platform
import argparse
import logging
import sys
import os

#sys.path.insert(1, os.path.join(os.path.dirname(__file__), '../data/velib_python'))
sys.path.insert(1, os.path.join(os.path.dirname(__file__), '/opt/victronenergy/dbus-modem'))
from vedbus import VeDbusService

class SystemBus(dbus.bus.BusConnection):
    def __new__(cls):
        return dbus.bus.BusConnection.__new__(cls, dbus.bus.BusConnection.TYPE_SYSTEM)

class SessionBus(dbus.bus.BusConnection):
    def __new__(cls):
        return dbus.bus.BusConnection.__new__(cls, dbus.bus.BusConnection.TYPE_SESSION)

def dbus_connection():
    return SessionBus() if 'DBUS_SESSION_BUS_ADDRESS' in os.environ else SystemBus()

def create_service(custom_name, service_base, device_type, physical_connection, logical_connection, id, device_instance):
    self =  VeDbusService("{}.{}.{}_id{:03d}".format(service_base, device_type, physical_connection,  id), dbus_connection())

    # COMMON OBJECT PATHS

    # object paths required by all services
    self.add_path('/Mgmt/ProcessName', __file__)
    self.add_path('/Mgmt/ProcessVersion', 'Python ' + platform.python_version())
    self.add_path('/Mgmt/Connection', logical_connection)
    # object paths required by all products
    self.add_path('/FirmwareVersion', 0)
    self.add_path('/HardwareVersion', 0)
    self.add_path('/Connected', 0)
    self.add_path('/DeviceInstance', device_instance)
    # object path for custom name for user reference
    self.add_path('/CustomName', custom_name)

    # DEVICE SPECIFIC OBJECT PATHS

    if device_type == 'battery':
        #object paths required by all products
        self.add_path('/ProductId', 49200)
        self.add_path('/ProductName', 'SmartShunt IP65 500A/50mV')
        # object paths for specific physical parameters
        self.add_path('/Dc/0/Power', 0, writeable = True)
        self.add_path('/Dc/0/Voltage', 0, writeable = True)
        self.add_path('/Dc/0/Current', 0, writeable = True)
        self.add_path('/Dc/0/Temperature', 22, writeable = True)

    return self

def main():
    from dbus.mainloop.glib import DBusGMainLoop
    DBusGMainLoop(set_as_default = True)
    dbus_services = {}
    service_base = 'com.victronenergy'

    # create services
    dbus_services['SHUNT1'] = create_service('SHUNT1', service_base, 'battery', 'ttyUSB1', 'VE.Direct via USB (ttyUSB1)',  0, 289) # 238
    dbus_services['SHUNT2'] = create_service('SHUNT2', service_base, 'battery', 'ttyUSB2', 'VE.Direct via USB (ttyUSB2)',  0, 290) # 237
    dbus_services['SHUNT3'] = create_service('SHUNT3', service_base, 'battery', 'ttyUSB3', 'VE.Direct via USB (ttyUSB3)',  0, 291) # 236
    dbus_services['SHUNT4'] = create_service('SHUNT4', service_base, 'battery', 'ttyUSB4', 'VE.Direct via USB (ttyUSB4)',  0, 292) # 235
    dbus_services['SHUNT5'] = create_service('SHUNT5', service_base, 'battery', 'ttyUSB5', 'VE.Direct via USB (ttyUSB5)',  0, 293) # 233
    dbus_services['SHUNT6'] = create_service('SHUNT6', service_base, 'battery', 'ttyUSB6', 'VE.Direct via USB (ttyUSB6)',  0, 294) # 232
    dbus_services['SHUNT7'] = create_service('SHUNT7', service_base, 'battery', 'ttyUSB7', 'VE.Direct via USB (ttyUSB7)',  0, 295) # 231

    # connect services (probably optional but doing it just to ensure error-free operation)
    dbus_services['SHUNT1']['/Connected'] = 1
    dbus_services['SHUNT2']['/Connected'] = 1
    dbus_services['SHUNT3']['/Connected'] = 1
    dbus_services['SHUNT4']['/Connected'] = 1
    dbus_services['SHUNT5']['/Connected'] = 1
    dbus_services['SHUNT6']['/Connected'] = 1
    dbus_services['SHUNT7']['/Connected'] = 1

    print('connected to dbus, and switching over to GLib.MainLoop()')
    mainloop = GLib.MainLoop()
    mainloop.run()

if __name__ == "__main__":
    main()
