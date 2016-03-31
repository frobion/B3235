#include <signal.h>
#include <unistd.h>
#include <sys/msg.h>
#include <algorithm>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <vector>
#include <errno.h>
#include <time.h>

#include "Entree.h"
#include "Outils.h"
#include "config.h"

static ParkingMP shmParking;
static RequeteMP shmRequete;

static vector<pid_t> listeFils;


// ------ Handler SIGUSR2 --------
static void HandlerUSR2 ( int noSig )
{
  //  DESTRUCTION

  // Destruction de tous les deplacements
  for(unsigned int i = 0; i < listeFils.size(); i++)
  {
  	kill(listeFils[i],SIGUSR2);
  	waitpid(listeFils[i],0,0);
  }

  // Detachement de la memoire

  shmdt(shmParking);
  shmdt(shmRequete);

  exit(0);
}

// TODO : HANDLER SIGCHLD
// Mise a jour de memoire partagee parking quand voiturier meurt

void GestionEntree(int canalEntree[][2], int canalSortie[2], TypeBarriere typeEntree, int shmIdParking, int shmIdRequete, int semId)
{
    // --  INITIALISATION  --
	
	
	// Canaux : On ferme tout sauf en lecture sur la barriere concernee
	 int numBarriere = typeEntree -1;
	 
	 const struct sembuf DECR_DANS_ENTREE = {{numBarriere, -1, 0}};
	 
	 for(unsigned int barriereEntree = 0; barriereEntree < NB_BARRIERES_ENTREE ; barriereEntree++)
	 {
		 if(barriereEntree != numBarriere) 
		 {
			close(canalEntree[barriereEntree][0]);
			close(canalEntree[barriereEntree][1]);
	     }
	     else
	     { 
			 close(canalEntree[barriereEntree][1]);
		 }
     }
     
     
     for(unsigned int barriereSortie = 0; barriereSortie < NB_BARRIERES_SORTIE ; barriereSortie++)
	 {
		close(canalEntree[barriereSortie][0]);
		close(canalEntree[barriereSortie][1]);
     }

     
    // Creation des handlers de SIGUSR2
    struct sigaction actionHandlerUSR2;
    actionHandlerUSR2.sa_handler = HandlerUSR2;
    sigemptyset(&actionHandlerUSR2.sa_mask); // vide le masque
    actionHandlerUSR2.sa_flags = 0;
    // HandlerSIGUSR2 ne doit pas etre interrompu par SIGCHLD
    sigaddset(&actionHandlerUSR2.sa_mask, SIGCHLD);

    // creation des handlers de SIGCHLD
    struct sigaction actionHandlerCHLD;
    actionHandlerCHLD.sa_handler = HandlerCHLD;
    sigemptyset(&actionHandlerCHLD.sa_mask);
    actionHandlerCHLD.sa_flags = 0;

    // -- Attachement des segments de memoires partagées --     
	 ParkingMP* ParkingMPPtr =(ParkingMP*) shmat(shmIdParking, NULL, 0);
	 RequeteMP* RequeteMPPtr =(RequeteMP*) shmat(shmIdRequete, NULL, 0);

    // -- Armement des signaux SIGUSR2 et SIGCHILD
    sigaction(SIGUSR2, &actionHandlerUSR2, NULL);
    sigaction(SIGCHLD, &actionHandlerCHLD, NULL);



    // --  MOTEUR --
    while(true)
    {
		
        Voiture voitRecue;
        pid_t pidVoiturier;
        
        // On attend qu'une voiture arrive
        while(read(canalEntree[numBarriere][0], &voitRecue , sizeof(Voiture) ) == -1 && errNo == EINTR ) ;
		time_t heureArrivee = time();
		voitRecue->dateArrive = heureArrivee;
		
		// Quand une voiture arrive a une barriere
        // On dessine la voiture a la barriere
        DessinerVoitureBarriere(typeEntree, voitRecue->usager);

        
        // On recupere le semaphore de Requete
        while(semop(semId, DECR_DANS_REQUETE, 1) == -1 && errno == EINTR);
        
        // Si y a de une place de libre, on gare la voiture, sinon 
        if(RequeteMPPtr->nbPlacesOccupees < NB_PLACES )
        {
			RequeteMPPtr->nbPlacesOccupees++;
			while(semop(semId, INCR_DANS_REQUETE, 1) == -1 && errno == EINTR);
			pidVoiturier = GarerVoiture(typeEntree);
        }
        else
        {
			RequeteMPPtr->requetes[numBarriere]->voiture = voitRecue;
			RequeteMPPtr->requetes[numBarriere]->barriere = typeEntree;
			while(semop(semId, DECR_DANS_ENTREE, 1) == -1 && errno == EINTR);
			pidVoiturier = GarerVoiture(typeEntree);
		}	
        

        // on garde le pid du Voiturier
		listeFils.push_back(pidVoiturier);


    }
}
