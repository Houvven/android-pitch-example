package com.houvven.pitch

import android.Manifest
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.google.accompanist.permissions.ExperimentalPermissionsApi
import com.google.accompanist.permissions.isGranted
import com.google.accompanist.permissions.rememberPermissionState
import java.lang.Thread.sleep
import kotlin.concurrent.thread

class MainActivity : ComponentActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            MaterialTheme {
                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    HomeScreen(
                        modifier = Modifier
                            .padding(innerPadding)
                            .fillMaxSize()
                    )
                }
            }
        }
    }
}

@OptIn(ExperimentalPermissionsApi::class)
@Composable
private fun HomeScreen(
    modifier: Modifier = Modifier
) {
    val pitchDetection = PitchDetectionNative()
    val permissionState = rememberPermissionState(permission = Manifest.permission.RECORD_AUDIO)
    var currentPitch by remember { mutableFloatStateOf(0f) }
    val animatedPitch by animateFloatAsState(targetValue = currentPitch, label = "Pitch")
    var isRunning by remember { mutableStateOf(false) }

    Column(
        modifier = modifier,
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ) {
        if (!permissionState.status.isGranted) {
            Button(onClick = {
                permissionState.launchPermissionRequest()
            }) {
                Text(text = "Request Permission")
            }
            return@Column
        }

        Text(
            text = String.format("%.3f", animatedPitch),
            style = MaterialTheme.typography.displayLarge
        )

        Spacer(Modifier.height(32.dp))
        Button(onClick = {
            if (isRunning) {
                pitchDetection.stopPitchDetection()
                isRunning = pitchDetection.isRunning()
            } else {
                pitchDetection.startPitchDetectionAsync(
                    object : PitchDetectionNative.PitchCallback {
                        override fun onPitchDetected(pitch: Float) {
                            currentPitch = pitch
                        }
                    }
                )
                thread {
                    repeat(5) {
                        isRunning = pitchDetection.isRunning()
                        if (isRunning == true) return@repeat
                        sleep(500)
                    }
                }
            }
        }) {
            val displayText = if (!isRunning) "Run" else "Stop"
            Text(text = displayText)
        }
    }
}
