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
#include <sys/wait.h>
#include <unistd.h>
#include <set>
#include <limits.h>
#include <errno.h>

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
static RequeteMP* requeteMPPtr;
static std::set<pid_t> listeVoiturierSortie;
static int descLectureCanal;

static struct sembuf incrSemParking [1] = {{NUM_SEM_PARKING, 1, 0}};
static struct sembuf decrSemParking [1] = {{NUM_SEM_PARKING, -1, 0}};
static struct sembuf incrSemRequete [1] = {{NUM_SEM_REQUETE, 1, 0}};
static struct sembuf decrSemRequete [1] = {{NUM_SEM_REQUETE, -1, 0}};

static struct sembuf incrSemProfBlaisePascal [1] = {{NUM_SEM_PROF_BLAISE_PASCAL, 1, 0}};
static struct sembuf incrSemAutreBlaisePascal [1] = {{NUM_SEM_AUTRE_BLAISE_PASCAL, 1, 0}};
static struct sembuf incrSemGastonBerger [1] = {{NUM_SEM_GASTON_BERGER, 1, 0}};


//------------------------------------------------------ Fonctions privées


// Acces au semaphore deja effectue avant l'appel de cette methode
static int chercheRequetesActuelles(Requete* requetesActuelles)
{
  if (requeteMPPtr->nbPlacesOccupees != NB_PLACES)
  {
    return 0;
  }
  int nbRequetesActuelles = 0;
  for (unsigned int i = 0; i < NB_BARRIERES_ENTREE; i++)
  {
    if (requeteMPPtr->requetes[i].voiture.usager != TypeUsager::AUCUN)
    {
	  requetesActuelles[nbRequetesActuelles++] = requeteMPPtr->requetes[i];
    }
  }
  return nbRequetesActuelles;
}

// Renvoie r1 si r1 autant ou plus prioritaire que r2, r2 sinon
static Requete * comparePrioriteRequetes(Requete* r1, Requete* r2)
{
  if (r1->voiture.usager != r2->voiture.usager)
  {
	return (r1->voiture.usager == TypeUsager::PROF) ? r1 : r2;
  }
  return (r1->voiture.dateArrive <= r2->voiture.dateArrive) ? r1 : r2;
}

static void initVoiture(Voiture* voiture)
{
  voiture->usager = TypeUsager::AUCUN;
  voiture->immatriculation = -1;
  voiture->dateArrive = -1;
}

