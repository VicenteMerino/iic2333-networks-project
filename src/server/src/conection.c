
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include "conection.h"
#include "comunication.h"
#include "utils.h"
#include <time.h>

//LINKS REFERENCIAS:
//https://www.man7.org/linux/man-pages/man2/socket.2.html
//https://man7.org/linux/man-pages/man7/socket.7.html
//https://www.man7.org/linux/man-pages/man3/setsockopt.3p.html
//https://man7.org/linux/man-pages/man2/setsockopt.2.html
//https://linux.die.net/man/3/htons
//https://linux.die.net/man/3/inet_aton
//https://www.howtogeek.com/225487/what-is-the-difference-between-127.0.0.1-and-0.0.0.0/
//https://www.man7.org/linux/man-pages/man2/bind.2.html
//https://www.man7.org/linux/man-pages/man2/accept.2.html


int prepare_socket(char * IP, int port){
  // Se define la estructura para almacenar info del socket del servidor al momento de su creación
  struct sockaddr_in server_addr;

  // Se solicita un socket al SO, que se usará para escuchar conexiones entrantes
  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  fcntl(server_socket, F_SETFL, O_NONBLOCK);

  // Se configura el socket a gusto (recomiendo fuertemente el REUSEPORT!)
  int opt = 1;
  int ret = setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

  // Se guardan el puerto e IP en la estructura antes definida
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  inet_aton(IP, &server_addr.sin_addr);
  server_addr.sin_port = htons(port);

  // Se le asigna al socket del servidor un puerto y una IP donde escuchar
  int ret2 = bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));

  // Se coloca el socket en modo listening
  int ret3 = listen(server_socket, 8);

  return server_socket;
}

PlayersInfo* init_players_info(int* colors)
{
  PlayersInfo* socket_clients = malloc(sizeof(PlayersInfo));
  socket_clients -> client_count = 0; //Al principio no hay nadie conectado
  socket_clients -> connected_clients = 0;
  socket_clients -> players = malloc(sizeof(Player) * 8);
  for (int i = 0; i < 8; i ++)
  {
    socket_clients -> players[i] = malloc(sizeof(Player));
    socket_clients -> players[i] -> socket = -1;
    socket_clients -> players[i] -> color_name_styled = string_from_color_styled(colors[i]);
    socket_clients -> players[i] -> color_name = string_from_color(colors[i]);
    socket_clients -> players[i] -> is_impostor = false;
    socket_clients -> players[i] -> status = 2; //Todos empiezan vivos
    socket_clients -> players[i] -> vote = NULL;

  }
  socket_clients->spy_opportunity = true;

  return socket_clients;
}

void get_client(PlayersInfo* players_info, int index, int server_socket, int color)
{
   // Se definen las estructuras para almacenar info sobre los sockets de los clientes
  struct sockaddr_in client_addr;
  socklen_t addr_size = sizeof(client_addr); 
  int accept_status = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
  if(accept_status != -1)
  {
    players_info -> connected_clients += 1;
    players_info-> players[index] -> socket = accept_status;
    
    char * welcome = "¡¡¡Bienvenido a AMONG RUZ!!!";
    server_send_message(players_info -> players[index] -> socket, 1, welcome);
    
    char* color_string = string_from_color_styled(color);
    char conexiones[255];
    sprintf(conexiones, "Hay %d jugadores conectados y tu color es %s", players_info-> connected_clients, color_string);
    server_send_message(players_info -> players[index] -> socket, 1, conexiones);

    players_info -> players[index] -> color = color;

    char player_alert[255];
    sprintf(player_alert, "El jugador %s se ha conectado", color_string);

    for (int j = 0; j < 8; j++)
    {
      if(j != index && players_info -> players[j] -> socket != -1)
      {
        server_send_message(players_info -> players[j] -> socket, 1, player_alert);
      }
    }
  }
}

void send_ghost_chat_msg(PlayersInfo* players_info, int index, char* message)
{
  char * response[255];
  sprintf(response, "[%s - FANTASMA]: %s", players_info -> players[index] -> color_name_styled, message);
  for( int i = 0; i <8; i++){
    if (players_info -> players[i] -> socket != -1 && players_info->players[i] -> status != 2)
    {
      server_send_message(players_info -> players[i] -> socket, 1, response);
    }
  }
}
void send_chat_msg(PlayersInfo* players_info, int index, char* message)
{
  char * response[255];
  sprintf(response, "[%s]: %s", players_info -> players[index] -> color_name_styled, message);
  for( int i = 0; i <8; i++){
    if (players_info -> players[i] -> socket != -1)
    {
      server_send_message(players_info -> players[i] -> socket, 1, response);
    }
  }
}

