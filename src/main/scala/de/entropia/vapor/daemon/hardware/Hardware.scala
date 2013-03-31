package de.entropia.vapor.hardware

import de.entropia.vapor.util._
import java.io.OutputStream
import grizzled.slf4j.Logging
import de.entropia.vapor.config.Config


/**
 * Facade for the rest of the package.
 */
class Hardware(val mapping: Mapping) extends Logging {

  /** Update a LED. Won't take effect until `strobe()` is called. */
  def set(led: Int, color: Color) = {
    info("set(%d, %s)".format(led, color))
    mapping.set(led, color)
  }

  /** Make previous LED state updates take effect. */
  def strobe() =
    mapping.strobe()
}


object Hardware {

  def apply(config: Config, rawOutputStream: OutputStream) =
    new Hardware(new Mapping(config, new Buffer(config, new Encoder(new Framer(rawOutputStream)))))
}
