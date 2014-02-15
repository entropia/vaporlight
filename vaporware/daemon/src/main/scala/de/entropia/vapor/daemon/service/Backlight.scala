package de.entropia.vapor.daemon.service

import de.entropia.vapor.mixer.Mixer
import de.entropia.vapor.daemon.config.Settings
import de.entropia.vapor.util.Color

class Backlight(settings: Settings, mixer: Mixer, notify: Color => Unit = (Color) => Unit) {
  val lowestLayer = mixer.register(Integer.MIN_VALUE, true)
  var color: Color = Color.black

  def setColor(color: Color) = synchronized {
    settings.leds foreach ((led) =>
      lowestLayer.set(led, color))
    lowestLayer.strobe
    this.color = color
    notify(color)
  }
}
