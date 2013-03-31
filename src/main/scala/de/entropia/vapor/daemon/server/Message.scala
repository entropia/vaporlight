package de.entropia.vapor.server

import de.entropia.vapor.util.Color
import de.entropia.vapor.util.UnsignedByte.Byte2UnsignedByte


case class ProtocolViolation(why: String = "") extends Exception(why)

case class SecurityViolation(why: String = "") extends Exception(why)


sealed abstract class Message() {
  def serialize: Seq[Byte]
}

object Message {
  def lookup(opcode: Byte): MessageType = opcode match {
    case AuthMessage.opcode => AuthMessage
    case SetMessage.opcode => SetMessage
    case StrobeMessage.opcode => StrobeMessage
    case _ => throw new ProtocolViolation("invalid opcode: %02X".format(opcode))
  }
}

trait MessageType {
  val payloadLength: Int

  def parse(bytes: Seq[Byte]): Message
}

case class StrobeMessage() extends Message {
  def serialize = Vector[Byte](StrobeMessage.opcode)
}

object StrobeMessage extends MessageType {
  val opcode = 0xFF.toByte
  val payloadLength = 0

  def parse(payload: Seq[Byte]) = {
    require(payload.size == payloadLength)
    new StrobeMessage()
  }
}

case class AuthMessage(val token: Seq[Byte]) extends Message {
  require(token.length == 16)

  def serialize = Vector(AuthMessage.opcode) ++ token
}

object AuthMessage extends MessageType {
  val opcode = 0x02.toByte
  val payloadLength = 16

  def parse(payload: Seq[Byte]) = {
    require(payload.size == payloadLength)
    new AuthMessage(payload)
  }
}

case class SetMessage(val led: Int, color: Color) extends Message {
  require(0 <= led && led <= 65535)

  def serialize = Vector(
    SetMessage.opcode,
    ((led & 0xff00) >> 8).toByte,
    (led & 0xff).toByte) ++ color.toByteVector
}

object SetMessage extends MessageType {
  val opcode = 0x01.toByte
  val payloadLength = 6

  def parse(payload: Seq[Byte]) = {
    require(payload.size == payloadLength)
    val led = (payload(0).toUnsignedInt << 8) + payload(1).toUnsignedInt
    val color = Color.fromByteSeq(payload.slice(2, payload.size))
    new SetMessage(led, color)
  }
}
