import pygame
import sys
import time

# Configuration
GRID_COLS = 10
GRID_ROWS = 6
SQUARE_SIZE = 60
INTERVAL_MS = 5  # Précision de la mesure

TOTAL_SQUARES = GRID_COLS * GRID_ROWS

pygame.init()
window_size = (GRID_COLS * SQUARE_SIZE, GRID_ROWS * SQUARE_SIZE)
screen = pygame.display.set_mode(window_size)
pygame.display.set_caption("Mire de Latence Haute Précision")

clock = pygame.time.Clock()
start_time = time.perf_counter()
current_counter = 0

print(f"Mire lancée. Résolution temporelle : {INTERVAL_MS}ms par bloc.")
print(f"Un cycle complet vaut : {TOTAL_SQUARES * INTERVAL_MS}ms.")

running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    # Calcul du temps écoulé
    elapsed_ms = (time.perf_counter() - start_time) * 1000
    
    # nombre de carrés à allumer
    current_counter = int(elapsed_ms / INTERVAL_MS) % TOTAL_SQUARES

    # Dessin de la grille
    screen.fill((20, 20, 20))
    
    for i in range(TOTAL_SQUARES):
        row = i // GRID_COLS
        col = i % GRID_COLS
        rect = pygame.Rect(col * SQUARE_SIZE + 2, row * SQUARE_SIZE + 2, SQUARE_SIZE - 4, SQUARE_SIZE - 4)
        
        if i <= current_counter:
            # Carré allumé 
            pygame.draw.rect(screen, (0, 255, 100), rect)
        else:
            # Carré éteint 
            pygame.draw.rect(screen, (40, 40, 40), rect)

    pygame.display.flip()
    
    clock.tick(500)

pygame.quit()
sys.exit()