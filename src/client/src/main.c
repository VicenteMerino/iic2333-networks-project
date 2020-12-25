#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "conection.h"
#include "comunication.h"

char * get_input(){
  char * response = malloc(20);
  int pos=0;
  while (1){
    char c = getchar();
    if (c == '\n') break;
    response[pos] = c;
    pos++;
  }
  response[pos] = '\0';
  return response;
}


int main (int argc, char *argv[]){
  srand(time(0));
  if(argc != 5 || strcmp(argv[1], "-i") != 0 || strcmp(argv[3], "-p") != 0)
  {
    printf("Modo de uso: ./client -i <ip_address> -p <tcp_port>\n");
    return 1;
  }
  //Se obtiene la ip y el puerto donde está escuchando el servidor (la ip y puerto de este cliente da igual)
  char * IP = argv[2];
  int PORT = atoi(argv[4]);

  // Se prepara el socket
  int server_socket = prepare_socket(IP, PORT);

  int pid = fork();

  if(pid > 0)
  {
    while(1)
    {
      int msg_code = client_receive_id(server_socket);
      if (msg_code == 9)
      {
        exit(1); 
        kill(pid, SIGINT);
      }
      if(msg_code)
      {
        char * message = client_receive_payload(server_socket);
        printf("\n%s\n", message);
        free(message);
      }
    }
  }
  else if(pid == 0)
  {
    while(1)
    {
      char* out_msg = get_input();
      client_send_message(server_socket, 1, out_msg);
      free(out_msg);
    }
  }
  close(server_socket);
  free(IP);

  return 0;
}
