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

#include <iostream>
#include <fstream>

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

static fstream fichier ("LogBarriereSortie.txt");


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
  fichier << time(NULL)%10000 << "  " << "debut handlerSigChld" << std::endl;
  time_t tempsSortie = (time(NULL)) % 10000;
  int numeroPlaceLibere;
  pid_t pidFilsMort = waitpid(-1, &numeroPlaceLibere, 0); // Destruction du fils mort
  numeroPlaceLibere = WEXITSTATUS(numeroPlaceLibere);
  listeVoiturierSortie.erase(pidFilsMort);
  fichier << time(NULL)%10000 << "  " << "pid fils mort : " << pidFilsMort << std::endl << "Place libere : " << numeroPlaceLibere << std::endl;
  
  // Acces au semaphore de protection de parking
  fichier << time(NULL)%10000 << "  " << "Demande acces semaphore parking" << std::endl;
  while(semop (semId, decrSemParking, 1) == -1 && errno == EINTR);
  fichier << time(NULL)%10000 << "  " << "Acces obtenu du semaphore parking" << std::endl;
  
  // Remise de la place dans l'etat sans voiture
  AfficherSortie(parkingMPPtr->parking[numeroPlaceLibere].usager, parkingMPPtr->parking[numeroPlaceLibere].immatriculation, 
      parkingMPPtr->parking[numeroPlaceLibere].dateArrive, tempsSortie);
  initVoiture(&(parkingMPPtr->parking[numeroPlaceLibere]));
  Effacer((TypeZone) numeroPlaceLibere); // Correspond a la bonne valeur de la zone de l'enum
  
  // Liberation du semaphore de protection de parking
  semop(semId, incrSemParking, 1);
  fichier << time(NULL)%10000 << "  " << "Liberation du semaphore parking" << std::endl;
  
  
  
  Requete requetesActuelles [NB_BARRIERES_ENTREE];
  int nbRequetesActuelles;
  // Acces au semaphore de protection des requetes
  fichier << time(NULL)%10000 << "  " << "Demande acces du semaphore requete" << std::endl;
  while(semop(semId, decrSemRequete, 1) == -1 && errno == EINTR);
  fichier << time(NULL)%10000 << "  " << "Acces au semaphore requete" << std::endl;
  
  nbRequetesActuelles = chercheRequetesActuelles(requetesActuelles);
  fichier << time(NULL)%10000 << "  " << "Nombre de requetes : " << nbRequetesActuelles << std::endl;
  
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
	fichier << time(NULL)%10000 << "  " << "Barriere de la meilleur requete : " << meilleurRequete.barriere << std::endl;
	switch(meilleurRequete.barriere)
	{
	  case PROF_BLAISE_PASCAL:
	    fichier << time(NULL)%10000 << "  " <<"Reveil PROF_BLAISE_PASCAL" << std::endl;
	    // Reveil de la tache entree correspondante
	    semop (semId, incrSemProfBlaisePascal, 1);
	    // Reinitilisation de la MP pour la requete correspondante
	    initVoiture(&(requeteMPPtr->requetes[TypeBarriere::PROF_BLAISE_PASCAL - 1].voiture));
	    requeteMPPtr->requetes[TypeBarriere::PROF_BLAISE_PASCAL - 1].barriere = TypeBarriere::AUCUNE;
	    // Gestion de l'affichage
	    Effacer(TypeZone::REQUETE_R1);
	    break;
	  case AUTRE_BLAISE_PASCAL:
	    fichier << time(NULL)%10000 << "  " << "Reveil AUTRE_BLAISE_PASCAL" << std::endl;
	    semop(semId, incrSemAutreBlaisePascal, 1);
	    initVoiture(&(requeteMPPtr->requetes[TypeBarriere::AUTRE_BLAISE_PASCAL - 1].voiture));
	    requeteMPPtr->requetes[TypeBarriere::AUTRE_BLAISE_PASCAL - 1].barriere = TypeBarriere::AUCUNE;
	    Effacer(TypeZone::REQUETE_R2);
	    break;
	  case ENTREE_GASTON_BERGER:
	    fichier << time(NULL)%10000 << "  " << "Reveil ENTREE_GASTON_BERGER" << std::endl;
	    semop(semId, incrSemGastonBerger, 1);
	    initVoiture(&(requeteMPPtr->requetes[TypeBarriere::ENTREE_GASTON_BERGER - 1].voiture));
	    requeteMPPtr->requetes[TypeBarriere::ENTREE_GASTON_BERGER - 1].barriere	 = TypeBarriere::AUCUNE;
	    Effacer(TypeZone::REQUETE_R3);
	    break;
	  default:
	    Afficher(TypeZone::MESSAGE, "Mauvaise barriere, handler SIGCHLD, barriereSortie");
	    fichier << time(NULL)%10000 << "  " << "default" << std::endl;
	    break;
	}
  }
  semop(semId, incrSemRequete, 1); // Liberation du semaphore de protection des requetes
  fichier << time(NULL)%10000 << "  " << "Fin sigChld" << std::endl;
}

static void handlerSigUsr2 (int noSignal)
{
  fichier << time(NULL)%10000 << "  " << "Debut handlerSigUsr2" << std::endl;
  Afficher(TypeZone::MESSAGE, "SIGUSR2 recu par sortie");
  close(descLectureCanal);
  
  shmdt(parkingMPPtr);
  shmdt(requeteMPPtr);
  
  std::set<pid_t>::iterator it;
  for (it = listeVoiturierSortie.begin(); it != listeVoiturierSortie.end(); it++)
  {
	kill(*it, SIGUSR2);
	fichier << time(NULL)%10000 << "  " << "Demande de mort de " << *it << std::endl;
	while(waitpid(*it, NULL, 0) == -1 && errno == EINTR);
	fichier << time(NULL)%10000 << "  " << "Mort de " << *it << std::endl;
  }
  
  fichier << time(NULL)%10000 << "  " << "Fin de barriereSortie" << std::endl;
  fichier.close();
  exit(0);
}

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
void BarriereSortie(int canauxBarriereEntree[][2], int canalBarriereSortie[], int shmParkingId, int shmRequeteId, int semIdParam)
{
  // INIT
  fichier << time(NULL)%10000 << "  " << "Debut barriereSortie" << std::endl;
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
  actionUSR2.sa_flags = 0;
  sigaction (SIGUSR2, &actionUSR2, NULL);
  
  // Deblocage des signaux SIGUSR2 et SIGCHLD
  sigset_t listeSignalDebloque;
  sigemptyset(&listeSignalDebloque);
  sigaddset(&listeSignalDebloque, SIGUSR2);
  sigaddset(&listeSignalDebloque, SIGCHLD);
  sigprocmask(SIG_UNBLOCK, &listeSignalDebloque, NULL);
	

  fichier << time(NULL)%10000 << "  " << "Debut moteur" << std::endl;
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
	fichier << time(NULL)%10000 << "  " << "appel voiturier sortie" << std::endl;
  }
}
