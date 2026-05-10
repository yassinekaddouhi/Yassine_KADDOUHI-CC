
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Nombre de drones à générer
#define NB_DRONES 10000
// Limite pour basculer vers la recherche par force brute
#define LIMITE_PETIT_PROBLEME 32

// Structure représentant un drone avec ses coordonnées 3D
typedef struct Drone {
    int id;
    float x;
    float y;
    float z;
} Drone;

typedef struct Resultat {
    Drone *d1;
    Drone *d2;
    float distance2;
} Resultat;

float distance_carre(Drone *a, Drone *b) {
    float dx = a->x - b->x;
    float dy = a->y - b->y;
    float dz = a->z - b->z;

    return dx * dx + dy * dy + dz * dz;
}

int comparer_x(const void *a, const void *b) {
    const Drone *d1 = (const Drone *)a;
    const Drone *d2 = (const Drone *)b;

    if (d1->x < d2->x) return -1;
    if (d1->x > d2->x) return 1;
    return 0;
}

int comparer_y(const void *a, const void *b) {
    const Drone *d1 = (const Drone *)a;
    const Drone *d2 = (const Drone *)b;

    if (d1->y < d2->y) return -1;
    if (d1->y > d2->y) return 1;
    return 0;
}

void generer_drones(Drone *essaim, int n) {
    Drone *p = essaim;
    Drone *fin = essaim + n;
    int id = 1;

    while (p < fin) {
        p->id = id;
        p->x = (float)(rand() % 100000) / 100.0f;
        p->y = (float)(rand() % 100000) / 100.0f;
        p->z = (float)(rand() % 100000) / 100.0f;

        p = p + 1;
        id = id + 1;
    }
}

// Recherche par force brute : compare tous les couples de drones pour trouver la distance minimale
Resultat recherche_brute(Drone *debut, int n) {
    Resultat r;
    r.d1 = debut;
    r.d2 = debut + 1;
    r.distance2 = distance_carre(r.d1, r.d2);

    Drone *i = debut;
    Drone *fin = debut + n;

    while (i < fin) {
        Drone *j = i + 1;

        while (j < fin) {
            float d2 = distance_carre(i, j);

            if (d2 < r.distance2) {
                r.distance2 = d2;
                r.d1 = i;
                r.d2 = j;
            }

            j = j + 1;
        }

        i = i + 1;
    }

    return r;
}

// Compare deux résultats et retourne le meilleur (distance minimale)
Resultat meilleur_resultat(Resultat a, Resultat b) {
    if (a.distance2 < b.distance2) {
        return a;
    }

    return b;
}

// trouve récursivement les deux drones les plus proches
// Utilise la "bande" pour améliorer l'efficacité en ne comparant que les drones près du point de division
Resultat plus_proches_rec(Drone *debut, int n) {
    //  utiliser la recherche par force brute pour les petits problèmes
    if (n <= LIMITE_PETIT_PROBLEME) {
        return recherche_brute(debut, n);
    }

    //  partager le problème en deux moitiés
    int milieu = n / 2;
    Drone *ptr_milieu = debut + milieu;
    float x_milieu = ptr_milieu->x;

    // Résoudre récursivement pour chaque moitié
    Resultat gauche = plus_proches_rec(debut, milieu);
    Resultat droite = plus_proches_rec(ptr_milieu, n - milieu);
    Resultat meilleur = meilleur_resultat(gauche, droite);

    // Allouer la mémoire pour la bande autour du point de division
    Drone *bande = malloc(n * sizeof(Drone));

    if (bande == NULL) {
        printf("Erreur allocation memoire pour la bande.\n");
        exit(1);
    }

    // Extraire les drones dans la bande (distance X < distance minimale actuelle)
    int compteur = 0;
    Drone *p = debut;
    Drone *fin = debut + n;

    while (p < fin) {
        float dx = p->x - x_milieu;

        if (dx * dx < meilleur.distance2) {
            *(bande + compteur) = *p;
            compteur = compteur + 1;
        }

        p = p + 1;
    }

    // Trier la bande par coordonnée Y pour améliorer les performances
    qsort(bande, compteur, sizeof(Drone), comparer_y);

    // Vérifier les couples dans la bande (seulement ceux dont la distance Y < distance minimale)
    Drone *i = bande;
    Drone *fin_bande = bande + compteur;

    while (i < fin_bande) {
        Drone *j = i + 1;

        while (j < fin_bande) {
            float dy = j->y - i->y;

            // Optimisation : sortir si la distance Y dépasse déjà la distance minimale
            if (dy * dy >= meilleur.distance2) {
                break;
            }

            float d2 = distance_carre(i, j);

            if (d2 < meilleur.distance2) {
                meilleur.distance2 = d2;
                meilleur.d1 = i;
                meilleur.d2 = j;
            }

            j = j + 1;
        }

        i = i + 1;
    }

    // Libérer la mémoire allouée pour la bande
    free(bande);

    return meilleur;
}

int main(void) {
    // Initialiser le générateur aléatoire avec la graine du temps
    srand((unsigned int)time(NULL));

    // Allouer la mémoire pour le tableau de drones
    Drone *essaim = malloc(NB_DRONES * sizeof(Drone));

    if (essaim == NULL) {
        printf("Erreur allocation memoire.\n");
        return 1;
    }

    // Générer les drones avec des coordonnées aléatoires
    generer_drones(essaim, NB_DRONES);

    // Trier les drones par coordonnée X (étape préparatoire pour l'algorithme diviser-pour-régner)
    qsort(essaim, NB_DRONES, sizeof(Drone), comparer_x);

    // Trouver les deux drones les plus proches
    Resultat r = plus_proches_rec(essaim, NB_DRONES);

    // Afficher les résultats
    printf("Drone 1 : ID = %d | x = %.2f | y = %.2f | z = %.2f\n",
           r.d1->id, r.d1->x, r.d1->y, r.d1->z);

    printf("Drone 2 : ID = %d | x = %.2f | y = %.2f | z = %.2f\n",
           r.d2->id, r.d2->x, r.d2->y, r.d2->z);

    printf("Distance minimale = %.4f\n", sqrtf(r.distance2));

    // Libérer la mémoire allouée
    free(essaim);

    return 0;
}