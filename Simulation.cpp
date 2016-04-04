/*************************************************************************
                           Simulation.cpp  -  description
                             -------------------
    début                : 14/03/2016
    copyright            : (C) 2016 par frobion
    e-mail               : francois.robion@insa-lyon.fr
*************************************************************************/

//---------- Réalisation de la tâche <Simulation> (fichier Simulation.cpp)

/////////////////////////////////////////////////////////////////  INCLUDE
//-------------------------------------------------------- Include système
#include <unistd.h>
#include <signal.h>
#include <fstream>
#include <iostream>

//------------------------------------------------------ Include personnel
#include "Simulation.h"
#include "Outils.h"
#include "Menu.h"
#include "config.h"

///////////////////////////////////////////////////////////////////  PRIVE
//------------------------------------------------------------- Constantes
const int IMMATRICULATION_MAX = 1000;

//------------------------------------------------------------------ Types

//---------------------------------------------------- Variables statiques
static int descEcritureCanauxBarriereEntree [NB_BARRIERES_ENTREE];
static int descEcritureCanalBarriereSortie;
static int compteurImmatriculation;

//------------------------------------------------------ Fonctions privées
static void destruction()
{
  // Fermeture canaux (3 + 1)
  for (unsigned int i = 0; i < NB_BARRIERES_ENTREE; i++)
  {
    close(descEcritureCanauxBarriereEntree[i]);
  }
  close(descEcritureCanalBarriereSortie);
  
  //delete simulation;
  exit(0);
}

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques

void Simulation(int canauxBarriereEntree[][2], int canalBarriereSortie[])
{  
  // Fermeture de canaux en lecture (3 + 1)
  for (unsigned int i = 0; i < NB_BARRIERES_ENTREE; i++)
  {
    close(canauxBarriereEntree[i][0]);
    descEcritureCanauxBarriereEntree[i] = canauxBarriereEntree[i][1];
  }
  close(canalBarriereSortie[0]);
  descEcritureCanalBarriereSortie = canalBarriereSortie[1];
 
  compteurImmatriculation = 0;
  
  // Phase moteur
  for(;;)
  {
    Menu();
  }
}

void Commande (char code, unsigned int valeur)
{
 
  switch(code)
  {
    case 'E':
    {
      destruction();
      break;
    }
    case 'A':
	{
	  Voiture voiture = {TypeUsager::AUTRE, compteurImmatriculation, -1};
	  compteurImmatriculation = (compteurImmatriculation + 1) % IMMATRICULATION_MAX;
      if (valeur == 1)
      {		
        write(descEcritureCanauxBarriereEntree[AUTRE_BLAISE_PASCAL - 1], &voiture, sizeof(Voiture));
      }
      else
      {
		write(descEcritureCanauxBarriereEntree[ENTREE_GASTON_BERGER - 1], &voiture, sizeof(Voiture));
	  }
      break;
	}
    case 'P':
    {
      Voiture voiture = {TypeUsager::PROF, compteurImmatriculation, -1};
      compteurImmatriculation = (compteurImmatriculation + 1) % IMMATRICULATION_MAX;
      if (valeur == 1)
      {
        write(descEcritureCanauxBarriereEntree[PROF_BLAISE_PASCAL - 1], &voiture, sizeof(Voiture));
      } 
      else
      {
        write(descEcritureCanauxBarriereEntree[ENTREE_GASTON_BERGER - 1], &voiture, sizeof(Voiture));
      }
      break;
    }
    case 'S':
    {
	  write(descEcritureCanalBarriereSortie, &valeur, sizeof(unsigned int));
	  break;
	}
    default:
      Afficher(MESSAGE, "Commande: code inconnue");
  }
}
