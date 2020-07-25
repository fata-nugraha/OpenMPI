#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

int N = 0;

int getmin_index(long **graph, bool pickedVertices[N], int sourceVertex) {
    int minDistance = INT_MAX;
    int min_index = -1;

    for (int j = 0; j < N; j++) {
        if (!pickedVertices[j] && graph[sourceVertex][j] <= minDistance) {
            minDistance = graph[sourceVertex][j];
            min_index = j;
        }
    }
    return min_index;
}

void print(long **graph){
    printf("Matrix: \n");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%ld ", graph[i][j]);
        }
        printf("\n");
    }
}

void dijkstra(long** graph, int sourceVertex) {

    // Distance from single source to all of the nodes
    bool pickedVertices[N];

    for (int vertex = 0; vertex < N; vertex++) {
        pickedVertices[vertex] = false;
    }

    for (int i = 0; i < N - 1; i++) {
        // Get minimum distance
        int min_index = getmin_index(graph, pickedVertices, sourceVertex);

        // Mark the vertice as picked
        pickedVertices[min_index] = true;

        // Update distance value
        for (int vertex = 0; vertex < N; vertex++) {
            if ((!pickedVertices[vertex]) && 
                (graph[min_index][vertex]) && 
                (graph[sourceVertex][min_index] != INT_MAX) &&
                (graph[sourceVertex][min_index] + graph[min_index][vertex] < graph[sourceVertex][vertex])) {
                
                graph[sourceVertex][vertex] = graph[sourceVertex][min_index] + graph[min_index][vertex];
            }
        }
    }
    return;
}

int main(int argc, char *argv[]) {
	
    // Get matrix size from argument vector in , convert to int
    N = strtol(argv[1], NULL, 10);

    long** graph;
    graph = (long**) malloc(sizeof(long*) * N);
    for (int i = 0; i < N; ++i)
    {
        graph[i] = (long*) malloc(sizeof(long) * N);
    }

    // int numtasks, rank, dest, source, rc, count, tag=1;
    // double start_time, end_time, total_time;

    srand(13517115);
	// Fill the matrix with rand() function
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            graph[i][j] = rand();
        }
    }

    // Assign with infinity
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (!(i == j || graph[i][j])){
                graph[i][j] = INT_MAX;
            }
        }
    }
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (i == j){
                graph[i][j] = 0;
            }
        }
    }

    clock_t time;
    time = clock();
    
    for (int i = 0; i < N; i++ ) {
        dijkstra(graph, i);
    }

    time = clock() - time;
    double time_taken = ((double)time); // in seconds

    printf("%f µs\n", time_taken);
  

    FILE *f = fopen("output.txt", "w");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            fprintf(f, "%ld ", graph[i][j]);
        }
        fprintf(f, "\n");
    }
    fclose(f);


    for (int i = 0; i < N; ++i) {
        free(graph[i]);
    }
    free(graph);

    return 0;
}