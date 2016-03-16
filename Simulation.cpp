#include "Outils.h"
#include "Menu.h"


#include <unistd.h>
#include <signal.h>

static int [NB_BARRIERES_ENTREE] descEcritureCanauxBarriereEntree;
static int descEcritureCanalBarriereSortie;

void Simulation(int canauxBarriereEntree[][2], int canalBarriereSortie[])
{
  // TODO INIT
  
  // Blocage SIGUSR1, SIGUSR2, SIGCHLD
  sigset_t listeSignalBloque;
  sigemptyset(&listeSignalBloque);
  sigaddset(&listeSignalBloque, SIGUSR1);
  sigaddset(&listeSignalBloque, SIGUSR2);
  sigaddset(&listeSignalBloque, SIGCHLD);
  
  sigprocmask(SIG_SETMASK, &listeSignalBloque, NULL);
  
  // Fermeture de canaux (3 + 1)
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
    close(descEcritureCanauxBarriereEntree[i];
  }
  close(descEcritureCanalBarriereSortie);
  exit(0);
}

void Commande (char code, unsigned int valeur)
{
  switch(code)
  {
    case 'E':
      destruction();
      break;
    case 'A':
      if (valeur == 1)
      {
        
      } else if (valeur == 2)
      {
        
      }
      else
      {
        
      }
      break;
    case 'P':
            if (valeur == 1)
      {
        
      } else if (valeur == 2)
      {
        
      }
      else
      {
        
      }
    default:
      Afficher(TypeZone::MESSAGE, "Commande: code inconnue");
      
  }
}
