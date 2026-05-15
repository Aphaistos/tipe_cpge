package fr.aphaistos.cryptedcall.model

enum class KeyObservationState {
    NOT_AVAILABLE,
    VERIFYING,
    STABLE,
    CHANGED
}

data class Identity(
    val id: String,
    val name: String,
    val fingerprint: String,
    val publicKey: String = "",
    val verified: Boolean,
    val keyState: KeyObservationState,
    val isSelf: Boolean = false,
    val generatedAt: String
)
