package de.entropia.vapor.hardware

import de.entropia.vapor.util.{Green, Red, Blue, Color, RgbChannel}
import de.entropia.vapor.daemon.config.Settings


/**
 * Maps virtual RGB LEDs to hardware modules/channels.
 *
 * <ul>
 * <li>software: global RGB LED ids, atomic RGB color specifications
 * <li>hardware: module IDs, modules have grayscale channels
 * </li>
 */
class Mapping(val settings: Settings, val buffer: Buffer) {

  def set(led: Int, color: Color) {
    set(led, color, Red)
    set(led, color, Green)
    set(led, color, Blue)
  }

  def set(led: Int, color: Color, channel: RgbChannel) {
    val maybeModuleAndPosition = settings.channels.get((led, channel))
    if (maybeModuleAndPosition.isDefined) {
      val (module, position) = maybeModuleAndPosition.get
      buffer.set(module, position, channel.extract(color).toByte)
    }
  }

  def strobe() {
    buffer.strobe()
  }
}
