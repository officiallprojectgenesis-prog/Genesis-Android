package com.worldrpg.neonlauncher.utils

import android.content.Context
import android.net.Uri
import android.util.Log
import com.worldrpg.neonlauncher.BuildConfig
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.currentCoroutineContext
import kotlinx.coroutines.delay
import kotlinx.coroutines.ensureActive
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flow
import kotlinx.coroutines.flow.flowOn
import kotlinx.coroutines.sync.Mutex
import org.json.JSONArray
import org.json.JSONException
import org.json.JSONObject
import java.io.ByteArrayOutputStream
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.io.InputStream
import java.net.HttpURLConnection
import java.net.URL
import java.nio.file.Files
import java.nio.file.StandardCopyOption
import java.security.MessageDigest
import java.util.Locale
import java.util.UUID

/** Installs and verifies GTA data from Hugging Face or a local folder before making it available to the engine. */
class NativeAssetBridge(private val context: Context) {
    private val storage = File(context.filesDir, "game_assets")
    private val generations = File(storage, "generations")
    private val active = File(storage, "active")

    sealed interface DownloadState {
        data object FetchingManifest : DownloadState
        data class Verifying(val current: Int, val total: Int) : DownloadState
        data class Downloading(
            val fileName: String,
            val percent: Int,
            val currentFile: Int = 0,
            val totalFiles: Int = 0
        ) : DownloadState
        data class Complete(val fileCount: Int) : DownloadState
        data class Error(val message: String) : DownloadState
    }

    /** Primary external game asset directory used for Hugging Face downloads and local storage. */
    fun getExternalAssetsDir(): File {
        return context.getExternalFilesDir(null)?.let { File(it, "game_assets") } ?: storage
    }

    /** Search for a local GTA San Andreas Mobile 2.10 assets folder on device or external storage. */
    fun findLocalAssetsFolder(): File? {
        val candidates = listOfNotNull(
            BuildConfig.DEVICE_ASSETS_PATH.takeIf { it.isNotBlank() }?.let { File(it) },
            context.getExternalFilesDir(null)?.let { File(it, "game_assets") },
            context.getExternalFilesDir(null)?.let { File(it, "gta_assets") },
            File(storage, "local_source"),
            File("/sdcard/Android/data/${context.packageName}/files/game_assets"),
            File("/sdcard/Download/WorldRPG_backup"),
            File("/sdcard/worldrpg_gamefiles"),
            File("/sdcard/mta_assets"),
            File("/sdcard/GTASA")
        )
        return candidates.firstOrNull { dir ->
            try {
                dir.isDirectory && REQUIRED_FILES.all { File(dir, it).isFile }
            } catch (_: SecurityException) {
                false
            }
        }
    }

    /** Fast startup gate. Returns installed generation path or direct external/local assets path. */
    fun getReadyStoragePath(): String? {
        return try {
            val externalDir = getExternalAssetsDir()
            if (externalDir.isDirectory && REQUIRED_FILES.all { File(externalDir, it).isFile }) {
                return externalDir.absolutePath
            }
            val generation = activeGeneration()
            if (generation != null) {
                val entries = installedEntries(generation)
                if (entries != null && entries.filter { it.path in REQUIRED_FILES }.all { entry ->
                        val file = assetFile(generation, entry.path)
                        file.isFile && file.length() == entry.size
                    }) return generation.absolutePath
            }
            val localFolder = findLocalAssetsFolder()
            if (localFolder != null && REQUIRED_FILES.all { File(localFolder, it).isFile }) {
                return localFolder.absolutePath
            }
            null
        } catch (error: Exception) {
            Log.w(TAG, "Cannot read installed game assets", error)
            null
        }
    }

