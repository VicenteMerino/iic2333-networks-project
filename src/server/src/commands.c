
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include "conection.h"
#include "comunication.h"
#include "utils.h"

void send_color_error_message(PlayersInfo* players_info, int i)
{
  char* out_msg = "Color inválido. (Revisa los colores válidos con \\players).";
  server_send_message(players_info -> players[i] -> socket, 1, out_msg);
}

int color_to_index(PlayersInfo* players_info, int color)
{
  for (int i = 0; i<8; i++)
    if (players_info->players[i]->color == color && players_info->players[i] -> socket != -1)
      return i;
  return -1;
}

char* index_to_color_string_styled(PlayersInfo* players_info, int index)
{
  return string_from_color_styled(players_info->players[index]->color);
}

void send_message_to_all(PlayersInfo* players_info, char* msg)
{
    for(int i = 0; i < 8; i++)
    {
      if(players_info -> players[i] -> socket != -1){
        server_send_message(players_info -> players[i] -> socket, 1, msg);
      }
    }
}

void send_winner(PlayersInfo* players_info, int winner)
{ 
  if(winner == 1)
  {
    char* msg = "Ganaron los RUZMATES";
    send_message_to_all(players_info, msg);
  }
  else if(winner == 2){
    char* msg = "Ganaron los IMPOSTORES";
    send_message_to_all(players_info, msg);
  }

  for(int i = 0; i < 8; i++)
  {
    players_info -> players[i] -> is_impostor = false;
    players_info -> players[i] -> status = ALIVE;
  }
}

void send_roles(PlayersInfo* players_info)
{
  for(int i = 0; i < 8; i++){
    if(players_info -> players[i] -> status != -1 && players_info -> players[i] -> is_impostor)
    {
      char msg[255];
      sprintf(msg, "El jugador %s era IMPOSTOR", players_info -> players[i] -> color_name_styled);
      send_message_to_all(players_info, msg);
    }
    else if(players_info -> players[i] -> socket != -1 && !players_info -> players[i] -> is_impostor)
    {
      char msg[255];
      sprintf(msg, "El jugador %s era RUZMATE", players_info -> players[i] -> color_name_styled);
      for(int j = 0; j < 8; j++)
      {
        if(players_info -> players[i] -> socket!= -1){
          server_send_message(players_info -> players[j] -> socket, 1, msg);
        }
      }
    }
  }
}

int check_win_status(PlayersInfo* players_info)
{
  int impostores_vivos = 0;
  int tripulantes_vivos = 0;
  for(int i = 0; i < 8; i++)
  {

    if (players_info -> players[i] -> is_impostor && players_info -> players[i] -> socket != -1 && players_info -> players[i] -> status == 2)
    {
      impostores_vivos += 1;
    }
    else if (!players_info -> players[i] -> is_impostor && players_info -> players[i] -> socket != -1 && players_info ->  players[i] -> status == 2)
    {
      tripulantes_vivos += 1;
    }
  }
  if(impostores_vivos == 0)
  {
    return 1; //Ganan los tripulantes 
  }
  else if(impostores_vivos >= tripulantes_vivos)
  {
    return 2; //Ganan  los impostores
  }
  return 0; //No ha ganado nadie
}

static void send_message_to_player(PlayersInfo* players_info, int index, char* msg)
{
  server_send_message(players_info -> players[index] -> socket, 1, msg);
}

static void set_to_expelled(PlayersInfo* players_info, int index, char* msg)
{
  if (players_info -> players[index] -> status == 2) 
  {
    players_info -> client_count -= 1;
  }
  players_info -> players[index] -> status = EXPELLED;
  send_message_to_all(players_info, msg);
}

static void set_to_disconected(PlayersInfo* players_info, int index, char* msg)
{
  if (players_info -> players[index] -> status == 2) 
  {
    players_info -> client_count -= 1;
  }
  players_info -> players[index] -> status = DISCONECTED;
  send_message_to_all(players_info, msg);
}

static void kick(PlayersInfo* players_info, int index)
{
  char msg[255];
  if(players_info -> players[index] -> is_impostor)
  {
    sprintf(msg, "El jugador [%s] se ha desconectado. Su rol era IMPOSTOR.", players_info -> players[index] -> color_name_styled);
  }
  else
  {
    sprintf(msg, "El jugador [%s] se ha desconectado. Su rol era RUZMATE.", players_info -> players[index] -> color_name_styled);
  }
  set_to_disconected(players_info, index, msg);
  send_message_to_player(players_info, index, "Has sido desconectado");
  server_send_message(players_info -> players[index] -> socket, 9, "desconectado");
  players_info->connected_clients -= 1;
}

