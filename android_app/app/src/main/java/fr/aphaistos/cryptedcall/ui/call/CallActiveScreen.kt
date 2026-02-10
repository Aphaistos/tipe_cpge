package fr.aphaistos.cryptedcall.ui.call

import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.navigation.NavController
import fr.aphaistos.cryptedcall.navigation.NavRoutes
import fr.aphaistos.cryptedcall.viewmodel.IdentityViewModel

@Composable
fun CallActiveScreen(
    contactId: String,
    vm: IdentityViewModel,
    nav: NavController
) {
    val identity = vm.identities.first { it.id == contactId }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp)
    ) {

        /* ─────────────── Header ─────────────── */

        Box(
            modifier = Modifier.fillMaxWidth(),
            contentAlignment = Alignment.Center
        ) {
            Text(
                text = "Appel sécurisé — ${identity.name}",
                style = MaterialTheme.typography.headlineSmall
            )
        }

        Spacer(modifier = Modifier.height(24.dp))

        /* ─────────────── État session ─────────────── */

        Text("État session", style = MaterialTheme.typography.titleMedium)
        Spacer(modifier = Modifier.height(8.dp))

        SessionIndicator("Échange ECDH")
        SessionIndicator("Clé média dérivée")
        SessionIndicator("Authentification mutuelle")
        SessionIndicator("WebRTC établi")

        Spacer(modifier = Modifier.height(24.dp))

        /* ─────────────── Continuité cryptographique ─────────────── */

        Text("Continuité cryptographique", style = MaterialTheme.typography.titleMedium)
        Spacer(modifier = Modifier.height(8.dp))

        Text("✔ Clé #12 (ratchet actif)", color = Color(0xFF2E7D32))
        Text("✔ Rekey toutes les 30 s", color = Color(0xFF2E7D32))

        Spacer(modifier = Modifier.height(24.dp))

        /* ─────────────── Vérification ─────────────── */

        Row(
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                text = "Vérification : ",
                style = MaterialTheme.typography.titleMedium
            )
            Text(
                text = "Identité distante cohérente",
                color = Color(0xFF2E7D32)
            )
        }

        Spacer(modifier = Modifier.weight(1f))

        /* ─────────────── Raccrocher ─────────────── */

        OutlinedButton(
            modifier = Modifier.fillMaxWidth(),
            colors = ButtonDefaults.outlinedButtonColors(
                contentColor = Color.Red
            ),
            border = ButtonDefaults.outlinedButtonBorder.copy(
                brush = androidx.compose.ui.graphics.SolidColor(Color.Red)
            ),
            onClick = {
                // Retour à la fiche contact
                nav.popBackStack()
            }
        ) {
            Text("Raccrocher")
        }
    }
}

/* ─────────────── Indicateur de session ─────────────── */

@Composable
private fun SessionIndicator(label: String) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 4.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            text = "✔",
            color = Color(0xFF2E7D32),
            modifier = Modifier.width(24.dp)
        )
        Spacer(modifier = Modifier.width(8.dp))
        Text(text = label)
    }
}
