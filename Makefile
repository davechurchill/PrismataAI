CC=g++

CFLAGS=-O3 -std=c++17 -flto -Wformat=0
LDFLAGS=-O3 -flto -pthread
LDFLAGS_GUI=-lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

INCLUDES=-Isource -Isource/rapidjson -Isource/ai -Isource/engine

SRC_AI=$(wildcard source/ai/*.cpp) 
OBJ_AI=$(SRC_AI:.cpp=.o)

SRC_ENGINE=$(wildcard source/engine/*.cpp) 
OBJ_ENGINE=$(SRC_ENGINE:.cpp=.o)

SRC_GUI=$(wildcard source/gui/*.cpp) 
OBJ_GUI=$(SRC_GUI:.cpp=.o)

SRC_TESTING=$(wildcard source/testing/*.cpp) 
OBJ_TESTING=$(SRC_TESTING:.cpp=.o)

SRC_STANDALONE=$(wildcard source/standalone/*.cpp) 
OBJ_STANDALONE=$(SRC_STANDALONE:.cpp=.o)

all:bin/PrismataAI_GUI bin/PrismataAI_Testing bin/PrismataAI_Standalone

bin/PrismataAI_GUI:$(OBJ_AI) $(OBJ_ENGINE) $(OBJ_GUI) Makefile
	$(CC) $(OBJ_AI) $(OBJ_ENGINE) $(OBJ_GUI) -o $@  $(LDFLAGS) $(LDFLAGS_GUI)

bin/PrismataAI_Testing:$(OBJ_AI) $(OBJ_ENGINE) $(OBJ_TESTING) Makefile
	$(CC) $(OBJ_AI) $(OBJ_ENGINE) $(OBJ_TESTING) -o $@  $(LDFLAGS)
	
bin/PrismataAI_Standalone:$(OBJ_AI) $(OBJ_ENGINE) $(OBJ_STANDALONE) Makefile
	$(CC) $(OBJ_AI) $(OBJ_ENGINE) $(OBJ_STANDALONE) -o $@  $(LDFLAGS)

.cpp.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@ 

clean:
	rm -f bin/PrismataAI_GUI bin/PrismataAI_Testing bin/PrismataAI_Standalone source/ai/*.o source/engine/*.o source/engine/gui/*.o

