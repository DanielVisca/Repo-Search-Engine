#include <stdio.h>

#include <string.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "freq_list.h"
#include "worker.h"

/* A program to model calling run_worker and to test it. Notice that run_worker
 * produces binary output, so the output from this program to STDOUT will 
 * not be human readable.  You will need to work out how to save it and view 
 * it (or process it) so that you can confirm that your run_worker 
 * is working properly.
 */
int main(int argc, char **argv) {
    char ch;
    char path[PATHLENGTH];
    char *startdir = ".";

    /* this models using getopt to process command-line flags and arguments */
    while ((ch = getopt(argc, argv, "d:")) != -1) {
        switch (ch) {
        case 'd':
            startdir = optarg;
            break;
        default:
            fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME]\n");
            exit(1);
        }
    }

    // Open the directory provided by the user (or current working directory)
    DIR *dirp;
    if ((dirp = opendir(startdir)) == NULL) {
        perror("opendir");
        exit(1);
    }

    /* For each entry in the directory, eliminate . and .., and check
     * to make sure that the entry is a directory, then call run_worker
     * to process the index file contained in the directory.
     * Note that this implementation of the query engine iterates
     * sequentially through the directories, and will expect to read
     * a word from standard input for each index it checks.
     */
    struct dirent *dp;
    while ((dp = readdir(dirp)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 ||
            strcmp(dp->d_name, "..") == 0 ||
            strcmp(dp->d_name, ".svn") == 0 ||
            strcmp(dp->d_name, ".git") == 0) {
                continue;
        }

        strncpy(path, startdir, PATHLENGTH);
        strncat(path, "/", PATHLENGTH - strlen(path));
        strncat(path, dp->d_name, PATHLENGTH - strlen(path));
        path[PATHLENGTH - 1] = '\0';
        

        // MAXWORKER is the length of the array
        struct stat sbuf;
        if (stat(path, &sbuf) == -1) {
            // This should only fail if we got the path wrong
            // or we don't have permissions on this entry.
            perror("stat");
            exit(1);
        }

        // Only call run_worker if it is a directory
        // Otherwise ignore it.
        // printf("path: %s\n", path);
        int num_of_children = 0; //used by child only
        if (S_ISDIR(sbuf.st_mode)) {
            while (1){
                char *word = malloc(MAXWORD);
                if (fgets(word ,MAXWORD ,stdin) == NULL){
                    perror("Standard input could not be read");
                    exit(1);
                }
                int p_to_w[MAXWORKERS][2];
                int w_to_p[MAXWORKERS][2];
                
                // case for when fork didnt work properly
                // create all of the pipes
                for (int i = 0; i < MAXWORKERS; i++){
                    pipe(p_to_w[i]);
                    pipe(w_to_p[i]);
                }

                int id = fork();
                //if child process
                if (id == 0) {
                    for (int j = 0; j < MAXWORKERS; j++){            
                        // close the read end of the pipe
                        close(p_to_w[j][1]);
                        close(w_to_p[j][0]);
                    }
                    // worker in gets written to by child, worker out gets read by parent                 
                    run_worker(path, p_to_w[num_of_children][0], w_to_p[num_of_children][1] );
                    // close the side that writes to the child
                    close(w_to_p[num_of_children][1]);
                    close(p_to_w[num_of_children][0]);
                    num_of_children++;
                    
                    free(word);
                    exit(1);
                }
            
                if (id > 0) { 
                    // for i in w_to_p read: 
                    // close the write end of the pipe
                    for (int i = 0; i < MAXWORKERS; i++){
                        close(p_to_w[i][0]);
                        close(w_to_p[i][1]);
                    }
                    // read an array of FreqRecords from pipe
                    FreqRecord **array = malloc(sizeof(FreqRecord) * MAXRECORDS);
                    int array_index = 0;
                    // also need to test for sentinel array
                    // printf("Lets read from the pipe!");
                    //printf("read(%d, array[index], MAXRECORDS)", w_to_p[0][0]);
                    //print_freq_records(w_to_p[0][0]);
                    
                    // write!!!
                    printf("parent is about to read from the pipe\n");
                    //write word to all children
                    for (int n = 0; n < MAXWORKERS; n++){
                        write(p_to_w[n][1], word, MAXWORD);
                    }
                    int m = 0;
                    while (read(w_to_p[m][0], array[array_index], MAXRECORDS) > 0){ // I dont like this because I am only reading from one child.
                    //sort array?
                        // Insert and order in master array
                        array_index++;
                    }                        
                    
                    free(array);
                }
         
            }

        }
    }
    if (closedir(dirp) < 0){
        perror("closedir");
    }

    return 0;
}
