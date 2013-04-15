package de.entropia.vapor.hardware

import java.io.{BufferedOutputStream, OutputStream}
import purejavacomm.{SerialPort, CommPortIdentifier}
import java.net.{Socket, InetAddress}
import de.entropia.vapor.daemon.config.{NetworkDeviceSettings, SerialDeviceSettings, Settings}


/**
 * Creates `OutputStream`s.
 */
object Physical {

  def openSerialPort(devicePath: String): OutputStream = {
    val portId = CommPortIdentifier.getPortIdentifier(devicePath)
    if (portId.getPortType != CommPortIdentifier.PORT_SERIAL) ???
    val port = portId.open("vaporlight", 3000).asInstanceOf[SerialPort]
    port.setSerialPortParams(500000, SerialPort.DATABITS_8, SerialPort.STOPBITS_1, SerialPort.PARITY_NONE)
    port.getOutputStream
  }

  def openSocket(host: String, port: Integer): OutputStream = {
    val socket = new Socket(InetAddress.getByName(host), port)
    new BufferedOutputStream(socket.getOutputStream);
  }

  def open(settings: Settings) = settings.device match {
    case SerialDeviceSettings(interface) => openSerialPort(interface)
    case NetworkDeviceSettings(host, port) => openSocket(host, port)
  }
}
