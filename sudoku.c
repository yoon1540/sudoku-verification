// ===============================
// Authors: Andrew Yoon, Justin Lu
// Assignment 3 Question 1
// 11/04/2023
// ===============================

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pthread.h>

#define SIZE 9

typedef struct {
    int row;
    int column;
    int (*sudoku)[SIZE]; 
} parameters;

// Reads sudoku element from the file into the buffer and then array. 
void read_sudoku(int fd, int sudoku[SIZE][SIZE]) {
    char ch;
    ssize_t bytes_read;
    int i = 0, j = 0;

    while ((bytes_read = read(fd, &ch, 1)) > 0 && i < SIZE) {
        if (ch >= '1' && ch <= '9') { 
            sudoku[i][j] = ch - '0'; 
            j++;
            if (j == SIZE) {
                j = 0;
                i++;
            }
        }
    }

}


// Checks if every column in sudoku element is valid 
void* check_column(void *args) {
    parameters* data = (parameters *)args;
    int (*sudoku)[SIZE] = data->sudoku; 

    for (int col = data->column; col < SIZE; col++) {
        int numCheck[SIZE] = {0};
        for (int i = 0; i < SIZE; i++) {
            int num = sudoku[i][col];
            if (numCheck[num - 1] == 1 || num > SIZE || num < 1) { 
                pthread_exit((void *)0);
            }
            numCheck[num - 1] = 1; 
        }
    }
    pthread_exit((void *)1);
}

// Checks if every row in sudoku element is valid 
void* check_row(void *args) {
    parameters* data = (parameters *)args;
    int (*sudoku)[SIZE] = data->sudoku; 

    for (int row = data->row; row < SIZE; row++) {
        int numCheck[SIZE] = {0};
        for (int i = 0; i < SIZE; i++) {
            int num = sudoku[row][i];
            if (numCheck[num - 1] == 1 || num > SIZE || num < 1) { 
                pthread_exit((void *)0);
            }
            numCheck[num - 1] = 1; 
        }
    }
    pthread_exit((void *)1);
}

// Checks if every subgrid in sudoku element is valid 
void* check_subgrid(void *args){
    parameters* data = (parameters *)args;
    int numCheck[SIZE] = {0};
    int (*sudoku)[SIZE] = data->sudoku;
    for(int col = data->column; col < data->column+3; col++){
        for (int row = data->row; row < data->row+3;row++){
            int num = sudoku[row][col];
            if (numCheck[num - 1] == 1 || num > SIZE || num < 1) {
                pthread_exit((void *)0);
            }
            numCheck[num - 1] = 1;
        }
    }
    pthread_exit((void *)1);

}


int main(int argc, char *args[]) {
    char *file_name = "sudoku.txt";// file name
    int sudoku[SIZE][SIZE];

    // Open the file
    int fd = open(file_name, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    read_sudoku(fd, sudoku);
    close(fd);

    parameters *data = (parameters *)malloc(sizeof(parameters));

    data->column = 0;
    data->row = 0;  
    data->sudoku = sudoku; 

    pthread_t t1, t2;
    pthread_t t3[SIZE];
    parameters subgrid[SIZE];
    subgrid->sudoku=sudoku;

    int index = 0;

    for (int i = 0; i < SIZE; i+=3){
        for (int j = 0; j < SIZE; j += 3){
            subgrid[index].row = i;
            subgrid[index].column = j;
            subgrid[index].sudoku = sudoku;
            pthread_create(&t3[index], NULL, check_subgrid, &subgrid[index]);
            index++;
        }
    }

    pthread_create(&t1, NULL, check_column, data);
    pthread_create(&t2, NULL, check_row, data);

    void *column_results;
    void *row_results;
    int subgrid_valid = 1; // int to  track  subgrid validity

    pthread_join(t1, &column_results);
    pthread_join(t2, &row_results);

    for (int i = 0; i < SIZE; i++) {
        void *subgrid_result;
        pthread_join(t3[i], &subgrid_result);
        if (subgrid_result == (void *)0) { // If subgrid is invalid, set subgrid_valid to 0
            subgrid_valid = 0;
        }
    }


    // Print the sudoku grid 
    printf("Sudoku Puzzle Solution is:\n");
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            printf("%d ", sudoku[i][j]);
        }
        printf("\n");
    }

    if (column_results == (void *)1 && row_results == (void *)1 && subgrid_valid) {
        printf("Sudoku is valid.\n");
    } else {
        printf("Sudoku is invalid.\n");
    }

    free(data);

    return EXIT_SUCCESS;
}
