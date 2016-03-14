#include "Outils.h"
#include "Menu.h"

void Simulation()
{
  // TODO INIT
  for(;;)
  {
    Menu();
  }
}

void Commande (char code, unsigned int valeur)
{
  switch(code)
  {
    case 'E':
      exit(0);
      break;
    default:
      Afficher(TypeZone::MESSAGE, "Commande: code inconnue");
      
  }
}
