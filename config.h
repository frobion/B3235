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

const int NUM_SEM_PARKING = 0;
const int NUM_SEM_REQUETE = 1;

#endif
