package com.re3.game;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;

import com.re3.game.databinding.ActivityMainBinding;

/**
 * Unused stub left over from the Android Studio "Native C++" project
 * template (not part of the intent-filter, GameActivity is the actual
 * launcher -- see AndroidManifest.xml and GameActivity's class doc). Kept
 * only so the generated ActivityMainBinding / activity_main.xml layout
 * aren't dead weight.
 */
public class MainActivity extends AppCompatActivity {

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
    }
}
