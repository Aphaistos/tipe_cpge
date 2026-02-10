package fr.aphaistos.cryptedcall

import android.os.Build
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.annotation.RequiresApi
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.systemBarsPadding
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.lifecycle.viewmodel.compose.viewModel
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import fr.aphaistos.cryptedcall.navigation.NavRoutes
import fr.aphaistos.cryptedcall.ui.bootstrap.BootstrapScreen
import fr.aphaistos.cryptedcall.ui.call.CallActiveScreen
import fr.aphaistos.cryptedcall.ui.call.CallIncidentScreen
import fr.aphaistos.cryptedcall.ui.call.CallSetupScreen
import fr.aphaistos.cryptedcall.ui.identity.ContactDetailScreen
import fr.aphaistos.cryptedcall.ui.identity.IdentityListScreen
import fr.aphaistos.cryptedcall.ui.identity.MyIdentityScreen
import fr.aphaistos.cryptedcall.viewmodel.IdentityViewModel

class MainActivity : ComponentActivity() {

    @RequiresApi(Build.VERSION_CODES.O)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        enableEdgeToEdge()
        setContent {
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .systemBarsPadding()
                    .padding(16.dp)
            ) {
                MyApp()
            }
        }
    }

    @RequiresApi(Build.VERSION_CODES.O)
    @Composable
    fun MyApp() {
        val navController = rememberNavController()
        val vm: IdentityViewModel = viewModel()

        NavHost(
            navController = navController,
            startDestination = NavRoutes.Bootstrap.route
        ) {

            composable(NavRoutes.Bootstrap.route) {
                BootstrapScreen {
                    navController.navigate(NavRoutes.IdentityList.route) {
                        popUpTo(0)
                    }
                }
            }

            composable(NavRoutes.IdentityList.route) {
                IdentityListScreen(vm, navController)
            }

            composable(NavRoutes.MyIdentity.route) {
                MyIdentityScreen(vm, navController)
            }

            composable(NavRoutes.ContactDetail.route) { backStack ->
                ContactDetailScreen(
                    id = backStack.arguments?.getString("id")!!,
                    vm = vm,
                    nav = navController
                )
            }

            composable(NavRoutes.CallSetup.route) { backStack ->
                CallSetupScreen(
                    contactId = backStack.arguments?.getString("id")!!,
                    nav = navController
                )
            }

            composable(NavRoutes.CallActive.route) { backStack ->
                CallActiveScreen(
                    contactId = backStack.arguments?.getString("id")!!,
                    nav = navController,
                    vm = vm
                )
            }

            composable(NavRoutes.CallIncident.route) {
                CallIncidentScreen(navController)
            }
        }
    }
}