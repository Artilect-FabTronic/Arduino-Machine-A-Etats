# -*- coding: utf-8 -*-

import serial.tools.list_ports


def get_port_list():
    ports = serial.tools.list_ports.comports(include_links=False)
    list_ports = []
    for port in ports:
        list_ports.append(port.device)
    return list_ports
