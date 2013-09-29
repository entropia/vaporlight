package de.entropia.vapor.server

import de.entropia.vapor.mixer.{Overlay, Mixer}
import de.entropia.vapor.util.Color
import de.entropia.vapor.daemon.config.{Settings, Token}
import de.entropia.vapor.daemon.config.Token.Seq2TokenId
import de.entropia.vapor.daemon.mixer.Manager
import com.typesafe.scalalogging.slf4j.Logging
import java.net.InetSocketAddress


class Client(val id: Int, val settings: Settings, val mixer: Mixer, val manager: Manager, val local: InetSocketAddress, val remote: InetSocketAddress, val kill: () => Unit) extends Logging {
  var token: Option[Token] = None
  var overlay: Option[Overlay] = None

  def dispatch(message: Message) = message match {
    case AuthMessage(bytes) => auth(bytes)
    case LowPrecisionSetMessage(led, color) => setLed(led, color)
    case HighPrecisionSetMessage(led, color) => setLed(led, color)
    case StrobeMessage() => strobe()
  }

  def auth(bytes: Seq[Byte]) {
    overlay.map(_.free())
    val paddedBytes = bytes.padTo(16, 0x00.toByte)
    token = settings.tokens.get(paddedBytes)
    token match {
      case None =>
        overlay = None
      case Some(t: Token) =>
        overlay = Some(manager.getOverlay(t))
    }
  }

  def setLed(i: Int, color: Color) {
    overlay match {
      case Some(o: Overlay) =>
        o.set(i, color)
      case None =>
    }
  }

  def strobe() {
    overlay match {
      case Some(o: Overlay) =>
        o.strobe()
      case None =>
    }
  }

  def connect() {
  }

  def disconnect() {
    overlay.map(_.free())
  }
}
