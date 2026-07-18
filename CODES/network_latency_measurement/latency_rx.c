#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define GPIO_BTN 17 
#define PORT 1337   

volatile unsigned *gpio;
volatile uint64_t t_start = 0;
volatile uint64_t t_end = 0;
volatile int packet_count = 0;

void setup_gpiomem() {
    int mem_fd = open("/dev/gpiomem", O_RDWR | O_SYNC);
    if (mem_fd < 0) { perror("Erreur gpiomem"); exit(1); }
    gpio = (volatile unsigned *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, 0);
    close(mem_fd);
    *(gpio + 1) &= ~(7 << 21); // GPIO 17 en ENTREE
}

uint64_t get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

// Thread reseau de reception UDP
void* udp_listener(void* arg) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);
    bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));

    uint8_t buffer[2000];
    
    while(1) {
        recvfrom(sockfd, (char *)buffer, sizeof(buffer), 0, NULL, NULL);
        t_end = get_time_ns(); // Mise a jour de l'heure d'arrivee
        packet_count++;        // Incremente le compteur de paquets recus
    }
    return NULL;
}

int main() {
    setup_gpiomem();
    
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, udp_listener, NULL);

    int test_step = 0;
    double sum_latencies = 0;

    printf(">>> RX CHRONO PRET. Initialisation...\n");

    while (1) {
        // Preparation des variables partagees avant l'appui du bouton
        packet_count = 0;
        
        // 1. Attente de l'appui du bouton physique commun
        while ( (*(gpio + 13) & (1 << GPIO_BTN)) == 0 );

        // 2. Lancement immediat du chronometre de test
        t_start = get_time_ns();
        
        int target_packets = (test_step < 10) ? 1 : 10;
        
        // 3. Attente de la reception du ou des paquets reseau
        while (packet_count < target_packets) {
            if (get_time_ns() - t_start > 1500000000ULL) { // Timeout de securite de 1.5s
                printf("[ERREUR] Timeout ! Seulement %d/%d paquet(s) recu(s).\n", packet_count, target_packets);
                test_step = 0; 
                sum_latencies = 0;
                break;
            }
        }

        // 4. Calculs de latence et Affichage
        if (packet_count == target_packets) {
            double latency_ms = (t_end - t_start) / 1000000.0;
            
            if (test_step < 10) {
                // Phase 1 : Test unitaire
                printf("Test %d/10 | Latence : %.3f ms\n", test_step + 1, latency_ms);
                sum_latencies += latency_ms;
                
                if (test_step == 9) {
                    printf("------------------------------------------\n");
                    printf("MOYENNE UNITAIRE : %.3f ms\n", sum_latencies / 10.0);
                    printf("------------------------------------------\n");
                }
                test_step++;
            } else {
                // Phase 2 : Test en rafale
                printf("==========================================\n");
                printf("LATENCE BURST TOTALE (10 paquets) : %.3f ms\n", latency_ms);
                printf("MOYENNE EN FLUX (Latence / 10)    : %.3f ms\n", latency_ms / 10.0);
                printf("==========================================\n\n");
                
                // Fin du cycle de test, reinitialisation
                test_step = 0;
                sum_latencies = 0;
            }
        }

        // Anti-rebond
        usleep(500000); 
    }
    return 0;
}