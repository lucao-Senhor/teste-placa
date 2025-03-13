//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                       _              //
//               _    _       _      _        _     _   _   _    _   _   _        _   _  _   _          //
//           |  | |  |_| |\| |_| |\ |_|   |\ |_|   |_| |_| | |  |   |_| |_| |\/| |_| |  |_| | |   /|    //    
//         |_|  |_|  |\  | | | | |/ | |   |/ | |   |   |\  |_|  |_| |\  | | |  | | | |_ | | |_|   _|_   //
//                                                                                       /              //
//////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
*   Programa básico para controle da placa durante a Jornada da Programação 1
*   Permite o controle das entradas e saídas digitais, entradas analógicas, display LCD e teclado. 
*   Cada biblioteca pode ser consultada na respectiva pasta em componentes
*   Existem algumas imagens e outros documentos na pasta Recursos
*   O código principal pode ser escrito a partir da linha 86
*/

// Área de inclusão das bibliotecas
//-----------------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "HCF_IOTEC.h"   // Vai se tornar HCF_IOTEC
#include "HCF_LCD.h"      // Vai se tornar HCF_LCD
#include "HCF_ADC.h"      // Vai se tornar HCF_ADC
#include "HCF_MP.h"       // Vai se tornar HCF_MP

// Área de macros
#define IN(x) (entradas>>x)&1

// Variáveis globais
static const char *TAG = "Calculadora";
static uint8_t entradas, saidas = 0;
static char tecla = '-' ;
char escrever[40];

// Variáveis para a calculadora
float num1 = 0, num2 = 0, resultado = 0;
char operador = '\0';  // Armazena o operador da operação
char display[40];

// Funções auxiliares
void limpar_tela() {
    limpar_lcd();
    escreve_lcd(1, 0, "                ");  // Limpa a linha
    escreve_lcd(2, 0, "                ");  // Limpa a linha
}

void mostrar_resultado() {
    snprintf(display, sizeof(display), "Resultado: %.2f", resultado);
    escreve_lcd(1, 0, display);
}

// Função que executa a operação
void realizar_operacao() {
    switch (operador) {
        case '+':
            resultado = num1 + num2;
            break;
        case '-':
            resultado = num1 - num2;
            break;
        case 'x':
            resultado = num1 * num2;
            break;
        case '/':
            if (num2 != 0) {
                resultado = num1 / num2;
            } else {
                resultado = 0;  // Evita divisão por zero
            }
            break;
        default:
            resultado = 0;
            break;
    }
    mostrar_resultado();
}

// Função para processar a tecla pressionada
void processar_tecla(char tecla) {
    static char input[20];
    static int idx = 0;
    
    if (tecla >= '0' && tecla <= '9') {
        // Adiciona o número ao input
        input[idx++] = tecla;
        input[idx] = '\0';  // Garante que a string esteja terminada
        snprintf(display, sizeof(display), "Entrada: %s", input);
        escreve_lcd(2, 0, display);  // Exibe a entrada atual no LCD
    } 
    else if (tecla == '+' || tecla == '-' || tecla == 'x' || tecla == '/') {
        num1 = atof(input);  // Salva o primeiro número
        operador = tecla;    // Salva o operador
        idx = 0;             // Reseta o input para o próximo número
        limpar_tela();       // Limpa a tela para o próximo número
    }
    else if (tecla == '=') {
        num2 = atof(input);  // Salva o segundo número
        realizar_operacao(); // Realiza a operação e mostra o resultado
        idx = 0;             // Reseta o input
    } 
    else if (tecla == 'C') {
        limpar_tela();       // Limpa a tela
        num1 = 0;
        num2 = 0;
        operador = '\0';
        idx = 0;
    }
}

void app_main(void) {
    // Inicializações
    escrever[39] = '\0';
    ESP_LOGI(TAG, "Iniciando...");
    ESP_LOGI(TAG, "Versão do IDF: %s", esp_get_idf_version());

    // Inicializar os IOs e teclado da placa
    iniciar_iotec();
    entradas = io_le_escreve(saidas); // Limpa as saídas e lê o estado das entradas

    // Inicializar o display LCD
    iniciar_lcd();
    escreve_lcd(1, 0, "Calculadora Básica");
    escreve_lcd(2, 0, "Aguarde...");
    
    // Inicializar o componente ADC
    esp_err_t init_result = iniciar_adc_CHX(0);
    if (init_result != ESP_OK) {
        ESP_LOGE("MAIN", "Erro ao inicializar o ADC");
    }

    // Delay inicial
    vTaskDelay(1000 / portTICK_PERIOD_MS); 
    limpar_lcd();

    // Programa principal
    while (1) {
        tecla = le_teclado();  // Lê a tecla pressionada

        if (tecla) {
            processar_tecla(tecla);  // Processa a tecla pressionada
        }

        entradas = io_le_escreve(saidas);  // Atualiza as entradas/saídas

        vTaskDelay(100 / portTICK_PERIOD_MS);  // Delay para evitar sobrecarga de CPU
    }
    
    // Limpar ADC após o uso
    adc_limpar();
}