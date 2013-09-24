package de.entropia.vapor.hardware


/**
 * Translates commands into byte sequences.
 */
class Encoder(val framer: Framer) {
  val SET_RAW_COMMAND = 0x00.toByte
  val SET_XYY_COMMAND = 0x01.toByte

  def update(module: Byte, values: Seq[Byte]) {
    framer.write(Vector(module, SET_RAW_COMMAND) ++ values.flatMap(v => Vector(v, v)))
  }

  def strobe() {
    framer.write(Vector(0xfe.toByte))
    framer.flush()
  }
}
