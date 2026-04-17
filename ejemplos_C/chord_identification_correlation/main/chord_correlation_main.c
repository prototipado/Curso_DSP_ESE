/*  FFT plot

*/

/*==================[INCLUSIONS]=============================================*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"
#include <math.h>
#include "esp_adc/adc_continuous.h"
#include "esp_dsp.h"
#include "rtc_wdt.h"

/*==================[MACROS Y DEFINICIONES]=================================*/

/* No modificar */
#define FS_1K           1000                    // Frecuencia de muestreo: 1kHz
#define FS_2K           2000                    // Frecuencia de muestreo: 2kHz
#define FS_4K           4000                    // Frecuencia de muestreo: 4kHz
#define FS_8K           8000                    // Frecuencia de muestreo: 8kHz
#define FS_16K          16000                   // Frecuencia de muestreo: 16kHz
#define FS_32K          32000                   // Frecuencia de muestreo: 32kHz   
#define FS_CAL          1.22                    // Factor de calibración
#define BUFFER_SIZE     1024                    // Tamaño del buffer para ADC continuo
#define MAX_ADC         4095                    // Máximo valor ADC (12 bits)
#define VDD             3300                    // VDD es 3.3V, 3300mV
#define PCP_SIZE        12                      // Tamaño del vector PCP (12 notas)

/* Macros modificables por el usuario */
#define SAMPLE_FREQ     FS_8K                   // Frecuencia de muestreo (seleccionar una de las definidas)
#define N_SAMPLES       4096                    // Número de muestras (debe ser múltiplo de 1024)
#define ADC_CHANNEL     ADC_CHANNEL_6           // Canal del ADC
#define PLOT_WIDTH      128                     // Ancho del gráfico en caracteres
#define PLOT_HEIGHT     20                      // Alto del gráfico en caracteres
#define TAG             "FFT_ACORDES"           // Tag para los logs

/*==================[DEFINICION DE DATOS INTERNOS]============================*/

/* Variables de FreeRTOS */
TaskHandle_t fftTaskHandle = NULL;
TaskHandle_t plotTaskHandle = NULL;

adc_continuous_handle_t adc_handle = NULL;

// Señal de entrada
__attribute__((aligned(16)))
float signal[N_SAMPLES];
// Coeficientes de la ventana
__attribute__((aligned(16)))
float wind[N_SAMPLES];
// Arreglo complejo para procesamiento FFT
__attribute__((aligned(16)))
float fft[N_SAMPLES*2];

/* Variables internas para cálculos de PCP (Pitch Class Profile) */
float pcp[PCP_SIZE];
float pcp_norm[PCP_SIZE];
float pcp_norm_disp[PCP_SIZE];
float pcp_coor[1];
float pcp_sum = 0;
float pcp_threshold = 0;
#define fref 130.81                             // Frecuencia de referencia (Do3)
#define threshold  0.75                         // Umbral de correlación para detección
bool pcp_send = false;                          // Bandera para indicar detección válida


typedef enum{FFT, PCP, LOGGING, PREDICTION}display;
typedef enum{LOG_FFT, LOG_PCP, LOG_TIME}logconf;
typedef enum{MV, DB}unit;

/* Selección de unidades para FFT */
unit fft_unit = MV; 
/* Selección de visualización */
display plot_config = PREDICTION;
/* Selección de logging */
logconf log_config = LOG_TIME;


float DO[12] = {1,0,0,0,1,0,0,1,0,0,0,0};
float FA[12] = {1,0,0,0,0,1,0,0,0,1,0,0};
float RE[12] = {0,0,1,0,0,0,1,0,0,1,0,0};
float SOL[12]= {0,0,1,0,0,0,0,1,0,0,0,1};

/*==================[DEFINICION DE FUNCIONES INTERNAS]========================*/

/**
 * @brief Mapea un bin de frecuencia de la FFT a una nota de la escala cromática.
 * 
 * Calcula a qué nota musical (0-11) corresponde un índice específico de la FFT
 * basado en la frecuencia de muestreo y una frecuencia de referencia.
 * 
 * @param l Índice del bin de la FFT.
 * @param p Parámetro de nota (no utilizado en el cálculo actual).
 * @return int Índice de la nota (0: Do, 1: Do#, ..., 11: Si). Retorna -1 si l es 0.
 */
