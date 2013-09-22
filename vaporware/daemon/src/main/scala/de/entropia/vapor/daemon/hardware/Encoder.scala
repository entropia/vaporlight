package de.entropia.vapor.hardware


/**
 * Translates commands into byte sequences.
 */
class Encoder(val framer: Framer) {

  def update(module: Byte, values: Seq[Byte]) {
    framer.write(Vector(module) ++ values)
  }

  def strobe() {
    framer.write(Vector(0xfe.toByte))
    framer.flush()
  }
}
