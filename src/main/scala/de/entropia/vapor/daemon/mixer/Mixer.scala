package de.entropia.vapor.mixer

import de.entropia.vapor.hardware.Hardware
import collection.mutable.ArrayBuffer
import de.entropia.vapor.util.Color


/**
 * Manages access to a single, shared, Vaporlight bus.
 */
class Mixer(val hw: Hardware) {
  val delay = new Delay(minSleepMillis = 10)
  val overlays = ArrayBuffer.empty[Overlay]

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
    overlays.foldLeft(Color.black)((bg, fgOverlay) => fgOverlay.ledValues(led).blendOver(bg))

  /**
   * Creates an overlay.
   *
   * Overlays are blended over one another in the order of ascending priority.
   * Multiple overlays may use the same priority. Overlays with equal priority
   * are blended in order of creation (ie, later overlays have "higher" priority).
   */
  def register(priority: Int) = {
    synchronized {
      val overlay = new Overlay(this, priority)
      overlays.append(overlay)
      overlays.sortBy(_.priority)
      overlay
    }
  }
}
