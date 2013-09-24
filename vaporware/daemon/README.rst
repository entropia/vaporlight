vapord
======

This is a Vaporlight router written in Scala.
The Vaporlight is our shiny RGB LED system[1].
This project offers access to the hardware
via a simple network protocol (described in
`HACKING` in the repository root directory).

It is a hopefully less unmaintainable albeit probably
slower rewrite of the existing C implementation.

License: revised BSD (see LICENSE)
Authors: see AUTHORS

[1] https://github.com/entropia/vaporlight


Configuration
-------------

You can parametrize the daemon using config files.
Default config files are included in the JAR.
You can specify alternate config files via command line parameters:

* Main config file: `-Dconfig.file=...`.
  They use the HOCON (like JSON) format, described here: [2].

* Logging config file: `-Dlogback.configurationFile=...`
  The format is explained in [3].

[2] https://github.com/typesafehub/config#json-superset
[3] http://logback.qos.ch/manual/configuration.html


Debugging
---------

The daemon can either connect directly to a serial
port or to another network server, such as the emulator
in the main Vaporlight repository. This way, one can
debug the daemon without the actual hardware.

See also: `application.conf`.
