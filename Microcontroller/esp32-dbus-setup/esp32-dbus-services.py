# esp32-dbus-services.py

# library for dbus services
from gi.repository import GLib
import platform
import argparse
import logging
import sys
import os

sys.path.insert(1, os.path.join(os.path.dirname(__file__), '../ext/velib_python'))
from vedbus import VeDbusService

class esp32_dbus_service(object):
    def __init__(self, service_name, object_paths, process_name, process_version, connection, product_id, product_name, firmware_version, hardware_version, device_instance, connected):
        self.dbus_service = VeDbusService(service_name)
        self.object_paths = object_paths

        # object paths that all services need to implement
        self.dbus_service.add_path('/Mgmt/ProcessName', process_name)
        self.dbus_service.add_path('/Mgmt/ProcessVersion', process_version)
        self.dbus_service.add_path('/Mgmt/Connection', connection)

        # object paths that all services representing products need to implement
        self.dbus_service.add_path('/ProductId', product_id)
        self.dbus_service.add_path('/ProductName', product_name)
        self.dbus_service.add_path('/FirmwareVersion', firmware_version)
        self.dbus_service.add_path('/HardwareVersion', hardware_version)
        self.dbus_service.add_path('/DeviceInstance', device_instance)
        self.dbus_service.add_path('/Connected', connected)

        # configure each object path to have a writable initial value be set
        for path, settings in self.object_paths.items():
            self.dbus_service.add_path(path, settings['initial'], writeable = True)
            
def main():

    # initialize default main loop event handler
    from dbus.mainloop.glib import DBusGMainLoop
    DBusGMainLoop(set_as_default = True)

    # set up and connect a real victron smart shunt into the cerbo gx
    smart_shunt_1 = esp32_dbus_service(
        service_name = 'com.victronenergy.battery.ttyO2',
        object_paths = {
            '/Dc/0/Voltage': {'initial': 0},
            '/Dc/0/Current': {'initial': 0},
            '/Dc/0/Power': {'initial': 0}
        },
        process_name = 'VE.Direct_DBus',
        process_version = 'Python ' + platform.python_version(),
        connection = 'VE.Direct port 2',
        product_id = 41865,
        product_name = 'SmartShunt 500A/50mV',
        firmware_version = 'Python ' + platform.python_version(),
        hardware_version = 'Python ' + platform.python_version(),
        device_instance = 258,
        connected = 1
    )
    
    # keep setting up more devices as needed, but this is functional

    # continuously run the script
    mainloop = GLib.MainLoop()
    mainloop.run()

if __name__ == "__main__":
    main()