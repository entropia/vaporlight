package de.entropia.vapor.util

import de.entropia.vapor.util.UnsignedByte.Byte2UnsignedByte


/**
 * Interface of our internal color representations.
 */
trait Color {

  /** Red (16 bit) */
  def r: Int

  /** Green (16 bit) */
  def g: Int

  /** Blue (16 bit) */
  def b: Int

  /** Alpha (16 bit, 0 is transparent, 65535 is opaque) */
  def a: Int

  /** Swizzling */
  def rgb = (r, g, b)

  /** Swizzling */
  def rgba = (r, g, b, a)

  /** The same color, but with maximum alpha value */
  def opaque: Color

  /** Combine with another color. The mixture ratio is determined by the first colors alpha value. */
  def blendOver(other: Color): Color

  /** Returns RGBA as bytes (ie, with 8 bit precision) */
  def as8BitRgbaByteVector: Vector[Byte] =
    Vector((r >> 8).toByte, (g >> 8).toByte, (b >> 8).toByte, (a >> 8).toByte)

  /** Returns RGBA as sequence of bytes (16 bit precision, big endian) */
  def asRrGgBbAaByteVector: Vector[Byte] =
    Vector((r >> 8).toByte, (r & 0xff).toByte, (g >> 8).toByte, (g & 0xff).toByte, (b >> 8).toByte, (b & 0xff).toByte, (a >> 8).toByte, (a & 0xff).toByte)

  /** Returns a hex string like "00FF00" */
  def asRgbHexString: String =
    f"${r >> 8}%02x${g >> 8}%02x${b >> 8}%02x"

  /** Returns a hex string like "0000FFFF0000" (for green) */
  def asRrGgBbHexString: String =
    f"$r%04x$g%04x$b%04x"
}

object Color {

  /** The maximum value representable using a 16 bit precision. */
  val MAX_VALUE = 65535 // = 2 ^ 16 - 1

  def black =
    RgbColor.from8BitRgba(0, 0, 0, 255)

  def dim(alpha: Int) =
    RgbColor.from16BitRgba(0, 0, 0, alpha)

  def transparent =
    RgbColor.from8BitRgba(0, 0, 0, 0)
}

/**
 * A color represented by "red", "green" and "blue" components.
 */
case class RgbColor(val r: Int, val g: Int, val b: Int, val a: Int) extends Color {
  require(0 <= r && r <= Color.MAX_VALUE, s"r = $r not in [0..${Color.MAX_VALUE}]")
  require(0 <= g && g <= Color.MAX_VALUE, s"g = $g not in [0..${Color.MAX_VALUE}]")
  require(0 <= b && b <= Color.MAX_VALUE, s"b = $b not in [0..${Color.MAX_VALUE}]")
  require(0 <= a && a <= Color.MAX_VALUE, s"a = $a not in [0..${Color.MAX_VALUE}]")

  def blendOver(bg: Color): Color = {
    val fg = this
    val alpha = fg.a.toFloat / Color.MAX_VALUE
    val blendOver = (fgVal: Int, bgVal: Int) => (alpha * fgVal + (1.0f - alpha) * bgVal).toInt
    new RgbColor(blendOver(fg.r, bg.r), blendOver(fg.g, bg.g), blendOver(fg.b, bg.b), Color.MAX_VALUE)
  }

  def opaque =
    RgbColor(r, g, b, Color.MAX_VALUE)
}

object RgbColor {

  def from16BitRgba(r: Int, g: Int, b: Int, a: Int): Color =
    RgbColor(r, g, b, a)

  def from16BitRgb(r: Int, g: Int, b: Int): Color =
    RgbColor(r, g, b, 65535)

  def fromRrGgBbAa(rHi: Int, rLo: Int, gHi: Int, gLo: Int, bHi: Int, bLo: Int, aHi: Int, aLo: Int): Color =
    from16BitRgba((rHi << 8) | rLo, (gHi << 8) | gLo, (bHi << 8) | bLo, (aHi << 8) | aLo)

  def fromRrGgBb(rHi: Int, rLo: Int, gHi: Int, gLo: Int, bHi: Int, bLo: Int): Color =
    from16BitRgb((rHi << 8) | rLo, (gHi << 8) | gLo, (bHi << 8) | bLo)

  def from8BitRgba(r: Int, g: Int, b: Int, a: Int): Color =
    fromRrGgBbAa(r, r, g, g, b, b, a, a)

  def from8BitRgb(r: Int, g: Int, b: Int): Color =
    fromRrGgBb(r, r, g, g, b, b)

  def from8BitRgbBytes(r: Byte, g: Byte, b: Byte): Color =
    from8BitRgb(r.toUnsignedInt, g.toUnsignedInt, b.toUnsignedInt)

  def from8BitRgbaBytes(r: Byte, g: Byte, b: Byte, a: Byte): Color =
    from8BitRgba(r.toUnsignedInt, g.toUnsignedInt, b.toUnsignedInt, a.toUnsignedInt)

  def fromRrGgBbBytes(rHi: Byte, rLo: Byte, gHi: Byte, gLo: Byte, bHi: Byte, bLo: Byte): Color =
    fromRrGgBb(rHi.toUnsignedInt, rLo.toUnsignedInt, gHi.toUnsignedInt, gLo.toUnsignedInt, bHi.toUnsignedInt, bLo.toUnsignedInt)

  def fromRrGgBbAaBytes(rHi: Byte, rLo: Byte, gHi: Byte, gLo: Byte, bHi: Byte, bLo: Byte, aHi: Byte, aLo: Byte): Color =
    fromRrGgBbAa(rHi.toUnsignedInt, rLo.toUnsignedInt, gHi.toUnsignedInt, gLo.toUnsignedInt, bHi.toUnsignedInt, bLo.toUnsignedInt, aHi.toUnsignedInt, aLo.toUnsignedInt)

  def fromRgbBytes(r: Byte, g: Byte, b: Byte): Color =
    from8BitRgb(r.toUnsignedInt, g.toUnsignedInt, b.toUnsignedInt)

  def fromRgbaBytes(r: Byte, g: Byte, b: Byte, a: Byte): Color =
    from8BitRgba(r.toUnsignedInt, g.toUnsignedInt, b.toUnsignedInt, a.toUnsignedInt)

  def fromRgbByteSeq(rgb: Seq[Byte]): Option[Color] = rgb match {
    case Seq(r, g, b) => Some(from8BitRgbBytes(r, g, b))
    case _ => None
  }

  def fromRgbaByteSeq(rgba: Seq[Byte]): Option[Color] = rgba match {
    case Seq(r, g, b, a) => Some(fromRgbaBytes(r, g, b, a))
    case _ => None
  }

  def fromRrGgBbByteSeq(rrggbb: Seq[Byte]): Option[Color] = rrggbb match {
    case Seq(rHi, rLo, gHi, gLo, bHi, bLo) => Some(fromRrGgBbBytes(rHi, rLo, gHi, gLo, bHi, bLo))
    case _ => None
  }

  def fromRrGgBbAaByteSeq(rrggbbaa: Seq[Byte]): Option[Color] = rrggbbaa match {
    case Seq(rHi, rLo, gHi, gLo, bHi, bLo, aHi, aLo) => Some(fromRrGgBbAaBytes(rHi, rLo, gHi, gLo, bHi, bLo, aHi, aLo))
    case _ => None
  }
}

