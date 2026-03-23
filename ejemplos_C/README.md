# Ejemplos C

Este directorio contiene diversos ejemplos (que se utilizaran a lo largo del curso) con aplicaciones de procesamiento digital de señales corriendo sobre un módulo ESP32.

## Modo de uso

> [!NOTE]
> Para trabajar con el ESP32 en este curso utilizaremos framework del fabricante [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/index.html) y como entorno de desarrollo el [Visual Studio Code](https://code.visualstudio.com/).

**1.** Instalar VSCode y el complemento de ESP-IDF siguiendo el [tutorial](https://docs.espressif.com/projects/vscode-esp-idf-extension/en/latest/).

**2.** Clonar este Repositorio en un directorio de su PC (se recomienda que en la ruta no haya carpetas con espacios ni caracteres especiales).

**3.** Para probar un ejemplo, desde `VSCode` abra la carpeta del proyecto correspondiente (`File->Open Folder...`).

![Open_folder](1.png)

​Debería ver la siguiente configuración de archivos:

![project](2.png)

> [!NOTE]
> VSCode le mostrará varios pop-up abajo a la derecha, recomendando la instalación de extensiones. Puede hacer caso omiso a la mismas.

**4.** Conecte la placa ESP32 a su PC.

**5.** Configure en VSCode el puerto COM correspondiente donde se encuentra conectado su ESP32.

![puerto_com](3.png)

**6.** Compile el ejemplo pulsando el boton ![db](https://raw.githubusercontent.com/microsoft/vscode-icons/2ca0f3225c1ecd16537107f60f109317fcfc3eb0/icons/dark/symbol-property.svg) ``ESP-IDF Build project``.

​![compile](4.png)
​
Se abrirá un nuevo terminal donde se irán mostrando las salidas del proceso de compilación. Un vez finalizado el proceso se abrirá un segundo terminal mostrando la información de memoria.

![compile2](5.png)

**7.** Programe la placa ESP32 pulsando el boton ![event](https://raw.githubusercontent.com/microsoft/vscode-icons/2ca0f3225c1ecd16537107f60f109317fcfc3eb0/icons/dark/symbol-event.svg) ``ESP-IDF Flash device``.

![flash](6.png)

**8.** Para observar los resultados de la ejecución del programa, abra un nuevo ![event](https://raw.githubusercontent.com/microsoft/vscode-icons/2ca0f3225c1ecd16537107f60f109317fcfc3eb0/icons/dark/console.svg) ``ESP-IDF Terminal`` y luego ejecute la siguiente línea:

```shell
idf.py -p "COMX" -b 921600 monitor
```

![monitor](7.png)

**9.** Para cerrar el monitor ejecute en la terminal ``Ctrl-T`` y luego ``Ctrl-X``.