int obtener_nota_musical(uint16_t l, uint16_t p){
	if (l == 0){return -1;}
	float fs = SAMPLE_FREQ;
	float num = fs * l;
	float den = N_SAMPLES * fref;
	float div = num/den;
	float lg = log2f(div);
	lg = round(12*lg);
    return ((int)lg % 12);
}

/**
 * @brief Calcula el Perfil de Clase de Altura (PCP) desde los datos de magnitud.
 * 
 * Esta función agrupa la energía de la FFT en las 12 notas de la escala cromática,
 * normaliza el vector resultante y determina si hay una detección significativa.
 * 
 * @param data Puntero a los datos de magnitud de la FFT.
 * @param N Número de bins de la FFT.
 */
void calcular_pcp(float* data, uint16_t N){
    pcp_sum = 0;
    for (int p = 0 ; p < PCP_SIZE ; p++){
        pcp[p] = 0;
    }
    
    // Acumulación de energía por nota musical
    for (int p = 0 ; p < PCP_SIZE ; p++) {
        for (int l = 0 ; l < N ; l++) {
            if (p == obtener_nota_musical(l, p)){
                pcp[p] += labs((int)data[l]) * labs((int)data[l]);
            }
        }
        pcp_sum += pcp[p];
    }
    
    // Normalización y detección de umbral
    for (int p = 0 ; p < PCP_SIZE ; p++) {
        pcp_norm[p] = (pcp[p]/pcp_sum);
        pcp_norm_disp[p] = pcp_norm[p] * 100.0;
        pcp_threshold += pcp_norm[p];
    }
    if(pcp_threshold > 0.1){
        pcp_send = true;
        gpio_set_level(GPIO_NUM_2, 1);
    }
}


/**
 * @brief Tarea encargada de procesar la FFT.
 * 
 * Lee los datos del ADC, aplica una ventana de Hann, calcula la FFT,
 * obtiene la magnitud y finalmente llama al cálculo del PCP.
 * 
 * @param arg Argumentos de la tarea (no utilizados).
 */
void tarea_procesamiento_fft(void *arg){
    esp_err_t ret;
    uint32_t ret_num = 0;
    uint8_t data[BUFFER_SIZE*SOC_ADC_DIGI_RESULT_BYTES];
    static uint16_t index = 0;
    const uint8_t STEP = FS_32K/SAMPLE_FREQ;
    
    ESP_LOGI(TAG, "*** Iniciando Tarea FFT ***");
    
    while(1){  
        /* Espera la notificación de que hay nuevos datos en el ADC */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        /* Lee el buffer del ADC y formatea los datos */
        ret = adc_continuous_read(adc_handle, data, BUFFER_SIZE*SOC_ADC_DIGI_RESULT_BYTES, &ret_num, 0);
        if (ret == ESP_OK) {
            for (int i = 0; i < BUFFER_SIZE/STEP; i++) {
                adc_digi_output_data_t *p = (void*)&data[i*SOC_ADC_DIGI_RESULT_BYTES*STEP];  
                signal[index+i] = (float)(p->type1.data) * (float)VDD / (float)MAX_ADC;
            }
        } else if (ret == ESP_ERR_TIMEOUT) {
                // Si el API retorna timeout, intentamos de nuevo en la siguiente iteración
                break;
            }
        index += (BUFFER_SIZE/STEP);
        
        if (index >= N_SAMPLES){
            // Limpia la parte imaginaria de la señal compleja
            memset(fft, 0, N_SAMPLES*SOC_ADC_DIGI_RESULT_BYTES*sizeof(float));
            // Multiplica el arreglo de entrada con la ventana y almacena como parte real
            dsps_mul_f32(signal, wind, fft, N_SAMPLES, 1, 1, 2);    
            // Calcula la FFT  
            dsps_fft2r_fc32(fft, N_SAMPLES);
            // Reordenamiento de bits (Bit reverse)
            dsps_bit_rev_fc32(fft, N_SAMPLES);
            // Convierte un vector complejo a dos vectores reales
            dsps_cplx2reC_fc32(fft, N_SAMPLES);
            
            switch(fft_unit){
                case MV:
                    // Calcula la magnitud de la FFT en mV
                    for (int j = 0; j < N_SAMPLES; j++){
                        fft[j] = 2*(sqrt(fft[j*2+0]*fft[j*2+0] + fft[j*2+1]*fft[j*2+1])) / (N_SAMPLES/2);
                    }
                    fft[0] = fft[0] / 2;
                    break;
                case DB:
                    // Calcula la magnitud de la FFT en dB
                    for (int i = 0 ; i < N_SAMPLES ; i++) {
    	                fft[i] = 10 * log10f((fft[i * 2 + 0] * fft[i * 2 + 0] + fft[i * 2 + 1] * fft[i * 2 + 1])/N_SAMPLES);
                    }
                    break;
            }
            
            // Calcula el PCP (Pitch Class Profile) para identificación de acordes
            calcular_pcp(fft, N_SAMPLES/2);
            
            /* Notifica a la tarea de visualización */
            xTaskNotifyGive(plotTaskHandle);
            index = 0;
        }
    }
}

