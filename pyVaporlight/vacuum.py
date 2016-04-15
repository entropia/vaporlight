import os
import time
#from spidermonkey import Runtime

from lupa import LuaRuntime
from lupa._lupa import LuaSyntaxError

from vapornet import VaporClient
import lupa




class ExecJS:
    def __init__(self, source, name, parent):
        """Throws LuaExceptions (LuaError or LuaSyntaxError)"""
        self.name = name
        self.source = source
        self.vapor_client = VaporClient()
        self.get_led_data = self.vapor_client.get_led_data
        self.runtime = LuaRuntime()
        self.render_function = None
        self.render_function = self.runtime.eval(source)
        if self.render_function is None:
            print "no render callback!"
    def getName(self):
        return self.name
    def render(self):
        try:
            self.render_function(self.vapor_client, time.time(), 35)  # TODO get num leds
            self.vapor_client.strobe()
        except ReferenceError:
            print "There is an error in your javascript"
            self.parent.remove_me(self)


class ExecJSManager:
    def __init__(self):
        self.running_instances = []
        pass

    def remove_me(self, me):
        self.running_instances.remove(me)

    def run_source(self, source, name="unknown"):
        self.running_instances.append(ExecJS(source, name, self))

    def run_file(self, filename):
        fp = open(filename, "rb")
        source = fp.read()
        fp.close()
        self.run_source(source, filename.split("/")[-1].rstrip(".js"))

    def render(self):
        for js in self.running_instances[::]:
            js.render()

    def get_data(self):
        return [n.get_led_data() for n in self.running_instances]

    def list_effect_scripts(self):
        return [n.rstrip(".js") for n in os.listdir("vaporscripts") if n.endswith(".js")]

    def get_effect_script(self, name):
        if name.find("..") >= 0 or name.startswith("/"):
            return
        with open(os.path.join("vaporscripts", name + ".js"), "rb") as fp:
            return fp.read()

    def put_effect_script(self, name, source):
        if name.find("..") >= 0 or name.startswith("/"):
            return
        with open(os.path.join("vaporscripts", name + ".js"), "wb") as fp:
            fp.write(source)

    def list_running_effect_scripts(self):
        return [n.getName() for n in self.running_instances]

    def run_effect(self, name):
        if name.find("..") >= 0 or name.startswith("/"):
            return
        self.run_file(os.path.join("vaporscripts", name + ".js"))