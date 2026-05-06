# Automatic Tile Flatness Detection System

This repository contains the final Arduino sketches for a three-controller BM53A367A tile flatness detection system.

## Hardware Target

- Board: BM53A367A (BMduino-UNO HT32F52367)
- Arduino FQBN: `HT32:HT32:BM53A367A`
- Platform: Holtek HT32 Boards `1.3.0`

## Final Controllers

| Folder | Source role | Main functions |
| --- | --- | --- |
| `arduino/final_environment` | Environment controller | Reads temperature/humidity with `BM25S2021-1` and displays the result on `BMD31M090` OLED. |
| `arduino/final_motor` | Motion controller | Drives the horizontal/vertical stepper motors, reads the `BML36M001` laser ranging module, and sends trigger pulses to the sensor controller. |
| `arduino/final_sensor` | Sensor/data controller | Reads three dial-gauge/displacement sensor channels, receives the trigger signal, displays `S1/S2/S3`, and sends framed serial data to the upper computer. |

## Required Arduino Libraries

Install these libraries through Arduino Library Manager:

- `BM25S2021-1`
- `BMD31M090`
- `BMP73T104`
- `BML36M001`

## Build Commands

```powershell
arduino-cli compile --fqbn HT32:HT32:BM53A367A .\arduino\final_environment
arduino-cli compile --fqbn HT32:HT32:BM53A367A .\arduino\final_motor
arduino-cli compile --fqbn HT32:HT32:BM53A367A .\arduino\final_sensor
```

## Original Source Mapping

These final sketches were copied from the original working folder. The original files were not moved or modified.

| Final sketch | Original source |
| --- | --- |
| `arduino/final_environment/final_environment.ino` | `wenshidu/wenshidu.ino` |
| `arduino/final_motor/final_motor.ino` | `5.15SHANGXIA/5.15SHANGXIA.ino` |
| `arduino/final_sensor/final_sensor.ino` | `5.15baifen/5.15baifen.ino` |
