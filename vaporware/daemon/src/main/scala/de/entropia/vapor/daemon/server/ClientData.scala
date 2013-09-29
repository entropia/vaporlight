package de.entropia.vapor.daemon.server

import java.net.InetSocketAddress

case class ClientData(val id: Int, val local: InetSocketAddress, val remote: InetSocketAddress)