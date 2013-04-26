package de.entropia.vapor.mixer

import de.entropia.vapor.util.Color
import collection.mutable


class Overlay(val mixer: Mixer, val priority: Int, val persistent: Boolean) {
  private var front = Map.empty[Int, Color].withDefaultValue(Color.transparent)
  private var frontDirty = Set.empty[Int]
  private val back = mutable.Map.empty[Int, Color]
  private val backDirty = mutable.Set.empty[Int]

  def dirtyLeds = frontDirty

  def ledValues = front

  def set(led: Int, color: Color) {
    synchronized {
      back(led) = color
      backDirty.add(led)
    }
  }

  def strobe() {
    synchronized {
      mixer.synchronized {
        front = back.toMap
        frontDirty = frontDirty ++ backDirty.toSet
        backDirty.clear()
        mixer.delay.requestRun()
      }
    }
  }

  def seen() {
    mixer.synchronized {
      frontDirty = Set.empty[Int]
    }
  }

  def free() {
    if (persistent) return
    synchronized {
      mixer.synchronized {
        mixer.overlays -= this
        mixer.markBackgroundDirty(front.keySet) // also calls requestRun
      }
    }
  }
}
