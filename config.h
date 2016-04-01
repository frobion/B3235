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

#endif



