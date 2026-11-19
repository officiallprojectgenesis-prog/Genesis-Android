package com.worldrpg.neonlauncher.ui

import android.content.Intent
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import com.worldrpg.neonlauncher.BuildConfig
import com.worldrpg.neonlauncher.R
import com.worldrpg.neonlauncher.databinding.FragmentSettingsBinding
import com.worldrpg.neonlauncher.game.EngineHostActivity
import com.worldrpg.neonlauncher.utils.NativeAssetBridge
import com.worldrpg.neonlauncher.utils.PreferencesManager
import kotlinx.coroutines.launch

class SettingsFragment : Fragment() {

    private var _binding: FragmentSettingsBinding? = null
    private val binding get() = _binding!!

    private lateinit var prefsManager: PreferencesManager
    private lateinit var nativeAssetBridge: NativeAssetBridge
    private var syncingAssets = false

    private val engineTest = registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result ->
        val error = result.data?.getStringExtra(EngineHostActivity.EXTRA_BOOT_ERROR)
        if (error != null) context?.let { Toast.makeText(it, error, Toast.LENGTH_LONG).show() }
        updateEngineTestButton()
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentSettingsBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        prefsManager = PreferencesManager(requireContext())
        nativeAssetBridge = NativeAssetBridge(requireContext())

        loadCurrentSettings()
        setupValueLabels()
        setupSaveButton()
        setupAssetSyncButton()
        setupEngineTestButton()
    }

    private fun loadCurrentSettings() {
        // The nickname is edited on the server screen, next to the play button.

        // Graphics Quality
        binding.graphicsToggle.check(
            when (prefsManager.graphicsQuality) {
                0 -> R.id.btnGraphicsLow
                2 -> R.id.btnGraphicsHigh
                else -> R.id.btnGraphicsMedium
            }
        )

        // Audio Volume
        binding.sliderVolume.value = prefsManager.audioVolume.toFloat()

        // Touch Sensitivity
        binding.sliderTouchSensitivity.value = prefsManager.touchSensitivity.toFloat()
    }

    private fun setupValueLabels() {
        binding.volumeValue.text = "${binding.sliderVolume.value.toInt()}%"
        binding.touchValue.text = "${binding.sliderTouchSensitivity.value.toInt()}"
        binding.sliderVolume.addOnChangeListener { _, value, _ ->
            binding.volumeValue.text = "${value.toInt()}%"
        }
        binding.sliderTouchSensitivity.addOnChangeListener { _, value, _ ->
            binding.touchValue.text = "${value.toInt()}"
        }
    }

    private fun setupSaveButton() {
        binding.btnSaveSettings.setOnClickListener {
            saveSettings()
        }
    }

    private fun setupAssetSyncButton() {
        val button = binding.btnSyncAssets
        button.setOnClickListener {
            button.isEnabled = false
            syncingAssets = true
            updateEngineTestButton()
            showAssetProgress(getString(R.string.downloading_assets), null)
            viewLifecycleOwner.lifecycleScope.launch {
                try {
                    nativeAssetBridge.syncAssets().collect { state ->
                        when (state) {
                            NativeAssetBridge.DownloadState.FetchingManifest ->
                                showAssetProgress("Fetching game asset manifest...", null)
                            is NativeAssetBridge.DownloadState.Verifying ->
                                showAssetProgress(
                                    "Verifying game assets ${state.current}/${state.total}",
                                    if (state.total > 0) state.current * 100 / state.total else null
                                )
                            is NativeAssetBridge.DownloadState.Downloading ->
                                showAssetProgress("Downloading ${state.fileName}", state.percent)
                            is NativeAssetBridge.DownloadState.Complete ->
                                Toast.makeText(
                                    context,
                                    "${state.fileCount} game files verified and ready",
                                    Toast.LENGTH_LONG
                                ).show()
                            is NativeAssetBridge.DownloadState.Error ->
                                Toast.makeText(context, state.message, Toast.LENGTH_LONG).show()
                        }
                    }
                } finally {
                    _binding?.let { b ->
                        b.assetStatus.visibility = View.GONE
                        b.assetProgress.visibility = View.GONE
                        b.btnSyncAssets.isEnabled = true
                    }
                    syncingAssets = false
                    updateEngineTestButton()
                }
            }
        }
    }

    /** [percent] null shows an indeterminate bar. */
    private fun showAssetProgress(status: String, percent: Int?) {
        val b = _binding ?: return
        b.assetStatus.visibility = View.VISIBLE
        b.assetStatus.text = status
        b.assetProgress.visibility = View.VISIBLE
        if (percent == null) {
            if (!b.assetProgress.isIndeterminate) {
                b.assetProgress.visibility = View.INVISIBLE
                b.assetProgress.isIndeterminate = true
                b.assetProgress.visibility = View.VISIBLE
            }
        } else {
            b.assetProgress.isIndeterminate = false
            b.assetProgress.setProgressCompat(percent.coerceIn(0, 100), true)
        }
    }

    private fun setupEngineTestButton() {
        binding.btnTestGta.visibility = if (BuildConfig.DEBUG) View.VISIBLE else View.GONE
        binding.btnTestGta.setOnClickListener {
            if (!BuildConfig.DEBUG || syncingAssets) return@setOnClickListener
            if (nativeAssetBridge.getReadyStoragePath() == null) {
                Toast.makeText(context, "Sincronize os arquivos do jogo antes de testar o GTA.", Toast.LENGTH_LONG).show()
                updateEngineTestButton()
                return@setOnClickListener
            }
            engineTest.launch(Intent(requireContext(), EngineHostActivity::class.java).apply {
                // Reuse a paused host already in this task; GTA's globals
                // cannot belong to two Activities in the engine process.
                addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT)
            })
        }
        updateEngineTestButton()
    }

    private fun updateEngineTestButton() {
        val viewBinding = _binding ?: return
        viewBinding.btnTestGta.isEnabled = BuildConfig.DEBUG && !syncingAssets &&
            ::nativeAssetBridge.isInitialized && nativeAssetBridge.getReadyStoragePath() != null
    }

    override fun onResume() {
        super.onResume()
        updateEngineTestButton()
    }

    private fun saveSettings() {
        // Save graphics quality
        prefsManager.graphicsQuality = when (binding.graphicsToggle.checkedButtonId) {
            R.id.btnGraphicsLow -> 0
            R.id.btnGraphicsHigh -> 2
            else -> 1
        }

        // Save audio volume
        prefsManager.audioVolume = binding.sliderVolume.value.toInt()

        // Save touch sensitivity
        prefsManager.touchSensitivity = binding.sliderTouchSensitivity.value.toInt()

        Toast.makeText(context, R.string.settings_saved, Toast.LENGTH_SHORT).show()
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}
