#IPK projekt 1 - Vilem Gottwald (xgottw07)
CC=g++
CPPFLAGSERROR= -Wall -Wextra -Werror -pedantic
SRC_FILE= server.cpp
OUT_FILE= hinfosvc

.PHONY: final strict

final: $(SRC_FILE)
	$(CC) $^ -o $(OUT_FILE)

strict: $(SRC_FILE)
	$(CC) $^ $(CPPFLAGSERROR) -o $(OUT_FILE)
