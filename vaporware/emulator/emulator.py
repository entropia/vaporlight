#!/usr/bin/env python3

"""
Quick/dirty Vaporlight emulator.

:Author: Felix Kaiser <felix.kaiser@fxkr.net>
:Version: 0.1.0
:License: revised BSD

Provides a network server implementing the bus protocol
and displays LED states in a GTK window. Useful when you
are working on the/a Vaporlight daemon.

Here's how you can try it out: start it, then
send it a commands via netcat::

    python emulator.py &
    netcat -c localhost 23429 < sample/rgb.bin

This should make the first three virtual LEDs
become red, green and blue.

Notes:
- only one client can be connected at a time.
- this emulator uses the bus protocol (which
  is the protocol used to speak to the actual
  hardware), not the "low level" protocol
  (which is used to speak to the router).
  Support for the latter could/should be added
  in the future.
"""

import argparse
import signal
import socket
import threading

import cairo
from gi.repository import Gtk, GObject


class Model(object):

    def __init__(self, modules, leds_per_module):
        self.observers = []
        self.modules = modules
        self.leds_per_module = leds_per_module
        self.leds = [[[0, 0, 0]
            for x in range(self.leds_per_module)]
            for y in range(self.modules)]

    def set_led(self, module, position, value):
        self.leds[module][position // 3][position % 3] = value

    def add_observer(self, func):
        self.observers.append(func)

    def strobe(self):
        for observer in self.observers:
            observer()


class GtkView(object):

    def __init__(self, model):
        self.model = model
        model.add_observer(self.redraw)
        self.init()

    def run(self):
        signal.signal(signal.SIGINT, signal.SIG_DFL) # fix ctrl-c
        GObject.threads_init()
        Gtk.main()

    def init(self):
        self.window = Gtk.Window(
            title="Vaporlight emulator",
            default_width=800,
            default_height=600,
            can_focus=False,
            window_position="center-always")
        self.window.connect("destroy", self.on_destroy)

        drawing = Gtk.DrawingArea(visible=True, can_focus=False)
        drawing.connect("draw", self.on_draw)
        drawing.connect("configure-event", self.on_configure)
        self.window.add(drawing)

        self.double_buffer = None
        self.window.show()

    def redraw(self):
        db = self.double_buffer
        cc = cairo.Context(db)
        cc.scale(db.get_width(), db.get_height())
        cc.set_source_rgb(1, 1, 1)

        rows = self.model.modules
        cols = self.model.leds_per_module
        cell_size_w = 1.0 / cols
        cell_size_h = 1.0 / rows
        line_width = 1.0
        line_width, _ = cc.device_to_user(line_width, 0.0)

        for y in range(self.model.modules):
            for x in range(self.model.leds_per_module):
                cc.rectangle(
                    x * cell_size_w, y * cell_size_h,
                    cell_size_w, cell_size_h)
                cc.set_line_width(line_width)
                cc.set_source_rgb(*self.model.leds[y][x])
                cc.fill()

        db.flush()

    def on_destroy(self, widget):
        Gtk.main_quit()

    def on_draw(self, widget, cr):
        cr.set_source_surface(self.double_buffer, 0.0, 0.0)
        cr.paint()
        return False

    def on_configure(self, widget, event, data=None):
        if self.double_buffer != None:
            self.double_buffer.finish()
            self.double_buffer = None
        self.double_buffer = cairo.ImageSurface(
            cairo.FORMAT_ARGB32,
            widget.get_allocated_width(),
            widget.get_allocated_height())
        self.redraw()
        return False


class Server(threading.Thread):

    def __init__(self, model, addr):
        self.addr = addr
        self.model = model
        threading.Thread.__init__(self)
        self.daemon = True

    def run(self):
        server_sck = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_sck.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_sck.bind(self.addr)
        server_sck.listen(5)
        while True:
            try:
                client_address = server_sck.accept()
                self.handle(client_address[0])
            except ProtocolViolation:
                pass
            finally:
                try:
                    client_address[0].close()
                except Exception:
                    pass

    def handle(self, sck):
        for frame in self.read_frames(sck):
            if frame == [0xff]:
                self.model.strobe()
            elif len(frame) >= 1 and frame[0] != 0xff:
                for channel, value in enumerate(frame[1:]):
                    self.model.set_led(frame[0], channel, value)
            else:
                raise ProtocolViolation()

    def read_frames(self, sck):
        STATE_IGNORE = 0 # I
        STATE_NORMAL = 1 # hate
        STATE_ESCAPE = 2 # Python

        ESCAPE_MARK = 0x54
        START_MARK = 0x55
        ESCAPE_ESCAPED = 0x00
        START_ESCAPED = 0x01

        payload_buf = []
        state = STATE_IGNORE

        for num in self.read_bytes(sck):

            if state == STATE_IGNORE:
                if num == START_MARK:
                    state = STATE_NORMAL

            elif state == STATE_NORMAL:
                if num == ESCAPE_MARK:
                    state = STATE_ESCAPE
                elif num == START_MARK:
                    yield payload_buf
                    payload_buf = []
                else:
                    payload_buf.append(num)

            elif state == STATE_ESCAPE:
                if num == ESCAPE_ESCAPED:
                    payload_buf.append(ESCAPE_MARK)
                    state = STATE_NORMAL
                elif num == START_ESCAPED:
                    payload_buf.append(START_MARK)
                    state = STATE_NORMAL
                else:
                    raise ProtocolViolation()

    def read_bytes(self, sck):
        while True:
            data = sck.recv(1024)
            if data == None:
                break # Connection closed
            yield from data


class ProtocolViolation(Exception):
    pass


def main():

    par = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        description='Vaporlight bus master emulator')
    par.add_argument('-a', '--addr', metavar="X",
        dest='addr', action='store', default="localhost",
        help='TCP address to listen on')
    par.add_argument('-p', '--port', metavar="X",
        dest='port', action='store', type=int, default=23429,
        help='TCP port to listen on')
    par.add_argument('-m', '--modules', metavar="X",
        dest='modules', action='store', type=int, default=4,
        help='number of modules')
    par.add_argument('-l', '--leds', metavar="X",
        dest='leds', action='store', type=int, default=5,
        help='number of leds per module')

    args = par.parse_args()
    model = Model(args.modules, args.leds)
    view = GtkView(model)
    server = Server(model, (args.addr, args.port))
    server.start()
    view.run()


if __name__ == "__main__":
    main()

