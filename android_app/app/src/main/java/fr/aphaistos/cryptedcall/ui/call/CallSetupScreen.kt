package fr.aphaistos.cryptedcall.ui.call

import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.navigation.NavController
import kotlinx.coroutines.delay
import androidx.compose.ui.Alignment

@Composable
fun CallSetupScreen(
    contactId: String,
    nav: NavController
) {

    // États d’avancement simulés
    var ecdhDone by remember { mutableStateOf(false) }
    var mediaKeyDone by remember { mutableStateOf(false) }
    var authDone by remember { mutableStateOf(false) }

    // Lancement de la "séquence crypto"
    LaunchedEffect(Unit) {
        delay(2000)
        ecdhDone = true

        delay(2000)
        mediaKeyDone = true

        delay(2000)
        authDone = true

        // Tout est prêt → écran d’appel actif
        nav.navigate("call_active/$contactId") {
            popUpTo("call_setup/$contactId") { inclusive = true }
        }
    }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp)
    ) {

        /* ─────────────── Titre ─────────────── */

        Text(
            text = "Établissement de l’appel sécurisé",
            style = MaterialTheme.typography.headlineSmall
        )

        Spacer(modifier = Modifier.height(24.dp))

        /* ─────────────── Étapes crypto ─────────────── */

        CryptoStep(
            label = "Échange ECDH",
            done = ecdhDone
        )

        CryptoStep(
            label = "Clé média dérivée",
            done = mediaKeyDone
        )

        CryptoStep(
            label = "Authentification mutuelle",
            done = authDone
        )

        Spacer(modifier = Modifier.weight(1f))

        /* ─────────────── Action d’arrêt ─────────────── */

        OutlinedButton(
            modifier = Modifier.fillMaxWidth(),
            colors = ButtonDefaults.outlinedButtonColors(
                contentColor = Color.Red
            ),
            border = ButtonDefaults.outlinedButtonBorder.copy(
                brush = androidx.compose.ui.graphics.SolidColor(Color.Red)
            ),
            onClick = {
                // Arrêt immédiat → retour fiche contact
                nav.popBackStack()
            }
        ) {
            Text("Arrêter")
        }
    }
}

/* ─────────────── Composant étape crypto ─────────────── */

@Composable
private fun CryptoStep(
    label: String,
    done: Boolean
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 8.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {

        Text(
            text = if (done) "✔" else "⌛",
            color = if (done) Color(0xFF2E7D32) else Color.Gray,
            modifier = Modifier.width(24.dp)
        )

        Spacer(modifier = Modifier.width(8.dp))

        Text(
            text = label,
            style = MaterialTheme.typography.bodyLarge
        )
    }
}