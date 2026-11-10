# lab4-monitor-temperatura-humedad

Práctica de laboratorio de *Arquitecturas para Automatización Industrial en la Nube* (710083C), Universidad del Valle, Ingeniería Electrónica. Autor: Esteban Salazar Guerrero.

Simulación en Wokwi de un ESP32 que lee temperatura y humedad de un sensor DHT22, activa una alarma con un LED cuando la temperatura supera un umbral y publica los datos por MQTT a ThingsBoard Cloud, donde un dashboard los muestra en tiempo real.

## Cómo funciona
1. El ESP32 lee el sensor cada 2 s.
2. Alarma local:el LED se enciende cuando la temperatura supera 30 °C y se apaga cuando baja de 29 °C. La alarma funciona aunque no haya red.
3. Telemetría: cada 5 s, y de inmediato cuando la alarma cambia, publica un mensaje JSON por MQTT:
   ```json
   {"temperatura":24.5,"humedad":60.2,"alarma":0}
   ```
   Topic `v1/devices/me/telemetry`, broker `mqtt.thingsboard.cloud:1883`, con el token del dispositivo como usuario MQTT.
4. En ThingsBoard o cualquier otra plataforma de IoT que soporte MQTT se visualizan los siguientes gráficos:
   - Gráfico de series de tiempo con `temperatura` (°C) y `humedad` (%).
   - *State chart* con `alarma` (0 → Off, 1 → On).

El circuito utilizado en wokwi es el siguiente:

<img width="567" height="824" alt="image" src="https://github.com/user-attachments/assets/04ebd3ad-c9bf-406a-922a-d52b38784bed" />

Para usar un configuración diferente de pines, se recomienda modificar el código scketch.ino.

## Archivos
- `sketch.ino`: código del ESP32, organizado en funciones por responsabilidad (sensor, alarma, red, lógica principal).
- `diagram.json`: circuito de Wokwi.
- `libraries.txt`: lista de librerías que Wokwi instala automáticamente.
- `secrets.h.example`: plantilla del token de acceso.

## Dependencias
Wokwi las instala solo desde `libraries.txt`. En el Arduino IDE o PlatformIO, instalarlas por nombre:
- PubSubClient (versión 2.8.0)
- DHT sensor library for ESPx, también llamada DHTesp (versión 1.19.0)

## Cómo reproducirlo
1. Wokwi: crear un proyecto ESP32 (Arduino) en https://wokwi.com/esp32 y copiar `sketch.ino`, `diagram.json` y `libraries.txt` a sus pestañas.
2. ThingsBoard: crear una cuenta en https://thingsboard.cloud, crear un dispositivo (Entities → Devices → +) y copiar su token con Copy access token.
3. Token: en Wokwi crear un archivo nuevo llamado `secrets.h` (menú ▼ junto a las pestañas → new file) con el contenido de `secrets.h`, reemplazando por el token real.
4. Dashboard: crear un dashboard con un gráfico de series de tiempo para la temperatura y la humedad y un State chart
5. Probar: iniciar la simulación y hacer clic en el DHT22 para mover su temperatura. Por encima de 30 °C el LED se enciende y `alarma` pasa a 1.

## Salida esperada en el monitor Serial
```
WiFi: conectando.......... OK
MQTT: conectando... OK
Temperatura: 24.0 C | Humedad: 40.0 % | Alarma: OFF
MQTT: {"temperatura":24.0,"humedad":40.0,"alarma":0} -> enviado
```

## Problemas frecuentes

| `MQTT: fallo rc=-2` | No se abrió la conexión con el broker. Suele ser pasajero: reiniciar la simulación. |
| `MQTT: fallo rc=5` | Token inválido: copiarlo de nuevo, sin espacios. |
| `WiFi Error: Connection to IoT Gateway ... failed` | Falla de la red de Wokwi: recargar la página, revisar bloqueadores del navegador o probar otra red. |
| El Serial muestra `enviado` pero el gráfico no cambia | El widget lee otra clave: verificar que sea `temperatura`, `humedad` o `alarma`, y que los estados del *State chart* sean numéricos. |

## Uso

Este repositorio es de libre acceso y libre uso.
