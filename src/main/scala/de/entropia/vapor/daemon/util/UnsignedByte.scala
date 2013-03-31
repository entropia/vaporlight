package de.entropia.vapor.util

class UnsignedByte(val b: Byte) {
  def toUnsignedInt: Int = 0xff & b.asInstanceOf[Int]
}

object UnsignedByte {
  implicit def Byte2UnsignedByte(b: Byte): UnsignedByte = new UnsignedByte(b)
}
