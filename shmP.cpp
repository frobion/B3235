#include <unistd.h>
#include <sys/sem.h>

#include "shmP.h"

struct sembuf Opv {0,1,0 } ;
struct sembuf Opp {0, -1, 0 } ;

int shmP::Lire(int indice)
{
    /* Lecture dans la memoire partagee */
			// les whiles correspondent au semaphore de protection
	int valeur;
	while(semop( idMutex, &Opp, 1)==-1);
	valeur = shm[indice];
	while(semop( idMutex, &Opv, 1)==-1);
	return valeur;
}

void shmP::Ecrire(int indice, int valeur)
{
    /* Ecriture dans la memoire partagee */
			// les whiles correspondent au semaphore de protection
	while(semop( idMutex, &Opp, 1)==-1);
	shm[indice] = valeur;
	while(semop( idMutex, &Opv, 1)==-1);


}
