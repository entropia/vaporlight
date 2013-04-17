package de.entropia.vapor.daemon.mixer

import de.entropia.vapor.mixer.Mixer
import de.entropia.vapor.daemon.config.{Token, Settings}

class Manager(val mixer: Mixer, val settings: Settings) {

  val persistentOverlays =
    for ((tokenId, token) <- settings.tokens if token.persistent)
      yield (tokenId, mixer.register(token.priority, true))

  def getOverlay(token: Token) =
    persistentOverlays.getOrElse(token.id, mixer.register(token.priority, false))
}
