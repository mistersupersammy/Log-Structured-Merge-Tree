#include "LSMTree.h"
#include <time.h>

// hard coding of value to test
#define SIZE 1000
#define VALUE_SIZE 10
#define FILENAME_SIZE 16

// Read a batch of random keys from an LSM_tree inside the range [key_down, key_up]
void batch_random_read(LSM_tree *lsm, int num_reads, int key_down, int key_up){
    // To store the value_read
    char* value_read;
    int index;
    // Initialize the seed
    srand(time(NULL));
    for (int i=0; i<num_reads; i++){
        index = (int)(key_down + (rand()/(float)RAND_MAX) * (key_up - key_down));
        value_read = read_lsm(lsm, index);
        if (value_read != NULL){
            if (VERBOSE == 1) printf("Reading key: %d; value found: %s\n",
                                       index, value_read);
        }
        else {
            if (VERBOSE == 1) printf("Reading key: %d; key not found:\n", index);
        }
        free(value_read);
    } 
}

// Generate and save to disk an LSM-Tree
void lsm_generation(char* name, int size_test){
    // Creating lsm structure:
    int Nc = 6;
    // List contains Nc+2 elements: [C0, buffer, C1, C2,...]
    // Last component with very big size (as finite number of components chosen)
    int Cs_size[] = {SIZE, 3*SIZE, 9*SIZE, 27*SIZE, 81*SIZE, 729*SIZE, 9*729*SIZE, 1000000000};

    LSM_tree *lsm = (LSM_tree*) malloc(sizeof(LSM_tree));
    build_lsm(lsm, name, Nc, Cs_size, VALUE_SIZE, FILENAME_SIZE);

    // ---------------- TEST OF APPENDING
    // filling the lsm
    char value[VALUE_SIZE];

    // Sorted keys
    // for (int i=0; i < size_test; i++){
    //     // Filling value
    //     sprintf(value, "hello%d", i%150);
    //     append_lsm(lsm, i, value);
    // }    

    // Unsorted keys
    // adding even keys
    int j;
    for (int i=0; i < size_test/2; i++){
        j = size_test/2 - i - 1;
        // Filling value
        sprintf(value, "hello%d", (2*j)%150);
        append_lsm(lsm, (2*j), value);
    }
    // adding odd keys
    for (int i=size_test/2; i < size_test; i++){
        // Filling value
        sprintf(value, "hello%d", (2*(i-size_test/2) + 1)%150);
        append_lsm(lsm, 2*(i-size_test/2) + 1, value);
    }

    // Print status of the lsm tree
    print_state(lsm);

    // Print some sequence of keys
    printf("First 10 keys of the buffer are\n");
    for (int i=0; i < 10; i++){
        printf("%d \n", lsm->buffer->keys[i]);
    }
    printf("Last 10 keys of the buffer are\n");
    for (int i=*(lsm->buffer->Ne) - 10; i < *(lsm->buffer->Ne); i++){
        printf("%d \n", lsm->buffer->keys[i]);
    }
    printf("First 10 keys of CO are\n");
    for (int i=0; i < 10; i++){
        printf("%d \n", lsm->C0->keys[i]);
    }

    // Printing first element of adisk component (read from disk)
    int disk_index = 5;
    char * comp_id = (char *) malloc(10*sizeof(char));
    sprintf(comp_id, "C%d", disk_index);

    component * C = (component *) malloc(sizeof(component));
    read_disk_component(C, name, lsm->Cs_Ne + disk_index+1, comp_id,
                        lsm->Cs_size + disk_index+1, VALUE_SIZE, FILENAME_SIZE);

    // Printing
    printf("First 10 keys of %s are\n", comp_id);
    for (int i=0; i < 10; i++){
        printf("%d \n", C->keys[i]);
    
    }

    // ---------------- TEST OF READING FROM LSM

    // Reading from lsm
    int length = 5;
    int targets[] = {-3, 0, 100, 7000, 60000};
    char* value_read;
    for (int i=0; i<length; i++){
        value_read = read_lsm(lsm, targets[i]);
        if (value_read != NULL) printf("Reading key: %d; value found: %s\n",
                                       targets[i], value_read);
        else printf("Reading key: %d; key not found\n", targets[i]);
        free(value_read);
    } 

    // Writing to disk
    write_lsm_to_disk(lsm);

    // Free memory
    free(comp_id);
    free_component(C);
    free_lsm(lsm);
}

//Test storing on disk an array
int main(){
    char name[] = "test";
    int size_test = 1000000;

    // ---------------- TEST READING LSM FROM DISK
    printf("Reading LSM from disk:\n");
    LSM_tree *lsm_backup = (LSM_tree *)malloc(sizeof(LSM_tree));
    read_lsm_from_disk(lsm_backup, name, FILENAME_SIZE);

    print_state(lsm_backup);

    batch_random_read(lsm_backup, 500, 0, 2*size_test);

    // Free memory
    free_lsm(lsm_backup);

    printf("Work done\n");
}