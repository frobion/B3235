/*************************************************************************
                           Entree  -  description
                             -------------------
    début                : 23/03/2016
    copyright            : (C) 2016 par narsac
    e-mail               : nathan.arsac@insa-lyon.fr
*************************************************************************/

//---------- Réalisation de la tâche <Entree> (fichier Entree.cpp)

/////////////////////////////////////////////////////////////////  INCLUDE
//-------------------------------------------------------- Include système
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

#include <fstream>
#include <iostream>

//------------------------------------------------------ Include personnel
#include "Entree.h"
#include "Outils.h"
#include "config.h"

///////////////////////////////////////////////////////////////////  PRIVE
//------------------------------------------------------------- Constantes

//------------------------------------------------------------------ Types
struct pidVoiture
{
	pid_t pid;
	Voiture voiture;
};
//---------------------------------------------------- Variables statiques

static ParkingMP* ParkingMPPtr;
static RequeteMP* RequeteMPPtr;

static vector<pidVoiture> listeFils;

static int semId;
static unsigned int numBarriere;

static struct sembuf incrSemParking [1] = {{NUM_SEM_PARKING, 1, 0}};
static struct sembuf decrSemParking [1] = {{NUM_SEM_PARKING, -1, 0}};
static struct sembuf incrSemRequete [1] = {{NUM_SEM_REQUETE, 1, 0}};
static struct sembuf decrSemRequete [1] = {{NUM_SEM_REQUETE, -1, 0}};
static struct sembuf decrSemEntree [1];

static fstream fichier[3];
fichier[0]("LogEntree1.txt");
fichier[1]("LogEntree2.txt");
fichier[2]("LogEntree3.txt");

static fstream fichier;

//------------------------------------------------------ Fonctions privées

// ------ Handler SIGUSR2 --------
static void HandlerUSR2 ( int noSig )
{

	fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : Appel SIGUSR2" << std::endl;
  //  DESTRUCTION
  // Destruction de tous les deplacements
  for(unsigned int i = 0; i < listeFils.size(); i++)
  {
		if(listeFils[i].pid != (pid_t) -1)
		{
			kill(listeFils[i].pid,SIGUSR2);
			waitpid(listeFils[i].pid,0,0);
		}
  }

  // Detachement de la memoire
  shmdt(ParkingMPPtr);
  shmdt(RequeteMPPtr);

  fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : Fin SIGUSR2" << std::endl;

  exit(0);
}

static void HandlerCHLD(int noSig)
{

	fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : Appel SIGCHLD" << std::endl;

	int numPlace;
	pid_t pidDestruct = waitpid(-1 ,&numPlace,0);
	numPlace = WEXITSTATUS(numPlace);

	fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : num place : " << numPlace << std::endl;

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

	//fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : Info voiture : " << voitureGarer.usager << " " << voitureGarer.immatriculation << " " << voitureGarer.dateArrive << std::endl;

	fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : Demande semaphore parking" << std::endl;

	while(semop(semId, decrSemParking, 1) == -1 && errno == EINTR);

	fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : Recup semaphore parking" << std::endl;

	ParkingMPPtr->parking[numPlace] = voitureGarer;
	fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " :  info voiture garee SIGCHLD : " << voitureGarer.usager << " "
	<< voitureGarer.immatriculation << " " << voitureGarer.dateArrive << std::endl;
	AfficherPlace(numPlace, voitureGarer.usager, voitureGarer.immatriculation, voitureGarer.dateArrive);

	semop(semId, incrSemParking, 1);

	fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : Liberation semaphore parking" << std::endl;
	fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : Fin SIGCHLD" << std::endl;

}


