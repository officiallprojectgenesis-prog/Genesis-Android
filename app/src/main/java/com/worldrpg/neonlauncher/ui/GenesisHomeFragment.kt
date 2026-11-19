package com.worldrpg.neonlauncher.ui

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import com.worldrpg.neonlauncher.BuildConfig
import com.worldrpg.neonlauncher.R
import com.worldrpg.neonlauncher.databinding.FragmentGenesisHomeBinding
import com.worldrpg.neonlauncher.network.ServerQuery
import com.worldrpg.neonlauncher.utils.NativeAssetBridge
import com.worldrpg.neonlauncher.utils.PreferencesManager
import kotlinx.coroutines.launch

class GenesisHomeFragment : Fragment() {

    private var _binding: FragmentGenesisHomeBinding? = null
    private val binding get() = _binding!!

    private lateinit var prefsManager: PreferencesManager
    private lateinit var nativeAssetBridge: NativeAssetBridge

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentGenesisHomeBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        prefsManager = PreferencesManager(requireContext())
        nativeAssetBridge = NativeAssetBridge(requireContext())

        binding.tvBrandTitle.text = BuildConfig.LAUNCHER_TITLE
        binding.tvServerEndpoint.text = "${BuildConfig.DEFAULT_SERVER_IP}:${BuildConfig.DEFAULT_SERVER_PORT}"
        binding.tvVersionInfo.text = "Project : Genesis Mobile • v${BuildConfig.VERSION_NAME}"
        binding.btnPlayGenesis.text = BuildConfig.LAUNCHER_PLAY_BUTTON_TEXT

        binding.btnPlayGenesis.setOnClickListener {
            val enteredNick = binding.editQuickNickname.text?.toString()?.trim().orEmpty()
            val nick = enteredNick.ifEmpty { BuildConfig.DEFAULT_PLAYER_NICK }
            when (NicknameRules.check(nick)) {
                NicknameRules.Result.EMPTY -> {
                    binding.nicknameInputLayout.error = getString(R.string.nickname_empty)
                    return@setOnClickListener
                }
                NicknameRules.Result.INVALID -> {
                    binding.nicknameInputLayout.error = getString(R.string.nickname_invalid)
                    return@setOnClickListener
                }
                NicknameRules.Result.RESERVED -> {
                    binding.nicknameInputLayout.error = getString(R.string.nickname_reserved)
                    return@setOnClickListener
                }
                NicknameRules.Result.OK -> {
                    binding.nicknameInputLayout.error = null
                }
            }
            if (nativeAssetBridge.getReadyStoragePath() == null) {
                binding.tvAssetsStatus.text = "Please sync game assets in Settings first"
                (activity as? MainActivity)?.openSettingsTab()
                return@setOnClickListener
            }
            prefsManager.username = nick
            (activity as? MainActivity)?.launchGenesisServer()
        }

        binding.statusPill.setOnClickListener {
            refreshServerStatus()
        }
    }

    override fun onResume() {
        super.onResume()
        val currentNick = prefsManager.username
        if (binding.editQuickNickname.text?.toString() != currentNick) {
            binding.editQuickNickname.setText(currentNick)
        }
        val assetsReady = nativeAssetBridge.getReadyStoragePath() != null
        binding.tvAssetsStatus.text = if (assetsReady) {
            "Game assets verified and ready"
        } else {
            "Sync game assets in Settings before joining"
        }
        refreshServerStatus()
    }

    private fun refreshServerStatus() {
        binding.tvServerStatus.text = getString(R.string.genesis_status_checking)
        viewLifecycleOwner.lifecycleScope.launch {
            val result = ServerQuery.query(BuildConfig.DEFAULT_SERVER_IP, BuildConfig.DEFAULT_SERVER_PORT)
            val b = _binding ?: return@launch
            b.tvServerStatus.text = if (result != null) {
                getString(R.string.genesis_status_online, result.players, result.maxPlayers, result.pingMs)
            } else {
                getString(R.string.genesis_status_ready)
            }
        }
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}
