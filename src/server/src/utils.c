#include <stdio.h> 
#include <stdlib.h> 
#include <time.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

int* get_colors()
{
  srand(time(NULL));
  static int array[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  for (int i = 0; i < 8; i++){
    int temp = array[i];
    int index = rand() % 8;
    array[i] = array[index];
    array[index] = temp;
  }
  return array;
} 

char *string_from_color_styled(int color)
{
    char *strings[8] = {"\033[38;5;196mRojo\033[0m", "\033[38;5;208mNaranja\033[0m", "\033[38;5;220mAmarillo\033[0m", "\033[38;5;34mVerde\033[0m", "\033[38;5;45mCeleste\033[0m", "\033[38;5;21mAzul\033[0m", "\033[38;5;128mVioleta\033[0m", "\033[38;5;206mRosado\033[0m"};
    return strings[color];
}

char *string_from_color(int color)
{
    char *strings[8] = {"Rojo", "Naranja", "Amarillo", "Verde", "Celeste", "Azul", "Violeta", "Rosado"};
    return strings[color];
}

bool starts_with(const char *pre, const char *str) // Sacado de: https://stackoverflow.com/questions/4770985/how-to-check-if-a-string-starts-with-another-string-in-c
{
    size_t lenpre = strlen(pre);
    size_t lenstr = strlen(str);
    return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}

bool check_start(char* message) // Solo reviso si comienza con \start
{
  return starts_with("\\start ", message);
}

char* lowercase(char* s) {
  for(char *p=s; *p; p++) *p=tolower(*p);
  return s;
}

int color_string_to_color_code(char* str)
{
  char *strings[8] = {"rojo", "naranja", "amarillo", "verde", "celeste", "azul", "violeta", "rosado"};
  for (int code = 0; code < 8; code++)
  {
    if (strcmp(strings[code], str)==0){
      return code;
    }
  }
  return -1;
}
