package fr.aphaistos.cryptedcall.ui.bootstrap

import androidx.compose.runtime.*
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.delay

@Composable
fun BootstrapScreen(onDone: () -> Unit) {

    LaunchedEffect(Unit) {
        delay(1500)
        onDone()
    }

    Box(
        modifier = Modifier.fillMaxSize(),
        contentAlignment = Alignment.Center
    ) {
        Column(horizontalAlignment = Alignment.CenterHorizontally) {
            Text("Initialisation de l’identité cryptographique")
            Spacer(Modifier.height(16.dp))
            CircularProgressIndicator()
        }
    }
}
