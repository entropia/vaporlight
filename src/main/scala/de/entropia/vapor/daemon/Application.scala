package de.entropia.vapor.daemon

import grizzled.slf4j.Logging
import de.entropia.vapor.config.Config
import de.entropia.vapor.hardware.{Physical, Hardware}
import de.entropia.vapor.mixer.Mixer
import de.entropia.vapor.server.Server


class Application(config: Config) extends Logging {
  //  val hardware = Hardware(config, Physical.openSerialPort("/dev/ttyUSB0"))
  val hardware = Hardware(config, Physical.openSocket("localhost", 23429))
  val mixer = new Mixer(hardware)
  val server = new Server(config, mixer)

  def start() {
    info("starting up")
    server.start()
    mixer.run()
  }
}

object Application {

  def main(args: Array[String]) {
    new Application(new Config()).start()
  }
}












