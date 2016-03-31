/*************************************************************************
                           BarriereSortie  -  description
                             -------------------
    début                : 23/03/2016
    copyright            : (C) 2016 par frobion
    e-mail               : francois.robion@insa-lyon.fr
*************************************************************************/

//---------- Réalisation de la tâche <BarriereSortie> (fichier BarriereSortie.cpp)

/////////////////////////////////////////////////////////////////  INCLUDE
//-------------------------------------------------------- Include système
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <vector>

//------------------------------------------------------ Include personnel
#include "BarriereSortie.h"
#include "config.h"
#include "Outils.h"

///////////////////////////////////////////////////////////////////  PRIVE
//------------------------------------------------------------- Constantes

//------------------------------------------------------------------ Types

//---------------------------------------------------- Variables statiques
static int semId;
static ParkingMP* parkingMPPtr;
static RequetesMP* requetesMPPtr;
static std::vector<pid_t> listeVoiturierSortie; 


//------------------------------------------------------ Fonctions privées

// Acces au semaphore deja effectue avant l'appel de cette methode
static int chercheRequetesActuelles(Voiture* requetesActuelles)
{
  if (requetesMPPtr->nbPlacesOccupees != 8)
  {
    return 0;
  }
  int nbRequetesActuelles = 0;
  for (unsigned int i = 0; i < NB_BARRIERES_ENTREE; i++)
  {
    if (requetesMPPtr->requetes[i]->usager != TypeUsager::AUCUN)
    {
	  requetesActuelles[nbRequetesActuelles++] = requetesMPPtr->requetes[i];
    }
  }
  return nbRequetesActuelles;
}

static void handlerSigChld (int noSignal)
{
  int numeroPlaceLibere;
  pid_t pidFilsMort = waitpid(-1, numeroPlaceLibere, 0); // Destruction du fils mort
  
  // Acces au semaphore de protection de parking
  semop (semId, DECR_DANS_PARKING, 1);
  
  // Remise de la place dans l'etat sans voiture
  parkingMPPtr->parking[numeroPlaceLibere]->usager = TypeUsager::AUCUN;
  parkingMPPtr->parking[numeroPlaceLibere]->immatriculation = -1;
  parkingMPPtr->parking[numeroPlaceLibere]->dateArrive = -1;
  
  // Liberation du semaphore de protection de parking
  semop(semId, INCR_DANS_PARKING, 1); 
  
  Voiture requetesActuelles [3];
  // Acces au semaphore de protection des requetes
  semop(semId, DECR_DANS_REQUETE, 1);
  
  int nbRequetesActuelles = chercheRequetesActuelles(requetesActuelles);
  if (nbRequetesActuelles == 0)
  {
    requetesMPPtr->nbPlacesOccupees--;
  }
  else // Analyse des requetes, pour savoir qui doit rentrer
  {
    bool auMoinsUneRequetesProf = false;
    for (int i = 0; i < nbRequetesActuelles; i++)
    {
	  if (
	}
  }
}

static void handlerSigUsr2 (int noSignal)
{
  
}

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
void BarriereSortie(int canauxBarriereEntree[][2], int canalBarriereSortie[], int shmParkingId, int shmRequeteId, int semIdParam)
{
  // INIT
  
  // Fermeture canaux inutilise
  for (unsigned int i = 0; i < NB_BARRIERES_ENTREE; i++)
  {
    close(canauxBarriereEntree[i][0]);
    close(canauxBarriereEntree[i][1]);
  }
  close(canalBarriereSortie[1]);
  int descLectureCanal = canalBarriereSortie[0];
  
  // Attachement de la memoire partagee
  parkingMPPtr = (ParkingMP*) shmat(shmParkingId, NULL, 0);
  requetesMPPtr = (RequetesMP*) shmat(shmRequeteId, NULL, 0);
  
  // Recuperer id du semaphore
  semId = semIdParam;
  
  // Creer et associer handler SIGCHLD
  struct sigaction actionCHLD;
  actionCHLD.sa_handler = handlerSigChld;
  sigemptyset(&actionCHLD.sa_mask);
  actionCHLD.sa_flags = 0;
  sigaction (SIGCHLD, &actionCHLD, NULL);
  
  // Creer et associer handler SIGUSR2 (destruction)
  struct sigaction actionUSR2;
  actionUSR2.sa_handler = handlerSigUsr2;
  sigemptyset(&actionUSR2.sa_mask);
  actionUSR2.sa_flags = 0;
  sigaction (SIGUSR2, &actionUSR2, NULL);
	
  
  // MOTEUR
  unsigned int numeroPlace;
  
  for (;;)
  {
	read (descLectureCanal, &numeroPlace, sizeof(unsigned int));
	listeVoiturierSortie.push_back(SortirVoiture(numeroPlace)); // Erreur (retour == -1) non gere	
  }
	
  // DESTRUCTION
  // Detachement de la MP
  shmdt(parkingMPPtr);
  shmdt(requetesMPPtr);
}
