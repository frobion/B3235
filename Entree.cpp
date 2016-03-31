#include <signal.h>
#include <unistd.h>
#include <sys/msg.h>
#include <algorithm>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <vector>

#include "Outils.h"

//static shmP shmDescripteur;
//static shmP shmRequete;
//static shmP shmCompteur;

static vector<pid_t> listeFils;


// ------ Handler SIGUSR2 --------
static void HandlerUSR2 ( int noSig )
{
  //  DESTRUCTION

  // Destruction de tous les deplacements
  for(unsigned int i = 0; i < listeFils.size(); i++)
  {
  	kill(listeFils[i],SIGUSR2);
  	waitpid(listeFils[i],0,0);
  }

  // Detachement de la memoire

  //shmdt(shmDescripteur.shm);
  //shmdt(shmRequete.shm);
  //shmdt(shmCompteur.shm);

  exit(0);
}

// TODO : HANDLER SIGCHLD


void GestionEntree(TypeBarriere typeEntree)
{
    // --  INITIALISATION  --

    // creation des handlers de SIGUSR2
    struct sigaction actionHandlerUSR2;
    actionHandlerUSR2.sa_handler = HandlerUSR2;
    sigemptyset(&actionHandlerUSR2.sa_mask); // vide le masque
    actionHandlerUSR2.sa_flags = 0;
    // HandlerSIGUSR2 ne doit pas etre interrompu par SIGCHLD
    // On ajoute donc SIGCHLD aux signals masquees
    sigaddset(&actionHandlerUSR2.sa_mask, SIGCHLD);

    // creation des handlers de SIGCHLD
    struct sigaction actionHandlerCHLD;
    actionHandlerCHLD.sa_handler = HandlerCHLD;
    sigemptyset(&actionHandlerCHLD.sa_mask);
    actionHandlerCHLD.sa_flags = 0;

    // -- Attachement memoire partagée --
        // Si le pointeur vers l'espace d'adressage est null, alors ca prend la 1ere dispo
        // ------- shmDescripteur
    shmDescripteur = p_ShmDescripteur;
	  shmDescripteur.shm =(int*)shmat(shmDescripteur.idShm, NULL, SHM_RDONLY);
  /*
        // ------- shmRequete
    shmRequete = p_ShmRequete;
    shmRequete.shm =(int*)shmat(shmRequete.idShm, NULL, SHM_RDONLY);
        // ------- shmCompteur
    shmCompteur = p_ShmCompteur;
    shmCompteur.shm =(int*)shmat(shmCompteur.idShm, NULL, SHM_RDONLY);
*/

    // Armement des signaux SIGUSR2 et SIGCHILD
    sigaction(SIGUSR2, &actionHandlerUSR2, NULL);
    sigaction(SIGCHLD, &actionHandlerCHLD, NULL);



    // --  MOTEUR --
    while(true)
    {

        // SI ON UTILISE UNE BOITE AU LETTRES
          // TODO : ADAPTER POUR UN CANAL DE COMMUNICATION
          // TODO : CREER UNE STRUCTURE VOITURE AVEC TYPE_USAGER ET TYPE_BARRIERE
        /*
        while(msgrcv(idFile, &MsgVoitRecue,TAILLE_MSG_VOITURE,typeEntree,0) == -1);
        voitRecue = MsgVoitRecue.uneVoiture;
        */

        // On dessine la voiture a la barriere
        DessinerVoitureBarriere(voitRecue.barriere, voitRecue.usager);


        // TODO : on regarde toutes les 0.5s si il a une place de libre et des requetes
        /*
        while(shmDescripteur.Lire() == 0  && shmDescripteur.Lire() != 0)
        {
            sleep(0.5);
        }
        */


        // Si il y a une place on gard la voiture
        pid_t pidVoiturier = GarerVoiture(voitRecue.barriere);


        // on garde le pid du Voiturier
		    listeFils.push_back(pidVoiturier);


    }
}
