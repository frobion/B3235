#include "Outils.h"
#include "Menu.h"
#include "config.h"


#include <unistd.h>
#include <signal.h>
#include <fstream>
#include <iostream>

static int descEcritureCanauxBarriereEntree [NB_BARRIERES_ENTREE];
static int descEcritureCanalBarriereSortie;
static int compteurImmatriculation;

static fstream fichier("test.txt");

void Simulation(int canauxBarriereEntree[][2], int canalBarriereSortie[])
{
  // TODO INIT
  
  compteurImmatriculation = 0;
  // Ouverture fichier (log)
  fstream fichier("test.txt");
  fichier << "Inir simulation" << std::endl;
  
  // Blocage SIGUSR1, SIGUSR2, SIGCHLD
  sigset_t listeSignalBloque;
  sigemptyset(&listeSignalBloque);
  sigaddset(&listeSignalBloque, SIGUSR1);
  sigaddset(&listeSignalBloque, SIGUSR2);
  sigaddset(&listeSignalBloque, SIGCHLD);
  
  sigprocmask(SIG_SETMASK, &listeSignalBloque, NULL);
  
  // Fermeture ressources passives non utilises
  
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

void destruction()
{
  // Fermeture canaux (3 + 1)
  for (unsigned int i = 0; i < NB_BARRIERES_ENTREE; i++)
  {
    close(descEcritureCanauxBarriereEntree[i]);
  }
  close(descEcritureCanalBarriereSortie);
  
  //delete fichier;
  exit(0);
}

void Commande (char code, unsigned int valeur)
{
 
  fichier << "Début commande" << std::endl;
  switch(code)
  {
    case 'E':
      fichier << "Case 'E'" << std::endl;
      destruction();
      break;
    case 'A':
      if (valeur == 1)
      {
		Voiture voiture = {TypeUsager::AUTRE, compteurImmatriculation++, 0};
        write(desEcritureCanalBarriereEntree[AUTRE_BLAISE_PASCAL - 1], &voiture, sizeof(Voiture));
      }
      else
      {
		  
	  }
      break;
    case 'P':
      if (valeur == 1)
      {
        
      } 
      else
      {
        
      }
      break;
    case 'S':
	  
	  break;
    default:
      fichier << "default" << std::endl;
      Afficher(MESSAGE, "Commande: code inconnue");
      
  }
  fichier << "Fin commane" << std::endl;
}
