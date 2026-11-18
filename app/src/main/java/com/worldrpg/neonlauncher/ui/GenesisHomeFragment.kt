package com.worldrpg.neonlauncher.ui

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.fragment.app.Fragment
import com.worldrpg.neonlauncher.BuildConfig
import com.worldrpg.neonlauncher.databinding.FragmentGenesisHomeBinding
import com.worldrpg.neonlauncher.utils.NativeAssetBridge
import com.worldrpg.neonlauncher.utils.PreferencesManager

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
        binding.tvVersionInfo.text = "Project Genesis Client • v${BuildConfig.VERSION_NAME}"
        binding.btnPlayGenesis.text = BuildConfig.LAUNCHER_PLAY_BUTTON_TEXT

        binding.btnPlayGenesis.setOnClickListener {
            val enteredNick = binding.editQuickNickname.text?.toString()?.trim().orEmpty()
            val nick = if (enteredNick.isNotEmpty()) enteredNick else BuildConfig.DEFAULT_PLAYER_NICK
            if (nick.any { it.code > 127 }) {
                Toast.makeText(requireContext(), "Use ASCII characters only in nickname", Toast.LENGTH_SHORT).show()
                return@setOnClickListener
            }
            prefsManager.username = nick
            (activity as? MainActivity)?.launchGenesisServer()
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
            "Tip: Sync game assets in Settings before joining"
        }
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}
