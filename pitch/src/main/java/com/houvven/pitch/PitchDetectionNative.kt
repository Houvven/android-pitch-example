package com.houvven.pitch

import kotlinx.coroutines.*

class PitchDetectionNative {

    interface PitchCallback {
        fun onPitchDetected(pitch: Float)
    }

    external fun startPitchDetection(callback: PitchCallback): Boolean
    external fun stopPitchDetection(): Boolean
    external fun isRunning(): Boolean

    companion object {
        init {
            System.loadLibrary("pitch")
        }
    }

    fun startPitchDetectionAsync(callback: PitchCallback): Job {
        return CoroutineScope(Dispatchers.Default).launch {
            startPitchDetection(callback)
        }
    }
}