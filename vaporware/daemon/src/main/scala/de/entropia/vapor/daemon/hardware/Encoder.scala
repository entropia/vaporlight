package de.entropia.vapor.hardware


/**
 * Translates commands into byte sequences.
 */
class Encoder(val framer: Framer) {
  val SET_RAW_COMMAND = 0x00.toByte
  val SET_XYY_COMMAND = 0x01.toByte

  val HIGH_MASK = 65280 // 0b1111111100000000
  val LOW_MASK = 255 // 0b0000000011111111

  def update(module: Byte, values: Seq[Int]) {
    framer.write(Vector(module, SET_RAW_COMMAND) ++ values.flatMap(v =>
      Vector(((v & HIGH_MASK) >> 8).toByte, (v & LOW_MASK).toByte)))
  }

  def strobe() {
    framer.write(Vector(0xfe.toByte))
    framer.flush()
  }
}
