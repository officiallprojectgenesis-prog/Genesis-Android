package com.worldrpg.neonlauncher.network

import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.channelFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock

/**
 * Server list: the server configured in .env plus the player's favorites, each
 * queried live over ASE ([ServerQuery]). There is no master server list yet.
 */
class ServerBrowserService {

    sealed class BrowserState {
        object Loading : BrowserState()
        data class Success(val servers: List<ServerInfo>) : BrowserState()
        data class Error(val message: String) : BrowserState()
    }

    /**
     * Emits the list right away with every server QUERYING, then again each time
     * a query answers or times out.
     */
    fun fetchServerList(favoriteKeys: Set<String> = emptySet()): Flow<BrowserState> = channelFlow {
        send(BrowserState.Loading)
        val configured = ServerInfo.configured()
        val keys = LinkedHashSet<String>().apply {
            add(configured.getKey())
            addAll(favoriteKeys.sorted())
        }
        val servers = keys.map { key ->
            (if (key == configured.getKey()) configured else ServerInfo.fromFavoriteKey(key))
                .copy(isFavorite = key in favoriteKeys)
        }.toMutableList()
        send(BrowserState.Success(servers.toList()))

        val lock = Mutex()
        coroutineScope {
            servers.indices.forEach { index ->
                launch {
                    val s = servers[index]
                    val result = ServerQuery.query(s.ip, s.port)
                    lock.withLock {
                        servers[index] = servers[index].withQuery(result)
                        send(BrowserState.Success(servers.toList()))
                    }
                }
            }
        }
    }

    /** Queries one server. */
    suspend fun refresh(server: ServerInfo): ServerInfo =
        server.withQuery(ServerQuery.query(server.ip, server.port))
}
