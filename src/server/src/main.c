#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "comunication.h"
#include "conection.h"
#include "utils.h"
#include "commands.h"
#define debug_print printf


static void reset_game(PlayersInfo* players_info)
{
  for (int i = 0; i < 8; players_info->players[i]->status == DISCONECTED? 
      players_info->players[i++]->status = 0 : 
      i++)
  {
    players_info -> players[i] -> is_impostor = false;
    players_info -> players[i] -> vote = NULL;
  }

  players_info -> spy_opportunity = true;

}

static char* get_command(char* msg)
{
  char* out = malloc(sizeof(char) * 255);
  for (int i = 0; i<255; out[i++] = 0);
  if (msg[0] == '\\')
  {
    int i = 0;
    for (; msg[i] && i<255; i++);
    if (i < 255)
      for (int j = 0; j < 255 - i && msg[i+j] != ' '; msg[j++] = msg[i+j]);
  }
  return out;
}

static char* get_arguments(char* msg)
{
  char* out = malloc(sizeof(char) * 255);
  for (int i = 0; i<255; out[i++] = 0);
  int i = 0;
  for (; msg[i]!=' ' && i < 255; i++);
  i++;
  if (i < 255)
    for (int j = 0; j < 255 - i; out[j++] = msg[i + j]);
  return out;
}

static void send_color_kill_error_message(PlayersInfo* players_info, int i)
{
  char* out_msg = "Modo de uso: kill <color>. (Revisa los colores válidos con \\players).";
  server_send_message(players_info -> players[i] -> socket, 1, out_msg);
}

static void send_color_error_message(PlayersInfo* players_info, int i)
{
  char* out_msg = "Modo de uso: vote <color>. (Revisa los colores válidos con \\players).";
  server_send_message(players_info -> players[i] -> socket, 1, out_msg);
}

