# Testes

Todos os testes foram executados no simulador Wokwi. **Não houve teste em hardware físico.** Só estão marcados como aprovados os casos que foram realmente executados, e cada um aponta a evidência (captura de tela ou log do Serial Monitor) usada para comprová-lo.

Convenções:

- **Código intermediário:** versão do `sketch.ino` durante o desenvolvimento. Na "Parte 1" o programa lia o sensor e mostrava os valores no OLED; na "Parte 2" passou a decidir o estado e acender os LEDs.
- **Código final:** a versão atual do `sketch.ino`, que redesenha o display apenas quando algo muda.
- Os logs abaixo foram copiados do Serial Monitor. Trechos longos repetem a mesma linha a cada 2 s, então aparece uma linha de cada valor, com a quantidade de repetições indicada.

## 1. Testes isolados dos componentes

Feitos com programas provisórios, antes do código final, para confirmar que cada componente responde.

| ID | Teste | Resultado observado | Evidência | Status |
|----|-------|---------------------|-----------|--------|
| I1 | LEDs alternando (GPIO 18 e 19) | Aos 5,8 s o vermelho estava aceso e o verde apagado; aos 36,8 s o verde aceso e o vermelho apagado. Os instantes são coerentes com o ciclo de 3 s do teste (verde de 0 a 1,5 s, vermelho de 1,5 a 3 s). | [vermelho](../images/teste-led-vermelho.png), [verde](../images/teste-led-verde.png) | Aprovado |
| I2 | OLED com mensagem fixa e endereço I2C | Texto "MONITOR AMBIENTAL / Teste do OLED: OK" legível, sem corte. Serial: `Dispositivo I2C respondeu em 0x3C` e `OLED inicializado`. | [print](../images/teste-oled-mensagem.png) e log | Aprovado |
| I3 | DHT22 no Serial Monitor | Leituras `25.0 C / 50.0 %`, depois `31.0 C / 70.0 %` e `18.0 C / 70.0 %` ao mexer nos sliders. Intervalo mínimo informado pela biblioteca: 2000 ms. | Log (sem captura) | Aprovado |
| I4 | Falha do DHT22 (fio de dados removido) | Serial: `LEITURA INVALIDA. Status: TIMEOUT`, sem valor numérico. Com o fio recolocado, voltaram `25.0 C / 50.0 %`. | Log (sem captura) | Aprovado |

## 2. Código intermediário, Parte 1: leitura e exibição

| ID | Teste | Resultado observado | Evidência | Status |
|----|-------|---------------------|-----------|--------|
| P1-1 | Valores no OLED e no Serial | 25.0 / 50.0, depois 31.0 / 70.0, 8.5 / 70.0 e -10.0 / 70.0; OLED e Serial coincidiram. | [25 °C](../images/teste-display-25c-parte1.png), [31 °C](../images/teste-display-31c.png), [8,5 °C](../images/teste-display-8-5c.png), [-10 °C](../images/teste-display-negativo.png) | Aprovado |
| P1-2 | Sem sobras de caracteres e dentro de 128 px | Ao passar de 31.0 para 8.5 e para -10.0 não restaram caracteres antigos; `T: -10.0 C` coube na tela. | Mesmas capturas | Aprovado |
| P1-3 | Fio de dados removido | OLED com `T: --`, `U: --` e `Estado: ERRO SENSOR`; Serial com `Falha na leitura. Status: TIMEOUT`. | [print](../images/teste-erro-sensor-parte1.png) e log | Aprovado |
| P1-4 | Fio recolocado | Voltaram `25.0 C / 50.0 %` no OLED e no Serial. | [print](../images/teste-recuperacao-sensor.png) e log | Aprovado |

## 3. Casos de teste do programa completo

A coluna "Código" indica a versão em que o caso foi executado. Quando o mesmo caso foi executado nas duas versões, as duas evidências estão listadas.

