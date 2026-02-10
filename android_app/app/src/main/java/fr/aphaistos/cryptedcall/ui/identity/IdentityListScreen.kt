package fr.aphaistos.cryptedcall.ui.identity

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.*
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.navigation.NavController
import fr.aphaistos.cryptedcall.navigation.NavRoutes
import fr.aphaistos.cryptedcall.viewmodel.IdentityViewModel

@Composable
fun IdentityListScreen(
    vm: IdentityViewModel,
    nav: NavController
) {
    LazyColumn {
        items(vm.identities) { identity ->
            val bgColor = if (identity.isSelf) Color(0xFFE3F2FD) else Color.Transparent

            Card(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(8.dp)
                    .background(bgColor)
                    .clickable {
                        if (identity.isSelf)
                            nav.navigate("my_identity")
                        else
                            nav.navigate("contact/${identity.id}")
                    }
            ) {
                Column(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(16.dp)
                ) {

                    /* ─────────────── Nom + État ─────────────── */
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceBetween
                    ) {
                        Text(
                            text = identity.name,
                            fontWeight = FontWeight.Bold,
                            style = MaterialTheme.typography.bodyLarge
                        )

                        if (!identity.isSelf) {
                            val stateText = when {
                                identity.verified -> "✔ Vérifiée"
                                else -> "⚠ Non vérifiée"
                            }

                            Text(
                                text = stateText,
                                style = MaterialTheme.typography.bodyMedium,
                                color = if (identity.verified) Color(0xFF2E7D32) else Color.Red
                            )
                        }
                    }

                    Spacer(modifier = Modifier.height(4.dp))

                    /* ─────────────── Empreinte ─────────────── */
                    Text(
                        text = identity.fingerprint,
                        fontSize = 12.sp,
                        color = Color.Gray
                    )
                }
            }
        }
    }
}