"""
Created on 03/03/2019

@author: https://github.com/fgirault

"""

import os
import pickle
import tkinter as tk
import tkinter.ttk as ttk

import serial

from .model import get_port_list

DEFAULT_TITLE = 'Comm with a serial port'

PREF_FILENAME = "nanoterm.prefs"


class NanoTermUI:

    def __init__(self, rx_refresh=500):
        self.root = None
        self.current_port_var = None
        self.serial = None
        self.rx_refresh = rx_refresh

    def build(self):
        self.root = tk.Tk()
        self.root.title(DEFAULT_TITLE)
        self.root.minsize(320, 200)

        self.root.bind("<Destroy>", self.destroy)

        self.current_port_var = tk.StringVar()
        self.current_label_var = tk.StringVar()
        self.current_label_var.set("Available ports")
        self.connect_label_var = tk.StringVar()
        self.connect_label_var.set("Connect")
        self.command_var = tk.StringVar()

        tk.Label(self.root, textvariable=self.current_label_var).grid(column=0, row=0, sticky=tk.E)
        self.combobox = ttk.Combobox(self.root, state="readonly", justify="left", textvariable=self.current_port_var)
        self.combobox.bind("<<ComboboxSelected>>", self.on_port_select)
        self.combobox.grid(column=1, row=0, sticky=tk.E+tk.W)
        tk.Button(self.root, textvariable=self.connect_label_var, command=self.on_connect_click).grid(column=2, row=0)

        tk.Label(self.root, text="Send").grid(column=0, row=1, sticky=tk.W)
        self.command_entry = entry = tk.Entry(self.root, textvariable=self.command_var, state="disabled")
        entry.grid(column=0, row=2, columnspan=2, sticky=tk.E+tk.W)
        entry.bind('<Return>', self.on_command_send)
        self.send_button = tk.Button(self.root, text="Send", command=self.on_command_send, state="disabled")
        self.send_button.grid(column=2, row=2, sticky=tk.E+tk.W)

        tk.Label(self.root, text="Received data").grid(column=0, row=3, sticky=tk.W)
        self.tktext = tktext = tk.Text(self.root)
        tktext.grid(column=0, row=4, columnspan=3, sticky=tk.N+tk.S+tk.E+tk.W)
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(4, weight=1)

    def destroy(self, event):
        if self.serial and self.serial.is_open:
            self.serial.close()

    def refresh_ports(self):
        values = get_port_list()
        self.combobox.configure(values=values)
        if os.path.exists(PREF_FILENAME):
            with open(PREF_FILENAME, "rb") as fd:
                pref_dict = pickle.load(fd)
                last_port = pref_dict["last_port"]
                if last_port in values:
                    index = values.index(last_port)
                    self.combobox.current(index)

                # self.current_port_var.set()

    def on_port_select(self, event):
        # self.save_settings()
        # print(self.current_port_var.get())
        pass

    @property
    def is_connected(self):
        return self.serial and self.serial.is_open

    def on_connect_click(self):
        port_name = self.current_port_var.get()
        if self.is_connected:
            self.serial.close()
            self.current_label_var.set("Available ports: ")
            self.connect_label_var.set("Connect")
            self.combobox.configure(state="readonly")
            self.send_button.configure(state="disabled")
            self.command_entry.configure(state="disabled")
        else:
            try:
                self.serial = serial.Serial(port_name, 115200)
            except serial.serialutil.SerialException as error:
                self.tktext.insert(tk.END, port_name + ":" + str(error) + os.linesep)
            else:
                self.rxmonitor()
                self.current_label_var.set("Connected to: ")
                self.connect_label_var.set("Disconnect")
                self.combobox.configure(state="disabled")
                self.send_button.configure(state="normal")
                self.command_entry.configure(state="normal")
                self.save_settings()

    def rxmonitor(self):
        if self.is_connected and self.serial.in_waiting:
            data = self.serial.read(self.serial.in_waiting)
            self.tktext.insert(tk.END, b"RX: " + data + bytes(os.linesep, "utf-8"))
            self.tktext.see(tk.END)
        self.root.after(self.rx_refresh, self.rxmonitor)

    def on_command_send(self, *args):
        if self.is_connected:
            self.tktext.insert(tk.END, "TX: " + self.command_var.get() + os.linesep)
            self.tktext.see(tk.END)
            self.serial.write(bytes(self.command_var.get(), "utf-8"))
            self.command_var.set("")

    def save_settings(self):
        pref_dict = {"last_port":  self.current_port_var.get()}
        with open(PREF_FILENAME, "wb") as fd:
            pickle.dump(pref_dict, fd)

    def launch(self):
        self.build()
        self.refresh_ports()
        self.root.mainloop()
