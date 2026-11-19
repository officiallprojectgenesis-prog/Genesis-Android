package com.worldrpg.neonlauncher.ui

import android.content.Intent
import android.os.Bundle
import android.widget.ImageView
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.ViewCompat
import androidx.core.view.WindowCompat
import androidx.core.view.WindowInsetsCompat
import androidx.core.view.updatePadding
import androidx.lifecycle.Lifecycle
import com.worldrpg.neonlauncher.BuildConfig
import com.worldrpg.neonlauncher.R
import com.worldrpg.neonlauncher.databinding.ActivityMainBinding
import com.worldrpg.neonlauncher.game.EngineHostActivity
import com.worldrpg.neonlauncher.game.GameActivity
import com.worldrpg.neonlauncher.network.ServerInfo
import com.worldrpg.neonlauncher.utils.DiscordRichPresence

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        WindowCompat.setDecorFitsSystemWindows(window, false)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        title = BuildConfig.LAUNCHER_TITLE
        DiscordRichPresence.start(applicationContext)

        applyWindowInsets()
        setupRailHeader()
        setupTabs(savedInstanceState == null)
        setupNavigation()
    }

    override fun onResume() {
        super.onResume()
        DiscordRichPresence.setInLauncher(applicationContext)
    }

    /**
     * Edge to edge: the rail background runs under the status bar and the
     * camera cutout, and the content is padded so nothing sits beneath them.
     */
    private fun applyWindowInsets() {
        ViewCompat.setOnApplyWindowInsetsListener(binding.rootLayout) { _, insets ->
            val bars = insets.getInsets(
                WindowInsetsCompat.Type.systemBars() or WindowInsetsCompat.Type.displayCutout()
            )
            binding.navigationRail.updatePadding(left = bars.left, top = bars.top, bottom = bars.bottom)
            binding.fragmentContainer.updatePadding(top = bars.top, right = bars.right, bottom = bars.bottom)
            WindowInsetsCompat.CONSUMED
        }
    }

    private fun setupRailHeader() {
        val logo = binding.navigationRail.headerView?.findViewById<ImageView>(R.id.railLogo) ?: return
        logo.contentDescription = BuildConfig.DEFAULT_SERVER_NAME
        if (BuildConfig.DEFAULT_SERVER_LOGO_URL.isNotBlank()) {
            ServerLogo.bind(logo, BuildConfig.DEFAULT_SERVER_NAME, BuildConfig.DEFAULT_SERVER_LOGO_URL)
        } else {
            logo.setImageResource(R.drawable.genesis_logo)
        }
    }

    private val tabTags = listOf("tab_home", "tab_settings")

    private fun setupTabs(fresh: Boolean) {
        if (fresh) {
            val tx = supportFragmentManager.beginTransaction().setReorderingAllowed(true)
            listOf(GenesisHomeFragment(), SettingsFragment()).forEachIndexed { i, f ->
                tx.add(R.id.fragmentContainer, f, tabTags[i])
            }
            tx.commitNow()
        }
        val ids = listOf(R.id.nav_home, R.id.nav_settings)
        showTab(ids.indexOf(binding.navigationRail.selectedItemId).coerceAtLeast(0))
    }

    private fun showTab(index: Int) {
        val tx = supportFragmentManager.beginTransaction().setReorderingAllowed(true)
        tabTags.forEachIndexed { i, tag ->
            val f = supportFragmentManager.findFragmentByTag(tag) ?: return@forEachIndexed
            if (i == index) {
                tx.show(f).setMaxLifecycle(f, Lifecycle.State.RESUMED)
            } else {
                tx.hide(f).setMaxLifecycle(f, Lifecycle.State.STARTED)
            }
        }
        tx.commit()
    }

    fun openSettingsTab() {
        binding.navigationRail.selectedItemId = R.id.nav_settings
        showTab(1)
    }

    private fun setupNavigation() {
        binding.navigationRail.setOnItemSelectedListener { item ->
            when (item.itemId) {
                R.id.nav_home -> { showTab(0); true }
                R.id.nav_settings -> { showTab(1); true }
                else -> false
            }
        }
    }

    /**
     * Launches the dedicated Project Genesis server session.
     */
    fun launchGenesisServer() {
        val intent = Intent(this, EngineHostActivity::class.java).apply {
            putExtra(GameActivity.EXTRA_SERVER_IP, BuildConfig.DEFAULT_SERVER_IP)
            putExtra(GameActivity.EXTRA_SERVER_PORT, BuildConfig.DEFAULT_SERVER_PORT)
            putExtra(GameActivity.EXTRA_SERVER_NAME, BuildConfig.DEFAULT_SERVER_NAME)
        }
        startActivity(intent)
    }

    /**
     * Kept for compatibility; always routes to the dedicated Project Genesis server.
     */
    @Suppress("UNUSED_PARAMETER")
    fun launchGame(server: ServerInfo) {
        launchGenesisServer()
    }
}
