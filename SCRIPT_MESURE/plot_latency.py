import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

# --- Configuration ---
# Détermine le chemin absolu du dossier contenant ce script
script_dir = os.path.dirname(os.path.abspath(__file__))

# Construit les chemins absolus pour le CSV et l'image de sortie
CSV_FILE = os.path.join(script_dir, 'mesures.csv')
OUTPUT_IMG = os.path.join(script_dir, 'latency_plot.png')

OBJECTIF_MS = 80  # Le seuil max défini dans ton cahier des charges

# --- Chargement des données ---
try:
    df = pd.read_csv(CSV_FILE)
except FileNotFoundError:
    print(f"Erreur : Le fichier n'a pas été trouvé au chemin :\n{CSV_FILE}")
    exit()

latences = df['latence_ms'].values
echantillons = np.arange(1, len(latences) + 1)

# --- Calculs statistiques ---
moyenne = np.mean(latences)
pic_bas = np.min(latences)
pic_haut = np.max(latences)

print(f"--- Statistiques de latence ---")
print(f"Moyenne  : {moyenne:.2f} ms")
print(f"Pic bas  : {pic_bas} ms")
print(f"Pic haut : {pic_haut} ms")

# --- Création du graphique ---
plt.figure(figsize=(10, 6))

# Plot principal des mesures
plt.plot(echantillons, latences, marker='o', linestyle='-', color='#1f77b4', 
         linewidth=2, markersize=6, label='Mesure Glass-to-Glass')

# Ligne de moyenne
plt.axhline(moyenne, color='green', linestyle='--', linewidth=2, 
            label=f'Moyenne ({moyenne:.1f} ms)')

# Ligne du seuil limite (Objectif du Bachelor)
plt.axhline(OBJECTIF_MS, color='red', linestyle=':', linewidth=2, 
            label=f'Seuil d\'acceptabilité ({OBJECTIF_MS} ms)')

# Remplissage de la zone sous le seuil d'acceptabilité pour montrer que tout est bon
plt.fill_between(echantillons, 0, OBJECTIF_MS, color='green', alpha=0.05)

# --- Formatage et esthétique ---
plt.title('Mesures empiriques de la latence Glass-to-Glass (Flux MJPEG / Unicast)', fontsize=14, pad=15)
plt.xlabel('Numéro de l\'échantillon (Photographie)', fontsize=12)
plt.ylabel('Latence mesurée (ms)', fontsize=12)

# Définir l'échelle de l'axe Y pour bien voir (par exemple de 0 à 100ms)
plt.ylim(0, max(100, pic_haut + 20))
plt.grid(True, linestyle='--', alpha=0.7)
plt.legend(loc='upper right', fontsize=11)

# Ajout d'une annotation pour le pic bas et haut
plt.annotate(f'Min: {pic_bas} ms', xy=(np.argmin(latences)+1, pic_bas), 
             xytext=(0, -20), textcoords='offset points', ha='center', color='blue')
plt.annotate(f'Max: {pic_haut} ms', xy=(np.argmax(latences)+1, pic_haut), 
             xytext=(0, 10), textcoords='offset points', ha='center', color='red')

# --- Sauvegarde ---
plt.tight_layout()
plt.savefig(OUTPUT_IMG, dpi=300)
print(f"\nGraphique généré avec succès sous :\n{OUTPUT_IMG}")
plt.show()