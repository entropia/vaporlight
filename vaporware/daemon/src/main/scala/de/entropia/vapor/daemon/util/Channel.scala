package de.entropia.vapor.daemon.util

import de.entropia.vapor.util.Color

sealed abstract class RgbChannel(val extract: Color => Int)
case object RedChannel extends RgbChannel(_.r)
case object GreenChannel extends RgbChannel(_.g)
case object BlueChannel extends RgbChannel(_.b)
