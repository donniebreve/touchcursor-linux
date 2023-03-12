#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "get_layer.h"

#define FNAME_CAPS "/sys/class/leds/input3::capslock/brightness"
#define FNAME_NUM "/sys/class/leds/input3::numlock/brightness"
#define FNAME_SCROLL "/sys/class/leds/input3::scrolllock/brightness"

char *files[] = {FNAME_CAPS, FNAME_NUM, FNAME_SCROLL};

FILE *fptr;

int check_files_exist(){
  int files_ret=0;
  for(int i=0; i<3; i++)
    files_ret += access(files[i], F_OK);
  return !files_ret;
}

int get_num_from_file(char* file_path)
{
    int num;
    FILE *fptr;

    if ((fptr = fopen(file_path,"r")) == NULL){
      printf("Error! opening file %s\n", file_path);exit(1);}
    fscanf(fptr,"%d", &num);
    fclose(fptr);
    return num;
}

unsigned int get_layer(){
  unsigned int layer_acc = 0;

  if (!check_files_exist())
    return 0;
  
  for(int i=0; i<3; i++){
    layer_acc *=2;
    layer_acc += get_num_from_file(files[i]);
  }

  return layer_acc <= 7 ? layer_acc : 0;
}