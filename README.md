<div align="center">

# 🧱 Breakout / Arkanoid

**Jogo Breakout para Arduino Uno com display OLED, controle por potenciômetro e efeitos sonoros no buzzer**

[![Arduino](https://img.shields.io/badge/Arduino-Uno-00979D?style=for-the-badge&logo=Arduino&logoColor=white)](https://www.arduino.cc/)
[![C%2B%2B](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)

</div>

---

## 📋 Descrição

O **Breakout / Arkanoid** adapta o clássico jogo de arcade para um Arduino Uno. O potenciômetro controla a raquete, a bola rebate nas bordas e destrói blocos exibidos em um OLED SSD1306. Os blocos têm diferentes pontos de resistência, e o buzzer passivo sinaliza colisões.

O firmware usa colisão AABB (Axis-Aligned Bounding Box) separada nos eixos X e Y, com estado de jogo mantido em memória estática para caber na SRAM limitada do Uno.

---

## ⚙️ Componentes Utilizados

| Quantidade | Componente | Especificação |
|:---:|---|---|
| 1x | Arduino Uno (ou compatível) | ATmega328P |
| 1x | Display OLED | SSD1306 I2C, 128×64, endereço 0x3C (confirmar no módulo) |
| 1x | Potenciômetro | 10 kΩ linear para mover a raquete |
| 1x | Buzzer piezo passivo | Para efeitos de colisão via `tone()` |
| — | Jumpers e protoboard | — |

> O firmware usa bibliotecas Adafruit GFX e Adafruit SSD1306. Confirme a tensão aceita pelo módulo OLED antes de ligá-lo ao Arduino.

---

## 🔌 Pinagem

```text
Arduino Uno
├── A0  → Terminal central do potenciômetro
├── 5V  → Extremidade do potenciômetro e VCC do OLED (se suportado)
├── GND → Outra extremidade do potenciômetro, GND do OLED e buzzer
├── A4  → SDA do OLED
├── A5  → SCL do OLED
└── D9  → Buzzer piezo passivo
```

O potenciômetro deve ser ligado como divisor de tensão entre 5V e GND, com o cursor em A0. Arduino Uno e OLED usam I2C em A4/SDA e A5/SCL. Use buzzer passivo para permitir diferentes frequências.

---

## 🖼️ Esquemático e montagem

### Esquemático elétrico

<!-- Adicione a imagem exportada do LTspice em circuit_images/esquematico.png -->
![Esquemático elétrico do projeto](circuit_images/esquematico.png)

O arquivo [breakout-arkanoid.asc](Esquemático/breakout-arkanoid.asc) registra as interconexões e os pinos usados pelo firmware. Arduino e OLED são módulos externos, não modelos SPICE simulados.

### Montagem real

<!-- Adicione uma foto da montagem em circuit_images/montagem_real.jpg -->
![Montagem real do projeto](circuit_images/montagem_real.jpg)

Adicione as imagens aos caminhos acima depois de montar o circuito.

---

## 💻 Como Funciona

### Controle e renderização

O valor analógico de A0 posiciona a raquete na parte inferior do OLED. O firmware desenha a bola, a raquete, os blocos, a pontuação e as vidas no framebuffer do SSD1306.

### Física e colisões

A posição da bola é atualizada separadamente em X e Y. Cada deslocamento verifica interseção AABB com as paredes, a raquete e os blocos; separar os eixos permite inverter apenas o componente de velocidade correspondente ao contato.

### Blocos e áudio

Os blocos têm níveis de HP diferentes. Cada contato reduz o HP; ao destruir um bloco, a pontuação aumenta. O buzzer toca efeitos curtos sem interromper o ciclo principal do jogo.

---

## 🗂️ Estrutura dos Arquivos

```text
breakout-arkanoid/
├── README.md
├── circuit_images/                  # Imagens do projeto (vazia inicialmente)
├── Esquemático/
│   └── breakout-arkanoid.asc        # Diagrama de interconexões LTspice
└── sketch_breakout-arkanoid/
    └── sketch_breakout-arkanoid.ino # Firmware principal
```

---

## 🚀 Como Usar

1. Instale a Arduino IDE e as bibliotecas Adafruit GFX Library e Adafruit SSD1306.
2. Monte o circuito conforme a pinagem e confira a tensão do OLED.
3. Abra `sketch_breakout-arkanoid/sketch_breakout-arkanoid.ino`.
4. Selecione Arduino Uno e a porta serial correta.
5. Faça upload e mova o potenciômetro para controlar a raquete.

---

## 🔧 Personalização

Os parâmetros de velocidade, dimensões da raquete e configuração da grade de blocos ficam no início do sketch. Ajuste-os conforme a resposta do controle e a taxa de atualização observadas no hardware.

---

## ✅ Estado e Próximos Passos

- [x] Criar estrutura inicial do projeto e firmware base.
- [x] Registrar pinagem e diagrama de interconexões LTspice.
- [ ] Compilar e validar o firmware no Arduino Uno com o OLED real.
- [ ] Calibrar movimento, dificuldade e efeitos sonoros no circuito montado.
- [ ] Adicionar imagens do esquemático e da montagem em `circuit_images/`.

*Desenvolvido por Felipe Grolla*
