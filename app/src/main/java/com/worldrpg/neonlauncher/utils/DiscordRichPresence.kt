package com.worldrpg.neonlauncher.utils

import android.content.Context
import android.content.Intent
import android.util.Log
import com.worldrpg.neonlauncher.BuildConfig
import java.util.concurrent.atomic.AtomicReference

/**
 * Project Genesis Discord Rich Presence manager for Android.
 * Starts automatically on launch, tracks elapsed playtime in-game,
 * and cannot be disabled by server scripts.
 */
object DiscordRichPresence {

    private const val TAG = "GenesisDiscordRPC"

    const val APPLICATION_ID = "1381148146637213736"
    const val SERVER_NAME = "Project : Genesis"
    const val LARGE_IMAGE_KEY = "genesis"
    const val LARGE_IMAGE_TEXT = "Project Genesis"
    const val SMALL_IMAGE_KEY = "android_client"
    const val SMALL_IMAGE_TEXT = "Android Mobile Client"

    data class PresenceSnapshot(
        val appId: String = APPLICATION_ID,
        val details: String = SERVER_NAME,
        val state: String = "In Launcher",
        val largeImageKey: String = LARGE_IMAGE_KEY,
        val largeImageText: String = LARGE_IMAGE_TEXT,
        val smallImageKey: String = SMALL_IMAGE_KEY,
        val smallImageText: String = SMALL_IMAGE_TEXT,
        val startTimestamp: Long = 0L,
        val enabled: Boolean = true
    )

    private val currentPresence = AtomicReference(PresenceSnapshot())

    @Volatile
    private var started = false

    @JvmStatic
    fun start(context: Context? = null) {
        if (!started) {
            started = true
            setInLauncher(context)
            Log.i(TAG, "Discord Rich Presence initialized for $SERVER_NAME")
        }
    }

    @JvmStatic
    fun setInLauncher(context: Context? = null) {
        val snapshot = PresenceSnapshot(
            details = SERVER_NAME,
            state = "In Launcher",
            startTimestamp = 0L,
            enabled = true
        )
        currentPresence.set(snapshot)
        broadcastUpdate(context, snapshot)
    }

    @JvmStatic
    fun setInGame(context: Context? = null, nickname: String? = null) {
        val endpoint = "${BuildConfig.DEFAULT_SERVER_IP}:${BuildConfig.DEFAULT_SERVER_PORT}"
        val cleanNick = nickname?.trim().orEmpty()
        val stateText = if (cleanNick.isNotEmpty()) {
            "Playing as $cleanNick"
        } else {
            "In-Game"
        }
        val snapshot = PresenceSnapshot(
            details = "Playing on $SERVER_NAME ($endpoint)",
            state = stateText,
            startTimestamp = System.currentTimeMillis() / 1000L,
            enabled = true
        )
        currentPresence.set(snapshot)
        broadcastUpdate(context, snapshot)
    }

    @JvmStatic
    fun getSnapshot(): PresenceSnapshot = currentPresence.get()

    @JvmStatic
    fun isEnabled(): Boolean = true

    private fun broadcastUpdate(context: Context?, snapshot: PresenceSnapshot) {
        Log.d(
            TAG,
            "RPC Update -> details='${snapshot.details}', state='${snapshot.state}', " +
                "large='${snapshot.largeImageKey}', small='${snapshot.smallImageKey}', start=${snapshot.startTimestamp}"
        )
        if (context == null) return
        try {
            val intent = Intent("com.worldrpg.neonlauncher.DISCORD_RPC_UPDATE").apply {
                setPackage(context.packageName)
                putExtra("app_id", snapshot.appId)
                putExtra("details", snapshot.details)
                putExtra("state", snapshot.state)
                putExtra("large_image_key", snapshot.largeImageKey)
                putExtra("large_image_text", snapshot.largeImageText)
                putExtra("small_image_key", snapshot.smallImageKey)
                putExtra("small_image_text", snapshot.smallImageText)
                putExtra("start_timestamp", snapshot.startTimestamp)
            }
            context.sendBroadcast(intent)
        } catch (e: RuntimeException) {
            Log.w(TAG, "Failed to broadcast RPC update", e)
        }
    }
}
