package fr.aphaistos.cryptedcall.ui.call

import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.navigation.NavController
import fr.aphaistos.cryptedcall.navigation.NavRoutes

@Composable
fun CallIncidentScreen(nav: NavController) {

    Column(Modifier.padding(16.dp)) {
        Text("🔴 INCIDENT DE SÉCURITÉ", color = Color.Red)
        Text("Changement de clé détecté")

        Spacer(Modifier.height(24.dp))

        Button(onClick = {
            nav.navigate(NavRoutes.IdentityList.route) {
                popUpTo(0)
            }
        }) {
            Text("RACCROCHER IMMÉDIATEMENT")
        }
    }
}
