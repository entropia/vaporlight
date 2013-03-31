package de.entropia.vapor.config

import collection.mutable
import de.entropia.vapor.util.{Blue, Green, Red, RgbChannel}

case class Config() {
  val port = 7534
  val addr = "127.0.0.1"
  val tokens = mutable.HashMap(
    TokenConfig.ofString("club event light", 10),
    TokenConfig.ofString("sixteen letters.", 10),
    TokenConfig.ofString("background light", 10),
    TokenConfig.ofByteSeq(Array.fill(16) {
      0.toByte
    }, 10))
  val device = "/dev/ttyUSB0"

  def getToken(bytes: Seq[Byte]): Option[Token] =
    tokens.get(bytes).map(_.instantiate())

  def getModuleChannelCounts(): Map[Int, Int] =
    Map(0 -> 16, 1 -> 16, 2 -> 16, 3 -> 16)

  def getChannelMapping(): Map[(Int, RgbChannel), (Byte, Int)] = Map(
    // Module 0
    (0, Red) -> (0.toByte, 0),
    (0, Green) -> (0.toByte, 1),
    (0, Blue) -> (0.toByte, 2),
    (1, Red) -> (0.toByte, 3),
    (1, Green) -> (0.toByte, 4),
    (1, Blue) -> (0.toByte, 5),
    (2, Red) -> (0.toByte, 6),
    (2, Green) -> (0.toByte, 7),
    (2, Blue) -> (0.toByte, 8),
    (3, Red) -> (0.toByte, 9),
    (3, Green) -> (0.toByte, 10),
    (3, Blue) -> (0.toByte, 11),
    (4, Red) -> (0.toByte, 12),
    (4, Green) -> (0.toByte, 13),
    (4, Blue) -> (0.toByte, 14),

    // Module 1
    (5, Red) -> (1.toByte, 0),
    (5, Green) -> (1.toByte, 1),
    (5, Blue) -> (1.toByte, 2),
    (6, Red) -> (1.toByte, 3),
    (6, Green) -> (1.toByte, 4),
    (6, Blue) -> (1.toByte, 5),
    (7, Red) -> (1.toByte, 6),
    (7, Green) -> (1.toByte, 7),
    (7, Blue) -> (1.toByte, 8),
    (8, Red) -> (1.toByte, 9),
    (8, Green) -> (1.toByte, 10),
    (8, Blue) -> (1.toByte, 11),
    (9, Red) -> (1.toByte, 12),
    (9, Green) -> (1.toByte, 13),
    (9, Blue) -> (1.toByte, 14),

    // Module 2
    (10, Red) -> (2.toByte, 0),
    (10, Green) -> (2.toByte, 1),
    (10, Blue) -> (2.toByte, 2),
    (11, Red) -> (2.toByte, 3),
    (11, Green) -> (2.toByte, 4),
    (11, Blue) -> (2.toByte, 5),
    (12, Red) -> (2.toByte, 6),
    (12, Green) -> (2.toByte, 7),
    (12, Blue) -> (2.toByte, 8),
    (13, Red) -> (2.toByte, 9),
    (13, Green) -> (2.toByte, 10),
    (13, Blue) -> (2.toByte, 11),
    (14, Red) -> (2.toByte, 12),
    (14, Green) -> (2.toByte, 13),
    (14, Blue) -> (2.toByte, 14),

    // Module 3
    (15, Red) ->(3.toByte, 0),
    (15, Green) ->(3.toByte, 1),
    (15, Blue) ->(3.toByte, 2),
    (16, Red) ->(3.toByte, 3),
    (16, Green) ->(3.toByte, 4),
    (16, Blue) ->(3.toByte, 5),
    (17, Red) ->(3.toByte, 6),
    (17, Green) ->(3.toByte, 7),
    (17, Blue) ->(3.toByte, 8),
    (18, Red) ->(3.toByte, 9),
    (18, Green) ->(3.toByte, 10),
    (18, Blue) ->(3.toByte, 11),
    (19, Red) ->(3.toByte, 12),
    (19, Green) ->(3.toByte, 13),
    (19, Blue) ->(3.toByte, 14))
}

case class TokenConfig(val token: Seq[Byte], priority: Int) {
  def tuple() = (token, this)

  def instantiate() = new Token(priority)
}

object TokenConfig {
  def ofString(token: String, priority: Int): TokenConfig =
    ofByteSeq(token.getBytes("US-ASCII").toSeq, priority)

  def ofByteSeq(token: Seq[Byte], priority: Int): TokenConfig =
    new TokenConfig(token, priority)

  implicit def toTuple(inst: TokenConfig): (Seq[Byte], TokenConfig) = (inst.token, inst)
}

class Token(val priority: Int) {
}
