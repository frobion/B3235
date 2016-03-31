#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <vector>
#include <algorithm>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#include "Entree.h"
#include "Outils.h"
#include "config.h"


struct pidVoiture 
{ 
	pid_t pid;
	Voiture voiture;
};


static ParkingMP* ParkingMPPtr;
static RequeteMP* RequeteMPPtr;

static vector<pidVoiture> listeFils;

static int semId;

static struct sembuf incrSemParking [1] = {{NUM_SEM_PARKING, 1, 0}};
static struct sembuf decrSemParking [1] = {{NUM_SEM_PARKING, -1, 0}};
static struct sembuf incrSemRequete [1] = {{NUM_SEM_REQUETE, 1, 0}};
static struct sembuf decrSemRequete [1] = {{NUM_SEM_REQUETE, -1, 0}};
static struct sembuf decrSemEntree [1];




// ------ Handler SIGUSR2 --------
static void HandlerUSR2 ( int noSig )
{
  //  DESTRUCTION

  // Destruction de tous les deplacements
  for(unsigned int i = 0; i < listeFils.size(); i++)
  {
  	kill(listeFils[i].pid,SIGUSR2);
  	waitpid(listeFils[i].pid,0,0);
  }

  // Detachement de la memoire

  shmdt(ParkingMPPtr);
  shmdt(RequeteMPPtr);

  exit(0);
}

static void HandlerCHLD(int noSig)
{
	int numPlace;
	pid_t pidDestruct = waitpid(-1 ,&numPlace,0);
	
	Voiture voitureGarer;
	std::vector<pidVoiture>::iterator ite;	
	for(ite = listeFils.begin(); ite != listeFils.end(); ite++)
	{
		if((*ite).pid == pidDestruct) 
		{
			voitureGarer = (*ite).voiture;
			listeFils.erase(ite);
			break;
		}
	}
	
	while(semop(semId, decrSemParking, 1) == -1 && errno == EINTR);
	ParkingMPPtr->parking[numPlace] = voitureGarer;
	AfficherPlace(numPlace, voitureGarer.usager, voitureGarer.immatriculation, voitureGarer.dateArrive);
	while(semop(semId, incrSemParking, 1) == -1 && errno == EINTR);	
	
}

// TODO : HANDLER SIGCHLD
// Mise a jour de memoire partagee parking quand voiturier meurt

void GestionEntree(int canalEntree[][2], int canalSortie[2], TypeBarriere typeEntree, int shmIdParking, int shmIdRequete, int semIdParam)
{
    // --  INITIALISATION  --
	
	semId = semIdParam;
	// Canaux : On ferme tout sauf en lecture sur la barriere concernee
	 unsigned int numBarriere = typeEntree -1;
	 
	 decrSemEntree[1].sem_num = numBarriere;
	 decrSemEntree[1].sem_op = -1;
	 decrSemEntree[1].sem_flg = 0;
	 	 
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
	 ParkingMPPtr =(ParkingMP*) shmat(shmIdParking, NULL, 0);
	 RequeteMPPtr =(RequeteMP*) shmat(shmIdRequete, NULL, 0);

    // -- Armement des signaux SIGUSR2 et SIGCHILD
    sigaction(SIGUSR2, &actionHandlerUSR2, NULL);
    sigaction(SIGCHLD, &actionHandlerCHLD, NULL);



    // --  MOTEUR --
    while(true)
    {
		
        Voiture voitRecue;
        pid_t pidVoiturier;
        
        // On attend qu'une voiture arrive
        while(read(canalEntree[numBarriere][0], &voitRecue , sizeof(Voiture) ) == -1 && errno == EINTR ) ;
		time_t heureArrivee = time(NULL);
		voitRecue.dateArrive = heureArrivee;
		
		// Quand une voiture arrive a une barriere
        // On dessine la voiture a la barriere
        DessinerVoitureBarriere(typeEntree, voitRecue.usager);

        
        // On recupere le semaphore de Requete
        while(semop(semId, decrSemRequete, 1) == -1 && errno == EINTR);
        
        // Si y a de une place de libre on gare la voiture, sinon on emet une requete et on attend qu'une place ce libere
        if(RequeteMPPtr->nbPlacesOccupees < NB_PLACES )
        {
			RequeteMPPtr->nbPlacesOccupees++;
			while(semop(semId, incrSemRequete, 1) == -1 && errno == EINTR);
			pidVoiturier = GarerVoiture(typeEntree);
        }
        else
        {
			RequeteMPPtr->requetes[numBarriere].voiture = voitRecue;
			RequeteMPPtr->requetes[numBarriere].barriere = typeEntree;
			AfficherRequete(typeEntree, voitRecue.usager, heureArrivee);
			while(semop(semId, decrSemEntree, 1) == -1 && errno == EINTR);
			pidVoiturier = GarerVoiture(typeEntree);
		}	
        

        // on garde le pid du Voiturier
        struct pidVoiture mapPidVoiture = {pidVoiturier, voitRecue};
		listeFils.push_back(mapPidVoiture);


    }
}
