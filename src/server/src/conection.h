#pragma once

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include "comunication.h"

typedef enum { ELIMINATED, EXPELLED, ALIVE, DISCONECTED } Status;

struct player;
typedef struct player Player;
struct player {
  int socket; // Socket del jugador
  int color; // Color code
  char* color_name; // color
  char* color_name_styled; // color con color
  bool is_impostor; //  true si es impostor, false si no
  Status status; // 0 si es eliminado, 1 si es expulsado, 2 si está vivo, 3 fue desconectado
  Player* vote; //El jugador por el que estoy votando
};

typedef struct players_info {

  int client_count;  //Cantidad de clientes actuales (es mutable durante la partida, por ejemplo con exit)
  int connected_clients; // La cantidad con la que se empieza la partida (no es mutable durante la partida)
  bool spy_opportunity;
  Player** players;
} PlayersInfo;


int prepare_socket(char * IP, int port);
PlayersInfo* init_players_info(int* colors);
void get_client(PlayersInfo* players_info, int index, int server_socket, int color);
void send_chat_msg(PlayersInfo* players_info, int index, char* message);
void send_ghost_chat_msg(PlayersInfo* players_info, int index, char* message);
bool check_start(char* message);
