#ifndef CONFIG_H
#define CONFIG_H

#include "Outils.h"

struct Voiture
{
  TypeUsager usager;
  int immatriculation;
  int dateArrive;
};

struct ParkingMP
{
  Voiture parking [8]; 
};

struct RequetesMP
{
	Voiture requetes [3];
	int nbPlacesOccupees;
};

const int NUM_SEM_PROF_BLAISE_PASCAL = 0;
const int NUM_SEM_AUTRE_BLAISE_PASCAL = 1;
const int NUM_SEM_GASTON_BERGER = 2;
const int NUM_SEM_PARKING = 3;
const int NUM_SEM_REQUETE = 4;

const struct sembuf INCR_DANS_PARKING = {{NUM_SEM_PARKING, 1, 0}};
const struct sembuf DECR_DANS_PARKING = {{NUM_SEM_PARKING, -1, 0}};
const struct sembuf INCR_DANS_REQUETE = {{NUM_SEM_REQUETE, 1, 0}};
const struct sembuf DECR_DANS_REQUETE = {{NUM_SEM_REQUETE, -1, 0}};

#endif
