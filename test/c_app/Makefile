CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lm

CLIENT = client_wav
SERVER = server_wav

SRC_CLIENT = client.c
SRC_SERVER = server.c

# Règle par défaut (lance 'all')
.PHONY: all clean fclean re

all: $(CLIENT) $(SERVER)

$(CLIENT): $(SRC_CLIENT)
	@echo "[+] Compilation du client..."
	$(CC) $(CFLAGS) $(SRC_CLIENT) -o $(CLIENT) $(LDFLAGS)

$(SERVER): $(SRC_SERVER)
	@echo "[+] Compilation du serveur..."
	$(CC) $(CFLAGS) $(SRC_SERVER) -o $(SERVER) $(LDFLAGS)

clean:
	@echo "[-] Nettoyage des fichiers temporaires..."
	@rm -f *.o

fclean: clean
	@echo "[-] Suppression des exécutables et des résidus..."
	@rm -f $(CLIENT) $(SERVER) output.wav

re: fclean all