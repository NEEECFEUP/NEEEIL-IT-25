![GitHub Octocat](https://neeecfeup.pt/storage/app/uploads/public/67c/e36/25f/67ce3625f06d8665873646.svg)

# Nelito 

Esta biblioteca fornece um conjunto de funções para interagir com os componentes fornecidos para o NEEEIL IT 25!

## Como instalar
Para a instalação da biblioteca podem usar o Arduino IDE:
- Faz o download da versão mais recente da biblioteca na página de [Releases]() deste repositório;

- No Arduino IDE, instala a biblioteca em Sketch > Incluir Biblioteca > Adicionar Biblioteca .ZIP;

- Seleciona o arquivo ZIP que descarregaste no primeiro passo;

- No fim estará disponível para incluir no código, basta selecionar Sketch > Incluir Biblioteca > Nelito.

### Inclusão da Biblioteca

Para usar a biblioteca terá que constar no início do ficheiro:

```cpp
#include <nelito.h>
```
---
# 📋 Lista de Funções e Variáveis Públicas

### **Funções**
1. **`motor_init()`**  
   - **Descrição**: Inicializa os pinos PWM dos motores e inicia a tarefa no Core 1 para controle assíncrono.  
   - **Uso**: Chamar uma vez no início do código.

2. **`set_motor(int esq, int dir)`**  
   - **Descrição**: Define a velocidade dos motores (valores de -32767 a 32767).  
   - **Parâmetros**:  
     - `esq`: PWM do motor esquerdo (valores negativos = andar para trás).  
     - `dir`: PWM do motor direito (valores negativos = andar para trás).  

3. **`encoder_init(int aEsq, int aDir)`**  
   - **Descrição**: Configura os encoders nos pinos especificados (Canal A e B devem ser consecutivos).  
   - **Retorno**: `true` se inicializados com sucesso, `false` em caso de erro.  
   - **Exemplo**: `encoder_init(2, 4)` → Encoder esquerdo (pinos 2 e 3), direito (pinos 4 e 5).

4. **`set_sample_rate(int x)`**  
   - **Descrição**: Define a taxa de amostragem dos encoders em milissegundos (ex: `25` → 25 ms).
   - **Uso**: Se quiser mudar a taxa amostragem. Default = 25 ms.  

5. **`sense_init(uint s1, s2, s3, s4, s5)`**  
   - **Descrição**: Configura os GPIOs dos 5 sensores de linha.  
   - **Exemplo**: `sense_init(6, 7, 8, 9, 10)` → Sensores nos pinos 6 a 10.

6. **`sense_read(bool states[5])`**  
   - **Descrição**: Lê os sensores e armazena os estados em um array (índices 0 a 4 = sensor 1 a 5).  
   - **Exemplo**: `states[i] = true` se o sensor `i+1` detectar linha.

7. **`sense_read_bit()`**  
   - **Descrição**: Lê os sensores e retorna um byte onde cada bit representa um sensor (bit 0 = sensor 1).  
   - **Exemplo**: `0b00100` → Sensor 3 ativo.

---

### **Variáveis Públicas**
1. **`ENCODER_SAMPLE_RATE`**  
   - **Tipo**: `uint16_t`  
   - **Descrição**: Taxa de amostragem dos encoders **em microssegundos** (valor padrão: 25 ms).  

2. **`TICKesq`**  
    - **Tipo**: `atomic int`  
   - **Descrição**: Contador atual do encoder esquerdo.  

3. **`TICKdir`**  
    - **Tipo**: `atomic int`   
   - **Descrição**: Contador atual do encoder direito.
  
     ## 🎯 Notas de Uso
- **Encoders**: Os pinos do Canal A e B dos encoders de cada motor devem ser consecutivos (ex: 2 e 3).
