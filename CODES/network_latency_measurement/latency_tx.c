#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <sys/mman.h>

#define CHUNK_SIZE 1400
#define SPI_DEVICE "/dev/spidev0.0"

#define GPIO_BTN 17       // Pin physique 11 (Le Bouton)
#define GPIO_HANDSHAKE 24 // Pin physique 18 (Le STM32)

volatile unsigned *gpio;

void setup_gpiomem() {
    int mem_fd = open("/dev/gpiomem", O_RDWR | O_SYNC);
    if (mem_fd < 0) { perror("Erreur gpiomem"); exit(1); }
    gpio = (volatile unsigned *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, 0);
    close(mem_fd);

    *(gpio + 2) &= ~(7 << 12); // GPIO 24 en ENTREE
    *(gpio + 1) &= ~(7 << 21); // GPIO 17 en ENTREE
}

int main() {
    int spi_fd;
    uint32_t speed = 8000000; // 8 MHz
    uint8_t bits = 8;
    uint32_t mode = 0;

    setup_gpiomem();

    spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0) { perror("SPI open"); return 1; }

    ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
    ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    uint8_t buffer[CHUNK_SIZE];
    memset(buffer, 0xAA, CHUNK_SIZE);

    struct spi_ioc_transfer tr = {
        .tx_buf = (uint64_t)(uintptr_t)buffer,
        .rx_buf = 0,
        .len = CHUNK_SIZE,
        .speed_hz = speed,
        .bits_per_word = bits,
        .cs_change = 0,
    };

    int test_step = 0;

    while (1) {
        if (test_step < 10) {
            printf("\n>>> [UNITAIRE %d/10] Appuie sur le bouton pour envoyer 1 paquet...\n", test_step + 1);
        } else {
            printf("\n>>> [BURST TEST] Appuie sur le bouton pour envoyer la RAFALE de 10 paquets...\n");
        }

        // 1. Attente du BOUTON
        // *(gpio + 13) lit les niveaux de pins d'entrees
        while ( (*(gpio + 13) & (1 << GPIO_BTN)) == 0 );

        if (test_step < 10) {
            // --- ENVOI UNITAIRE ---
            int timeout = 0, stm_ready = 1;
            while ( (*(gpio + 13) & (1 << GPIO_HANDSHAKE)) == 0 ) {
                if (timeout++ > 5000000) { stm_ready = 0; break; }
            }
            if (stm_ready) {
                ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
                printf(" -> Paquet envoye.\n");
                test_step++;
            } else {
                printf("[ERREUR] STM32 bloque.\n");
            }
        } else {
            // --- ENVOI EN RAFALE (BURST) ---
            int paquets_ok = 0;
            for (int i = 0; i < 10; i++) {
                int timeout = 0, stm_ready = 1;
                while ( (*(gpio + 13) & (1 << GPIO_HANDSHAKE)) == 0 ) {
                    if (timeout++ > 5000000) { stm_ready = 0; break; }
                }
                if (!stm_ready) {
                    printf("[ERREUR] STM32 a plante au paquet %d.\n", i+1);
                    break;
                }
                ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
                paquets_ok++;
                usleep(100); // 100us pour laisser le STM32 reagir
            }
            if (paquets_ok == 10) {
                printf(" -> Rafale de 10 paquets envoyee !\n");
                test_step = 0; // Reinitialise le cycle de test
            }
        }

        // Anti-rebond du bouton
        usleep(500000); 
    }

    close(spi_fd);
    return 0;
}