    fun syncAssets(): Flow<DownloadState> = flow {
        if (!installationLock.tryLock()) {
            emit(DownloadState.Error("Game assets are already syncing."))
            return@flow
        }

        try {
            val targetDir = getExternalAssetsDir()

            // Priority 1: Check for an external manual local assets folder (different from targetDir)
            val localSource = findLocalAssetsFolder()
            if (localSource != null && localSource.canonicalFile != targetDir.canonicalFile) {
                emit(DownloadState.FetchingManifest)
                syncFromLocalFolder(localSource) { emit(it) }
                return@flow
            }

            // Priority 2: If custom CDN_ROOT_URL is explicitly configured in .env, use legacy manifest flow
            val customCdn = BuildConfig.CDN_ROOT_URL.trim()
            if (customCdn.isNotEmpty()) {
                syncFromCustomCdn(customCdn) { emit(it) }
                return@flow
            }

            // Priority 3 (Default): Dynamic Hugging Face Dataset Tree API
            syncFromHuggingFace(targetDir) { emit(it) }
        } catch (error: IOException) {
            Log.w(TAG, "Game asset sync failed", error)
            emit(DownloadState.Error(error.message ?: "Game asset sync failed."))
        } catch (error: JSONException) {
            Log.w(TAG, "Invalid game asset manifest", error)
            emit(DownloadState.Error("Invalid game asset manifest from Hugging Face."))
        } finally {
            installationLock.unlock()
        }
    }.flowOn(Dispatchers.IO)

    private suspend fun syncFromHuggingFace(
        targetDir: File,
        emitState: suspend (DownloadState) -> Unit
    ) {
        emitState(DownloadState.FetchingManifest)
        ensureDirectory(targetDir)

        val entries = fetchHuggingFaceTree()
        if (entries.isEmpty()) {
            throw IOException("No game asset files found in Hugging Face dataset.")
        }

        val totalFiles = entries.size
        val partFile = File(targetDir, ".asset-download.part")

        try {
            entries.forEachIndexed { index, entry ->
                currentCoroutineContext().ensureActive()
                emitState(DownloadState.Verifying(index + 1, totalFiles))

                val destination = assetFile(targetDir, entry.relativePath)
                if (destination.isFile && destination.length() == entry.size) {
                    return@forEachIndexed
                }

                ensureDirectory(destination.parentFile ?: targetDir)
                if (targetDir.usableSpace < entry.size) {
                    throw IOException("Not enough free storage space for ${entry.relativePath}.")
                }

                val encodedPath = entry.repoPath.split('/').joinToString("/") { Uri.encode(it) }
                val downloadUrl = URL("$HF_RESOLVE_BASE/$encodedPath")

                downloadWithRetry(downloadUrl, partFile, entry, index + 1, totalFiles, emitState)
                atomicMove(partFile, destination)
            }
        } finally {
            partFile.delete()
        }

        emitState(DownloadState.Complete(totalFiles))
    }

    private suspend fun downloadWithRetry(
        url: URL,
        partFile: File,
        entry: HfEntry,
        currentFile: Int,
        totalFiles: Int,
        emitState: suspend (DownloadState) -> Unit
    ) {
        var lastError: IOException? = null
        repeat(MAX_DOWNLOAD_RETRIES) { attempt ->
            currentCoroutineContext().ensureActive()
            if (attempt > 0) {
                delay(1500L * attempt)
            }
            val connection = try {
                open(url)
            } catch (e: IOException) {
                lastError = e
                Log.w(TAG, "Attempt ${attempt + 1} failed to open ${entry.relativePath}: ${e.message}")
                return@repeat
            }
            try {
                emitState(DownloadState.Downloading(entry.relativePath, 0, currentFile, totalFiles))
                connection.inputStream.buffered().use { input ->
                    copyHfStream(input, partFile, entry) { percent ->
                        emitState(DownloadState.Downloading(entry.relativePath, percent, currentFile, totalFiles))
                    }
                }
                return
            } catch (e: IOException) {
                lastError = e
                partFile.delete()
                Log.w(TAG, "Attempt ${attempt + 1} failed downloading ${entry.relativePath}: ${e.message}")
            } finally {
                connection.disconnect()
            }
        }
        throw lastError ?: IOException("Failed to download ${entry.relativePath}.")
    }

