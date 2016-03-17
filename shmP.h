#ifndef SHMP_H
#define SHMP_H


/* structure representant une memoire partag�e proteg�e comprenant un mutex et une memoire partag�e
Les fonctions Lire et Ecrire sont prot�g�es par le mutex*/
struct shmP {
	int idShm;
	int idMutex;
	int * shm;



void Ecrire(int indice, int valeur);


int Lire(int indice);



};
#endif
