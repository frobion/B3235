EXE = Parking

# Compilateur et editeur de lien
COMP = g++
LINK = g++

# Options de compilation et editions de liens
CHEMIN = /shares/public/tp/tp-multitache
INC = -I$(CHEMIN)
LIB = -L$(CHEMIN)
CPPFLAGS = -Wall -ansi -ggdb -std=c++11 -g $(INC)
EDLFLAGS = $(LIB)


#Fichiers

SRC =
INT = Mere.h Simulation.h  #Mettre les .h ici
REAL = $(INT:.h=.cpp)
OBJ = $(INT:.h=.o) Mere.o #Mettre le .o du programme de test la où ya le main

#Autres commandes et message
ECHO = @echo
RM = @rm
MESSAGE = "Compilation terminée"

$(EXE): $(OBJ)
	$(LINK)  -o $(EXE) $^ $(EDLFLAGS) -ltp -lncurses -ltcl
	$(ECHO) $(MESSAGE)

#Mettre les dependances particulieres ici

%.o:%.cpp
	$(COMP) -c $(CPPFLAGS) $<

Mere.cpp:Mere.h Simulation.h
Simulation.cpp:Simulation.h

clean:
	$(RM) -fv *.o $(EXE)
