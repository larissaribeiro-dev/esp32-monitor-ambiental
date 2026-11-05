# esp32-monitor-ambiental

![C++](https://img.shields.io/badge/C%2B%2B-Arduino-00979D)
![ESP32](https://img.shields.io/badge/ESP32-DevKit_V1-E7352C)
![Wokwi](https://img.shields.io/badge/Wokwi-simulation-green)
![DHT22](https://img.shields.io/badge/sensor-DHT22-blue)
![License](https://img.shields.io/badge/license-MIT-green)

Monitor de temperatura e umidade feito com ESP32, sensor DHT22 e display OLED SSD1306, escrito em C++ (Arduino) e simulado no [Wokwi](https://wokwi.com/).

O mais interessante é que o dado só é usado se for confiável. A leitura passa por três verificações antes de qualquer decisão, e uma leitura inválida nunca entra na comparação com o limite de temperatura. Se o sensor falhar, o sistema mostra `ERRO SENSOR` e volta ao normal sozinho quando ele responde de novo.

```
T < 30,0 °C     ->  NORMAL        (LED verde aceso)
T >= 30,0 °C    ->  ALERTA        (LED vermelho aceso)
leitura falhou  ->  ERRO SENSOR   (os dois LEDs apagados)
```

**Simulação online:** [abrir o projeto no Wokwi](https://wokwi.com/projects/479891308176792577)

![Sistema em funcionamento](images/sistema-inicializado.png)

## Recursos

- Leitura de temperatura e umidade com o DHT22 a cada 2 s, o intervalo mínimo do sensor
- Regra de alerta com limite de 30,0 °C, ajustável pela constante `LIMITE_ALERTA_C`
- Três estados (`NORMAL`, `ALERTA` e `ERRO SENSOR`), mostrados no OLED e nos LEDs
- Validação da leitura (status da biblioteca, NaN e faixa do sensor) antes de qualquer decisão
- Recuperação automática quando o sensor volta a responder, sem reiniciar
- Temporização com `millis()`, sem travar o programa
- Display redesenhado só quando temperatura, umidade ou estado mudam
- Registro de cada leitura e estado no Serial Monitor
- Casos de teste executados em simulação, com capturas de tela e logs reais

## Como funciona

A cada 2 segundos o programa lê o DHT22, valida a leitura, decide o estado e atualiza os LEDs, o OLED e o Serial Monitor. O fluxo completo está em [`docs/funcionamento.md`](docs/funcionamento.md).

```
        +-------------------+
        |  DHT22 (GPIO 15)  |
        +---------+---------+
                  | temperatura e umidade
          +-------v--------+   I2C (GPIO 21 e 22)   +--------------------+
          |     ESP32      |----------------------->|   OLED SSD1306     |
          |   sketch.ino   |                        |   128x64, 0x3C     |
          +---+--------+---+                        +--------------------+
              |        |
      GPIO 18 |        | GPIO 19
         +----v---+ +--v-----+
         | LED    | | LED    |
         | verde  | |vermelho|
         +--------+ +--------+
```

![Circuito no Wokwi](images/circuito-wokwi.png)

O circuito acima é uma captura do Wokwi e mostra o ESP32, o DHT22, o OLED, os dois LEDs com seus resistores de 220 Ω e os fios.

### Componentes

| Componente | Função |
|------------|--------|
| ESP32 DevKit V1 | Microcontrolador que executa o programa |
| DHT22 | Sensor de temperatura e umidade |
| OLED SSD1306 128x64 | Display I2C com título, temperatura, umidade e estado |
| LED verde + resistor de 220 Ω | Indica `NORMAL` |
| LED vermelho + resistor de 220 Ω | Indica `ALERTA` |

### Funções do programa

| Função | Arquivo | Papel |
|--------|---------|-------|
| `lerSensor()` | `sketch.ino` | Lê o DHT22 e só marca a leitura como válida se passar nas três verificações |
| `determinarEstado()` | `sketch.ino` | Aplica a regra de temperatura e devolve o estado |
| `atualizarLeds()` | `sketch.ino` | Acende o LED verde, o vermelho ou nenhum, conforme o estado |
| `atualizarDisplay()` | `sketch.ino` | Redesenha o OLED com temperatura, umidade e estado |
| `displayPrecisaAtualizar()` | `sketch.ino` | Compara a leitura nova com o que já está na tela |
| `registrarNoSerial()` | `sketch.ino` | Escreve a leitura e o estado no Serial Monitor |

### Validação da leitura

Uma leitura só é usada se passar nas três verificações, nesta ordem.

1. O status informado pela biblioteca é "sem erro" (com o fio de dados removido, por exemplo, o status é `TIMEOUT`).
2. Nenhum dos valores é NaN.
3. Temperatura entre -40 e 80 °C e umidade entre 0 e 100 %, a faixa de medição do DHT22.

### Estados

| Condição | Estado | LED verde | LED vermelho | Linha no OLED |
|----------|--------|-----------|--------------|---------------|
| Leitura inválida | `ERRO SENSOR` | apagado | apagado | `Estado: ERRO SENSOR` |
| Temperatura < 30,0 °C | `NORMAL` | aceso | apagado | `Estado: NORMAL` |
| Temperatura >= 30,0 °C | `ALERTA` | apagado | aceso | `Estado: ALERTA` |

A umidade é apenas exibida e não influencia o estado.

### Mapeamento de pinos

| Sinal | Pino | Função |
|-------|------|--------|
| DHT22, dados | `GPIO 15` | Linha de dados do sensor |
| OLED, SDA | `GPIO 21` | Dados I2C |
| OLED, SCL | `GPIO 22` | Clock I2C |
| LED verde | `GPIO 18` | Indicador de `NORMAL`, via resistor de 220 Ω |
| LED vermelho | `GPIO 19` | Indicador de `ALERTA`, via resistor de 220 Ω |
| DHT22 e OLED, alimentação | `3V3` e `GND` | Alimentação e referência |

A tabela completa, com cada fio do `diagram.json`, está em [`docs/conexoes.md`](docs/conexoes.md).

## Como rodar

### Abrir o projeto pronto no Wokwi

1. Abra o [projeto no Wokwi](https://wokwi.com/projects/479891308176792577).
2. Clique no botão verde **Play** e abra o Serial Monitor para acompanhar as leituras.

### Montar a partir dos arquivos do repositório

1. Acesse [wokwi.com](https://wokwi.com/) e entre na sua conta (ou crie uma).
2. Clique em **New Project** e escolha **ESP32**, com o modelo ESP32 de Arduino (C++).
3. No editor, abra o `sketch.ino` e substitua o conteúdo pelo deste repositório.
4. Abra a aba `diagram.json` e substitua o conteúdo pelo [`diagram.json`](diagram.json).
5. Se não existir a aba `libraries.txt`, crie o arquivo e cole o conteúdo de [`libraries.txt`](libraries.txt).
6. Confira no circuito se aparecem o ESP32, o DHT22, o OLED, dois LEDs e dois resistores com os fios ligados.
7. Clique em **Play**.

### Como testar NORMAL e ALERTA

Com a simulação rodando, clique no DHT22 para abrir os sliders de temperatura e umidade. O display e os LEDs se atualizam em até 2 s.

| Ação | Resultado esperado |
|------|--------------------|
| Temperatura abaixo de 30,0 °C (por exemplo, 22 °C) | `Estado: NORMAL`, LED verde aceso |
| Temperatura exatamente em 30,0 °C | `Estado: ALERTA`, LED vermelho aceso |
| Temperatura acima de 30,0 °C (por exemplo, 35 °C) | `Estado: ALERTA`, LED vermelho aceso |
| Voltar a uma temperatura abaixo de 30,0 °C | Volta a `NORMAL`, sem reiniciar |
| Mudar só a umidade | O valor de umidade muda; o estado não |
| Remover o fio de dados do DHT22 e rodar | `ERRO SENSOR`, `T: --`, `U: --`, os dois LEDs apagados |

Para reproduzir a falha, pare a simulação, apague na aba `diagram.json` a linha da ligação `esp:D15` → `dht1:SDA`, rode e, depois, recoloque a linha para restaurar o circuito.

## Testes

Todos os testes foram feitos em simulação, e cada caso tem a evidência (captura de tela ou log do Serial Monitor) registrada em [`docs/testes.md`](docs/testes.md).

| Caso | Entrada | Resultado |
|------|---------|-----------|
| Inicialização | 25 °C, 50 % | Aprovado: `NORMAL`, verde aceso |
| Abaixo do limite | 22,0; 24,0; 25,0 e 29,9 °C | Aprovado: `NORMAL`, verde aceso |
| Exatamente no limite | 30,0 °C | Aprovado: `ALERTA`, vermelho aceso |
| Acima do limite | 35,0 °C | Aprovado na versão intermediária do código, não repetido no código final |
| Retorno ao normal | 30,0 para 24,0 °C, sem reiniciar | Aprovado: `NORMAL` de volta |
| Alterar só a umidade | 80 % e 65 % | Aprovado: valor atualizado, estado igual |
| Atualização contínua | Valores mantidos por vários ciclos | Aprovado: sem sobreposição nem estado incoerente |
| Falha do sensor | Fio de dados removido | Aprovado: `ERRO SENSOR`, LEDs apagados, `TIMEOUT` no Serial |
| Recuperação | Fio de dados recolocado | Aprovado: leituras normais de volta |

O `docs/testes.md` também lista o que **não** foi executado.

### Capturas da simulação

| | |
|:---:|:---:|
| ![NORMAL](images/teste-temperatura-normal.png) | ![ALERTA](images/teste-temperatura-alerta.png) |
| Estado NORMAL, 22 °C | Estado ALERTA, exatamente 30,0 °C |
| ![Retorno ao normal](images/retorno-ao-normal.png) | ![Erro do sensor](images/teste-erro-sensor.png) |
| Retorno ao NORMAL, 24 °C | ERRO SENSOR, fio de dados removido |

## Resultado na simulação

Ao iniciar, o OLED mostra o título, a temperatura, a umidade e o estado, e o LED correspondente acende.

- `T: 25.0 C` e `U: 50.0 %`, os valores iniciais do DHT22
- `Estado: NORMAL` na última linha
- LED verde aceso e LED vermelho apagado

No Serial Monitor, cada leitura aparece assim.

```
Temperatura: 30.0 C | Umidade: 50.0 % | Estado: ALERTA
```

## Estrutura do projeto

```
esp32-monitor-ambiental/
├── sketch.ino                    # código C++ do ESP32
├── diagram.json                  # circuito do Wokwi
├── libraries.txt                 # bibliotecas usadas
├── docs/                         # documentação detalhada
│   ├── funcionamento.md
│   ├── conexoes.md
│   └── testes.md
├── images/                       # capturas de tela reais da simulação
├── .gitignore
├── LICENSE
└── README.md
```

## Decisões de projeto

**Por que a biblioteca `DHT sensor library for ESPx`?** A documentação do Wokwi recomenda essa biblioteca para o ESP32 e avisa que outras bibliotecas de DHT podem não funcionar de forma confiável nele.

**Por que ler a cada 2 segundos?** É o intervalo mínimo que a própria biblioteca informa para o DHT22 (2000 ms, mostrado no Serial Monitor nos testes). Ler mais rápido não traria dados novos.

**Por que `millis()` e não `delay()`?** O `loop()` só compara o tempo atual com o da última leitura e segue em frente, então o programa nunca fica travado esperando.

**Por que comparar a temperatura em décimos de grau?** O DHT22 tem resolução de 0,1 °C. Comparar `300 >= 300` em inteiros evita que um erro de arredondamento de ponto flutuante (por exemplo, 29,999998) faça o limite exato de 30,0 °C ser classificado como `NORMAL`.

**Por que os dois LEDs ficam apagados em `ERRO SENSOR`?** Sem medição confiável, nenhum dos dois indicadores de condição ambiental deve parecer válido.

**Por que o display só é redesenhado quando algo muda?** Evita reescrever a tela sem necessidade. A tela é limpa por inteiro a cada redesenho, para não sobrarem caracteres antigos quando o número de dígitos muda.

**Por que `C` e não `°C` no display?** A fonte padrão do OLED não garante o símbolo de grau.

## Limitações da simulação

- Todos os testes foram feitos no Wokwi. Não houve validação em hardware real.
- O DHT22 simulado entrega valores exatos e sem ruído, e só se consegue provocar falha removendo o fio de dados. Um sensor real tem variações e outros modos de falha.
- O DHT22 simulado não precisa de resistor de pull-up na linha de dados. Em hardware real, o sensor de 4 pinos costuma precisar de um (tipicamente na faixa de 4,7 kΩ a 10 kΩ), enquanto muitos módulos de 3 pinos já o trazem.
- O GPIO 15 é um pino de *strapping* do ESP32. Em hardware real, convém conferir se o circuito ligado a ele não atrapalha a inicialização.
- O desempenho do simulador varia (nos testes, entre cerca de 78 % e 105 % do tempo real), então os tempos medidos nas capturas são aproximados.
- Os ramos de NaN e de valor fora da faixa do sensor existem no código, mas não puderam ser provocados no simulador.

## Roadmap

- [ ] Histerese no alerta, para o estado não alternar quando a temperatura oscila perto de 30 °C
- [ ] Alerta sonoro com um buzzer
- [ ] Alerta também por umidade
- [ ] Alterar o limite sem recompilar, com um botão ou potenciômetro
- [ ] Envio das leituras por Wi-Fi
- [ ] Teste em hardware real

## Referências

1. WOKWI. *wokwi-dht22 Reference*. Disponível em: <https://docs.wokwi.com/parts/wokwi-dht22>. Acesso em: 5 nov. 2026.
2. WOKWI. *board-ssd1306 Reference*. Disponível em: <https://docs.wokwi.com/parts/board-ssd1306>. Acesso em: 5 nov. 2026.

## Autoria

Projeto pessoal de estudo e pesquisa.

| | |
|---|---|
| **Autora** | Larissa Rafaela Ribeiro de Souza |
| **GitHub** | [@larissaribeiro-dev](https://github.com/larissaribeiro-dev) |

## Licença

Distribuído sob a licença MIT. Veja o arquivo [LICENSE](LICENSE).
