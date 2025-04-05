/**
 * @file nelito.h
 * @brief Header da biblioteca de apoio ao NEEEILIT2025
 */

#ifndef MOTOR_SENSOR_LIB_H
#define MOTOR_SENSOR_LIB_H

#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "hardware/timer.h"
#include "pio_encoder.h"
#include <atomic>

#ifdef __cplusplus
extern "C" {
#endif

// Pinos do enables
#define EN1A 9   //Enable motor esquerdo A
#define EN1B 1   //Enable motor esquerdo B
#define EN2A 15    //Enable motor direito A
#define EN2B 14    //Enable motor direito B

// PWM vars
extern const uint16_t PWM_WRAP;  // 2^15 - 1 (15-bit de resolução) signed int16
extern const float CLOCK_DIV;    // 125 MHz clock rate

// Atomic variables for inter-core access
extern std::atomic<bool> ENCODER_AVALIABLE;
extern std::atomic<int> velEsquerdo;
extern std::atomic<int> velDireito;

// Macros de acesso direto para ler os contadores dos encoders
#define TICKesq velEsquerdo.load(std::memory_order_relaxed)
#define TICKdir velDireito.load(std::memory_order_relaxed)

// Taxa de amostragem dos encoders ( microsegundos)
extern uint16_t ENCODER_SAMPLE_RATE;

// Data structures
typedef struct {
    int16_t esq;  // Valor da esquerda
    int16_t dir;  // Valor da direita
} Par;

union Packet {
    Par pwm;         // Acesso da struct
    uint32_t raw;    // Acesso aos 32 bits
};

/**
 * @brief Inicializa controlo dos motores e task no segundo core
 * 
 */
void motor_init(void);

/**
 * @brief Defenir velocidade dos motores
 * 
 * @param esq Velocidade motor esquerdo (-2^15 a 2^15)
 * @param dir Right motor speed (-2^15 a 2^15)
 */
void set_motor(int esq, int dir);

/**
 * @brief Declara e inicializa encoders nas PIOs. O Canal A e B necessitam de estar em pinos seguidos para o seu bom funcionamento
 * 
 * @param aEsq Pino do Canal A do Encoder Esquerdo
 * @param aDir Pino do Canal A do Encoder Direito
 * @return True se inicializar com sucesso senão retorna falso
 */
bool encoder_init(int aEsq, int aDir);

/**
 * @brief Defenir taxa de amostragem dos encoders. Default=25
 * 
 * @param x Taxa de amostragem em milisegundos
 */
void set_sample_rate(int x);

/**
 * @brief Inicializar a NeeecSense
 * 
 * @param s1 GPIO do sensor 1
 * @param s2 GPIO do sensor 2
 * @param s3 GPIO do sensor 3
 * @param s4 GPIO do sensor 4
 * @param s5 GPIO do sensor 5
 */
void sense_init(uint s1, uint s2, uint s3, uint s4, uint s5);

/**
 * @brief Ler valor da NeeecSense e guardar em array
 * 
 * @param states Array aonde guardar valores da NeeecSense
 */
void sense_read(bool states[5]);

/**
 * @brief Ler valor da NeeecSense e guardar num byte
 * 
 * @return uint8_t 5-bit a representar os valores na NeeecSense
 */
uint8_t sense_read_bit(void);

#ifdef __cplusplus
}
#endif

#endif // nelito