    private suspend fun copyHfStream(
        input: InputStream,
        partFile: File,
        entry: HfEntry,
        progress: suspend (Int) -> Unit
    ) {
        val buffer = ByteArray(BUFFER_SIZE)
        var total = 0L
        var previousPercent = 0
        FileOutputStream(partFile).use { output ->
            while (true) {
                currentCoroutineContext().ensureActive()
                val count = input.read(buffer)
                if (count < 0) break
                total += count
                if (entry.size > 0 && total > entry.size) {
                    throw IOException("Unexpected size for ${entry.relativePath}.")
                }
                output.write(buffer, 0, count)
                if (entry.size > 0) {
                    val percent = (100L * total / entry.size).toInt().coerceIn(0, 100)
                    if (percent > previousPercent) {
                        progress(percent)
                        previousPercent = percent
                    }
                }
            }
            if (entry.size > 0 && total != entry.size) {
                throw IOException("Incomplete download for ${entry.relativePath} ($total/${entry.size} bytes).")
            }
            output.fd.sync()
        }
        if (previousPercent < 100) {
            progress(100)
        }
    }

    private suspend fun fetchHuggingFaceTree(): List<HfEntry> {
        var lastError: IOException? = null
        repeat(MAX_DOWNLOAD_RETRIES) { attempt ->
            currentCoroutineContext().ensureActive()
            if (attempt > 0) {
                delay(1500L * attempt)
            }
            val connection = try {
                open(URL(HF_TREE_API_URL))
            } catch (e: IOException) {
                lastError = e
                return@repeat
            }
            try {
                val json = connection.inputStream.use { input ->
                    val output = ByteArrayOutputStream()
                    val buffer = ByteArray(BUFFER_SIZE)
                    while (true) {
                        currentCoroutineContext().ensureActive()
                        val count = input.read(buffer)
                        if (count < 0) break
                        if (output.size() + count > MAX_MANIFEST_BYTES) {
                            throw IOException("Hugging Face file tree response is too large.")
                        }
                        output.write(buffer, 0, count)
                    }
                    output.toString(Charsets.UTF_8.name())
                }
                return parseHuggingFaceTree(json)
            } catch (e: IOException) {
                lastError = e
            } finally {
                connection.disconnect()
            }
        }
        throw lastError ?: IOException("Failed to fetch game asset list from Hugging Face.")
    }

    private fun parseHuggingFaceTree(json: String): List<HfEntry> {
        val array = JSONArray(json)
        val prefix = "$CONTENT_DIR/"
        val entries = mutableListOf<HfEntry>()
        for (i in 0 until array.length()) {
            val item = array.getJSONObject(i)
            if (item.optString("type") != "file") continue
            val repoPath = item.getString("path")
            if (!repoPath.startsWith(prefix)) continue
            val relativePath = repoPath.removePrefix(prefix)
            if (relativePath.isBlank() || relativePath == MANIFEST_NAME) continue
            val parts = relativePath.split('/')
            if (parts.any { it.isEmpty() || it == "." || it == ".." }) {
                throw IOException("Invalid game asset path: $relativePath")
            }
            val size = item.optLong("size", -1L)
            if (size < 0L || size > MAX_ASSET_BYTES) {
                throw IOException("Invalid file size for $relativePath")
            }
            val oid = item.optString("oid", "")
            entries.add(HfEntry(repoPath = repoPath, relativePath = relativePath, size = size, oid = oid))
        }
        return entries
    }