int main(int argc, char *argv[])
{
  srand(time(0));
  if (argc != 5 || strcmp(argv[1], "-i") != 0 || strcmp(argv[3], "-p") != 0)
  {
    printf("Modo de uso: ./server -i <ip_address> -p <tcp_port>\n");
    return 1;
  }
  // Se define una IP y un puerto
  char * IP = argv[2];
  int PORT = atoi(argv[4]);

  int server_socket = prepare_socket(IP, PORT);
  bool started = false; // Aun no comienza la partida

  // Se crea el servidor y se obtienen los sockets de ambos clientes.
  int * colors = get_colors();
  PlayersInfo* players_info = init_players_info(colors);

  while(1)
  {
    for (int i = 0; i < 8; i ++) //Checkeo constantemente el estado de los sockets
    {
      if(players_info -> players[i] -> socket == -1 && !started) //Si vale -1 y no ha comenzado el juego, le hago accept
      {
        if(players_info -> client_count < 8)
        {
          get_client(players_info, i, server_socket, colors[i]);
        }
      }
      else if(players_info -> players[i] -> socket != -1)
      {
        int msg_code = server_receive_id(players_info -> players[i] -> socket);
        
        if(msg_code)
        {
          //* Recibimos el mensaje por parte del cliente y lo procesamos
          char* client_message = server_receive_payload(players_info -> players[i] -> socket);
          char* message_copy = strdup(client_message);
          char* command = strtok(message_copy, " ");
          if(client_message[0] == 0)
          {
            continue;
          }
          if(check_start(client_message) && !started)
          {
            started = cmd_start(players_info, i, client_message, started); // Actualizo el estado de la partida si ya ha partido
          }
          else if(strcmp("\\exit", command) == 0 && started)
          {
            char* rest = strtok(NULL, "");
            if (!rest)
            {
              cmd_exit(players_info, i);
              int win_status = check_win_status(players_info);
              if(win_status){
                send_roles(players_info);
                send_winner(players_info, win_status);
                reset_game(players_info);
                started = false;
              }
            }
            else
            {
              char* out_msg = "Modo de uso: \\exit.";
              server_send_message(players_info -> players[i] -> socket, 1, out_msg);
            }
          }
          else if(strcmp("\\players", command) == 0 && started)
          {
            char* rest = strtok(NULL, "");
            if (!rest)
            {
              cmd_players(players_info, i);
            }
            else
            {
              char* out_msg = "Modo de uso: \\players.";
              server_send_message(players_info -> players[i] -> socket, 1, out_msg);
            }
          }
          else if(starts_with("\\kill", client_message) && started){
            char* arguments = get_arguments(client_message);
            int color_code = color_string_to_color_code(arguments);
            free(arguments);
            if (color_code != -1)
            {
              /*send_message_to_all(players_info, "entered color code.");*/
              cmd_kill(players_info, i, color_code);
              int win_status = check_win_status(players_info);
              if(win_status){
                send_roles(players_info);
                send_winner(players_info, win_status);
                reset_game(players_info);
                started = false;
              }
            }
            else 
              send_color_kill_error_message(players_info, i);
          }          
          else if(strcmp("\\vote", command) == 0 && started){
            char* color_target = strtok(NULL, " ");
            char* rest = strtok(NULL, "");
            if (!color_target || rest)
            {
              send_color_error_message(players_info, i);
            }
            else
            {
              int color_code = color_string_to_color_code(color_target);
              if (color_code != -1)
              {
                cmd_vote(players_info, i, color_code);
                int win_status = check_win_status(players_info);
                if(win_status){
                  send_roles(players_info);
                  send_winner(players_info, win_status);
                  reset_game(players_info);
                  started = false;
            }
              }
              else
              {
                char* out_msg = "El color seleccionado no es válido."; 
                server_send_message(players_info -> players[i] -> socket, 1, out_msg);
              } 
            }
          }
          else if (strcmp("\\spy", command) == 0 && started)
          {
            if (players_info->spy_opportunity)
            {
              char* color_target = strtok(NULL, " ");
              char* rest = strtok(NULL, "");
              if (!color_target || rest)
              {
                char* out_msg = "Modo de uso: spy <color>.";
                server_send_message(players_info -> players[i] -> socket, 1, out_msg);
              }
              else
              {
                int color_code = color_string_to_color_code(color_target);
                if (color_code == -1)
                {
                  char* out_msg = "El color seleccionado no es válido."; 
                  server_send_message(players_info -> players[i] -> socket, 1, out_msg);
                }
                else
                {
                  cmd_spy(players_info, i, color_code);
                }
              }
            }
            else
            {
              char* out_msg = "El comando ya fue utilizado por otro jugador.";
              server_send_message(players_info -> players[i] -> socket, 1, out_msg);
            }
          }
          else if (strcmp("\\whisper", command) == 0 && started)
          {
            char* color_target = strtok(NULL, " ");
            if (!color_target)
            {
              char* out_msg = "Modo de uso: \\whisper <color> <mensaje>.";
              server_send_message(players_info -> players[i] -> socket, 1, out_msg);
            }
            else
            {
              int color_code = color_string_to_color_code(color_target);
              if (color_code == -1)
              {
                char* out_msg = "El color seleccionado no es válido.";
                server_send_message(players_info -> players[i] -> socket, 1, out_msg);
              }
              else
              {
                char* color_message = strtok(NULL, "");
                if (!color_message)
                {
                  char* out_msg = "No escribiste un mensaje para el destinatario.";
                  server_send_message(players_info -> players[i] -> socket, 1, out_msg);
                }
                else
                {
                  cmd_whisper(players_info, i, color_code, color_message);
                }
              }
            }
          }
          else if (players_info -> players[i] -> status != 2)
          {
            send_ghost_chat_msg(players_info, i, client_message);
          }
          else
          {
            send_chat_msg(players_info, i, client_message);
          }
          free(message_copy);
        }
      }
    }
  }
  return 0;
}
