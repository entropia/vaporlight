package de.entropia.vapor.daemon.web

import unfiltered.response._
import de.entropia.vapor.daemon.config.Settings
import unfiltered.request._
import unfiltered.response.ResponseString
import de.entropia.vapor.daemon.service.{Dimmer, Backlight}
import de.entropia.vapor.util.{Color, RgbColor}
import de.entropia.vapor.daemon.web.Webserver._

class Webserver(val settings: Settings, val dimmer: Dimmer, val backlight: Backlight) {

  def start() {
    settings.webServerInterface match {
      case Some((host, port)) => start(host, port)
      case _ => // pass
    }
  }

  def start(host: String, port: Int) {
    unfiltered.netty.Http(port, host).plan(plan).start()
  }

  def plan() = unfiltered.netty.cycle.Planify {
    case Path(Seg("hello" :: Nil)) => ResponseString("hello, world")

    case req@Path(Seg("api" :: "dimmer" :: Nil)) => req match {
      case GET(_) => ResponseString("%02x".format(dimmer.dimness))
      case POST(_) => Body.string(req) match {
        case "on" =>
          dimmer.dimToBrightest()
          ResponseString("dimmed to 100%")
        case "off" =>
          dimmer.dimToDarkest()
          ResponseString("dimmed to 0%")
        case HexAlpha(dimness8Bit) =>
          dimmer.dimTo(dimness8Bit << 8 | dimness8Bit)
          ResponseString("dimmed to ${dimness8Bit / 255}%")
        case _ => BadRequest ~> ResponseString("bad request")
      }
      case _ => MethodNotAllowed ~> ResponseString("method not allowed")
    }

    case req@Path(Seg("api" :: "backlight" :: Nil)) => req match {
      case GET(_) =>
        ResponseString(backlight.color.asRgbHexString)
      case POST(_) => Body.string(req) match {
        case HexRgbColor(color) =>
          backlight.setColor(color)
          ResponseString(s"ok, set backlight to ${color}")
        case _ => BadRequest ~> ResponseString("bad request")
      }
      case _ => MethodNotAllowed ~> ResponseString("method not allowed")
    }

    case _ => NotFound ~> ResponseString("not found")
  }
}

object Webserver {
  val rgbHexRegex = "([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})".r
  val HexRgbColor = new {
    def unapply(value: String): Option[Color] = value match {
      case rgbHexRegex(r, g, b) => Some(RgbColor.from8BitRgb(Integer.valueOf(r, 16), Integer.valueOf(g, 16), Integer.valueOf(b, 16)))
      case _ => None
    }
  }

  val alphaHexRegex = "([0-9a-fA-F]{2})".r
  val HexAlpha = new {
    def unapply(value: String): Option[Int] = value match {
      case alphaHexRegex(hex) => Some(Integer.valueOf(hex, 16))
      case _ => None
    }
  }
}