    private suspend fun syncFromCustomCdn(
        cdnRoot: String,
        emitState: suspend (DownloadState) -> Unit
    ) {
        emitState(DownloadState.FetchingManifest)
        val manifest = fetchManifest(cdnRoot)
        val previous = activeGeneration()
        val unchanged = previous != null && installedEntries(previous) == manifest.entries &&
            manifest.entries.withIndex().all { (index, entry) ->
                emitState(DownloadState.Verifying(index + 1, manifest.entries.size))
                matches(assetFile(previous, entry.path), entry)
            }
        if (unchanged) {
            emitState(DownloadState.Complete(manifest.entries.size))
            return
        }

        ensureDirectory(generations)
        val staging = assetFile(storage, "staging-${manifest.revision}")
        ensureDirectory(staging)
        val partial = assetFile(storage, "asset-download.part")
        try {
            manifest.entries.forEachIndexed { index, entry ->
                emitState(DownloadState.Verifying(index + 1, manifest.entries.size))
                val target = assetFile(staging, entry.path)
                if (matches(target, entry)) return@forEachIndexed
                ensureDirectory(target.parentFile ?: staging)
                if (staging.usableSpace < entry.size) {
                    throw IOException("Not enough free space for game assets.")
                }
                val old = previous?.let { assetFile(it, entry.path) }
                if (old != null && matches(old, entry)) {
                    old.inputStream().buffered().use { input ->
                        copyVerified(input, partial, entry) {}
                    }
                } else {
                    val path = entry.path.split('/').joinToString("/") { Uri.encode(it) }
                    val url = URL("$cdnRoot/${manifest.revision}/$CONTENT_DIR/$path")
                    val connection = open(url)
                    try {
                        emitState(DownloadState.Downloading(entry.path, 0, index + 1, manifest.entries.size))
                        connection.inputStream.buffered().use { input ->
                            copyVerified(input, partial, entry) { percent ->
                                emitState(DownloadState.Downloading(entry.path, percent, index + 1, manifest.entries.size))
                            }
                        }
                    } finally {
                        connection.disconnect()
                    }
                }
                atomicMove(partial, target)
            }

            currentCoroutineContext().ensureActive()
            val installed = File(generations, "${manifest.revision}-${UUID.randomUUID()}")
            writeSynced(File(staging, MANIFEST_NAME), manifest.json)
            atomicMove(staging, installed)
            writeSynced(partial, installed.name)
            atomicMove(partial, active)
        } finally {
            partial.delete()
        }
        emitState(DownloadState.Complete(manifest.entries.size))
    }

    private suspend fun syncFromLocalFolder(
        sourceDir: File,
        emitState: suspend (DownloadState) -> Unit
    ) {
        val files = mutableListOf<File>()
        sourceDir.walkTopDown().filter { it.isFile && it.name != MANIFEST_NAME }.forEach { files.add(it) }

        val entries = mutableListOf<Entry>()
        val total = files.size
        ensureDirectory(generations)
        val staging = assetFile(storage, "staging-local")
        ensureDirectory(staging)

        files.forEachIndexed { index, file ->
            val relativePath = file.relativeTo(sourceDir).path.replace('\\', '/')
            emitState(DownloadState.Verifying(index + 1, total))

            val target = assetFile(staging, relativePath)
            ensureDirectory(target.parentFile ?: staging)
            if (!target.isFile || target.length() != file.length()) {
                file.copyTo(target, overwrite = true)
            }
            entries.add(Entry(relativePath, file.length(), ""))
        }

        val jsonArray = JSONArray()
        entries.forEach { entry ->
            val obj = JSONObject()
            obj.put("path", entry.path)
            obj.put("size", entry.size)
            obj.put("sha256", "")
            jsonArray.put(obj)
        }
        val manifestObj = JSONObject().apply {
            put("files", jsonArray)
            put("total_files", entries.size)
            put("total_size", entries.sumOf { it.size })
        }

        val installed = File(generations, "local-${UUID.randomUUID()}")
        writeSynced(File(staging, MANIFEST_NAME), manifestObj.toString())
        atomicMove(staging, installed)
        val partial = assetFile(storage, "asset-download.part")
        writeSynced(partial, installed.name)
        atomicMove(partial, active)

        emitState(DownloadState.Complete(entries.size))
    }

    private data class HfEntry(val repoPath: String, val relativePath: String, val size: Long, val oid: String)
    private data class Entry(val path: String, val size: Long, val sha256: String)
    private data class Manifest(val revision: String, val json: String, val entries: List<Entry>)

