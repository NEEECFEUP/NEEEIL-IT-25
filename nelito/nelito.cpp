#include "nelito.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "hardware/timer.h"
#include "pio_encoder.h"
#include <atomic>
#define EN1A 9   //Enable motor esquerdo A
#define EN1B 1   //Enable motor esquerdo B
#define EN2A 15    //Enable motor direito A
#define EN2B 14    //Enable motor direito B
uint16_t ENCODER_SAMPLE_RATE = 25000; //micro segundos / tempo de cada sample dos encoders na pio / default: 25000
const uint16_t PWM_WRAP = 32767;      // 2^15 - 1 (15-bit de resoluçao) signed int16
const float CLOCK_DIV = 1.0f;         // 125 MHz clock rate

//variabeis atomicas para acesso inter core e macros para acesso mais fácil
std::atomic<bool> ENCODER_AVALIABLE{false};
std::atomic<int> velEsquerdo{0};
std::atomic<int> velDireito{0};
#define TICKesq velEsquerdo.load(std::memory_order_relaxed)
#define TICKdir velDireito.load(std::memory_order_relaxed)

PioEncoder* encoderEsq = nullptr; //declarei mesmo à pai uns pointers aqui para a malta
PioEncoder* encoderDir = nullptr;

// NeeecSense vars
static uint32_t sensor_mask = 0;  //bitmask para sacar bits do rgister dos gpios
uint sensor_pins[5] = {0};        // Guardar que GPIOs estão os sensores


void __isr __not_in_flash_func(encoder_timer_isr)(alarm_id_t id){
  velEsquerdo.store(encoderEsq->getCount(), std::memory_order_relaxed);
  velDireito.store(encoderDir->getCount(), std::memory_order_relaxed);
  encoderEsq->reset();
  encoderDir->reset();
  hardware_alarm_set_target(id, delayed_by_us(get_absolute_time(), ENCODER_SAMPLE_RATE));
}

// Interrupt Service Routine (ISR)
void __isr __not_in_flash_func(fifo_isr)() {
    // Check if interrupt was from FIFO
    if (multicore_fifo_rvalid()) {
        Packet pkt;
        pkt.raw = multicore_fifo_pop_blocking();

        //Motor Esquerdo
        pwm_set_gpio_level(EN1A, pkt.pwm.esq > 0 ? pkt.pwm.esq : 0);  //se maior que zero escreve em A e pões B a zero
        pwm_set_gpio_level(EN1B, pkt.pwm.esq < 0 ? -pkt.pwm.esq : 0);  //se menor que zero B escreve e A a zero
        
        // Motor Direito
        pwm_set_gpio_level(EN2A, pkt.pwm.dir > 0 ? pkt.pwm.dir : 0);
        pwm_set_gpio_level(EN2B, pkt.pwm.dir < 0 ? -pkt.pwm.dir : 0);
    }
    multicore_fifo_clear_irq(); // Clear IRQ flags
}


// A função a correr no segundo core
void motor_task() {
  multicore_fifo_clear_irq();
  irq_set_exclusive_handler(SIO_IRQ_PROC1, fifo_isr);
  irq_set_enabled(SIO_IRQ_PROC1, true);

    // Encoder Timer Setup
  while(!ENCODER_AVALIABLE.load(std::memory_order_acquire)) __wfe();

  hardware_alarm_claim(0);
  hardware_alarm_set_callback(0,(hardware_alarm_callback_t)encoder_timer_isr);
  hardware_alarm_set_target(0, delayed_by_us(get_absolute_time(), ENCODER_SAMPLE_RATE));

  while(true) __wfe(); // sigma não dorme
    
}

