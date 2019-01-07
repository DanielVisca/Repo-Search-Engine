#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>                                           

#include "freq_list.h" 
#include "worker.h"
//#include "freq_list.h"

int main (){
    Node *head;
    char* filename_array[MAXFILES];
    read_list("index", "filenames", &head, filename_array);
    FreqRecord *returned = get_word ("char", head, filename_array);
    print_freq_records(returned);



}
