# OBS Audio WebSocket Streamer Plugin

Este plugin de OBS escrito en C++ te permite añadir un **Filtro de Audio** a cualquier fuente en OBS. Captura el audio puro (raw PCM float) y lo transmite a todos los clientes que se conecten a través de WebSockets en el puerto configurado.

## Instalación

### Opción 1: Instalar desde la compilación local (Ya compilado por el agente)
1. Ve a la carpeta `build/Release/` de este proyecto.
2. Copia el archivo `obs-audio-websocket.dll`.
3. Pégalo en tu carpeta de plugins de OBS Studio. Por lo general se encuentra en:
   `C:\Program Files\obs-studio\obs-plugins\64bit\`
4. Reinicia OBS Studio.

### Opción 2: Compilar manualmente
Si necesitas volver a compilar el plugin en Windows:
1. Abre una terminal (Powershell o Símbolo de sistema).
2. Ve a la carpeta de este proyecto.
3. Ejecuta `cmake -S . -B build` (esto descargará las dependencias como libobs y libwebsockets).
4. Ejecuta `cmake --build build --config Release`.
5. El DLL resultante estará en `build/Release/obs-audio-websocket.dll`.

## Cómo usarlo

1. Abre **OBS Studio**.
2. Haz clic derecho en la **Fuente de Audio** (ej: tu Micrófono o Captura de salida de audio) en el panel de Mezclador de audio, y selecciona **Filtros**.
3. En la sección inferior izquierda, haz clic en el botón **+** y selecciona **Audio WebSocket Streamer**.
4. En las propiedades del filtro, puedes configurar el **Puerto** (por defecto `8080`).
5. Abre cualquier cliente WebSocket (o script) y conéctate a:
   `ws://localhost:8080` (o el puerto que hayas escogido).
6. ¡Listo! Automáticamente empezarás a recibir datos binarios de la fuente de audio (Formato Raw Float Planar).
