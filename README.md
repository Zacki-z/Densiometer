# Densiometer

# Densitometer (Arduino Nano Every) – v2.3

Detta repo innehåller firmware, hårdvaruunderlag och dokumentation för en enkel densitometer:
UV-ljuskälla + fotodiod (BPW34) + transimpedansförstärkare (LMC6462) + Arduino Nano Every.

## Funktion
- Mäter sensorns spänning (TIA-utgång) på A0
- Beräknar optisk densitet: OD = log10(V0 / V)
- V0 (referens) kalibreras med långtryck på Measure/Cal
- Visar Level, V och OD på 16x2 I2C LCD (0x27) och i Serial Monitor

## Pinout (Nano Every)
- D2: Status-LED
- D3: ON/OFF (toggle)
- D4: Measure/Cal (kort tryck = mätning/logg, långt tryck = kalibrera V0)
- D5–D8: Rotary one-hot: OFF/LOW/MID/HIGH (aktiv LOW, INPUT_PULLUP)
- D9: MOSFET_1
- D10: MOSFET_2
- A0: Sensor (TIA output)
- I2C: LCD 16x2 (PCF8574), adress 0x27

## Nivålogik (MOSFET)
- OFF: D9=0, D10=0
- LOW: D9=1, D10=0
- MID: D9=0, D10=1
- HIGH: D9=1, D10=1

## Bygga och köra
1. Öppna `firmware/densitometer_v2_3/` i Arduino IDE
2. Välj board: Arduino Nano Every
3. Kontrollera LCD-adress (default 0x27)
4. Upload

## Firmware-varianter
- `densitometer_v2_3_single`: allt i en .ino
- `densitometer_v2_3`: modulär kod (rekommenderas)

## Notering om analog referens
Nano Every använder INTERNAL1V1. Klassiska AVR använder INTERNAL (~1.1V).
>>>>>>> 4b4b5f7 (Initial commit)
