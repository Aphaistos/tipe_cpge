package fr.aphaistos.cryptedcall.crypto

import android.os.Build
import androidx.annotation.RequiresApi
import java.security.MessageDigest
import java.time.OffsetDateTime
import java.time.ZoneOffset
import java.time.format.DateTimeFormatter
import java.util.Base64
import kotlin.random.Random

object CryptoStub {

    private val ED25519_SPKI_PREFIX = byteArrayOf(
        0x30, 0x2A,                 // SEQUENCE (42 bytes)
        0x30, 0x05,                 // SEQUENCE (5 bytes)
        0x06, 0x03, 0x2B, 0x65, 0x70,// OID 1.3.101.112
        0x03, 0x21, 0x00            // BIT STRING (33 bytes, 0 unused bits)
    )

    @RequiresApi(Build.VERSION_CODES.O)
    fun generatePublicKeyPem(): String {
        val publicKeyRaw = ByteArray(32)
        Random.nextBytes(publicKeyRaw)

        val spki = ED25519_SPKI_PREFIX + publicKeyRaw
        val base64 = Base64.getEncoder().encodeToString(spki)

        return """
            -----BEGIN PUBLIC KEY-----
            ${base64.chunked(64).joinToString("\n")}
            -----END PUBLIC KEY-----
        """.trimIndent()
    }

    /**
     * Génère l’empreinte SHA-256 (hex groupé)
     */
    fun generateFingerprint(): String {
        val randomData = ByteArray(64)
        Random.nextBytes(randomData)

        val hash = MessageDigest.getInstance("SHA-256").digest(randomData)

        return hash
            .joinToString("") { "%02X".format(it) }
            .chunked(4)
            .take(8)
            .joinToString(" ")
    }

    fun generateIdentity() {
    }

    fun establishSecureSession() {
        // Ne fait rien
    }

    fun detectKeyChange(): Boolean {
        return false
    }
}
