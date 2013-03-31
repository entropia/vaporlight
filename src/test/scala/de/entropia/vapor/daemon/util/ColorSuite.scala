package de.entropia.vapor.util

import org.junit.runner.RunWith
import org.scalatest.junit.JUnitRunner
import org.scalatest.FunSuite

@RunWith(classOf[JUnitRunner])
class ColorSuite extends FunSuite {

  test("swizzling") {
    val testee = Color(130, 140, 150, 160)
    assert((130) === testee.r)
    assert((140) === testee.g)
    assert((150) === testee.b)
    assert((160) === testee.a)
    assert((130, 140, 150) === testee.rgb)
    assert((130, 140, 150, 160) === testee.rgba)
  }

  test("opaque version of color") {
    assert(Color(10, 20, 30, 255) === Color(10, 20, 30, 40).opaque)
  }

  test("serialization") {
    assert(Vector(130.toByte, 140.toByte, 150.toByte, 160.toByte) === Color(130, 140, 150, 160).toByteVector)
  }

  test("blending") {
    val fg = Color(0, 255, 100, 128)
    val bg = Color(0, 0, 0, 0)
    assert(Color(0, 128, 50, 255) === fg.blendOver(bg))
  }
}
