#ifndef SHMP_H
#define SHMP_H


/* structure representant une memoire partagée protegée comprenant un mutex et une memoire partagée
Les fonctions Lire et Ecrire sont protégées par le mutex*/
struct shmP {
	int idShm;
	int idMutex;
	int * shm;



void Ecrire(int indice, int valeur);


int Lire(int indice);



};
#endif