//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
void GestionEntree(int canalEntree[][2], int canalSortie[2], TypeBarriere typeEntree, int shmIdParking, int shmIdRequete, int semIdParam)
{

    // ---  INITIALISATION  ---
	semId = semIdParam;
	numBarriere = typeEntree -1;

	fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : Init Entree" << std::endl;

	decrSemEntree[0].sem_num = numBarriere;
	decrSemEntree[0].sem_op = -1;
	decrSemEntree[0].sem_flg = 0;

	// Canaux : On ferme tout sauf en lecture sur la barriere concernee
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
   close(canalSortie[0]);
   close(canalSortie[1]);


    // -- Creation des handlers de SIGUSR2
		// HandlerSIGUSR2 ne doit pas etre interrompu par SIGCHLD
    struct sigaction actionHandlerUSR2;
    actionHandlerUSR2.sa_handler = HandlerUSR2;
    sigemptyset(&actionHandlerUSR2.sa_mask);
    actionHandlerUSR2.sa_flags = 0;
    sigaddset(&actionHandlerUSR2.sa_mask, SIGCHLD);

    // -- Creation des handlers de SIGCHLD
    struct sigaction actionHandlerCHLD;
    actionHandlerCHLD.sa_handler = HandlerCHLD;
    sigemptyset(&actionHandlerCHLD.sa_mask);
    actionHandlerCHLD.sa_flags = 0;

    // -- Attachement des segments de memoires partagées
	 ParkingMPPtr =(ParkingMP*) shmat(shmIdParking, NULL, 0);
	 RequeteMPPtr =(RequeteMP*) shmat(shmIdRequete, NULL, 0);

    // -- Armement des signaux SIGUSR2 et SIGCHILD
    sigaction(SIGUSR2, &actionHandlerUSR2, NULL);
    sigaction(SIGCHLD, &actionHandlerCHLD, NULL);

    //  -- Deblocage de SIGCHLD et SIGUSR2
    sigset_t listeSignalDebloque;
	sigemptyset(&listeSignalDebloque);
	sigaddset(&listeSignalDebloque, SIGUSR2);
	sigaddset(&listeSignalDebloque, SIGCHLD);
	sigprocmask(SIG_UNBLOCK, &listeSignalDebloque, NULL);



    // --  MOTEUR --
    while(true)
    {

		fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : debut boucle infini" << std::endl;

        Voiture voitRecue;
        pid_t pidVoiturier;

        // On attend qu'une voiture arrive et on lui affecte son heure d'arrivee
        while(read(canalEntree[numBarriere][0], &voitRecue , sizeof(Voiture) ) == -1 && errno == EINTR ) ;
		time_t heureArrivee = (time(NULL)) % DECIMAL_TEMPS;
		voitRecue.dateArrive = heureArrivee;

		fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : Une voiture est arrivee et heure attribue : " << heureArrivee << std::endl;

		// -- Quand une voiture arrive a une barriere

        // On dessine la voiture a la barriere
        DessinerVoitureBarriere(typeEntree, voitRecue.usager);

        // On recupere le semaphore de Requete
        fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : Demande du semaphore requete" << std::endl;
        //semop(semId, incrSemRequete, 1);
        while(semop(semId, decrSemRequete, 1) == -1 && errno == EINTR);
        fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : Recup semaphore requete" << std::endl;
        // Si y a de une place de libre on gare la voiture, sinon on emet une requete et on attend qu'une place ce libere
        fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : nbPlaceOccupe avant if : " << RequeteMPPtr->nbPlacesOccupees << std::endl;
        if(RequeteMPPtr->nbPlacesOccupees < NB_PLACES )
        {
			    RequeteMPPtr->nbPlacesOccupees++;
			    fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : nbPlaceOccupe apres incr : " << RequeteMPPtr->nbPlacesOccupees << std::endl;
			    semop(semId, incrSemRequete, 1);
			    fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : Liberation semaphore requete" << std::endl;
			    pidVoiturier = GarerVoiture(typeEntree);
			    //fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : pidVoiturier (then) " << pidVoiturier << std::endl;
        }
        else
        {
			    RequeteMPPtr->requetes[numBarriere].voiture.dateArrive =  voitRecue.dateArrive;
			    RequeteMPPtr->requetes[numBarriere].voiture.immatriculation =  voitRecue.immatriculation;
			    RequeteMPPtr->requetes[numBarriere].voiture.usager =  voitRecue.usager;

			    fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " :  Ecriture de la requete : " << RequeteMPPtr->requetes[numBarriere].voiture.usager << " "
			    << RequeteMPPtr->requetes[numBarriere].voiture.immatriculation << " " << RequeteMPPtr->requetes[numBarriere].voiture.dateArrive << std::endl;

			    RequeteMPPtr->requetes[numBarriere].barriere = typeEntree;
			    AfficherRequete(typeEntree, voitRecue.usager, heureArrivee);
		  	  //fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : Requete barriere " << RequeteMPPtr->requetes[numBarriere].barriere << std::endl;
			    semop(semId, incrSemRequete, 1);

			    fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : Liberation semaphore requete" << std::endl;

			    fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : Demande semaphore entree" << std::endl;
			    //fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : Valeur du semaphore avant : " << semctl(semId, numBarriere, GETVAL, NULL) << std::endl;
			    while(semop(semId, decrSemEntree, 1) == -1 && errno == EINTR);
			    //fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : Valeur du semaphore apres : " << semctl(semId, numBarriere, GETVAL, NULL) << std::endl;
			    fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : Recup semaphore entree" << std::endl;

			    pidVoiturier = GarerVoiture(typeEntree);
			   //fichier[numBarriere] << time(NULL)%TEMPS_MAX << "  " << numBarriere << " : pidVoiturier (else) " << pidVoiturier << std::endl;

    		}


        // on garde le pid du Voiturier
        struct pidVoiture mapPidVoiture = {pidVoiturier, voitRecue};
		    listeFils.push_back(mapPidVoiture);

    }
}
