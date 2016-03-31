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

#include "Simulation.h"
#include "Entree.h"


int main(void) {

	// VT220 Si SSH
	// XTERM par défaut
	InitialiserApplication (XTERM);

	//Liste des ID Processus
	pid_t pidHeure;         //Heure
    pid_t pidSimu;          //Simulation
	pid_t pidEntrees [3];   //Entrees
	pid_t pidSortie; 		//Sortie


	// CREATION DES OBJETS PASSIFS
		const char * pathname = "./Mere";

	//Memoire partagee Creation

		// -------- Descripteur
		//shmP shmDescripteur;

					// Création du segment de mémoire
						// Genere cle en fct du chemin vers le fichier
						// Le 2nd param : caractere pour differencier les differentes cles
		key_t cle2 = ftok(pathname, 'P');
		//shmDescripteur.idShm = shmget(cle2, sizeof(int) * 2 + sizeof(TypeUsager) + sizeof(time_t), IPC_CREAT | 0660);

					// Création du semaphore
		cle2 = ftok(pathname, 'Q');
		//shmDescripteur.idMutex = semget(cle2, 1,IPC_CREAT | 0660);
					// Initialisation du sémaphore
		//semctl(shmDescripteur.idMutex, 0, SETVAL, 1);

/*
		// -------- Requete
		shmP shmRequete;
				// Segment de memoire
		key_t cle3 = ftok(pathname, 'R');
		shmRequete.idShm = shmget(cle3, sizeof(TypeUsager) + sizeof(time_t), IPC_CREAT | 0660);
				// Semaphore
		cle3 = ftok(pathname, 'S');
		shmRequete.idMutex = semget(cle3, 1,IPC_CREAT | 0660);
		semctl(shmRequete.idMutex, 0, SETVAL, 1);

		// -------- Compteur
		shmP shmCompteur;
				// Segment de memoire
		key_t cle4 = ftok(pathname, 'T');
		shmCompteur.idShm = shmget(cle4, sizeof(int), IPC_CREAT | 0660);
				// Semaphore
		cle4 = ftok(pathname, 'U');
		shmCompteur.idMutex = semget(cle4, 1,IPC_CREAT | 0660);
		semctl(shmCompteur.idMutex, 0, SETVAL, 1);
*/

	//Heure : Creation
	pidHeure = ActiverHeure();
	if (pidHeure == -1) // Gestion des erreurs sur creation d'Heure
  {
		TerminerApplication(false);
		exit(0);
	}


//Simulation
	if ((pidSimu = fork()) == 0)
  	{
		// Fonction faite par Francois
    int a [3][2];
    int b [2];
	Simulation(a, b);
    //delete [] a;
    //delete [] b;
	}
//Sortie
//	else if ((pidSortie = fork()) == 0)
//  	{
//		// Fonction faite par Francois
//		GestionSortie();
//	}
//Entree Blaise Prof
//	else if ((pidEntrees[0] = fork()) == 0)
//   	{
//		GestionEntree(shmDescripteur,filePlace, PROF_BLAISE_PASCAL);
//	}
//Entree Blaise Autre
//  	else if ((pidEntrees[1] = fork()) == 0)
//    	{
//		GestionEntree(shmDescripteur,filePlace, AUTRE_BLAISE_PASCAL);
//	}
//Entree Gaston Berger
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

				// ----- Descripteur
	Afficher(MESSAGE,"Suppression memoire descripteur");
	//shmctl(shmDescripteur.idShm, IPC_RMID, 0);

	Afficher(MESSAGE,"Suppression semaphore descripteur");
	//semctl(shmDescripteur.idMutex, 0, IPC_RMID, 0);

	/*
				// ----- Requete
	Afficher(MESSAGE,"Suppression memoire requete");
	shmctl(shmRequete.idShm, IPC_RMID, 0);

	Afficher(MESSAGE,"Suppression semaphore requete");
	semctl(shmRequete.idMutex, 0, IPC_RMID, 0);

				// ----- Compteur
	Afficher(MESSAGE,"Suppression memoire compteur");
	shmctl(shmCompteur.idShm, IPC_RMID, 0);

	Afficher(MESSAGE,"Suppression semaphore compteur");
	semctl(shmCompteur.idMutex, 0, IPC_RMID, 0);

*/


	// true pour les tests, false pour la version finale
	TerminerApplication(true);

	exit(0);

	}

}
