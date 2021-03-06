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
	unsigned int nbPlacesOccupees;
	Requete requetes [3];
};

const int NUM_SEM_PROF_BLAISE_PASCAL = 0;
const int NUM_SEM_AUTRE_BLAISE_PASCAL = 1;
const int NUM_SEM_GASTON_BERGER = 2;
const int NUM_SEM_PARKING = 3;
const int NUM_SEM_REQUETE = 4;

const int TEMPO = 1; // temporisation d'attente avant l'arrivee d'un nouveau vehicule

const int TEMPS_MAX = 100000;
const int DECIMAL_TEMPS = 10000;

#endif
