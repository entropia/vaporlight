package de.entropia.vapor.daemon.config

import com.typesafe.config.{Config, ConfigFactory}
import RichConfig.Config2RichConfig
import Token.TokenId
import de.entropia.vapor.util.{Blue, Green, Red, RgbChannel}
import scala.collection.mutable
import collection.JavaConversions._
import collection.JavaConverters._

class Settings(config: Config) {
  val device = config.getDeviceSettings("hardware.device")
  val tokens = config.getTokens("mixer.tokens")
  val channels = config.getChannels("mixer.channels")
  val channelCounts = config.getChannelCounts("hardware.channels")
}

object Settings {
  def load() = {
    new Settings(ConfigFactory.load())
  }
}

abstract sealed class DeviceSettings()

final case class SerialDeviceSettings(val interface: String) extends DeviceSettings

final case class NetworkDeviceSettings(val host: String, val port: Int) extends DeviceSettings

class RichConfig(val config: Config) {

  def getDeviceSettings(baseKey: String): DeviceSettings = {
    val subconf = config.getConfig(baseKey)
    val deviceType = subconf.getString("type")
    deviceType match {
      case "serial" => SerialDeviceSettings(subconf.getString("interface"))
      case "network" => NetworkDeviceSettings(subconf.getString("host"), subconf.getInt("port"))
      case _ => throw new IllegalArgumentException("invalid value: " + subconf.origin())
    }
  }

  def getTokens(key: String): Map[TokenId, Token] = {
    getMap(key)(_.getBytes.toList, (key, properties) => {
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
        result.put((hiChannel, Red), (loChannels(0)(0).toByte, loChannels(0)(1)))
      if (!loChannels(1).isEmpty)
        result.put((hiChannel, Green), (loChannels(1)(0).toByte, loChannels(1)(1)))
      if (!loChannels(2).isEmpty)
        result.put((hiChannel, Blue), (loChannels(2)(0).toByte, loChannels(2)(1)))
    }
    result.toMap
  }

  def getChannelCounts(key: String): Map[Byte, Int] =
    getMap(key)(_.toByte, (k, v) => v.asInstanceOf[Int])

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
}

object Token {
  type TokenId = List[Byte]

  implicit def Seq2TokenId(bytes: Seq[Byte]) = bytes.toList
}
