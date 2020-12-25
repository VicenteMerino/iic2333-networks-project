
#pragma once

#include <stdbool.h>
#include "conection.h"

void send_message_to_all(PlayersInfo* players_info, char* msg);
bool cmd_start(PlayersInfo* players_info, int index, char* message, bool started);
void cmd_exit(PlayersInfo* players_info, int index);
int cmd_players(PlayersInfo* players_info, int index);
int cmd_vote(PlayersInfo* players_info, int index, int color);
int cmd_kill(PlayersInfo* players_info, int index, int color);
int cmd_spy(PlayersInfo* players_info, int index, int color);
int cmd_whisper(PlayersInfo* players_info, int index, int color, char* message);

int check_win_status(PlayersInfo* players_info); // 1 si ganaron los tripulantes, 2 si ganaron los impostores, 0 si no ha ganado nadie
void send_roles(PlayersInfo* players_info);
void send_winner(PlayersInfo* players_info, int winner); //winner = 1 si ganaron los tripulantes, 2 si ganaron los impostores
