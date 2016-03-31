#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <signal.h>

#include "Outils.h"
#include "Heure.h"
#include "config.h"

#include "Simulation.h"
#include "Entree.h"
#include "BarriereSortie.h"


int main(void) {

	// VT220 Si SSH
	// XTERM par défaut
	InitialiserApplication (XTERM);
	
	// Blocage des signaux 
	sigset_t listeSignalBloque;
    sigemptyset(&listeSignalBloque);
    sigaddset(&listeSignalBloque, SIGUSR1);
    sigaddset(&listeSignalBloque, SIGUSR2);
    sigaddset(&listeSignalBloque, SIGCHLD);
    sigprocmask(SIG_SETMASK, &listeSignalBloque, NULL);

	//Liste des ID Processus
	pid_t pidHeure;         //Heure
    pid_t pidSimu;          //Simulation
	pid_t pidEntrees [3];   //Entrees
	pid_t pidSortie; 		//Sortie


	//Heure : Creation
	pidHeure = ActiverHeure();
	if (pidHeure == -1)
	{
		TerminerApplication(false);
		exit(0);
	}
	

	// ***** CREATION DES OBJETS PASSIFS  *****
	
		const char * pathname = "./Mere";

	  // ----- Memoire partagee Creation
		
		// -- Memoire Parking
		key_t cle2 = ftok(pathname, 'P');
		int shmIdParking = shmget(cle2, sizeof(ParkingMP), IPC_CREAT | 0660);
		
		  // Attachement
		ParkingMP* ParkingMPPtr = (ParkingMP*) shmat(shmIdParking, NULL, 0);
		
		  // Initialisation 
		for(unsigned int numPlace = 0 ; numPlace < 8 ; numPlace++ )
		{
			ParkingMPPtr -> parking[numPlace] -> usager = AUCUN ;
			ParkingMPPtr -> parking[numPlace] -> immatriculation = -1 ;
			ParkingMPPtr -> parking[numPlace] -> dateArrive =  -1 ;
		}

		// -- Memoire Requete + Compteur
		key_t cle3 = ftok(pathname, 'R');
		int shmIdRequete = shmget(cle3, sizeof(RequetesMP), IPC_CREAT | 0660);
		
		  // Attachement
		RequeteMP* RequeteMPPtr = (RequeteMP*) shmat(shmIdRequete, NULL, 0);
		
		  // Initialisation 
		for(unsigned int numRequete = 0 ; numRequete < 3 ; numRequete++ )
		{
			RequeteMPPtr -> requetes[numRequete] -> usager = AUCUN ;
			RequeteMPPtr -> requetes[numRequete] -> immatriculation = -1 ;
			RequeteMPPtr -> requetes[numRequete] -> dateArrive =  -1 ;
		}
		RequeteMPPtdr -> nbPlacesOccupees = 0 ;
		
		
		// -- Semaphore
		cle2 = ftok(pathname, 'Q');
		int semId = semget(cle2, 1,IPC_CREAT | 0660);
		semctl(semId, 0, SETVAL, 1);
	
	
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
//	else if ((pidEntrees[0] = fork()) == 0)
//   	{
//		GestionEntree(shmDescripteur,filePlace, PROF_BLAISE_PASCAL);
//	}
// Entree Blaise Autre
//  	else if ((pidEntrees[1] = fork()) == 0)
//    	{
//		GestionEntree(shmDescripteur,filePlace, AUTRE_BLAISE_PASCAL);
//	}
// Entree Gaston Berger
//  	else if ((pidEntrees[2] = fork()) == 0)
//   	{
//		GestionEntree(shmDescripteur,filePlace, ENTREE_GASTON_BERGER);
//	}

	else //Code Mere
	{

	waitpid(pidSimu, 0, 0);

	// Destruction des processus fils
	kill(pidSortie, SIGUSR2);
	Afficher(MESSAGE,"FIN sortie");
	waitpid(pidSortie, 0, 0);

	kill(pidEntrees[0], SIGUSR2);
	Afficher(MESSAGE,"FIN Entree Blaise Prof");
	waitpid(pidEntrees[0], 0, 0);

	kill(pidEntrees[1], SIGUSR2);
	Afficher(MESSAGE,"FIN Entree Blaise Autres");
	waitpid(pidEntrees[1], 0, 0);

	kill(pidEntrees[2], SIGUSR2);
	Afficher(MESSAGE,"FIN Entree Gaston Berger");
	waitpid(pidEntrees[2], 0, 0);

	Effacer(MESSAGE);


	//Heure : Destruction
	kill(pidHeure, SIGUSR2);
	Afficher(MESSAGE,"Suppression heure");
	waitpid(pidHeure, 0, 0);


	//Memoire partagee  : Destruction

		// Destruction Semaphore
    Afficher(MESSAGE,"Suppression semaphore");
	semctl(semId, 0, IPC_RMID, 0);
	
	Afficher(MESSAGE,"Suppression memoire Requete");
	shmctl(shmIdRequete, IPC_RMID, 0);

	Afficher(MESSAGE,"Suppression memoire Parking");
	shmctl(shmIdParking, IPC_RMID, 0);


	// true pour les tests, false pour la version finale
	TerminerApplication(true);

	exit(0);

	}

}
