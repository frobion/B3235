/*************************************************************************
                           Entree  -  description
                             -------------------
    début                : 23/03/2016
    copyright            : (C) 2016 par Nathan Arsac
*************************************************************************/

//---------- Interface de la tâche <Entree> (fichier Entree.h) -------
#ifndef ENTREE_H
#define ENTREE_H

//------------------------------------------------------------------------
// Rôle de la tâche <Entree>
//	La tache Entrée permet de gérer les voitures qui rentrent dans le
//	Parking
//------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////  INCLUDE
//--------------------------------------------------- Interfaces utilisées
#include "Outils.h"
//------------------------------------------------------------- Constantes

//------------------------------------------------------------------ Types

//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques

void GestionEntree(int canalEntree[][2], int canalSortie[2], TypeBarriere typeEntree, int shmIdParking, int shmIdRequete, int semId);
//  Mode d'emploi :
//	Processus fils Entree
//  Contrat :
//

#endif
