package de.entropia.vapor.hardware

import java.io.OutputStream
import grizzled.slf4j.Logging
import de.entropia.vapor.util.UnsignedByte.Byte2UnsignedByte


/**
 * Frames byte sequences.
 *
 * A self-synchronizing code is used on the Vaporlight bus,
 * and this class encodes individual messages with that code.
 */
class Framer(val rawDevice: OutputStream) extends Logging {
  val ESCAPE_MARK = 0x54.toByte
  val START_MARK = 0x55.toByte

  def write(framePayloadBytes: Seq[Byte]) {
    // note: make sure there are no unintended byte->int conversions
    debug("writing: %s".format(framePayloadBytes))
    rawDevice.write(START_MARK.toUnsignedInt)
    for (b <- framePayloadBytes) {
      b match {
        case ESCAPE_MARK =>
          rawDevice.write(ESCAPE_MARK.toUnsignedInt)
          rawDevice.write(0x00)
        case START_MARK =>
          rawDevice.write(ESCAPE_MARK.toUnsignedInt)
          rawDevice.write(0x01)
        case _ =>
          rawDevice.write(b.toUnsignedInt)
      }
    }
  }

  def flush() {
    rawDevice.flush()
  }
}
