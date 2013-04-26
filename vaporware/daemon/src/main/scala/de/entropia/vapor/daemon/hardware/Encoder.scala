package de.entropia.vapor.hardware


/**
 * Translates commands into byte sequences.
 */
class Encoder(val framer: Framer) {

  def update(module: Byte, values: Seq[Byte]) {
    framer.write(Vector(module) ++ values)
  }

  def strobe() {
    // always send *two* strobes - a workaround for a problem with the bus protocol:
    // because of how framing works, the Vaporlight firmware can only detect the end
    // of a frame by waiting for the beginning of the next frame. This is problematic
    // if only a single strobe without further messages is sent.
    framer.write(Vector(0xfe.toByte))
    framer.write(Vector(0xfe.toByte))
    framer.flush()
  }
}