static void set_to_eliminated(PlayersInfo* players_info, int index, char* msg)
{
  players_info -> players[index] -> status = ELIMINATED;
  players_info->client_count -= 1;
  send_message_to_all(players_info, msg);
}

bool cmd_start(PlayersInfo* players_info, int index, char* message,bool started)
{
  int connections = players_info -> connected_clients;
  if(strcmp(message, "\\start 1") == 0 && connections >= 3 && !started)
  {
    int conexiones_validas[connections];
    int j = 0;
    for(int i = 0; i < 8; i++){
      if (players_info -> players[i] -> socket != -1){
        conexiones_validas[j] = i; //Guardo el indice de la conexion valida
        j++;
      }
    }
    int impostor_index = rand() % connections;
    players_info -> players[conexiones_validas[impostor_index]] -> is_impostor = true;
    char* impostor_response = "La partida ha empezado. Tu rol es IMPOSTOR";
    char* ruzmate_response = "La partida ha empezado. Tu rol es RUZMATE";

    for(int i = 0; i < 8; i++)
    {
      if(players_info -> players[i] -> is_impostor)
      {
        server_send_message(players_info -> players[i] -> socket, 1, impostor_response);
      }
      else
      {
        server_send_message(players_info -> players[i] -> socket, 1, ruzmate_response);
      }
    }
    players_info -> client_count = players_info -> connected_clients;
    return true; //La partida ha comenzado
  }
  else if(strcmp(message, "\\start 1") == 0 && connections < 3 && !started)
  {
    char* response = "Deben haber al menos 3 jugadores conectados";
    server_send_message(players_info -> players[index] -> socket, 1, response);
  }
  else if(strcmp(message, "\\start 1") == 0 && started)
  {
    char* response = "El juego ya ha comenzado";
    server_send_message(players_info -> players[index] -> socket, 1, response);
  }

  else if (strcmp(message, "\\start 2") == 0 && connections >= 5 && !started)
  {
    int conexiones_validas[connections];
    int j = 0;
    for(int i = 0; i < 8; i++)
    {
      if (players_info -> players[i] -> socket != -1)
      {
        conexiones_validas[j] = i; //Guardo el indice de la conexion valida
        j++;
      }
    }
    
    int impostor_index1 = rand() % connections;
    int impostor_index2 = rand() % connections;

    while (impostor_index1 == impostor_index2)
    {
      impostor_index2 = rand() % connections;
    }

    char* impostor_response = "La partida ha empezado. Tu rol es IMPOSTOR";
    char* ruzmate_response = "La partida ha empezado. Tu rol es RUZMATE";
    players_info -> players[conexiones_validas[impostor_index1]] -> is_impostor = true;
    players_info -> players[conexiones_validas[impostor_index2]] -> is_impostor = true;

    for(int i = 0; i < 8; i++)
    {
      if(players_info -> players[i] -> is_impostor)
      {
        server_send_message(players_info -> players[i] -> socket, 1, impostor_response);
      }
      else
      {
        server_send_message(players_info -> players[i] -> socket, 1, ruzmate_response);
      }
    }
    players_info -> client_count = players_info -> connected_clients;
    return true; // La partida ha comenzado
  }
  else if(strcmp(message, "\\start 2") == 0 && connections < 5 && !started)
  {
    char* response = "Deben haber al menos 5 jugadores conectados";
    server_send_message(players_info -> players[index] -> socket, 1, response);
  }
  else if(strcmp(message, "\\start 2") == 0 && started)
  {
    char* response = "El juego ya ha comenzado";
    server_send_message(players_info -> players[index] -> socket, 1, response);
  }
  else if(starts_with("\\start ", message)){
    char* response = "Modo de uso: \\start n, con n = 1 o 2.";
    server_send_message(players_info -> players[index] -> socket, 1, response);
  }
  return false; 
}

void cmd_exit(PlayersInfo* players_info, int index)
{
  int socket_to_close = players_info -> players[index] -> socket;
  kick(players_info, index);
  close(socket_to_close);
  players_info -> players[index] -> socket = -1;
}

