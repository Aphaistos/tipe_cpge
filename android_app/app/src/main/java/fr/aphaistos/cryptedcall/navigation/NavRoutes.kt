package fr.aphaistos.cryptedcall.navigation

sealed class NavRoutes(val route: String) {

    object Bootstrap : NavRoutes("bootstrap")
    object IdentityList : NavRoutes("identity_list")
    object MyIdentity : NavRoutes("my_identity")

    object ContactDetail : NavRoutes("contact/{id}")
    object CallSetup : NavRoutes("call_setup/{id}")
    object CallActive : NavRoutes("call_active/{id}")

    object CallIncident : NavRoutes("call_incident")
}
