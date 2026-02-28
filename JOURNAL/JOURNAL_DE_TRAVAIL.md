# Journal de travail
Nathan Tschantz, printemps 2026.

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
### Samedi 28.02
- fini la première version du cahier des charges.

#### Gantt
_Note :Le Gantt sera update au fure et a mesure du projets lorsque les tâches se préciserons_ 
```mermaid
gantt
    title Planning 450h - Drone FPV HaLow
    dateFormat  YYYY-MM-DD
    axisFormat  %d/%m

    excludes 2026-03-09,2026-03-10,2026-03-11,2026-03-12,2026-03-13,2026-04-06,2026-04-07,2026-04-08,2026-04-09,2026-04-10
    
    section Indisponible
    Crunch      :2026-03-09, 2026-03-14
    Vacances    :2026-04-06, 2026-04-11

    section Semestre (15h/sem)
    Faisabilité Driver :done, p1, 2026-02-16, 2026-02-28
    Setup SDK MM       :active, p1, 2026-02-29, 2026-03-06
    ekh19 desactiver encryption-monitor mode :active, p1, 2026-02-29, 2026-03-06
    Dév. Segment TX (UDP)    : tx1,2026-03-06, 2026-06-14
    Dév. Segment RX (RTOS)   :rx1, 2026-03-06, 2026-06-14

    section Post-Semestre (40h/sem)
    Intégration & MVP        :crit, int1, 2026-06-15, 2026-07-08
    Tests Vidéo & Portée     :int2, 2026-07-08, 2026-07-23
    Préparation Défense      :int3, 2026-07-16, 2026-07-23

    section Documentation
    Rédaction du CDC         :active, doc1, 2026-02-16, 2026-03-09
    Journal de bord et Note jouenalière :active, doc2, 2026-02-16, 2026-07-23
    Rédaction Rapport : Étude & TX :doc3, 2026-02-27, 2026-05-15
    Rédaction Rapport : RX & Intégration :doc4, 2026-05-16, 2026-07-10
    Finalisation & Mise en page :doc5, 2026-07-11, 2026-07-20
```

## Mars

## Avril

## Mai

### Mecredi 20.05 15h00
**Rendu intermédiaire**

## Juin
## Juillet

### Jeudi 23.07 avant 11h00
**Rendu final du rapport**