int cmd_vote(PlayersInfo* players_info, int index, int color) 
{
  if (players_info -> players[index] -> status != 2)
  {
    char msg[255];
    sprintf(msg, "No puedes usar \\vote si estás muerto.");
    send_message_to_player(players_info, index, msg);
    return 1;
  }
  for (int i = 0; i < 8; i++)
  {
    if (players_info -> players[i] -> socket != -1 && players_info -> players[i] -> color == color) 
    {
      char out_msg[255];
      if (players_info -> players[i] -> status == ALIVE)
      {
        players_info-> players[index] -> vote = players_info -> players[i];
        int majority = ((int) players_info -> client_count / 2) + 1;
        int vote_count = 0;
        for (int i = 0; i < 8; i++)
        {
          if(players_info -> players[i] -> vote && players_info -> players[i] -> vote -> color == color)
          {
            vote_count += 1;
          }
        }
        if (vote_count >= majority)
        {
          for(int i = 0; i < 8; players_info -> players[i++] -> vote = NULL);
          if (players_info -> players[i] -> is_impostor)
          {
            sprintf(out_msg, "¡¡Hubo mayoría!! El jugador [%s] ha sido expulsado. Su rol era IMPOSTOR.", players_info -> players[i] -> color_name_styled);
          }
          else
          {
            sprintf(out_msg, "¡¡Hubo mayoría!! El jugador [%s] ha sido expulsado. Su rol era RUZMATE.", players_info -> players[i] -> color_name_styled);
          }
          set_to_expelled(players_info, i, out_msg);
          return 0;
        }
        sprintf(out_msg, "Votaste por el jugador %s. Aún no se alcanza una mayoría.", players_info -> players[i] -> color_name_styled);
        send_message_to_player(players_info, index, out_msg);
        return 0;
      }
      else
      {
        sprintf(out_msg, "El Jugador %s no está vivo, no se puede votar.", players_info -> players[i] -> color_name_styled);
        send_message_to_player(players_info, index, out_msg);
        return 1;
      }
    }
  }
  char* error_msg = "El color seleccionado no está en juego.";
  send_message_to_player(players_info, index, error_msg);
  return 1;
}

int cmd_players(PlayersInfo* players_info, int index) 
{
  // 0 si es eliminado, 1 si es expulsado, 2 si está vivo
  //TODO: Enviar;
  char* banner = "PLAYERS INFO";
  server_send_message(players_info -> players[index] -> socket, 1, banner);
  for (int i = 0; i < 8; i++)
  {
    if (players_info -> players[i] -> socket != -1 || players_info -> players[i] -> status == DISCONECTED)
    {
      char player_color[255];
      if (i == index) 
      {
        sprintf(player_color,"Jugador %s (Yo)", players_info-> players[i] -> color_name_styled);
      }
      else
      {
        sprintf(player_color,"Jugador %s", players_info-> players[i] -> color_name_styled);
      }    
      server_send_message(players_info -> players[index] -> socket, 1, player_color);
      switch (players_info-> players[i] -> status)
      {
        case 0:
          server_send_message(players_info -> players[index] -> socket, 1, "--> STATUS: eliminado");
          break;
        case 1:
          server_send_message(players_info -> players[index] -> socket, 1, "--> STATUS: expulsado");
          break;
        case 2:
          server_send_message(players_info -> players[index] -> socket, 1, "--> STATUS: vivo");
          break;
        case 3:
          send_message_to_player(players_info, index,  "--> STATUS: expulsado (desconectado)");
      }
      
      if (players_info-> players[index] -> is_impostor == true && players_info-> players[i] -> is_impostor)
      {
        server_send_message(players_info -> players[index] -> socket, 1, "    IMPOSTOR");
      }
      char vote[255];
      if(players_info -> players[i] -> vote != NULL)
      {
        sprintf(vote, "    Voto: %s", players_info -> players[i] -> vote -> color_name_styled);
        server_send_message(players_info -> players[index] -> socket, 1, vote);
        server_send_message(players_info -> players[index] -> socket, 1, "------------------------");
      }
      else{
        sprintf(vote, "    Voto: %s", "Pass");
        server_send_message(players_info -> players[index] -> socket, 1, vote);
        server_send_message(players_info -> players[index] -> socket, 1, "------------------------");
      }
    }
  }
  return 0;
}

