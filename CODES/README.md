# Codes sources

Vous retrouverez ici les codes sources des différents éléments du système.
- Firmware_STM32 : contient un lien vers le commit du fork du SDK utilisé lors de la rédaction du rapport.
- glass_to_glass_latency_measurement : contient le script python permetant de mesurer la latence visuelle (un carré vert = 5ms, cette valeur doit être adaptée en fonction du framerate de votre écran)
- MJPEG_to_SPI : contient le code C a exécuter sur la RPI4B, pour générer le flux mjpeg et l'envoyer sur le
bus SPI.
- network_latency_measurement : voir rapport pour le montage, nécessite 2 rpi et permet de mesuré la latence purement réseau du système.
- 