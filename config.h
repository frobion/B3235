#ifndef CONFIG_H
#define CONFIG_H


#include <ctime>
#include <sys/sem.h>

#include "Outils.h"

struct Voiture
{
  TypeUsager usager;
  int immatriculation;
  time_t dateArrive;
};

struct Requete
{
  Voiture voiture;
  TypeBarriere barriere;
};

struct ParkingMP
{
  Voiture parking [8]; 
};

struct RequeteMP
{
	Requete requetes [3];
	unsigned int nbPlacesOccupees;
};

const int NUM_SEM_PROF_BLAISE_PASCAL = 0;
const int NUM_SEM_AUTRE_BLAISE_PASCAL = 1;
const int NUM_SEM_GASTON_BERGER = 2;
const int NUM_SEM_PARKING = 3;
const int NUM_SEM_REQUETE = 4;

//~ const struct sembuf INCR_DANS_PARKING [1] = {{NUM_SEM_PARKING, 1, 0}};
//~ const struct sembuf DECR_DANS_PARKING [1] = {{NUM_SEM_PARKING, -1, 0}};
//~ const struct sembuf INCR_DANS_REQUETE [1] = {{NUM_SEM_REQUETE, 1, 0}};
//~ const struct sembuf DECR_DANS_REQUETE [1] = {{NUM_SEM_REQUETE, -1, 0}};
//~ 
//~ const struct sembuf INCR_DANS_PROF_BLAISE_PASCAL [1] = {{NUM_SEM_PROF_BLAISE_PASCAL, 1, 0}};
//~ const struct sembuf INCR_DANS_AUTRE_BLAISE_PASCAL [1] = {{NUM_SEM_AUTRE_BLAISE_PASCAL, 1, 0}};
//~ const struct sembuf INCR_DANS_GASTON_BERGER [1] = {{NUM_SEM_GASTON_BERGER, 1, 0}};

#endif



