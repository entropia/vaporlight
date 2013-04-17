package de.entropia.vapor.mixer

import de.entropia.vapor.hardware.Hardware
import collection.mutable.ArrayBuffer
import de.entropia.vapor.util.Color
import de.entropia.vapor.daemon.config.Settings


/**
 * Manages access to a single, shared, Vaporlight bus.
 */
class Mixer(val settings: Settings, val hw: Hardware) {
  val delay = new Delay(minSleepMillis = 10)
  val background = new Overlay(this, Integer.MIN_VALUE, true)
  val overlays = ArrayBuffer(background)
  var allDirty = false;

  def run() {
    while (true) {
      delay.awaitRun()
      strobe();
    }
  }

  protected def strobe() {
    synchronized {
      getDirtyLeds().foreach(led => hw.set(led, getBlendedColor(led)))
      overlays.foreach(_.seen())
      hw.strobe()
    }
  }

  private def getDirtyLeds() =
    overlays.flatMap(ov => ov.dirtyLeds)

  private def getBlendedColor(led: Int) =
    overlays.foldLeft(Color.black)((bg, fgOverlay) => fgOverlay.ledValues.get(led).getOrElse(Color.transparent).blendOver(bg))

  /**
   * Creates an overlay.
   *
   * Overlays are blended over one another in the order of ascending priority.
   * Multiple overlays may use the same priority. Overlays with equal priority
   * are blended in order of creation (ie, later overlays have "higher" priority).
   */
  def register(priority: Int, persistent: Boolean) = {
    synchronized {
      val overlay = new Overlay(this, priority, persistent)
      overlays.append(overlay)
      overlays.sortBy(_.priority)
      overlay
    }
  }

  def markBackgroundDirty(leds: Set[Int]) {
    leds.foreach(background.set(_, Color.transparent))
    background.strobe()
  }
}
