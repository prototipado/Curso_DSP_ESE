# Ejemplo FFT en tiempo real

Este ejemplo muestra como realizar adquisición en modo continuo utilizado el ADC del ESP32, para luego calcular la FFT de la señal adquirida, y visualizar la misma utilizando el terminal.

## Funcionamiento

* El ADC está configurado en modo continuo. Funciona en segundo plano de manera autónoma enviando muestras a la RAM. Cuando llega a cierto tamaño (BUFFER_SIZE), dispara ConvDoneCallback. Este callback solo despierta a la tarea `fftTask`, cediendo el CPU inmediatamente. En modo contínuo no es posible configurar el ADC para que adquiera a frecuencias menores a 20kHz, por lo que se utiliza un muestreo a 32kHz y se descartan muestras (en `fftTask`) para obtener la frecuencia de muestreo deseada.
* Core 0 acumula muestras y procesa: La tarea `fftTask` acumula bloques de datos (saltando muestras si la frecuencia de adquisicón deseada es menor a 32kHz) hasta alcanzar el total necesario para la FFT (N_SAMPLES). Solo cuando completa el buffer, realiza el consumo de CPU requierido para la FFT.
* Core 1 únicamente grafica: Cuando el Core 0 finaliza el cálculo, notifica a `plotTask` en el Core 1. Imprimir múltiples caracteres por la UART (dsps_view) puede ser lento; al mandarlo al núcleo secundario garantizamos que no se interrumpa ni ralentice la siguiente adquisición/procesamiento de audio del núcleo principal.

### Diagrama de flujo

```mermaid
graph TD
    subgraph Periférico
        A1[Hardware ADC] -->|DMA deposita muestras| A2[(Buffer Interno ADC)]
    end

    subgraph ISR Context
        A2 -->|Evento de Hardware| B1(ConvDoneCallback)
    end

    subgraph Core 0 : Procesamiento FFT
        B1 -.->|1. vTaskNotifyGiveFromISR| C1(fftTask)
        C1 --> C2[adc_continuous_read]
        C2 --> C3{¿Array 'signal' <br> tiene N_SAMPLES?}
        
        C3 -- No --> C1
        
        C3 -- Sí --> C4[Aplicar Ventana Hann]
        C4 --> C5[Ejecutar dsps_fft...]
        C5 --> C6[Calcular Magnitud y Nivelar]
    end

    subgraph Core 1 : Visualización
        C6 -.->|2. xTaskNotifyGive| D1(plotTask)
        D1 --> D2[Dibujar Gráfico dsps_view]
        D2 --> D3[Enviar por Puerto Serie / Consola]
    end
    
    style A1 fill:#e1f5fe,stroke:#01579b
    style A2 fill:#e1f5fe,stroke:#01579b
    style B1 fill:#ffcdd2,stroke:#b71c1c
    style C1 fill:#e8f5e9,stroke:#1b5e20
    style D1 fill:#fff3e0,stroke:#e65100
```

### Diagrama de Secuencia

```mermaid
sequenceDiagram
    participant HW as ADC (Hardware)
    participant ISR as ConvDoneCallback (ISR)
    participant Core0 as fftTask (Core 0)
    participant Core1 as plotTask (Core 1)
    
    note over Core0: Bloqueado en ulTaskNotifyTake()
    note over Core1: Bloqueado en ulTaskNotifyTake()

    HW->>ISR: Interrupción: Frame de ADC lleno (DMA)
    activate ISR
    note over ISR: Limpia flags y avisa
    ISR->>Core0: vTaskNotifyGiveFromISR()
    deactivate ISR
    
    activate Core0
    Core0->>HW: adc_continuous_read(): Extrae las nuevas muestras
    note over Core0: Cast a float y escala (VDD/MAX_ADC)
    
    alt ¿El índice alcanzó N_SAMPLES?
        note over Core0: Convierte, Ventana Hann,<br/>dsps_fft2r_fc32,<br/>Magnitud y Escala
        Core0->>Core1: xTaskNotifyGive()
        note over Core0: Reinicia índice a 0
    end
    deactivate Core0
    note over Core0: Vuelve a bloquearse en ulTaskNotifyTake()
    
    activate Core1
    note over Core1: Tarea Desbloqueada!
    Core1->>Core1: dsps_view() dibuja el espectro
    note over Core1: Se imprime log con la Frecuencia
    deactivate Core1
    note over Core1: Vuelve a bloquearse en ulTaskNotifyTake()
```

## Cómo usar el ejemplo

### Hardware requerido

1. ESP32
2. Módulo micrófono
2. Cables 

Conexiones:

| Micrófono | ESP32     | 
| :---:	    | :---:	    |
| +     	| VIN (5V)  | 
| G       	| GND 	    | 
| AO      	| GPIO34    | 

### Configurar el proyecto

Seleccionar la frecuencia de muestreo, eligiendo uno de los valores previamente definidos: ``FS_1K``, ``FS_2K``, etc.
```
#define SAMPLE_FREQ     FS_8K                   
```
Seleccionar número de muetras a graficar (debe ser múltiplo de 1024):
```
#define N_SAMPLES       1024                   
```

### Ejecutar la aplicación
Generar una señal de señal con excursión en el rango de tensiones soportados por el ADC (0 a 3.3V), y conectarla a la entrada del ADC (en este ejemplo está configurado el ADC1_6, que se corresponde con el GPIO34).

Luego de compilar y cargar el programa en la ESP32, para poder ver los resultados de la ejecución del programa es necesario abrir un monitor serie. Para los ejemplos de este curso utilizamos el puerto serie configurado a 921.600 baudios.
Si se desea utilizar un monitor integrado a la consola de VSCode, primero abra un nuevo ``ESP-IDF Terminal`` y luego ejecute la siguiente línea:

```
idf.py -p "COMX" -b 921600 monitor
```

Para cerrar el monitor ejecute en la terminal ``Ctrl-T`` y luego ``Ctrl-X``.

## Salida esperada
Silbando cerca del micrófono:

```
I (32696) view: Data min[124] = 1.524851, Data max[432] = 1895.813477
 ________________________________________________________________________________________________________________________________
0                                                                                                                                |
1                                                                                                                                |
2                                                                                                                                |
3                                                                                                                                |
4                                                                                                                                |
5                                                                                                                                |
6                                                                                                                                |
7                                                                                                                                |
8                                                                                                            *                   |
9*                                                                                                           *                   |
0*                                                                                                           *                   |
1*                                                                                                          **                   |
2*                                                                                                          **                   |
3*                                                                                                          **                   |
4*                                                                                                          **                   |
5*                                       *                                                                  **                   |
6*                                      **                                                                  **                   |
7*                                      **                                                                  **                   |
8 **************************************  ******************************************************************* *******************|
9                                                                                                                                |
 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567
I (32736) view: Plot: Length=512, min=0.000000, max=3300.000000
I (32746) Plot: Fs: 4000Hz
.
.
.
```