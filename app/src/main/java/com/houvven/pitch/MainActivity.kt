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

        Row(horizontalArrangement = Arrangement.Center) {
            Button(onClick = {
                pitchDetection.startPitchDetectionAsync(object : PitchDetectionNative.PitchCallback {
                    override fun onPitchDetected(pitch: Float) {
                        currentPitch = pitch
                    }
                })
            }) {
                Text(text = "Run")
            }

            Spacer(Modifier.width(8.dp))

            Button(onClick = { pitchDetection.stopPitchDetection() }) {
                Text(text = "Stop")
            }
        }
    }
}
