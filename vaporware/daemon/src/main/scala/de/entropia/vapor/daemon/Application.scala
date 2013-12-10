package de.entropia.vapor.daemon

import de.entropia.vapor.hardware.Hardware
import de.entropia.vapor.mixer.Mixer
import de.entropia.vapor.server.Server
import de.entropia.vapor.daemon.config.Settings
import de.entropia.vapor.daemon.mixer.Manager
import com.typesafe.scalalogging.slf4j.Logging
import de.entropia.vapor.daemon.web.Webserver
import de.entropia.vapor.daemon.service.{Dimmer, Backlight}


class Application(settings: Settings) extends Logging {
  val hardware = Hardware(settings)
  val mixer = new Mixer(settings, hardware)
  val manager = new Manager(mixer, settings)
  val server = new Server(settings, mixer, manager)
  val webserver = new Webserver(settings,
    new Dimmer(settings, mixer),
    new Backlight(settings, mixer),
    server.status)

  def start() {
    logger.info("starting up")
    server.start()
    webserver.start()
    mixer.run()
  }
}

object Application {

  def main(args: Array[String]) {
    new Application(Settings.load()).start()
  }
}












