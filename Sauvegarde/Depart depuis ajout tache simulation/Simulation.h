/*************************************************************************
                           Simulation.h  -  description
                             -------------------
    début                : Lundi 14 mars 2016
    copyright            : (C) 2016 par Francois Robion
    e-mail               : francois.robion@insa-lyon.fr
*************************************************************************/

//---------- Interface du module <Simulation> (fichier Simulation.h) -----
#if ! defined SIMULATION_H
#define SIMULATION_H

//------------------------------------------------------------------------
// Rôle du module <Simulation>
//------------------------------------------------------------------------

////////////////////////////////////////////////////////////////// INCLUDE
//--------------------------------------------------- Interfaces utilisées

//------------------------------------------------------------- Constantes

//------------------------------------------------------------------ Types

/////////////////////////////////////////////////////////////////// PUBLIC
//---------------------------------------------------- Fonctions publiques
void Simulation(int canauxBarriereEntree[][2], int canalBarriereSortie[]);
void Commande(char code, unsigned int valeur);


#endif // SIMULATION_H
