package de.entropia.vapor.daemon.service

import de.entropia.vapor.mixer.Mixer
import de.entropia.vapor.daemon.config.Settings
import de.entropia.vapor.util.Color

class Dimmer(settings: Settings, mixer: Mixer) {
  val highestLayer = mixer.register(Integer.MAX_VALUE, true)
  var dimness = 65535

  def dimToBrightest() =
    dimTo(65535)

  def dimToDarkest() =
    dimTo(0)

  def dimTo(brightness: Int) = synchronized {
    this.dimness = brightness
    settings.leds foreach ((led) =>
      highestLayer.set(led, Color.dim(Color.MAX_VALUE - brightness)))
    highestLayer.strobe
  }
}
