package de.entropia.vapor.util

import de.entropia.vapor.util.UnsignedByte.Byte2UnsignedByte

case class Color(r: Int, g: Int, b: Int, a: Int = 255) {
  require(0 <= r && r <= 255, "r = %d not in [0..255]".format(r))
  require(0 <= g && g <= 255, "g = %d not in [0..255]".format(g))
  require(0 <= b && b <= 255, "b = %d not in [0..255]".format(b))
  require(0 <= a && a <= 255, "a = %d not in [0..255]".format(a))

  def toJavaColor =
    new java.awt.Color(r, g, b, a)

  def toByteVector: Seq[Byte] =
    Vector(r.toByte, g.toByte, b.toByte, a.toByte)

  def rgb = (r, g, b)

  def rgba = (r, g, b, a)

  def opaque = Color(r, g, b, 255)

  def blendOver(bg: Color): Color = {
    val fg = this
    val alpha = fg.a.toFloat / 255.0f
    val blendOver = (fgVal: Int, bgVal: Int) => (alpha * fgVal + (1.0f - alpha) * bgVal).toInt
    new Color(blendOver(fg.r, bg.r), blendOver(fg.g, bg.g), blendOver(fg.b, bg.b), 255)
  }

  override def toString = "Color(%d, %d, %d, %d)".format(r, g, b, a)
}

case object Color {

  def fromBytes(r: Byte, g: Byte, b: Byte, a: Byte = 0xFF.toByte): Color =
    new Color(r.toUnsignedInt, g.toUnsignedInt, b.toUnsignedInt, a = a.toUnsignedInt)

  def fromByteSeq(rgba: Seq[Byte]): Color = rgba.size match {
    case 3 => fromBytes(rgba(0), rgba(1), rgba(2))
    case 4 => fromBytes(rgba(0), rgba(1), rgba(2), rgba(3))
    case _ => throw new IllegalArgumentException
  }

  def black = apply(0, 0, 0)

  def transparent = apply(0, 0, 0, 0)
}


sealed abstract class RgbChannel {
  def extract(c: Color): Int
}

case object Red extends RgbChannel {
  def extract(c: Color) = c.r
}

case object Green extends RgbChannel {
  def extract(c: Color) = c.g
}

case object Blue extends RgbChannel {
  def extract(c: Color) = c.b
}
