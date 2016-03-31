#ifndef ENTREE_H
#define ENTREE_H

void GestionEntree(int canalEntree[][2], int canalSortie[2], TypeBarriere typeEntree, int shmIdParking, int shmIdRequete, int semId);
/* Procedure representant le fonctionnement de la tache GestionEntree
contrat:
-TypeBarriere doit etre une valeur de voie parmi PROF_BLAISE_PASCAL, AUTRE_BLAISE_PASCAL, ENTREE_GASTON_BERGER
*/

#endif
