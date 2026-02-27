# Journal de travail
Nathan Tschantz, printemps 2026.

## Gantt 
LES TACHES SONT PUREMENT FACTICE POUR TESTER LA CAPACITE DE mermaid POUR LA RÉALISATION DU DIAGRAMME DE Gantt
```mermaid
gantt
title Planning Bachelor Transmission vidéo FPV
dateFormat  YYYY-MM-DD
axisFormat %d/%m

section Préparation
Analyse des spécifications et état de l'art :a1, 2026-03-01, 14d
Fork & Setup SDK Morse Micro / STM32CubeIDE :a2, after a1, 7d

section Développement RTOS
Implémentation pilotes de base (SPI/UART) :a3, after a2, 14d
Développement tâches FreeRTOS principales :a4, after a3, 21d
Intégration pile Wi-Fi HaLow :a5, after a4, 14d

section Tests & Validation
Tests unitaires & d'intégration :a6, after a5, 14d
Tests de performance réseau :a7, after a6, 10d

section Rédaction
Rédaction chapitres techniques :a8, 2026-04-15, 30d
Finalisation rapport & correction :a9, after a7, 14d
```

## Février
### Vendredi 20.02
- Lancement du travail de bachelor. 
- Première recherche sur la faisabilité de l'utilisation du MM8108-EKH19. 
- recherche d'option possible pour set le MCS et le code correcteur LDPC.

7h30.
### Samedi - Mardi 21.02 - 24.02

- compilation du kernel et du driver
- modification du driver 
- test du driver modifier sur le module wifi
- Les options semble être setter et la fonction voulue

sais plus combien de temps ~10h
### Vendredi 27.02
- clarification de la strucuture du projet
- rédaction de rapport et du cahier des charges
- Lecture de documentation RTOS et EKH05

## Mars

## Avril

## Mai

### Mecredi 20.05 15h00
**Rendu intermédiaire**

## Juin
## Juillet

### Jeudi 23.07 avant 11h00
**Rendu final du rapport**