package de.entropia.vapor.hardware

import de.entropia.vapor.util.{Green, Red, Blue, Color, RgbChannel}
import de.entropia.vapor.config.Config


/**
 * Maps virtual RGB LEDs to hardware modules/channels.
 *
 * <ul>
 * <li>software: global RGB LED ids, atomic RGB color specifications
 * <li>hardware: module IDs, modules have grayscale channels
 * </li>
 */
class Mapping(val config: Config, val buffer: Buffer) {
  val channels = config.getChannelMapping()

  def set(led: Int, color: Color) {
    set(led, color, Red)
    set(led, color, Green)
    set(led, color, Blue)
  }

  def set(led: Int, color: Color, channel: RgbChannel) {
    val (module, position) = channels(led, channel)
    buffer.set(module, position, channel.extract(color).toByte)
  }

  def strobe() {
    buffer.strobe()
  }
}
