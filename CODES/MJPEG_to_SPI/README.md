


### TX
#### 1. Figer la fréquence d'horloge (Overclock & Force Turbo)

Pour éviter les micro-saccades induites par les changements de fréquences du CPU de la Raspberry Pi, on va forcer le processeur à tourner à sa fréquence maximale en permanence.

Ouvrir le fichier de configuration de boot de la Pi :

```bash
sudo nano /boot/firmware/config.txt

```
On ajoute les lignes suivantes tout à la fin du fichier :

```text
# Forcer le CPU à sa fréquence maximale sans jamais redescendre
force_turbo=1

# Pour une RPi 4B : 2000 MHz (2.0 GHz) au lieu de 1.5 GHz
arm_freq=2000
over_voltage=6

```

#### 2. Isoler un cœur CPU


```bash
sudo nano /boot/firmware/cmdline.txt

```

*(Sur une seule ligne, ne pas créer de retour à la ligne !)* Ajouter un espace à la toute fin du texte existant et insèrer ceci :
```text
isolcpus=3
```
```bash
sudo reboot
```

#### 3. Exécuter en priorité maximale sur le coeur isolé
```bash
gcc -O3 gateway_spi_noserv.c -o mjpeg_to_spi
./mjpeg_to_spi
```

```bash
sudo taskset -c 3 nice -n -20 ./mjpeg_to_spi
```

### RX
```bash
gst - launch -1.0 -v udpsrc port =1337 do - timestamp = true ! \
jpegparse ! jpegdec ! queue max - size - buffers =1 leaky = downstream ! \
videoconvert ! autovideosink sync = false
```
(Uniquement tester sur Windows...)