    private suspend fun fetchManifest(cdnRoot: String): Manifest {
        val connection = open(URL("$cdnRoot/main/$MANIFEST_NAME"))
        try {
            val revision = connection.getHeaderField("X-Repo-Commit") ?: "head"
            val json = connection.inputStream.use { input ->
                val output = ByteArrayOutputStream()
                val buffer = ByteArray(BUFFER_SIZE)
                while (true) {
                    currentCoroutineContext().ensureActive()
                    val count = input.read(buffer)
                    if (count < 0) break
                    if (output.size() + count > MAX_MANIFEST_BYTES) {
                        throw IOException("Game asset manifest is too large.")
                    }
                    output.write(buffer, 0, count)
                }
                output.toString(Charsets.UTF_8.name())
            }
            return Manifest(revision, json, parseManifest(json))
        } finally {
            connection.disconnect()
        }
    }

    private fun parseManifest(json: String): List<Entry> {
        val document = JSONObject(json)
        val files = document.getJSONArray("files")
        if (files.length() !in 1..4096 || document.getInt("total_files") != files.length()) {
            throw IOException("Invalid game asset file count.")
        }
        val filesByPath = mutableSetOf<String>()
        val directories = mutableMapOf<String, String>()
        val entries = List(files.length()) { index ->
            val item = files.getJSONObject(index)
            val path = item.getString("path")
            val parts = path.split('/')
            val normalized = path.lowercase(Locale.ROOT)
            if (path.length > 1024 || parts.any { !PATH_PART.matches(it) || it == "." || it == ".." } ||
                normalized == MANIFEST_NAME || normalized.startsWith("$MANIFEST_NAME/") ||
                normalized in directories || !filesByPath.add(normalized)
            ) throw IOException("Invalid or duplicate game asset path.")
            for (depth in 1 until parts.size) {
                val parent = parts.take(depth).joinToString("/")
                val folded = parent.lowercase(Locale.ROOT)
                if (folded in filesByPath || (directories.putIfAbsent(folded, parent) ?: parent) != parent) {
                    throw IOException("Conflicting game asset directory.")
                }
            }
            val size = item.getLong("size")
            val sha = item.optString("sha256", "").lowercase(Locale.ROOT)
            if (size !in 1..MAX_ASSET_BYTES || (sha.isNotEmpty() && !SHA256.matches(sha))) {
                throw IOException("Invalid game asset integrity metadata.")
            }
            Entry(path, size, sha)
        }
        if (!entries.map { it.path }.containsAll(REQUIRED_FILES) ||
            entries.sumOf { it.size } != document.getLong("total_size")
        ) throw IOException("Incomplete game asset manifest.")
        return entries
    }

    private fun installedEntries(directory: File): List<Entry>? {
        return try {
            val file = assetFile(directory, MANIFEST_NAME)
            if (file.length() !in 1..MAX_MANIFEST_BYTES.toLong()) return null
            parseManifest(file.readText())
        } catch (_: IOException) {
            null
        } catch (_: JSONException) {
            null
        }
    }

    private fun activeGeneration(): File? {
        if (!active.isFile || active.length() !in 1..128) return null
        val name = active.readText()
        if (!GENERATION.matches(name)) return null
        val directory = assetFile(generations, name)
        return directory.takeIf { it.isDirectory && File(it, MANIFEST_NAME).isFile }
    }

    private fun assetFile(root: File, path: String): File {
        val file = File(root.canonicalFile, path)
        if (file.canonicalFile != file) throw IOException("Unsafe game asset path.")
        return file
    }

    private fun ensureDirectory(directory: File) {
        if (!directory.isDirectory && !directory.mkdirs()) {
            throw IOException("Cannot create game asset storage.")
        }
    }

    private fun writeSynced(file: File, contents: String) {
        FileOutputStream(file).use { output ->
            output.write(contents.toByteArray(Charsets.UTF_8))
            output.fd.sync()
        }
    }

    private fun atomicMove(from: File, to: File) {
        Files.move(from.toPath(), to.toPath(), StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING)
    }

