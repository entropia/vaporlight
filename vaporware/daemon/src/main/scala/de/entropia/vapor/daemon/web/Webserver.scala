package de.entropia.vapor.daemon.web

import unfiltered.response._
import de.entropia.vapor.daemon.config.Settings
import unfiltered.request._
import unfiltered.response.ResponseString
import de.entropia.vapor.daemon.service.{Dimmer, Backlight}
import de.entropia.vapor.util.{Color, RgbColor}
import de.entropia.vapor.daemon.web.Webserver._
import de.entropia.vapor.daemon.server.ServerStatus
import com.fasterxml.jackson.databind.ObjectMapper
import com.fasterxml.jackson.module.scala.DefaultScalaModule
import scala.util.{Success, Try}
import java.io.ByteArrayOutputStream

class Webserver(val settings: Settings, val dimmer: Dimmer, val backlight: Backlight, val serverStatus: ServerStatus) {
  val docUrl = "https://github.com/entropia/vaporlight/blob/master/HACKING"
  val docHtml =
  <body>
    <p>Welcome! You've reached the Vaporlight Daemons web interface.</p>
    <p>Documentation: <a href={docUrl}>{docUrl}</a></p>
  </body>

  val mapper = new ObjectMapper()
  mapper.registerModule(DefaultScalaModule)

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
    case Path(Seg(Nil)) => Html(docHtml)
    case Path(Seg("api" :: Nil)) => Html(docHtml)

    case Path(Seg("hello" :: Nil)) => ResponseString("hello, world")

    case req@Path(Seg("api" :: "dimmer" :: Nil)) => req match {
      case GET(_) & Params(p) =>
        makeUnsafe(ResponseString(supportJsonP(p, "%04x".format(dimmer.dimness))))
      case POST(_) => Body.string(req) match {
        case "on" =>
          dimmer.dimToBrightest()
          makeUnsafe(ResponseString("dimmed to 100%"))
        case "off" =>
          dimmer.dimToDarkest()
          makeUnsafe(ResponseString("dimmed to 0%"))
        case HexAlpha(dimness16Bit) =>
          dimmer.dimTo(dimness16Bit)
          makeUnsafe(ResponseString(f"dimmed to ${dimness16Bit / 655.350}%.1f%%"))
        case _ => BadRequest ~> ResponseString("bad request")
      }
      case _ => MethodNotAllowed ~> ResponseString("method not allowed")
    }

    case req@Path(Seg("api" :: "backlight" :: Nil)) => req match {
      case GET(_) & Params(p) =>
        makeUnsafe(ResponseString(supportJsonP(p, backlight.color.asRgbHexString)))
      case POST(_) => Body.string(req) match {
        case HexRgbColor(color) =>
          backlight.setColor(color)
          makeUnsafe(ResponseString(s"ok, set backlight to ${color}"))
        case _ => BadRequest ~> ResponseString("bad request")
      }
      case _ => MethodNotAllowed ~> ResponseString("method not allowed")
    }

    case req@Path(Seg("api" :: "clients" :: Nil)) => req match {
      case GET(_) =>
        val response = new ByteArrayOutputStream()
        for ((id, client) <- serverStatus.getClients) {
          mapper.writeValue(response, Map(
            "id" -> id,
            "local-host" -> client.local.getAddress.getHostAddress,
            "local-port" -> client.local.getPort,
            "remote-host" -> client.remote.getAddress.getHostAddress,
            "remote-port" -> client.remote.getPort,
            "token" -> client.token.map(_.shortId).getOrElse(null),
            "priority" -> client.token.map(_.priority).getOrElse(null),
            "persistent" -> client.token.map(_.persistent).getOrElse(null)
          ))
          response.write('\n')
        }
        makeUnsafe(ResponseString(response.toString))
      case _ => MethodNotAllowed ~> ResponseString("method not allowed")
    }

    case req@Path(Seg("api" :: "clients" :: id :: Nil)) => req match {
      case POST(_) => Body.string(req) match {
        case "kill" =>
          Try(serverStatus.getClients(Integer.parseInt(id))) match {
            case Success(client) =>
              client.kill()
              makeUnsafe(ResponseString(s"ok, killed client $id"))
            case _ =>
              BadRequest ~> ResponseString("bad request")
          }
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

  val alphaHexRegex = "([0-9a-fA-F]{4})".r
  val HexAlpha = new {
    def unapply(value: String): Option[Int] = value match {
      case alphaHexRegex(hex) => Some(Integer.valueOf(hex, 16))
      case _ => None
    }
  }

  def makeUnsafe(rsp: ResponseString) = // allow javascript calls to read the response even if violating the same-origin policy
    new ResponseHeader("Access-Control-Allow-Origin", Set("*")) ~> rsp

  def supportJsonP(params: Map[String, Seq[String]], responseText: String): String =
    if (params.contains("jsonp"))
      "%s(\"%s\")".format(params("jsonp")(0), responseText)
    else
      responseText
}