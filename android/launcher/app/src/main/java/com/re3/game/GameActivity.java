package com.re3.game;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.provider.Settings;
import android.widget.Toast;

import android.view.ViewGroup;
import android.view.WindowManager;

import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Actividad principal del juego (única actividad -- ver AndroidManifest.xml).
 *
 * Le indica al motor nativo (via el argumento "--dir", que ya soporta en
 * todas las plataformas) que busque los archivos del juego (data/, models/,
 * audio/, anim/, etc.) en una carpeta simple y visible en cualquier
 * explorador de archivos: el almacenamiento interno del dispositivo,
 * carpeta "re3GTA" (por ej. "Almacenamiento interno/re3GTA").
 *
 * Evita depender de Android/data/com.re3.game/files o de un OBB, que la
 * mayoría de usuarios no sabe ubicar ni gestionar.
 *
 * NOTA sobre el permiso "Todos los archivos": una versión anterior de esto
 * usaba una segunda actividad (MainActivity) solo para pedir el permiso
 * antes de arrancar esta -- se sacó porque esa transición entre actividades
 * (GameActivity arrancada como una activity NUEVA, no la misma instancia)
 * hace que SDL2 se cuelgue esperando a que la ventana quede lista en
 * Android 15/16 (se ve un parpadeo de foco justo ahí en logcat, y el
 * handshake nativo de creación de ventana de SDL nunca vuelve). Por eso el
 * chequeo de permiso vuelve a estar acá, en la ÚNICA actividad, con
 * super.onCreate() llamado SIEMPRE primero (a diferencia del bug original:
 * un onCreate() que retorna sin llamar a super.onCreate() cuando falta el
 * permiso hace que Android tire SuperNotCalledException, un crash real
 * verificado en este dispositivo). Si falta el permiso, el motor nativo
 * arranca brevemente contra una carpeta vacía/inaccesible y la actividad se
 * cierra enseguida con finish() -- no es lindo, pero no cuelga ni crashea
 * feo, y el usuario reabre la app ya con el permiso concedido.
 */
public class GameActivity extends SDLActivity {

    /**
     * Nombre de la carpeta con los archivos del juego, en la raíz del
     * almacenamiento interno. Viene de BuildConfig (un valor distinto por
     * product flavor -- "re3GTA" para el juego base, "re3GTA_FWR" para la
     * variante del mod Frosted Winter Remastered, ver app/build.gradle) para
     * que ambas apps convivan instaladas sin pisarse los archivos.
     */
    public static final String GAME_FOLDER_NAME = BuildConfig.GAME_FOLDER_NAME;

    private static final int REQUEST_LEGACY_STORAGE_PERMISSION = 1001;

    public static File getGameDir() {
        return new File(Environment.getExternalStorageDirectory(), GAME_FOLDER_NAME);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        File gameDir = getGameDir();
        if (!gameDir.exists()) {
            gameDir.mkdirs();
        }

        super.onCreate(savedInstanceState);

        if (!hasFullStorageAccess()) {
            requestFullStorageAccess();
            finish();
            return;
        }

        ensureGameControllerDb(gameDir);

        // Draw the game (and our touch controls) behind the camera cutout
        // consistently in landscape, instead of the system letterboxing
        // around it -- our TouchControlsView reads the actual safe-area
        // insets and steers controls clear of it either way, but the game
        // view itself should still fill the whole screen.
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            getWindow().getAttributes().layoutInDisplayCutoutMode =
                    WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
        }

        // libre3.so is already loaded at this point (loadLibraries(), called
        // from within super.onCreate(), just did it), so TouchControlsView's
        // native methods resolve fine without a separate System.loadLibrary.
        mLayout.addView(new TouchControlsView(this),
                new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
    }

    @Override
    protected String[] getArguments() {
        return new String[]{"--dir", getGameDir().getAbsolutePath()};
    }

    @Override
    protected String[] getLibraries() {
        // El target de CMake se llama "re3" (ver /CMakeLists.txt, set(EXECUTABLE re3)),
        // por lo que el binario resultante es "libre3.so". El nombre acá debe
        // coincidir EXACTO: Android es case-sensitive y System.loadLibrary con el
        // case equivocado falla silenciosamente sin cargar nada.
        return new String[]{"SDL2", "openal", "mpg123", "re3"};
    }

    /** Called via JNI from CJavaWrapper::ExitGame() (skel/android/JavaWrapper.cpp). */
    public void exitGame() {
        finishAndRemoveTask();
    }

    /**
     * gamecontrollerdb.txt es la base de mapeos de SDL2 para reconocer
     * mandos físicos (sobre todo Bluetooth genéricos que Android no
     * identifica como "GameController" de por sí -- sin esta base, SDL2 no
     * sabe qué botón físico corresponde a cada botón lógico, D-Pad
     * incluido). No es contenido del juego -- es la base de datos
     * open-source de SDL (https://github.com/mdqinc/SDL_GameControllerDB,
     * licencia MIT), así que la empaquetamos en la app y la copiamos sola
     * a la carpeta del juego si todavía no está, en vez de depender de que
     * el usuario la traiga junto con sus propios archivos.
     */
    private void ensureGameControllerDb(File gameDir) {
        File dst = new File(gameDir, "gamecontrollerdb.txt");
        if (dst.exists()) return;
        try (InputStream in = getAssets().open("gamecontrollerdb.txt");
             OutputStream out = new FileOutputStream(dst)) {
            byte[] buf = new byte[8192];
            int n;
            while ((n = in.read(buf)) > 0) out.write(buf, 0, n);
        } catch (IOException e) {
            // No crítico: sin esto, algunos mandos genéricos no se
            // reconocen, pero el juego sigue funcionando con los que sí
            // (y con los controles táctiles) igual.
        }
    }

    private boolean hasFullStorageAccess() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            return Environment.isExternalStorageManager();
        }
        return ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                == PackageManager.PERMISSION_GRANTED;
    }

    private void requestFullStorageAccess() {
        Toast.makeText(this,
                "Concedé el permiso \"Todos los archivos\" y volvé a abrir el juego.\n"
                        + "Copiá los archivos del juego en: Almacenamiento interno/" + GAME_FOLDER_NAME,
                Toast.LENGTH_LONG).show();

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            try {
                Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION,
                        Uri.parse("package:" + getPackageName()));
                startActivity(intent);
            } catch (Exception e) {
                startActivity(new Intent(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION));
            }
        } else {
            ActivityCompat.requestPermissions(this,
                    new String[]{Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE},
                    REQUEST_LEGACY_STORAGE_PERMISSION);
        }
    }
}
