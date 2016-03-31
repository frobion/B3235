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
	int compteur;
};

const int numSemParking = 0;
const int numSemRequeteCompteur = 1;

#endif
