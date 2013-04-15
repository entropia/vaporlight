package de.entropia.vapor.daemon

import grizzled.slf4j.Logging
import de.entropia.vapor.hardware.Hardware
import de.entropia.vapor.mixer.Mixer
import de.entropia.vapor.server.Server
import de.entropia.vapor.daemon.config.Settings


class Application(settings: Settings) extends Logging {
  val hardware = Hardware(settings)
  val mixer = new Mixer(settings, hardware)
  val server = new Server(settings, mixer)

  def start() {
    info("starting up")
    server.start()
    mixer.run()
  }
}

object Application {

  def main(args: Array[String]) {
    new Application(Settings.load()).start()
  }
}












