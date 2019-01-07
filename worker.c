#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

#include "freq_list.h"
#include "worker.h"

/* Given a word, return an array of structs that contain the files the word appears
 * in, as well as the frequency of which the word occurs in each file. 
*/
FreqRecord *get_word(char *word, Node *head, char **file_names) {
    
    // Search through the Linked List
    while (head != NULL){

        // Found the word in the linked list
        if (strcmp(head->word, word) == 0) {
            
            // Find how long the freq array is until it reaches a zero
            int sizeof_freq = sizeof(head->freq)/sizeof(int); // calculate max size    
            FreqRecord *foundWord = malloc(sizeof_freq * sizeof(FreqRecord) + 1);

            //search through frequencies
            int j = 0;
            int times_found = 0;
            while (j < sizeof_freq) {
                if( head->freq[j] != 0 ) {
                    // Update FreqRecord in the Foundword array at the appropriate index 
                    
                                                      
                    foundWord[times_found].freq = head->freq[j];
                    strcpy(foundWord[times_found].filename, file_names[j]);
                                                        
                    times_found++;
                }
                j++;
            }
            
            // set last element to empty
            foundWord[times_found-1].freq = 0;
            strcpy(foundWord[times_found-1].filename, "");
                        
            return foundWord;
        }
        else {
            head = head->next;
        }
    }
    return NULL;
}

/* Print to standard output the frequency records for a word.
* Use this for your own testing and also for query.c
*/
void print_freq_records(FreqRecord *frp) {
    int i = 0;
    
    while (frp != NULL && frp[i].freq != 0) {
        printf("%d    %s\n", frp[i].freq, frp[i].filename);
        i++;
    }
}

/* Read a word in from a given file descriptor. Call get_word on it then write .
 * the output to another given file descriptor
*/
void run_worker(char *dirname, int in, int out) {
    
    // Open file, have perror warning
    char index_path[1024];
    strcpy(index_path, dirname);
    char filenames_path[1024];
    //char *index_path = strcat(dirname, "/index");
    strcpy(filenames_path, dirname);
    strcat(index_path, "/index");
    strcat(filenames_path, "/filenames");
    // Fill linkedlist and files with directory content
    Node *head = malloc (sizeof(FreqRecord)); // maybe malloc
    char **filenames_array = malloc(MAXFILES * (MAXLINE* sizeof(char)));
   
    read_list(index_path, filenames_path, &head, filenames_array);
    
    char word[MAXLINE];
 
    if (read(in, word, MAXLINE) <= 0){
        printf("there is an error reading in the run_worker method :0");
        exit(1);
    };
    
    char * newLine = strstr(word, "\n");
    *newLine = '\0';    
    FreqRecord *get_word_returned = get_word(word, head, filenames_array);
    
    // print_freq_records(get_word_returned);
    if (write(out, get_word_returned, MAXRECORDS) <= 0) {
        perror("worker could not write to parent");
        exit(1);
    }
    free(head);
    free(filenames_array);
    free(get_word_returned);
    
}