/**
 * @brief Tarea encargada de la visualización de datos.
 * 
 * Muestra por consola los resultados de la FFT, del PCP o de la predicción
 * de acordes según la configuración seleccionada.
 * 
 * @param arg Argumentos de la tarea (no utilizados).
 */
void tarea_visualizacion(void *arg){
    
    ESP_LOGI(TAG, "*** Iniciando Tarea de Visualización ***");
    while(1){
        /* Espera la notificación de que el procesamiento está listo para mostrar */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  
        switch(plot_config){
        case FFT:
            /* Graficar FFT */
            dsps_view(fft, N_SAMPLES/2, PLOT_WIDTH, PLOT_HEIGHT, -20, 100, '*');
            ESP_LOGI("Visualizacion", "Fs: %dHz", SAMPLE_FREQ);
            break;
        case PCP:
            /* Graficar Perfil de Clase de Altura (PCP) */
             dsps_view(pcp_norm_disp, PCP_SIZE, PLOT_WIDTH, PLOT_HEIGHT,  0, 100, '_');
             ESP_LOGI("Visualizacion", "Fs: %dHz", SAMPLE_FREQ);
             break;
        case LOGGING:
            /* Registro de Datos (Logging) */
            if(pcp_send){
                switch(log_config){
                    case LOG_PCP:
                        for (int j = 0; j < PCP_SIZE-1; j++){
                            printf("%.2f,", pcp_norm[j]);
                        }
                        printf("%.2f\r\n", pcp_norm[PCP_SIZE-1]);
                        break;
                    case LOG_TIME:
                        for (int j = 0; j < N_SAMPLES-1; j++){
                            printf("%.2f,", signal[j]);
                        }       
                        printf("%.2f\r\n", signal[N_SAMPLES-1]);
                        break;
                    case LOG_FFT:
                        for (int j = 0; j < N_SAMPLES/2-1; j++){
                            printf("%.2f,", fft[j]);
                        }       
                        printf("%.2f\r\n", fft[N_SAMPLES/2-1]);
                        break;
                }
                
                pcp_send = false;
            }
            break;
        case PREDICTION:
            /* Predicción e Inferencia del Modelo */
            if(pcp_send){
                
                 dsps_dotprod_f32(pcp_norm, DO, pcp_coor, PCP_SIZE);
                 if(pcp_coor[0] > threshold){
                    printf("Acorde: DO -> Correlación %.2f %%\r\n", pcp_coor[0]*100);
                 }
                 dsps_dotprod_f32(pcp_norm,  FA, pcp_coor, PCP_SIZE);
                 if(pcp_coor[0] > threshold){
                    printf("Acorde: FA -> Correlación %.2f %%\r\n", pcp_coor[0]*100);
                 }
                 dsps_dotprod_f32(pcp_norm,  RE, pcp_coor, PCP_SIZE);
                 if(pcp_coor[0] > threshold){
                    printf("Acorde: RE -> Correlación %.2f %%\r\n", pcp_coor[0]*100);
                 }
                 dsps_dotprod_f32(pcp_norm,  SOL, pcp_coor, PCP_SIZE);
                 if(pcp_coor[0] > threshold){
                    printf("Acorde: SOL -> Correlación %.2f %%\r\n", pcp_coor[0]*100);
                 }
                 pcp_send = false;
                 gpio_set_level(GPIO_NUM_2, 0);
                
            }
            break;
        }
    }
}