// Começar task em core1
void motor_init() {
  //inicializar os pinos dos enables

  gpio_set_function(EN1A, GPIO_FUNC_PWM);
  gpio_set_function(EN1B, GPIO_FUNC_PWM);
  gpio_set_function(EN2A, GPIO_FUNC_PWM);
  gpio_set_function(EN2B, GPIO_FUNC_PWM);

  uint slice_EN1A = pwm_gpio_to_slice_num(EN1A);
  uint slice_EN2A = pwm_gpio_to_slice_num(EN2A);
  uint slice_EN1B = pwm_gpio_to_slice_num(EN1B);
  uint slice_EN2B = pwm_gpio_to_slice_num(EN2B);

  pwm_config config = pwm_get_default_config();
  pwm_config_set_clkdiv(&config, CLOCK_DIV);
  pwm_config_set_wrap(&config, PWM_WRAP);
  pwm_config_set_phase_correct(&config, true);  // Output mais smooth

  pwm_init(slice_EN1A, &config, true);  //init dos enables todos
  pwm_init(slice_EN1B, &config, true);
  pwm_init(slice_EN2A, &config, true);
  pwm_init(slice_EN2B, &config, true);


  multicore_fifo_drain(); //limpar FIFO
  multicore_launch_core1(motor_task); //Começar task no segundo core
}

void set_motor(int esq, int dir){
  //constrain dos valores ao wrap do pwm
  Packet pkt;
  pkt.pwm.esq = esq < -PWM_WRAP ? -PWM_WRAP : 
            esq > PWM_WRAP ? PWM_WRAP : esq; 
  
  pkt.pwm.dir = dir < -PWM_WRAP ? -PWM_WRAP : 
            dir > PWM_WRAP ? PWM_WRAP : dir;

  // Push pacote para a FIFO. Bloqueia se a FIFO estiesqr cheia
  multicore_fifo_push_blocking(pkt.raw);
}

bool encoder_init(int aEsq, int aDir) {
  // inicializa a esquerda
  encoderEsq = new PioEncoder(aEsq);
  if (encoderEsq == nullptr) {
    return false; // Correu mal guys
  }
  encoderEsq->begin();
  
  // Tenta inicializar o da direita
  encoderDir = new PioEncoder(aDir);
  if (encoderDir == nullptr) {
    delete encoderEsq; // correu mal no segundo limpa o primeiro
    encoderEsq = nullptr;
    return false; 
  }
  encoderDir->begin();
  
  // Correu bem zau
  ENCODER_AVALIABLE.store(true, std::memory_order_release); 
  multicore_fifo_push_blocking(0);  // Acorda o core1
  return true;
}

void set_sample_rate(int x){
  //input em milisegundo para microsegundo
  ENCODER_SAMPLE_RATE = x * 1000;
}

void sense_init(uint s1, uint s2, uint s3, uint s4, uint s5) {
    // Guardar posição dos Gpios
    sensor_pins[0] = s1;
    sensor_pins[1] = s2;
    sensor_pins[2] = s3;
    sensor_pins[3] = s4;
    sensor_pins[4] = s5;

    
    sensor_mask = 0;
    for (int i = 0; i < 5; i++) { //Criar Bitmask e init dos GPIOs
        gpio_init(sensor_pins[i]);
        gpio_set_dir(sensor_pins[i], GPIO_IN);
        gpio_pull_up(sensor_pins[i]); // Adjust based on your sensors
        sensor_mask |= (1UL << sensor_pins[i]);
    }
}

// Ler da NeeecSense
void sense_read(bool states[5]) {
  // Ler o register das GPIOs
    uint32_t gpio_state = gpio_get_all() & sensor_mask;
    
    for (int i = 0; i < 5; i++) {
        states[i] = (gpio_state >> sensor_pins[i]) & 1;
    }
}

uint8_t sense_read_bit() {
    // Ler o register das GPIOs
    uint32_t gpio_state = gpio_get_all() & sensor_mask;
    //Inicializar saida
    uint8_t out = 0;
    
    // processar os valores todos para um byte
    for (int i = 0; i < 5; i++) {
        // zeros uns para a posição certa LSB = sensor_pin[0]
        if (gpio_state & (1UL << sensor_pins[i])) {
            out |= (1 << i);
        }
    }
    return out;
}