package de.entropia.vapor.mixer

import java.util.concurrent.locks.ReentrantLock


/**
 * A better `sleep`/`notify`-like mechanism.
 *
 * <ul>
 * <li>Wakups are rate-limited.
 * <li>Multiple notifications before a wakeup will only cause a single wakeup.
 * <li>Wakeups are guaranteed to be non-spurious.
 * </ul>
 *
 * Only one thread should call `awaitRun`, but any number of threads may call `requestRun`.
 */
class Delay(val minSleepMillis: Long) {
  val lock = new ReentrantLock()
  val runRequested = lock.newCondition()
  var realWakeup: Boolean = false // to detect spurious wakeups

  def awaitRun() {
    Thread.sleep(minSleepMillis)
    lock.lock()
    try {
      while (!realWakeup) {
        runRequested.awaitUninterruptibly()
      }
      realWakeup = false
    } finally {
      lock.unlock()
    }
  }

  def requestRun() {
    lock.lock()
    try {
      realWakeup = true
      runRequested.signal()
    } finally {
      lock.unlock()
    }
  }
}