static void handlerSigChld (int noSignal)
{
  time_t tempsSortie = (time(NULL)) % TEMPS_MAX	;
  int numeroPlaceLibere;
  pid_t pidFilsMort = waitpid(-1, &numeroPlaceLibere, 0); // Destruction du fils mort
  numeroPlaceLibere = WEXITSTATUS(numeroPlaceLibere);
  listeVoiturierSortie.erase(pidFilsMort);
  
  // Acces au semaphore de protection de parking
  while(semop (semId, decrSemParking, 1) == -1 && errno == EINTR);
  
  // Remise de la place dans l'etat sans voiture
  AfficherSortie(parkingMPPtr->parking[numeroPlaceLibere - 1].usager, parkingMPPtr->parking[numeroPlaceLibere - 1].immatriculation, 
      parkingMPPtr->parking[numeroPlaceLibere - 1].dateArrive, tempsSortie);
  initVoiture(&(parkingMPPtr->parking[numeroPlaceLibere - 1]));
  Effacer((TypeZone) numeroPlaceLibere); // Correspond a la bonne valeur de la zone de l'enum
  
  // Liberation du semaphore de protection de parking
  semop(semId, incrSemParking, 1);
  
  
  
  Requete requetesActuelles [NB_BARRIERES_ENTREE];
  int nbRequetesActuelles;
  // Acces au semaphore de protection des requetes
  while(semop(semId, decrSemRequete, 1) == -1 && errno == EINTR);
  
  nbRequetesActuelles = chercheRequetesActuelles(requetesActuelles);
  if (nbRequetesActuelles == 0)
  {
    requeteMPPtr->nbPlacesOccupees--;
  }
  else // Analyse des requetes, pour savoir qui doit rentrer
  {
	Requete meilleurRequete = (nbRequetesActuelles == 1) ? requetesActuelles[0] : *comparePrioriteRequetes(&requetesActuelles[0], &requetesActuelles[1]);
	for (int i = 2; i < nbRequetesActuelles; i++)
	{
	  meilleurRequete = *comparePrioriteRequetes(&meilleurRequete, &requetesActuelles[i]);
	}
	switch(meilleurRequete.barriere)
	{
	  case PROF_BLAISE_PASCAL:
	    // Reveil de la tache entree correspondante
	    semop (semId, incrSemProfBlaisePascal, 1);
	    // Reinitilisation de la MP pour la requete correspondante
	    initVoiture(&(requeteMPPtr->requetes[TypeBarriere::PROF_BLAISE_PASCAL - 1].voiture));
	    requeteMPPtr->requetes[TypeBarriere::PROF_BLAISE_PASCAL - 1].barriere = TypeBarriere::AUCUNE;
	    // Gestion de l'affichage
	    Effacer(TypeZone::REQUETE_R1);
	    break;
	  case AUTRE_BLAISE_PASCAL:
	    semop(semId, incrSemAutreBlaisePascal, 1);
	    initVoiture(&(requeteMPPtr->requetes[TypeBarriere::AUTRE_BLAISE_PASCAL - 1].voiture));
	    requeteMPPtr->requetes[TypeBarriere::AUTRE_BLAISE_PASCAL - 1].barriere = TypeBarriere::AUCUNE;
	    Effacer(TypeZone::REQUETE_R2);
	    break;
	  case ENTREE_GASTON_BERGER:
	    semop(semId, incrSemGastonBerger, 1);
	    initVoiture(&(requeteMPPtr->requetes[TypeBarriere::ENTREE_GASTON_BERGER - 1].voiture));
	    requeteMPPtr->requetes[TypeBarriere::ENTREE_GASTON_BERGER - 1].barriere	 = TypeBarriere::AUCUNE;
	    Effacer(TypeZone::REQUETE_R3);
	    break;
	  default:
	    Afficher(TypeZone::MESSAGE, "Mauvaise barriere, handler SIGCHLD, barriereSortie");
	    break;
	}
  }
  semop(semId, incrSemRequete, 1); // Liberation du semaphore de protection des requetes
}

static void handlerSigUsr2 (int noSignal)
{
  Afficher(TypeZone::MESSAGE, "SIGUSR2 recu par sortie");
  close(descLectureCanal);
  
  shmdt(parkingMPPtr);
  shmdt(requeteMPPtr);
  
  std::set<pid_t>::iterator it;
  for (it = listeVoiturierSortie.begin(); it != listeVoiturierSortie.end(); it++)
  {
	kill(*it, SIGUSR2);
	while(waitpid(*it, NULL, 0) == -1 && errno == EINTR);
  }
  
  exit(0);
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
  descLectureCanal = canalBarriereSortie[0];
  
  // Attachement de la memoire partagee
  parkingMPPtr = (ParkingMP*) shmat(shmParkingId, NULL, 0);
  requeteMPPtr = (RequeteMP*) shmat(shmRequeteId, NULL, 0);
  
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
  sigaddset(&actionUSR2.sa_mask, SIGCHLD); // Si un voiturier meurt pendant la phase de destruction, il sera traite comme les autres lors du waitpid (non bloquand dans ce cas)
  actionUSR2.sa_flags = 0;
  sigaction (SIGUSR2, &actionUSR2, NULL);
  
  // Deblocage des signaux SIGUSR2 et SIGCHLD
  sigset_t listeSignalDebloque;
  sigemptyset(&listeSignalDebloque);
  sigaddset(&listeSignalDebloque, SIGUSR2);
  sigaddset(&listeSignalDebloque, SIGCHLD);
  sigprocmask(SIG_UNBLOCK, &listeSignalDebloque, NULL);
	

  
  // MOTEUR
  unsigned int numeroPlace;
  
  for (;;)
  {
	while(read (descLectureCanal, &numeroPlace, sizeof(unsigned int)) == -1 && errno == EINTR);
	pid_t pid = SortirVoiture(numeroPlace);
	if (pid != (pid_t) -1) // Sinon risque de fermeture de la session lors de la destruction de l'application
	{
	  listeVoiturierSortie.insert(pid);
	}
  }
}
