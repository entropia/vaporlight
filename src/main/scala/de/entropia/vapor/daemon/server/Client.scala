package de.entropia.vapor.server

import de.entropia.vapor.mixer.{Overlay, Mixer}
import de.entropia.vapor.util.Color
import grizzled.slf4j.Logging
import de.entropia.vapor.config.{Token, Config}


class Client(val config: Config, val mixer: Mixer) extends Logging {
  var token: Option[Token] = None
  var overlay: Option[Overlay] = None

  def dispatch(message: Message) = message match {
    case AuthMessage(bytes) => auth(bytes)
    case SetMessage(led, color) => setLed(led, color)
    case StrobeMessage() => strobe()
  }

  def auth(bytes: Seq[Byte]) {
    info("auth(%s)".format(bytes))
    overlay.map(_.free())
    token = config.getToken(bytes)
    token match {
      case None =>
        overlay = None
      case Some(t: Token) =>
        overlay = Some(mixer.register(t.priority))
    }
  }

  def setLed(i: Int, color: Color) {
    info("setLed(%d, %s)".format(i, color))
    overlay match {
      case Some(o: Overlay) =>
        o.set(i, color)
      case None =>
    }
  }

  def strobe() {
    info("strobe()")
    overlay match {
      case Some(o: Overlay) =>
        o.strobe()
      case None =>
    }
  }

  def connect() {
    info("connect()")
  }

  def disconnect() {
    info("disconnect()")
    overlay.map(_.free())
  }
}