/**
 * @brief Callback invocado cuando el ADC termina una conversión por hardware.
 * 
 * Notifica a la tarea de procesamiento de la FFT que hay nuevos datos disponibles.
 * 
 * @param handle Handle del ADC continuo.
 * @param edata Datos del evento.
 * @param user_data Datos de usuario adicionales.
 * @return bool Retorna true si se requiere un cambio de contexto (yield).
 */
static bool IRAM_ATTR callback_adc_terminado(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data){
    BaseType_t mustYield = pdFALSE;
    // Notifica que el driver ADC continuo ha completado el número requerido de conversiones
    vTaskNotifyGiveFromISR(fftTaskHandle, &mustYield);

    return (mustYield == pdTRUE);
}

/**
 * @brief Función principal de la aplicación.
 * 
 * Configura los periféricos (GPIO, UART, ADC), inicializa el procesamiento de señal (FFT, ventana)
 * y lanza las tareas de FreeRTOS.
 */
void app_main(void){
    /* Configuración de GPIO2 para monitoreo de tiempos de proceso */    
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;      // Deshabilitar interrupción
    io_conf.mode = GPIO_MODE_OUTPUT;            // Configurar como salida   
    io_conf.pin_bit_mask = 1ULL << GPIO_NUM_2;  // Máscara de bit para el pin seleccionado
    io_conf.pull_down_en = 0;                   // Deshabilitar pull-down
    io_conf.pull_up_en = 0;                     // Deshabilitar pull-up
    gpio_config(&io_conf);                      
    ESP_LOGI(TAG, "GPIO Configurado");
    
    /* Configuración de la UART para comunicación serie */
    uart_config_t uart_config = {
        .baud_rate = 921600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_0, 1024 * 2, 0, 0, NULL, 0);
    ESP_LOGI(TAG, "UART Configurada");

    /* Configuración del ADC en modo continuo */
    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = 2*BUFFER_SIZE*SOC_ADC_DIGI_RESULT_BYTES,      // Espacio máximo para resultados en bytes
        .conv_frame_size = BUFFER_SIZE*SOC_ADC_DIGI_RESULT_BYTES,           // Tamaño de la trama de conversión
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &adc_handle));
    
    adc_continuous_config_t dig_cfg = {
        .pattern_num = 1,                       // Número de canales ADC a utilizar 
        .sample_freq_hz = (int)(FS_32K*FS_CAL), // Frecuencia de muestreo esperada del hardware
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,    // Utilizar solo ADC1
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE1, // Formato de salida de los datos
    };
    
    adc_digi_pattern_config_t adc_pattern = {
        .atten = ADC_ATTEN_DB_2_5,               // Atenuación del ADC
        .channel = ADC_CHANNEL,                 // Canal correspondiente al pin IO
        .unit = ADC_UNIT_1,                     // Unidad ADC subordinada
        .bit_width = SOC_ADC_DIGI_MAX_BITWIDTH, // Ancho de bits del resultado crudo
    };
    dig_cfg.adc_pattern = &adc_pattern;
    ESP_ERROR_CHECK(adc_continuous_config(adc_handle, &dig_cfg));
    
    adc_continuous_evt_cbs_t adc_cb = {
        .on_conv_done = callback_adc_terminado,  // Asignar callback para evento de conversión terminada
    };
    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(adc_handle, &adc_cb, NULL));
    ESP_LOGI(TAG, "ADC Configurado");

    /* Inicialización de la librería de DSP y la FFT */
    esp_err_t ret = dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
    if (ret != ESP_OK){
        ESP_LOGE(TAG, "No fue posible inicializar la FFT. Error = %i", ret);
        return;
    }
    // Generar ventana de Hann para suavizar la señal de entrada
    dsps_wind_hann_f32(wind, N_SAMPLES);

    ESP_LOGI(TAG, "*** Iniciando Ejemplo ***");

    /* Creación de tareas de FreeRTOS */
    xTaskCreatePinnedToCore(tarea_procesamiento_fft, "fftTask", 8192, NULL, 10, &fftTaskHandle, 0);
    xTaskCreatePinnedToCore(tarea_visualizacion, "plotTask", 2048, NULL, 8, &plotTaskHandle, 1);

    /* Iniciar captación del ADC */
    ESP_ERROR_CHECK(adc_continuous_start(adc_handle));
}