int cmd_kill(PlayersInfo* players_info, int index, int color)
{
  char msg[255];
  if (color == -1)
  {
    send_color_error_message(players_info, index);
    return 1;
  }
  /*send_message_to_all(players_info, "Se esta matando a alguien\n"); //borrar esto */
  if (!players_info -> players[index] -> is_impostor)
  {
    send_message_to_player(players_info, index, "¡Estás loco! No puedes matar a un ruzmate.");
    return 1;
  }
  else if (players_info -> players[index] -> status != 2)
  {
    sprintf(msg, "No puedes matar a alguien si estás muerto.");
    send_message_to_player(players_info, index, msg);
    return 1;
  }
  int to_kill = color_to_index(players_info, color);
  if (to_kill == -1)
  {
    sprintf(msg, "No puedes matar a alguien que no nunca ha existido.");
    send_message_to_player(players_info, index, msg);
    return 1;
  }
  if (players_info -> players[to_kill]->status != 2)
  {
    sprintf(msg, "No puedes matar a alguien que no está vivo.");
    send_message_to_player(players_info, index, msg);
    return 1;
  }
  int roll = rand() % 10;
  if (roll <= 2)
  {
    set_to_eliminated(players_info, to_kill, msg);
    send_message_to_player(players_info, index, "Asesinato exitoso.\n");
    sprintf(msg, "Jugador %s a sido eliminado.\n", string_from_color_styled(color));
    send_message_to_all(players_info, msg);
  }
  else if (roll <= 5)
  {
    sprintf(msg, "%s te trató de asesinar.\n", index_to_color_string_styled(players_info, index));
    send_message_to_player(players_info, to_kill, msg);
  }
  else 
  {
    sprintf(msg, "Se intentó matar a %s. \n", string_from_color(color));
    send_message_to_all(players_info, msg);
  }
  return 0;
}

int cmd_spy(PlayersInfo* players_info, int index, int color)
{
  //* Si el jugador es impostor: Retornamos 1
  if (players_info -> players[index] -> status != 2)
  {
    char msg[255];
    sprintf(msg, "No puedes usar spy si estás muerto.");
    send_message_to_player(players_info, index, msg);
    return 1;
  }

  if (players_info -> players[index] -> is_impostor)
  {
    char* out_msg = "No puedes utilizar el comando siendo impostor.";
    server_send_message(players_info -> players[index] -> socket, 1, out_msg);
    return 1;
  }
  else
  {
    //* Si el jugador es ruzmante: Usa el comando y retornamos 0
    for (int i = 0; i < 8; i++)
    {
      if (players_info -> players[i] -> socket != -1 && color == players_info -> players[i] -> color && players_info -> players[i] -> status == ALIVE)
      {
        char out_msg[255];
        if (players_info -> players[i] -> is_impostor)
        {
          sprintf(out_msg, "El rol del jugador %s es: IMPOSTOR", players_info -> players[i] -> color_name_styled);
        }
        else
        {
          sprintf(out_msg, "El rol del jugador %s es: RUZMATE", players_info -> players[i] -> color_name_styled);
        }
        server_send_message(players_info -> players[index] -> socket, 1, out_msg);
        players_info->spy_opportunity = false;
        return 0;
      }
    }
    //* Si el color no existe: Retornamos 2
    char* out_msg = "El color seleccionado no está en juego.";
    server_send_message(players_info -> players[index] -> socket, 1, out_msg);
    return 2;
  }
}

int cmd_whisper(PlayersInfo* players_info, int index, int color, char* message)
{
  //* Retornamos 0 si el mensaje fue exitoso
  if (players_info -> players[index] -> status != 2)
  {
    char msg[255];
    sprintf(msg, "No hay un necromántico que permita que mandes un mensaje a una persona viva... o muerta.");
    send_message_to_player(players_info, index, msg);
    return 1;
  }

  for (int i = 0; i < 8; i++)
  {
    char out_msg1[255];
    char out_msg2[255];
    if (players_info -> players[i] -> socket != -1 && players_info -> players[i] -> color == color)
    {
      sprintf(out_msg1, "Mensaje privado enviado al jugador %s", players_info -> players[i] -> color_name_styled);
      sprintf(out_msg2, "[PRIVADO - %s]: %s", players_info -> players[index] -> color_name_styled, message);
      server_send_message(players_info -> players[index] -> socket, 1, out_msg1);
      server_send_message(players_info -> players[i] -> socket, 1, out_msg2);
      return 0;
    }
  }
  //* Retornamos 1 si el mensaje no fue exitoso (el color no está en la partida)
  char* out_msg = "El color seleccionado no está en juego.";
  server_send_message(players_info -> players[index] -> socket, 1, out_msg);
  return 1;
}