    private suspend fun matches(file: File, entry: Entry): Boolean {
        if (!file.isFile || file.length() != entry.size) return false
        if (entry.sha256.isEmpty()) return true
        val digest = MessageDigest.getInstance("SHA-256")
        val buffer = ByteArray(BUFFER_SIZE)
        file.inputStream().buffered().use { input ->
            while (true) {
                currentCoroutineContext().ensureActive()
                val count = input.read(buffer)
                if (count < 0) break
                digest.update(buffer, 0, count)
            }
        }
        return digest.hex() == entry.sha256
    }

    private suspend fun copyVerified(
        input: InputStream,
        partial: File,
        entry: Entry,
        progress: suspend (Int) -> Unit
    ) {
        val digest = MessageDigest.getInstance("SHA-256")
        val buffer = ByteArray(BUFFER_SIZE)
        var total = 0L
        var previousPercent = 0
        FileOutputStream(partial).use { output ->
            while (true) {
                currentCoroutineContext().ensureActive()
                val count = input.read(buffer)
                if (count < 0) break
                total += count
                if (total > entry.size) throw IOException("Unexpected size for ${entry.path}.")
                digest.update(buffer, 0, count)
                output.write(buffer, 0, count)
                val percent = (100L * total / entry.size).toInt().coerceAtMost(99)
                if (percent > previousPercent) {
                    progress(percent)
                    previousPercent = percent
                }
            }
            if (total != entry.size || (entry.sha256.isNotEmpty() && digest.hex() != entry.sha256)) {
                throw IOException("Integrity check failed for ${entry.path}.")
            }
            output.fd.sync()
        }
    }

    private fun MessageDigest.hex(): String = digest().joinToString("") { "%02x".format(it) }

    private fun open(url: URL): HttpURLConnection {
        var endpoint = url
        repeat(6) {
            if (endpoint.protocol != "https" && endpoint.protocol != "http") {
                throw IOException("Game asset URLs must use HTTP or HTTPS.")
            }
            val connection = (endpoint.openConnection() as HttpURLConnection).apply {
                instanceFollowRedirects = false
                connectTimeout = 20_000
                readTimeout = 45_000
                setRequestProperty("User-Agent", "ProjectGenesis-Android/1.0")
            }
            var keepOpen = false
            try {
                val status = connection.responseCode
                if (status == HttpURLConnection.HTTP_OK) {
                    keepOpen = true
                    return connection
                }
                if (status !in REDIRECTS) throw IOException("Game assets CDN returned HTTP $status.")
                val location = connection.getHeaderField("Location")
                    ?: throw IOException("Missing game asset redirect.")
                endpoint = URL(endpoint, location)
            } finally {
                if (!keepOpen) connection.disconnect()
            }
        }
        throw IOException("Too many game asset redirects.")
    }

    private companion object {
        const val TAG = "NativeAssetBridge"
        const val CONTENT_DIR = "game_files_android_mta"
        const val HF_TREE_API_URL =
            "https://huggingface.co/api/datasets/WorldRPG/worldrpg-game-files/tree/main/game_files_android_mta?recursive=true"
        const val HF_RESOLVE_BASE =
            "https://huggingface.co/datasets/WorldRPG/worldrpg-game-files/resolve/main"
        const val MANIFEST_NAME = "manifest_android_mta.json"
        const val BUFFER_SIZE = 64 * 1024
        const val MAX_MANIFEST_BYTES = 8 * 1024 * 1024
        const val MAX_ASSET_BYTES = 4L * 1024 * 1024 * 1024
        const val MAX_DOWNLOAD_RETRIES = 3
        val SHA256 = Regex("[0-9a-f]{64}")
        val GENERATION = Regex("([0-9a-f]{40}|local)-[0-9a-f-]{36}")
        val PATH_PART = Regex("[A-Za-z0-9_ .()\\-]{1,255}")
        val REQUIRED_FILES = setOf("data/gta.dat", "data/handling.cfg", "texdb/gta3.img", "texdb/menu/menu.pvr.dat")
        val REDIRECTS = setOf(301, 302, 303, 307, 308)
        val installationLock = Mutex()
    }
}
