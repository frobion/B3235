/*************************************************************************
                           Mere  -  description
                             -------------------
    début                : 23/03/2016
    copyright            : (C) 2016 par narsac
    e-mail               : nathan.arsac@insa-lyon.fr
*************************************************************************/

//---------- Réalisation de la tâche <Mere> (fichier Mere.cpp)

/////////////////////////////////////////////////////////////////  INCLUDE
//-------------------------------------------------------- Include système
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

//------------------------------------------------------ Include personnel
#include "Outils.h"
#include "Heure.h"
#include "config.h"
#include "Simulation.h"
#include "Entree.h"
#include "BarriereSortie.h"

///////////////////////////////////////////////////////////////////  PRIVE
//------------------------------------------------------------- Constantes

//------------------------------------------------------------------ Types

//---------------------------------------------------- Variables statiques

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
int main(void) {

	InitialiserApplication (XTERM);

	// Liste des ID Processus
	pid_t pidHeure;         //Heure
    pid_t pidSimu;          //Simulation
	pid_t pidEntrees [3];   //Entrees
	pid_t pidSortie; 		//Sortie


	// Heure : Creation
	pidHeure = ActiverHeure();
	if (pidHeure == -1)
	{
		TerminerApplication(false);
		exit(0);
	}

	// Blocage des signaux
	sigset_t listeSignalBloque;
    sigemptyset(&listeSignalBloque);
    sigaddset(&listeSignalBloque, SIGUSR1);
    sigaddset(&listeSignalBloque, SIGUSR2);
    sigaddset(&listeSignalBloque, SIGCHLD);
    sigprocmask(SIG_SETMASK, &listeSignalBloque, NULL);


	// ***** CREATION DES OBJETS PASSIFS  *****

		const char * pathname = "./Parking";

	  // ----- Memoire partagee Creation

		// -- Memoire Parking
		key_t cle2 = ftok(pathname, 'P');
		int shmIdParking = shmget(cle2, sizeof(ParkingMP), IPC_CREAT | 0660);

		  // Attachement memoire partagee parking
		ParkingMP* ParkingMPPtr = (ParkingMP*) shmat(shmIdParking, NULL, 0);

		  // Initialisation memoire partagee parking
		for(unsigned int numPlace = 0 ; numPlace < 8 ; numPlace++ )
		{
			ParkingMPPtr->parking[numPlace].usager = AUCUN ;
			ParkingMPPtr->parking[numPlace].immatriculation = -1 ;
			ParkingMPPtr->parking[numPlace].dateArrive =  -1 ;
		}

		// -- Memoire Requete + Compteur
		key_t cle3 = ftok(pathname, 'R');
		int shmIdRequete = shmget(cle3, sizeof(RequeteMP), IPC_CREAT | 0660);

		  // Attachement memoire partagee requete
		RequeteMP* RequeteMPPtr = (RequeteMP*) shmat(shmIdRequete, NULL, 0);

		  // Initialisation memoire partagee requete
		for(unsigned int numRequete = 0 ; numRequete < 3 ; numRequete++ )
		{
			RequeteMPPtr->requetes[numRequete].voiture.usager = AUCUN ;
			RequeteMPPtr->requetes[numRequete].voiture.immatriculation = -1 ;
			RequeteMPPtr->requetes[numRequete].voiture.dateArrive =  -1 ;
		}
		RequeteMPPtr->nbPlacesOccupees = 0 ;


		// -- Semaphore
		cle2 = ftok(pathname, 'Q');
		int semId = semget(cle2, 5,IPC_CREAT | 0660);

		  // Initialisation des semaphores
		semctl(semId, NUM_SEM_PARKING, SETVAL, 1);
		semctl(semId, NUM_SEM_REQUETE, SETVAL, 1);
		semctl(semId, NUM_SEM_PROF_BLAISE_PASCAL, SETVAL, 0);
		semctl(semId, NUM_SEM_AUTRE_BLAISE_PASCAL, SETVAL, 0);
		semctl(semId, NUM_SEM_GASTON_BERGER, SETVAL, 0);


	 // ----- Creation des canaux

		int canalEntree[3][2];
		int canalSortie[2];

		pipe(canalEntree[0]);
		pipe(canalEntree[1]);
		pipe(canalEntree[2]);
		pipe(canalSortie);

// Simulation
	if ((pidSimu = fork()) == 0)
  	{
	Simulation(canalEntree, canalSortie);
	}
// Sortie
	else if ((pidSortie = fork()) == 0)
  	{
		BarriereSortie(canalEntree, canalSortie, shmIdParking, shmIdRequete, semId);
	}
// Entree Blaise Prof
	else if ((pidEntrees[0] = fork()) == 0)
  	{
		GestionEntree(canalEntree, canalSortie, PROF_BLAISE_PASCAL, shmIdParking, shmIdRequete, semId);
	}
// Entree Blaise Autre
  	else if ((pidEntrees[1] = fork()) == 0)
    	{
		GestionEntree(canalEntree, canalSortie, AUTRE_BLAISE_PASCAL, shmIdParking, shmIdRequete, semId);
	}
// Entree Gaston Berger
  	else if ((pidEntrees[2] = fork()) == 0)
   	{
		GestionEntree(canalEntree, canalSortie, ENTREE_GASTON_BERGER, shmIdParking, shmIdRequete, semId);
	}
// Mere
	else
	{

	waitpid(pidSimu, 0, 0);

	// Destruction des processus fils
	Afficher(TypeZone::MESSAGE, pidSortie);
	kill(pidSortie, SIGUSR2);
	waitpid(pidSortie, 0, 0);
	Afficher(MESSAGE,"FIN sortie");

	kill(pidEntrees[0], SIGUSR2);
	waitpid(pidEntrees[0], 0, 0);
	Afficher(MESSAGE,"FIN Entree Blaise Prof");

	kill(pidEntrees[1], SIGUSR2);
	waitpid(pidEntrees[1], 0, 0);
	Afficher(MESSAGE,"FIN Entree Blaise Autres");

	kill(pidEntrees[2], SIGUSR2);
	waitpid(pidEntrees[2], 0, 0);
	Afficher(MESSAGE,"FIN Entree Gaston Berger");

	Effacer(MESSAGE);


	// Heure : Destruction
	kill(pidHeure, SIGUSR2);
	Afficher(MESSAGE,"Suppression heure");
	waitpid(pidHeure, 0, 0);


	// Memoire partagee  : Destruction

    Afficher(MESSAGE,"Suppression semaphore");
	semctl(semId, 0, IPC_RMID, 0);

	Afficher(MESSAGE,"Suppression memoire Requete");
	shmctl(shmIdRequete, IPC_RMID, 0);

	Afficher(MESSAGE,"Suppression memoire Parking");
	shmctl(shmIdParking, IPC_RMID, 0);


	
	TerminerApplication(true);

	exit(0);

	}

}
