package com.worldrpg.neonlauncher.ui

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.Fragment
import androidx.viewpager2.adapter.FragmentStateAdapter
import com.worldrpg.neonlauncher.BuildConfig
import com.worldrpg.neonlauncher.R
import com.worldrpg.neonlauncher.databinding.ActivityMainBinding
import com.worldrpg.neonlauncher.game.EngineHostActivity
import com.worldrpg.neonlauncher.game.GameActivity
import com.worldrpg.neonlauncher.network.ServerInfo

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        setupViewPager()
        setupBottomNavigation()
    }

    private fun setupViewPager() {
        binding.viewPager.adapter = object : FragmentStateAdapter(supportFragmentManager, lifecycle) {
            override fun getItemCount(): Int = 2

            override fun createFragment(position: Int): Fragment {
                return when (position) {
                    0 -> GenesisHomeFragment()
                    1 -> SettingsFragment()
                    else -> GenesisHomeFragment()
                }
            }
        }

        // Disable swiping to prevent accidental tab changes
        binding.viewPager.isUserInputEnabled = false
    }

    private fun setupBottomNavigation() {
        binding.bottomNavigation.setOnItemSelectedListener { item ->
            when (item.itemId) {
                R.id.nav_home -> {
                    binding.viewPager.currentItem = 0
                    true
                }
                R.id.nav_settings -> {
                    binding.viewPager.currentItem = 1
                    true
                }
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
