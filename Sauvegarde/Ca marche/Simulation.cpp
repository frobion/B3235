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

static fstream fichier("LogSimulation.txt");

//------------------------------------------------------ Fonctions privées
static void destruction()
{
  // Fermeture canaux (3 + 1)
  fichier << time(NULL)%TEMPS_MAX << "  " << "destruction" << std::endl;
  for (unsigned int i = 0; i < NB_BARRIERES_ENTREE; i++)
  {
    close(descEcritureCanauxBarriereEntree[i]);
  }
  close(descEcritureCanalBarriereSortie);
  
  //delete simulation;
  fichier.close();
  exit(0);
}

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques

void Simulation(int canauxBarriereEntree[][2], int canalBarriereSortie[])
{
  // TODO INIT
  
  compteurImmatriculation = 0;
  fichier << time(NULL)%TEMPS_MAX << "  " << "Init simulation" << std::endl;
  
  // Fermeture de canaux en lecture (3 + 1)
  for (unsigned int i = 0; i < NB_BARRIERES_ENTREE; i++)
  {
    close(canauxBarriereEntree[i][0]);
    descEcritureCanauxBarriereEntree[i] = canauxBarriereEntree[i][1];
  }
  close(canalBarriereSortie[0]);
  descEcritureCanalBarriereSortie = canalBarriereSortie[1];
 
 
  // Phase moteur
  for(;;)
  {
    Menu();
  }
}

void Commande (char code, unsigned int valeur)
{
 
  fichier << time(NULL)%TEMPS_MAX << "  " << "Début commande" << std::endl;
  switch(code)
  {
    case 'E':
    {
	  Afficher(MESSAGE, "case E");
      fichier << time(NULL)%TEMPS_MAX << "  " << "Case 'E'" << std::endl;
      destruction();
      break;
    }
    case 'A':
	{
	  Afficher(MESSAGE, "case A");
	  fichier << time(NULL)%TEMPS_MAX << "  " << "Case 'A'" << std::endl;
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
	  Afficher(MESSAGE, "case P");
	  fichier << time(NULL)%TEMPS_MAX << "  " << "Case 'P'" << std::endl;
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
	  Afficher(MESSAGE, "case S");
	  fichier << time(NULL)%TEMPS_MAX << "  " << "Case 'S'" << std::endl;
	  write(descEcritureCanalBarriereSortie, &valeur, sizeof(unsigned int));
	  break;
	}
    default:
      fichier << time(NULL)%TEMPS_MAX << "  " << "default" << std::endl;
      Afficher(MESSAGE, "Commande: code inconnue");
      
  }
  fichier << time(NULL)%TEMPS_MAX << "  " << "Fin commande" << std::endl;
}
