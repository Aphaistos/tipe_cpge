package fr.aphaistos.cryptedcall.viewmodel

import android.os.Build
import androidx.annotation.RequiresApi
import androidx.compose.runtime.mutableStateListOf
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import fr.aphaistos.cryptedcall.crypto.CryptoStub
import fr.aphaistos.cryptedcall.model.Identity
import fr.aphaistos.cryptedcall.model.KeyObservationState
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import java.time.OffsetDateTime
import java.time.ZoneOffset
import java.time.format.DateTimeFormatter
import java.util.Base64
import kotlin.random.Random

class IdentityViewModel : ViewModel() {

    @RequiresApi(Build.VERSION_CODES.O)
    private fun nowUtc(): String =
        OffsetDateTime.now(ZoneOffset.UTC).format(DateTimeFormatter.ofPattern("yyyy-MM-DD HH:mm:ss 'UTC'"))

    @RequiresApi(Build.VERSION_CODES.O)
    var now = nowUtc()

    @RequiresApi(Build.VERSION_CODES.O)
    val identities = mutableStateListOf(
        Identity(
            id = "self",
            name = "Moi",
            fingerprint = CryptoStub.generateFingerprint(),
            publicKey = CryptoStub.generatePublicKeyPem(),
            verified = true,
            KeyObservationState.STABLE,
            isSelf = true,
            generatedAt = now
        ),
        Identity("alpha", "Alpha-2", CryptoStub.generateFingerprint(), CryptoStub.generatePublicKeyPem(), false, KeyObservationState.NOT_AVAILABLE, generatedAt = now),
        Identity("bravo", "Bravo-7", CryptoStub.generateFingerprint(), CryptoStub.generatePublicKeyPem(), false, KeyObservationState.NOT_AVAILABLE, generatedAt =now),
        Identity("charlie", "Charlie-4", CryptoStub.generateFingerprint(), CryptoStub.generatePublicKeyPem(), false,KeyObservationState.NOT_AVAILABLE, generatedAt =now),
        Identity("delta", "Delta-9", CryptoStub.generateFingerprint(), CryptoStub.generatePublicKeyPem(), false, KeyObservationState.NOT_AVAILABLE,generatedAt =now),
        Identity("echo", "Echo-1", CryptoStub.generateFingerprint(), CryptoStub.generatePublicKeyPem(), false, KeyObservationState.NOT_AVAILABLE,generatedAt =now),
        Identity("foxtrot", "Foxtrot-6", CryptoStub.generateFingerprint(), CryptoStub.generatePublicKeyPem(), false, KeyObservationState.NOT_AVAILABLE,generatedAt =now),
        Identity("golf", "Golf-3", CryptoStub.generateFingerprint(), CryptoStub.generatePublicKeyPem(), false, KeyObservationState.NOT_AVAILABLE,generatedAt =now),
        Identity("hotel", "Hotel-8", CryptoStub.generateFingerprint(), CryptoStub.generatePublicKeyPem(), false, KeyObservationState.NOT_AVAILABLE,generatedAt =now),
        Identity("kilo", "Kilo-5", CryptoStub.generateFingerprint(), CryptoStub.generatePublicKeyPem(), false, KeyObservationState.NOT_AVAILABLE,generatedAt =now)
    )

    @RequiresApi(Build.VERSION_CODES.O)
    fun regenerateIdentity() {
        val index = identities.indexOfFirst { it.isSelf }
        if (index == -1) return

        identities[index] = identities[index].copy(
            fingerprint = CryptoStub.generateFingerprint(),
            publicKey = CryptoStub.generatePublicKeyPem(),
            generatedAt = nowUtc()
        )
    }

    @RequiresApi(Build.VERSION_CODES.O)
    fun verifyContact(id: String) {
        val index = identities.indexOfFirst { it.id == id }
        if (index == -1) return

        // Phase 1 : vérification en cours
        identities[index] = identities[index].copy(
            verified = false,
            keyState = KeyObservationState.VERIFYING
        )

        viewModelScope.launch {
            delay(2000)

            // Phase 2 : identité vérifiée + clé stable
            identities[index] = identities[index].copy(
                verified = true,
                keyState = KeyObservationState.STABLE
            )
        }
    }

}