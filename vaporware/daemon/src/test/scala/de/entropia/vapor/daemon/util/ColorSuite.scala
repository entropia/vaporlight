package de.entropia.vapor.util

import org.junit.runner.RunWith
import org.scalatest.junit.JUnitRunner
import org.scalatest.FunSuite

@RunWith(classOf[JUnitRunner])
class ColorSuite extends FunSuite {

  test("swizzling") {
    val testee = RgbColor(130, 140, 150, 160)
    assert((130) === testee.r)
    assert((140) === testee.g)
    assert((150) === testee.b)
    assert((160) === testee.a)
    assert((130, 140, 150) === testee.rgb)
    assert((130, 140, 150, 160) === testee.rgba)
  }

  test("opaque version of color") {
    assert(RgbColor(10<<8|10, 20<<8|20, 30<<8|30, 255<<8|255) === RgbColor.from8BitRgba(10, 20, 30, 40).opaque)
  }

  test("serialization") {
    assert(Vector(130.toByte, 140.toByte, 150.toByte, 160.toByte) === RgbColor.from8BitRgba(130, 140, 150, 160).as8BitRgbaByteVector)
  }

  test("blending") {
    val fg = RgbColor.from8BitRgba(0, 255, 100, 128)
    val bg = RgbColor.from8BitRgba(0, 0, 0, 0)
    assert(RgbColor(0,32896,12900,65535) === fg.blendOver(bg))
  }

  test("blending of a transparent color over another color") {
    val fg = RgbColor.from8BitRgba(100, 100, 100, 0)
    val bg = RgbColor.from8BitRgba(222, 222, 222, 0)
    assert(RgbColor.from8BitRgba(222, 222, 222, 255) === fg.blendOver(bg))
  }
}
