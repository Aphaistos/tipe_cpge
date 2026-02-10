package fr.aphaistos.cryptedcall.ui.identity

import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context.CLIPBOARD_SERVICE
import android.widget.Toast
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.navigation.NavController
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import fr.aphaistos.cryptedcall.model.KeyObservationState
import fr.aphaistos.cryptedcall.viewmodel.IdentityViewModel

@Composable
fun ContactDetailScreen(
    id: String,
    vm: IdentityViewModel,
    nav: NavController
) {
    var shouldNavigateBack by remember { mutableStateOf(false) }

    val identity = vm.identities.firstOrNull { it.id == id }

    // 🔒 Si le contact n’existe plus → déclencher navigation via state
    if (identity == null) {
        LaunchedEffect(shouldNavigateBack) {
            if (!shouldNavigateBack) return@LaunchedEffect
            nav.popBackStack()
        }
        // Afficher un composable neutre pendant la frame
        Box(modifier = Modifier.fillMaxSize())
        return
    }

    Column(modifier = Modifier.padding(16.dp)) {

        /* ─────────────── Header ─────────────── */

        Box(
            modifier = Modifier.fillMaxWidth(),
            contentAlignment = Alignment.Center
        ) {
            IconButton(
                modifier = Modifier.align(Alignment.CenterStart),
                onClick = { nav.popBackStack() }
            ) {
                Icon(
                    imageVector = Icons.Default.ArrowBack,
                    contentDescription = "Retour"
                )
            }

            Text(
                text = identity.name,
                style = MaterialTheme.typography.headlineSmall
            )
        }

        Spacer(modifier = Modifier.height(24.dp))

        /* ─────────────── Empreinte ─────────────── */

        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text("Empreinte", style = MaterialTheme.typography.titleMedium)
            TextButton(onClick = {
                val clipboardManager = nav.context.getSystemService(CLIPBOARD_SERVICE) as ClipboardManager
                clipboardManager.setPrimaryClip(ClipData.newPlainText   ("", identity.fingerprint))
                Toast.makeText(nav.context, "Empreinte copiée", Toast.LENGTH_SHORT).show()
            }) {
                Text("Copier")
            }
        }

        Text(
            text = identity.fingerprint,
            style = MaterialTheme.typography.bodyMedium
        )

        Spacer(modifier = Modifier.height(24.dp))

        /* ─────────────── État de vérification ─────────────── */

        Text("État de vérification", style = MaterialTheme.typography.titleMedium)

        Text(
            text = if (identity.verified) "✔ Vérifiée" else "✖ Non vérifiée",
            color = if (identity.verified) Color(0xFF2E7D32) else Color.Red
        )

        Spacer(modifier = Modifier.height(24.dp))

        /* ─────────────── Dernière clé observée ─────────────── */

        Text("Dernière clé observée", style = MaterialTheme.typography.titleMedium)
        val keyText = when (identity.keyState) {
            KeyObservationState.NOT_AVAILABLE ->
                "Non disponible — identité non vérifiée"
            KeyObservationState.VERIFYING ->
                "Vérification en cours…"
            KeyObservationState.STABLE ->
                "Stable (aucun changement)"
            KeyObservationState.CHANGED ->
                "⚠ Non vérifiée Changement détecté"
        }
        Text(
            keyText,
            color = when (identity.keyState) {
                KeyObservationState.STABLE -> Color(0xFF2E7D32)
                KeyObservationState.CHANGED -> MaterialTheme.colorScheme.error
                else -> MaterialTheme.colorScheme.onSurfaceVariant
            }
        )
        Spacer(modifier = Modifier.height(32.dp))

        /* ─────────────── Actions ─────────────── */

        Button(
            modifier = Modifier.fillMaxWidth(),
            enabled = identity.verified,
            onClick = {
                nav.navigate("call_setup/${identity.id}")
            }
        ) {
            Text("Appeler")
        }

        Spacer(modifier = Modifier.height(12.dp))


        OutlinedButton(
            modifier = Modifier.fillMaxWidth(),
            enabled = !identity.verified && identity.keyState != KeyObservationState.VERIFYING,
            onClick = {
                vm.verifyContact(identity.id)
            }
        ) {
            Text("Marquer comme vérifié")
        }

        Spacer(modifier = Modifier.height(12.dp))

        OutlinedButton(
            modifier = Modifier.fillMaxWidth(),
            colors = ButtonDefaults.outlinedButtonColors(
                contentColor = Color.Red
            ),
            border = ButtonDefaults.outlinedButtonBorder.copy(
                brush = androidx.compose.ui.graphics.SolidColor(Color.Red)
            ),
            onClick = {
                vm.identities.remove(identity)
                nav.navigateUp()
            }
        ) {
            Text("Supprimer")
        }
    }
}