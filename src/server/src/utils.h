#pragma once
#include <stdbool.h>

int* get_colors();
char *string_from_color(int color);
char *string_from_color_styled(int color);
bool starts_with(const char *pre, const char *str);
bool check_start(char* message);
int color_string_to_color_code(char* str);