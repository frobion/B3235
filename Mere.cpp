#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "Outils.h"
#include "Heure.h"

#include "Simulation.h"

int main(void) {

	// VT220 Si SSH
	// XTERM par défaut
	InitialiserApplication (XTERM);
	
	//Liste des ID Processus
	pid_t pidHeure;         //Heure
  	pid_t pidSimu;          //Simulation
	pid_t pidEntrees [3];   //Entrees
	pid_t pidSortie; 	//Sortie



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
//		GestionSortie(shmDescripteur);
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
	
	waitpid(pidSimu, NULL, 0);
	kill(pidHeure, SIGUSR2);
	// waitpid heure??
	TerminerApplication(true);

	return 0;

	}

}