| ID | Caso | Entrada no DHT22 | Resultado esperado | Resultado observado | Código | Evidência | Status |
|----|------|------------------|--------------------|---------------------|--------|-----------|--------|
| T1 | Inicialização | 25 °C, 50 % (padrão) | Compila, OLED inicia, leitura coerente, NORMAL, verde aceso | Observado como esperado | Intermediário e final | [intermediário](../images/sistema-inicializado.png), [final](../images/regressao-normal.png) | Aprovado |
| T2 | Abaixo de 30 °C | 22,0 °C; 29,9 °C; 25,0 °C; 24,0 °C | NORMAL, verde aceso, vermelho apagado | Observado como esperado nos quatro valores. No código final, 29,9 °C ficou em NORMAL com o verde aceso; Serial: `Temperatura: 29.9 C \| Umidade: 35.0 % \| Estado: NORMAL` (10 leituras seguidas) | 22,0: intermediário; 29,9: intermediário e final; 25,0 e 24,0: final | [22 °C](../images/teste-temperatura-normal.png), [29,9 °C intermediário](../images/teste-limite-29-9c.png), [29,9 °C final](../images/regressao-limite-29-9c.png), [25 °C](../images/regressao-normal.png), [24 °C](../images/regressao-retorno-normal.png) e log | Aprovado |
| T3 | Exatamente 30,0 °C | 30,0 °C | ALERTA, vermelho aceso, verde apagado (prova que a regra é `>=`) | ALERTA com vermelho aceso; Serial: `Temperatura: 30.0 C \| Umidade: 50.0 % \| Estado: ALERTA` | Intermediário e final | [intermediário](../images/teste-temperatura-alerta.png), [final](../images/regressao-alerta-30c.png) e log | Aprovado |
| T4 | Acima de 30 °C | 35,0 °C | ALERTA, vermelho aceso | Observado como esperado | Intermediário | [print](../images/teste-temperatura-35c.png) | Aprovado (não repetido no código final) |
| T5 | Retorno ao normal | De 30,0 °C para 24,0 °C, sem reiniciar | NORMAL de volta, verde aceso, vermelho apagado | Observado na mesma simulação, sem reiniciar | Intermediário e final | [intermediário](../images/retorno-ao-normal.png), [final](../images/regressao-retorno-normal.png) e log | Aprovado |
| T6 | Alterar só a umidade | 80 % (intermediário) e 65 % (final), temperatura em 24,0 °C | Umidade atualizada no OLED; estado não muda | Umidade atualizada, estado continuou NORMAL | Intermediário e final | [80 %](../images/teste-umidade-80.png), [65 %](../images/regressao-umidade-65.png) e log | Aprovado |
| T7 | Atualização contínua | Valores mantidos por vários ciclos | Sem texto sobreposto, sem estado ou LED incoerente | Tela estável; o log do código final registrou 39 leituras consecutivas sem inconsistência | Intermediário e final | [print](../images/teste-atualizacao-continua.png) e log | Aprovado |
| T8 | Falha de leitura | Fio de dados removido | `ERRO SENSOR`, nenhum NaN apresentado, os dois LEDs apagados | `T: --`, `U: --`, `Estado: ERRO SENSOR`, LEDs apagados; Serial: `Falha na leitura. Status: TIMEOUT` e `Estado: ERRO SENSOR` | Intermediário e final | [intermediário](../images/teste-erro-sensor.png), [final](../images/regressao-erro-sensor.png) e log | Aprovado |
| T9 | Recuperação após a falha | Fio de dados recolocado | Leituras e estado normais de volta, sem alterar o código | `25.0 C / 50.0 %`, NORMAL, verde aceso | Intermediário e final | [intermediário](../images/teste-saidas-leds-normal.png), [final](../images/regressao-recuperacao.png) e log | Aprovado |

No print [`teste-saidas-leds-normal.png`](../images/teste-saidas-leds-normal.png), o Wokwi mostra o estado dos pinos: `D18` em `OUT High` (LED verde) e `D19` em `OUT Low` (LED vermelho), o que confirma o mapeamento dos GPIOs.

### Registros do Serial Monitor (código final)

Execução contínua com a temperatura mudando durante a simulação. Cada linha abaixo se repetiu a cada 2 s pelo número de vezes indicado:

```
Temperatura: 25.0 C | Umidade: 50.0 % | Estado: NORMAL     (11 vezes)
Temperatura: 30.0 C | Umidade: 50.0 % | Estado: ALERTA     (8 vezes)
Temperatura: 24.0 C | Umidade: 50.0 % | Estado: NORMAL     (8 vezes)
Temperatura: 24.0 C | Umidade: 65.0 % | Estado: NORMAL     (12 vezes)
```

Com a temperatura imediatamente abaixo do limite, no código final (início da execução e depois 29,9 °C, com a umidade em 35 %):

```
Temperatura: 25.0 C | Umidade: 50.0 % | Estado: NORMAL     (2 vezes)
Temperatura: 29.9 C | Umidade: 35.0 % | Estado: NORMAL     (10 vezes)
```

Com o fio de dados removido (3 leituras seguidas):

```
Falha na leitura. Status: TIMEOUT
Estado: ERRO SENSOR
```

## 4. Não executado ou não comprovado

- **Hardware físico:** nenhum teste foi feito em ESP32, DHT22 ou display reais.
- **Ramos de NaN e de valor fora da faixa em `lerSensor()`:** o código os trata, mas o simulador não permitiu provocá-los. Só a falha por `TIMEOUT` (fio de dados removido) foi reproduzida.
- **Redesenho condicional do display:** a lógica foi implementada e os testes mostram que nenhuma atualização foi perdida, mas a quantidade de redesenhos evitados não foi medida.
- **Caso T4 (35 °C):** executado só na versão intermediária do código. A alteração posterior mexeu apenas na decisão de redesenhar o display, não na regra de temperatura, mas esse valor não foi repetido no código final.
- **Outro limite de alerta:** não foi testada a troca de `LIMITE_ALERTA_C` por outro valor.
- **Serial Monitor nos testes dos LEDs (I1):** as mensagens daquele teste provisório não foram registradas.
- **Capturas sem imagem:** os testes I3 e I4 foram comprovados por log do Serial Monitor, sem captura de tela.
