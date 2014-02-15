package de.entropia.vapor.daemon.web

import de.entropia.vapor.util.Color
import scala.concurrent._
import java.io.IOException
import com.typesafe.scalalogging.slf4j.Logging
import java.util.concurrent.Executors

abstract class Notifications[T](val targetUrls: Seq[String]) extends Logging {
  implicit val ec = ExecutionContext.fromExecutor(Executors.newScheduledThreadPool(5))

  def execute(param: T): Unit

  protected def execute(params: Map[String, String]) {
    val paramStr = "?" + params.toList.map(item => s"${item._1}=${item._2}").mkString("&")
    for (url <- targetUrls) future {
      val fullUrl = url + paramStr
      try {
        val connection = new java.net.URL(fullUrl).openConnection()
        connection.setAllowUserInteraction(false)
        connection.setConnectTimeout(1000)
        connection.setReadTimeout(1000)
        connection.setUseCaches(false)
        connection.setDoInput(true)
        connection.setDoOutput(false)
        connection.connect()
        connection.getInputStream().close() // actual request happens here. yes.
      } catch {
        case e: IOException =>
          logger.info(s"failed to notify $fullUrl", e)
      }
    }
  }
}

class BacklightNotifications(targetUrls: Seq[String]) extends Notifications[Color](targetUrls) {

  def execute(param: Color) =
    execute(Map("color" -> param.asRrGgBbHexString))
}

class DimmerNotifications(targetUrls: Seq[String]) extends Notifications[Int](targetUrls) {

  def execute(param: Int) =
    execute(Map("dimmer" -> param.toString))
}
