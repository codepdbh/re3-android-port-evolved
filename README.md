<img src="https://github.com/GTAmodding/re3/blob/master/res/images/logo_1024.png?raw=true" alt="re3 logo" width="200">

## Intro

En este repositorio vas a encontrar el código fuente completamente reversado de GTA III, con un port a Android agregado en este fork.

re3 (el proyecto original, [GTAmodding/re3](https://github.com/GTAmodding/re3)) fue dado de baja por DMCA en 2021; este fork parte de [mrxenginner/re3](https://github.com/mrxenginner/re3), un mirror preservado del código.

Funciona en Windows, Linux, MacOS, FreeBSD y **Android** (este fork), en x86, amd64, arm y arm64.\
El renderizado lo maneja [librw](https://github.com/aap/librw) (D3D9, OpenGL 2.1+, OpenGL ES 2.0+), la reimplementación open-source de RenderWare.\
El audio funciona con OpenAL.

## 📱 Sobre este fork: re3-android-port-evolved

Este fork es hermano de [revc-android-port-evolved](https://github.com/codepdbh/revc-android-port-evolved) (el mismo trabajo, para GTA Vice City) — misma metodología, mismo autor.

A diferencia de reVC, este repositorio (re3) **no tenía ningún soporte de Android previo, en ningún lado** — ni siquiera un esqueleto parcial. Todo lo que hay en `android/`, `cmake/android/`, `src/skel/android/` y `src/skel/sdl2/` fue portado desde cero, usando revc-android-port-evolved como plantilla y adaptando cada pieza línea por línea contra el código real de este repo (que en varios archivos compartidos seguía siendo GLFW-only, sin la rama SDL2 que reVC ya tenía).

**Estado actual: build funcional, arranque sin crashear, controles táctiles operativos — con un cuelgue nativo pendiente de resolver.** Ver la sección de pendientes más abajo.

### ✅ Arreglos aplicados

**Build / compilación**
- `android/launcher/`: proyecto Gradle + CMake completo desde cero (no existía).
- `cmake/android/AndroidConfig.cmake`: conecta los binarios prebuilt de SDL2/OpenAL/mpg123 por ABI a los mismos `find_package()` que usan los builds de escritorio vía Conan. También fuerza `LIBRW_PLATFORM=GL3` y `LIBRW_GL3_GFXLIB=SDL2` en librw (por defecto no tienen equivalente para Android).
- `vendor/librw` re-apuntado a [codepdbh/librw-re3-android](https://github.com/codepdbh/librw-re3-android) (rama `android-sdl2-fixes`): librw nunca había compilado su backend SDL2 para Android — `find_package(OpenGL)` no tiene rama Android (linkeado directo a GLESv3+EGL en su lugar) y `DEVICEGETNUMSUBSYSTEMS`/`DEVICEGETCURRENTSUBSYSTEM`/etc. estaban sin implementar para SDL2 (`assert(0)` en cada arranque — `psSelectDevice()` los consulta siempre).
- `src/CMakeLists.txt`: lib compartida (no ejecutable) en Android, `ANDROID_x32` por ABI, todos los defines que el código espera (`ANDROID`, `RW_GL3`, `LIBRW_SDL2`, etc.).
- `src/skel/crossplatform.h`, `core/Pad.cpp`, `core/Frontend.cpp`, `core/ControllerConfig.cpp`: a diferencia de reVC, ninguno de estos tenía una rama `LIBRW_SDL2` — usaban GLFW sin condicional (`GLFWwindow*` a secas, `GLFW_MOUSE_BUTTON_*`, `glfwGetCursorPos()` directo). Agregadas las ramas SDL2 que faltaban.

**Assets / almacenamiento**
- `GameActivity.getArguments()` pasa `--dir` apuntando a `Almacenamiento interno/re3GTA`, pide "Todos los archivos" (`MANAGE_EXTERNAL_STORAGE`) en runtime si falta.
- `main()` (sdl2.cpp) hace `chdir()` a esa carpeta apenas parsea `--dir` — el motor entero resuelve rutas relativas contra el directorio de trabajo, que en Android no tiene ningún valor útil por defecto. Tiene que pasar ahí específicamente: `CGame::InitialiseOnceBeforeRW()` (streaming del .img) corre antes que `psInitialize()`, que era donde vivía este chdir originalmente.
- `casepath()` (crossplatform.cpp, el resolvedor de rutas case-insensitive): abría `opendir("/")` para cualquier ruta absoluta — Android no permite listar la raíz del filesystem aunque tengas el permiso de almacenamiento (ese permiso cubre `/storage/emulated/0/...`, no la raíz). Crasheaba (`FORTIFY: readdir: null DIR*`) en el primer archivo que tocaba CFileMgr. Ahora recorta el prefijo conocido de `StorageRootBuffer` primero, así arranca desde `.` (el directorio de trabajo, sí accesible).
- `re3.ini` (mINI::INIFile) era un global no-lazy construido con ruta relativa antes de que `--dir` se parsee — ahora es lazy (`GetIniFile()`), se construye recién en el primer uso real.
- `USE_UNNAMED_SEM` ahora también cubre Android (antes solo Switch) — bionic no soporta semáforos con nombre (`sem_open()` siempre falla), abortaba en el primer streaming thread.
- Bug de concatenación sin `/` en la ruta de `gamecontrollerdb.txt` (mismo tipo de bug que reVC tenía en su logger).

**Controles táctiles**
Port completo de la arquitectura de reVC (`TouchControls.h/.cpp`, `TouchControlsView.java`, `CaptureTouchPad()`/`CapturePad()`), pero con la semántica de botones re-verificada contra los bindings reales de GTA III (no asumida igual a reVC):
- III no tiene celular — el botón L1 en el layout "a pie" es "centrar cámara detrás del jugador" (CAM), no "atender teléfono" como en reVC.
- `MapIdToButtonId()` en este repo switchea sobre el orden ordinal de GLFW (nunca tuvo backend SDL2 propio), no sobre `SDL_CONTROLLER_BUTTON_*` como en reVC. `CaptureTouchPad()`/`CapturePad()` usan un enum `GAME_BTN_*` que respeta ese orden GLFW en vez de reusar los índices SDL2 de reVC directamente.
- `GlfwJoyState` acá no tiene sensibilidad/deadzone por eje ni buttons como array fijo (es un puntero, igual que en `glfw.cpp`) — se sigue la convención de `glfw.cpp` (deadzone fijo 0.3f, sin multiplicador) en vez de portar los campos de reVC.
- `InitDefaultControlConfigJoyPad(16)` llamado una vez al arrancar si hace falta (mismo fix que reVC necesitó, mismo motivo: solo se dispara con un gamepad físico conectado).

**Radar / HUD**
- Radar reposicionado arriba-izquierda (`RADAR_TOP`) en `Radar.cpp` y `Hud.cpp` — el stick de movimiento está abajo-izquierda y lo tapaba.

**Apuntado / target-lock**
- `CCamera::m_bUseMouse3rdPerson` (default `true`, "cámara con mouse en 3ra persona") bloqueaba `FindWeaponLockOnTarget()` por completo — sin mouse en Android, quedaba permanentemente en `true`. Arreglo puntual (variable local, solo dentro de `ProcessPlayerWeapon()`, solo Android) sin tocar la bandera global (también maneja la cámara con stick en otros archivos).

### 📋 Pendientes

- **🔴 Cuelgue nativo sin resolver (bloqueante):** `RsEventHandler(rsRWINITIALIZE, ...)` nunca retorna en al menos un dispositivo probado (Adreno clase Snapdragon 8 Elite, Android 16) — el hilo queda vivo y activo (confirmado por `/proc/<pid>/task/<tid>/stat`, sin crash). Cada etapa DENTRO de esa llamada (creación de ventana/contexto EGL/GL, carga de `glad`, `initOpenGL()`) fue confirmada individualmente exitosa vía logging temporal (ya removido) — el cuelgue está específicamente en el camino de retorno hacia arriba, sin llamadas propias de por medio. No se pudo conseguir un backtrace nativo del hilo trabado sin root (`debuggerd` lo requiere en este dispositivo). Antes de esto: build limpio, instala, arranca sin crashear, controles táctiles renderizan correctamente (ver captura en el PR).
- Sin probar en otro dispositivo/GPU — no se sabe si el cuelgue es específico de este Adreno/Android 16 o más general.
- R3 (mirar atrás / alternar sumisiones) no tiene botón táctil propio en el layout actual (mismo gap que reVC).
- Sin editor de layout en vivo todavía (reVC sí lo tiene) — se podría portar directamente.
- Libs vendored (SDL2/OpenAL/mpg123) no están alineadas a 16KB de página (Android 15+ lo pide para APKs nuevos) — hoy es solo una advertencia de compatibilidad, no bloquea instalación.

## Installation

- re3 requiere los assets originales del juego — necesitás tener [una copia de GTA III](https://store.steampowered.com/app/12100/Grand_Theft_Auto_III/) legítima.
- **Android**: instalá el APK (ver Releases), copiá los archivos del juego a `Almacenamiento interno/re3GTA`, concedé el permiso "Todos los archivos" cuando se pida.
- **PC**: compilá desde código fuente (ver más abajo) o descargá un build de [GTAmodding/re3](https://github.com/GTAmodding/re3) (el proyecto original, con builds automatizados para Windows/Linux/MacOS).

## Screenshots

![re3 2021-02-11 22-57-03-23](https://user-images.githubusercontent.com/1521437/107704085-fbdabd00-6cbc-11eb-8406-8951a80ccb16.png)
![re3 2021-02-11 22-43-44-98](https://user-images.githubusercontent.com/1521437/107703339-cbdeea00-6cbb-11eb-8f0b-07daa105d470.png)

## Improvements

Los mismos cambios y mejoras del re3 original (configurables en `core/config.h`):

* Muchos bugs grandes y chicos arreglados
* Archivos de usuario (guardados y configuración) en la carpeta raíz del juego
* Configuración en `re3.ini` en vez de `gta3.set`
* Menú de debug (Ctrl-M)
* Cámara de debug (Ctrl-B)
* Cámara rotable
* Soporte XInput (Windows)
* Sin pantallas de carga entre islas
* Soporte de peds "skinned" (modelos de Xbox/Mobile)
* Widescreen (HUD, menú y FOV correctamente escalados)
* MatFX, alpha test, partículas, iluminación y lluvia estilo PS2/Xbox
* Menú con mapa, más opciones, configuración de controlador

## Building from Source

### Android

Requisitos: Android SDK + NDK 27.2.12479018, CMake 3.22.1 (instalables vía `sdkmanager`).

```
git clone --recurse-submodules https://github.com/codepdbh/re3-android-port-evolved.git
cd re3-android-port-evolved/android/launcher
```

Creá `android/launcher/local.properties` con `sdk.dir=<ruta a tu Android SDK>`.

Si tu JDK del sistema es muy nuevo para Gradle 8.11.x, agregá a `android/launcher/gradle.properties` (no lo commitees):
```
org.gradle.java.home=<ruta al JBR de Android Studio>
```

Luego:
```
./gradlew assembleDebug
```

El APK queda en `android/launcher/app/build/outputs/apk/debug/`.

### PC (Windows / Linux / MacOS / FreeBSD)

Ver las instrucciones del [re3 original](https://github.com/GTAmodding/re3#building-from-source) — sin cambios en este fork para esas plataformas.

## Contributing

Este fork sigue las mismas reglas de contribución que el re3 original — ver [CODING_STYLE.md](CODING_STYLE.md). Los cambios específicos de Android están detrás de `#if defined ANDROID` en todos lados, no afectan el resto de las plataformas.

## License

No estamos en posición de darle una licencia a este código.\
Debe usarse solo con fines educativos, de documentación y de modding.\
No fomentamos la piratería ni el uso comercial.\
Por favor mantené el trabajo derivado open source y dale crédito apropiado.
