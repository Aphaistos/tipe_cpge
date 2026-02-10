package fr.aphaistos.cryptedcall.ui.identity

import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context.CLIPBOARD_SERVICE
import android.os.Build
import android.widget.Toast
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.annotation.RequiresApi
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.selection.SelectionContainer
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.unit.dp
import androidx.navigation.NavController
import fr.aphaistos.cryptedcall.viewmodel.IdentityViewModel

@RequiresApi(Build.VERSION_CODES.O)
@Composable
fun MyIdentityScreen(
    vm: IdentityViewModel,
    nav: NavController
) {

    val self = vm.identities.first { it.isSelf }
    var showPublicKey by remember { mutableStateOf(false) }

    val context = LocalContext.current
    val exportLauncher = rememberLauncherForActivityResult(
        ActivityResultContracts.CreateDocument("text/plain")
    ) { uri ->
        if (uri != null) {
            context.contentResolver.openOutputStream(uri)?.use {
                it.write(self.publicKey.toByteArray())
            }
        }
    }


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
                text = "Mon identité",
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
                clipboardManager.setPrimaryClip(ClipData.newPlainText   ("", self.fingerprint))
                Toast.makeText(nav.context, "Empreinte copiée", Toast.LENGTH_SHORT).show()
            }) {
                Text("Copier")
            }
        }

        Text(
            text = self.fingerprint,
            style = MaterialTheme.typography.bodyMedium
        )

        Spacer(modifier = Modifier.height(24.dp))

        /* ─────────────── Clé publique ─────────────── */

        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text("Clé publique", style = MaterialTheme.typography.titleMedium)

            Row {
                TextButton(onClick = { showPublicKey = true }) {
                    Text("Afficher")
                }
                Spacer(modifier = Modifier.width(8.dp))
                TextButton(onClick = {
                    exportLauncher.launch("public_key.txt")
                }) {
                    Text("Exporter")
                }
            }
        }

        Spacer(modifier = Modifier.height(24.dp))

        /* ─────────────── Date génération ─────────────── */

        Text("Identité générée le", style = MaterialTheme.typography.titleMedium)
        Text(self.generatedAt)

        Spacer(modifier = Modifier.weight(1f))

        /* ─────────────── Action destructive ─────────────── */

        OutlinedButton(
            modifier = Modifier.fillMaxWidth(),
            colors = ButtonDefaults.outlinedButtonColors(
                contentColor = Color.Red
            ),
            border = ButtonDefaults.outlinedButtonBorder.copy(
                brush = androidx.compose.ui.graphics.SolidColor(Color.Red)
            ),
            onClick = {
                vm.regenerateIdentity()
            }
        ) {
            Text("Régénérer l’identité")
        }
    }

    if (showPublicKey) {
        AlertDialog(
            onDismissRequest = { showPublicKey = false },
            confirmButton = {
                TextButton(onClick = { showPublicKey = false }) {
                    Text("Fermer")
                }
            },
            title = { Text("Clé publique") },
            text = {
                Box(
                    Modifier
                        .fillMaxWidth()
                        .heightIn(max = 300.dp) // limite hauteur dialog
                        .background(
                            MaterialTheme.colorScheme.surfaceVariant,
                            RoundedCornerShape(8.dp)
                        )
                        .padding(12.dp)
                ) {
                    SelectionContainer {
                        Text(
                            text = self.publicKey,
                            fontFamily = FontFamily.Monospace,
                            modifier = Modifier.verticalScroll(
                                rememberScrollState()
                            )
                        )
                    }
                }
            }
        )
    }
}