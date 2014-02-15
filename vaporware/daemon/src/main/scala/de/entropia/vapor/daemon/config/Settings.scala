package de.entropia.vapor.daemon.config

import com.typesafe.config.{Config, ConfigFactory}
import RichConfig.Config2RichConfig
import Token.TokenId
import scala.collection.mutable
import collection.JavaConversions._
import collection.JavaConverters._
import de.entropia.vapor.daemon.util.{BlueChannel, GreenChannel, RedChannel, RgbChannel}

class Settings(config: Config) {
  val device = config.getDeviceSettings("hardware.device")
  val tokens = config.getTokens("mixer.tokens")
  val channels = config.getChannels("mixer.channels")
  val leds = channels.keySet.map(_._1)
  val channelCounts = config.getChannelCounts("hardware.channels")
  val lowlevelServerInterface = config.getServerSettings("server.lowlevel")
  val webServerInterface = config.getServerSettings("server.web")
  val notifications = config.getNotificationSettings("notifications")
}

object Settings {
  def load() = {
    new Settings(ConfigFactory.load())
  }
}

abstract sealed class DeviceSettings()

final case class SerialDeviceSettings(val interface: String, val baudrate: Int) extends DeviceSettings

final case class NetworkDeviceSettings(val host: String, val port: Int) extends DeviceSettings

final case class FileDeviceSettings(val path: String) extends DeviceSettings

final case class Notifications(val backlight: Seq[String], val dimmer: Seq[String])

class RichConfig(val config: Config) {

  def getServerSettings(baseKey: String): Option[(String, Int)] = {
    if (config.hasPath(baseKey)) {
      val subconf = config.getConfig(baseKey)
      Some((subconf.getString("interface"), subconf.getInt("port")))
    } else {
      None
    }
  }

  def getNotificationSettings(baseKey: String): Notifications = {
    val subconf = config.getConfig("notifications")
    Notifications(
      subconf.getStringList("backlight"),
      subconf.getStringList("dimmer"))
  }

  def getDeviceSettings(baseKey: String): DeviceSettings = {
    val subconf = config.getConfig(baseKey)
    val deviceType = subconf.getString("type")
    deviceType match {
      case "serial" => SerialDeviceSettings(subconf.getString("interface"), subconf.getInt("baudrate"))
      case "network" => NetworkDeviceSettings(subconf.getString("host"), subconf.getInt("port"))
      case "file" => FileDeviceSettings(subconf.getString("path"))
      case _ => throw new IllegalArgumentException("invalid value: " + subconf.origin())
    }
  }

  def getTokens(key: String): Map[TokenId, Token] = {
    getMap(key)(_.getBytes.toList.padTo(16, 0x00.toByte), (key, properties) => {
      val propertyMap = properties.asInstanceOf[java.util.Map[String, Object]].toMap
      new Token(key,
        propertyMap.get("priority").asInstanceOf[Option[Int]].get,
        propertyMap.getOrElse("persistent", false).asInstanceOf[Boolean])
    })
  }

  def getChannels(key: String): Map[(Int, RgbChannel), (Byte, Int)] = {
    val result = mutable.Map[(Int, RgbChannel), (Byte, Int)]()
    for ((hiChannel, loChannels) <- getMap[Int, java.util.List[java.util.List[Int]]](key)(_.toInt, (k, v) => v.asInstanceOf[java.util.List[java.util.List[Int]]])) {
      if (!loChannels(0).isEmpty)
        result.put((hiChannel, RedChannel), (loChannels(0)(0).toByte, loChannels(0)(1)))
      if (!loChannels(1).isEmpty)
        result.put((hiChannel, GreenChannel), (loChannels(1)(0).toByte, loChannels(1)(1)))
      if (!loChannels(2).isEmpty)
        result.put((hiChannel, BlueChannel), (loChannels(2)(0).toByte, loChannels(2)(1)))
    }
    result.toMap
  }

  def getChannelCounts(key: String): Map[Byte, Int] =
    getMap(key)(_.toInt.toByte, (k, v) => v.asInstanceOf[Int]) // .toInt.toByte necessary for module ids > 127

  private def getMap[outK, outV](configKey: String)(keyFunc: (String) => outK, valueFunc: (outK, Any) => outV): Map[outK, outV] = {
    val map = config.getObject(configKey).unwrapped().asInstanceOf[java.util.Map[String, Any]]
    for ((nameInConfig, valueInConfig) <- map.asScala.toMap) yield {
      val key = keyFunc(nameInConfig)
      val value = valueFunc(key, valueInConfig)
      (key, value)
    }
  }
}

object RichConfig {
  implicit def Config2RichConfig(config: Config): RichConfig = new RichConfig(config)
}

case class Token(val id: List[Byte], val priority: Int, val persistent: Boolean) {

  def shortId =
    new String(id.toArray).reverse.dropWhile(_ == '\0').reverse
}

object Token {
  type TokenId = List[Byte]

  implicit def Seq2TokenId(bytes: Seq[Byte]) = bytes.toList
}
