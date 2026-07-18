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
#define GPIO_PIN 24
#define RING_BUFFER_SIZE (1024 * 64)

volatile unsigned *gpio;

void setup_gpiomem() {
    int mem_fd = open("/dev/gpiomem", O_RDWR | O_SYNC);
    if (mem_fd < 0) { perror("Erreur gpiomem"); exit(1); }
    gpio = (volatile unsigned *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, 0);
    close(mem_fd);
    *(gpio + 2) &= ~(7 << 12); // GPIO 24 en input
}

int wait_for_stm32_ready() {
    int timeout_counter = 0;
    // Attente que le STM32 soit ready
    while ((* (gpio + 13) & (1 << GPIO_PIN)) == 0) {
        timeout_counter++;
        if (timeout_counter > 2000000) { return 0; }
    }
    return 1;
}

int main() {
    int spi_fd;
    FILE *cam_pipe;
    int pipe_fd;
    uint32_t speed = 8000000; // 8 MHz
    uint8_t bits = 8;
    uint32_t mode = 0;

    setup_gpiomem();

    spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0) { perror("SPI open"); return 1; }
    ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
    ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    // 240p60
    const char *cmd = "rpicam-vid -t 0 -n -o - --width 320 --height 240 --framerate 60 "
                      "--codec mjpeg --denoise off --exposure sport --metering centre "
                      "--awb daylight --quality 8 --flush";

    printf(">>> Lancement de la capture camera directe...\n");
    cam_pipe = popen(cmd, "r");
    if (!cam_pipe) {
        perror("Echec du lancement");
        close(spi_fd);
        return 1;
    }

    pipe_fd = fileno(cam_pipe);
    int flags = fcntl(pipe_fd, F_GETFL, 0);
    fcntl(pipe_fd, F_SETFL, flags | O_NONBLOCK);

    uint8_t spi_buffer[CHUNK_SIZE];
    uint8_t *recv_buf = malloc(RING_BUFFER_SIZE);
    size_t recv_len = 0;
    int drop_counter = 0;

    struct spi_ioc_transfer tr = {
        .tx_buf = (uint64_t)(uintptr_t)spi_buffer,
        .rx_buf = 0,
        .len = CHUNK_SIZE,
        .speed_hz = speed,
        .bits_per_word = bits,
        .cs_change = 0,
    };

    printf(">>> Routage SPI actif\n");

    while (1) {
        int n = read(pipe_fd, recv_buf + recv_len, RING_BUFFER_SIZE - recv_len);

        if (n < 0) {
            usleep(1000);
            continue;
        }
        if (n == 0) {
            printf("Flux camera interrompu.\n");
            break;
        }
        recv_len += n;

        while (recv_len > 4) {
            int start_idx = -1;
            int end_idx = -1;

            for (size_t i = 0; i < recv_len - 1; i++) {
                if (recv_buf[i] == 0xFF && recv_buf[i+1] == 0xD8) { start_idx = i; break; }
            }
            if (start_idx == -1) { recv_len = 0; break; }

            for (size_t i = start_idx; i < recv_len - 1; i++) {
                if (recv_buf[i] == 0xFF && recv_buf[i+1] == 0xD9) { end_idx = i + 1; break; }
            }

            if (start_idx != -1 && end_idx != -1) {
                size_t jpeg_size = end_idx - start_idx + 1;

                uint8_t *frame_buffer = malloc(jpeg_size);
                memcpy(frame_buffer, recv_buf + start_idx, jpeg_size);

                size_t remaining = recv_len - (end_idx + 1);
                memmove(recv_buf, recv_buf + end_idx + 1, remaining);
                recv_len = remaining;

                size_t bytes_sent = 0;
                int frame_corrupted = 0;

                while (bytes_sent < jpeg_size) {
                    size_t to_send = jpeg_size - bytes_sent;
                    if (to_send > CHUNK_SIZE) to_send = CHUNK_SIZE;

                    memcpy(spi_buffer, frame_buffer + bytes_sent, to_send);

                    if (to_send < CHUNK_SIZE) {
                        memset(spi_buffer + to_send, 0x00, CHUNK_SIZE - to_send);
                    }

                    if (wait_for_stm32_ready()) {
                        if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 1) {
                            frame_corrupted = 1;
                            break;
                        }

                        int ack_timeout = 0;
                        while ((* (gpio + 13) & (1 << GPIO_PIN)) != 0) {
                            ack_timeout++;
                            if (ack_timeout > 50000) break; // Sécurité
                        }

                        drop_counter = 0;
                    } else {
                        drop_counter++;
                        if (drop_counter == 1 || drop_counter % 120 == 0) {
                            printf("[ATTENTION] Timeout STM32 ! (%d paquets droper)\n", drop_counter);
                        }
                        frame_corrupted = 1;
                        break;
                    }
                    bytes_sent += to_send;
                }

                free(frame_buffer);
                if (frame_corrupted) continue;

            } else {
                break;
            }
        }
    }

    free(recv_buf);
    pclose(cam_pipe);
    close(spi_fd);
    return 0;
}
