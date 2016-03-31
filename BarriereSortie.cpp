#include "BarriereSortie.h"
#include "config.h"

#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

static int descLectureCanal;

static void handlerSigChld (int noSignal)
{
  
}

static void handlerSigUsr2 (int noSignal)
{
  
}

void BarriereSortie(int canauxBarriereEntree[][2], int canalBarriereSortie[], int shmId, int semId)
{
  // INIT
  // Blocage SIGUSR1, SIGUSR2, SIGCHLD
  sigset_t listeSignalBloque;
  sigemptyset(&listeSignalBloque);
  sigaddset(&listeSignalBloque, SIGUSR1);
  sigaddset(&listeSignalBloque, SIGUSR2);
  sigaddset(&listeSignalBloque, SIGCHLD);
  
  sigprocmask(SIG_SETMASK, &listeSignalBloque, NULL);
  
  // Fermeture canaux inutilise
  //~ for (int i = 0; i < NB_BARRIERE_ENTREE; i++)
  //~ {
    //~ close(canauxBarriereEntree[i][0];
    //~ close(canauxBarriereEntree[i][1];
  //~ }
  //~ close(canalBarriereSortie[1];
  //~ descLectureCanal = canalBarriereSortie[0];
  
  // Attachement de la memoire partagee
  void * shmAdresse = shmat(shmId, 	NULL, 0);
  
  
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
  struct sembuf listeOperation[1];
  
  for (;;)
  {
	read (descLectureCanal, &numeroPlace, sizeof(unsigned int));
	listeOperation[0].sem_num = numSemParking;
	listeOperation[0].sem_op = -1; 
	semop (semId, listeOperation, 1); // Decrementation de 1 dans le semaphore
	// lecture MP
	
	
  }
	
  // DESTRUCTION
  // Detachement de la MP
  shmdt( shmAdresse);
}
