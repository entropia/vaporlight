package de.entropia.vapor.daemon.server

import scala.collection.mutable
import de.entropia.vapor.server.Client

class ServerStatus {
  private val clients = mutable.Map[Int, Client]()

  def addClient(client: Client) = synchronized {
    clients(client.id) = client
  }

  def removeClient(client: Client) = synchronized {
    clients.remove(client.id)
  }

  def getClients = synchronized {
    clients.toMap
  }
}
