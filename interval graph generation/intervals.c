#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_N 12  // Adjust based on expected maximum n

int n;
int permutation[2 * MAX_N];
int used_start[MAX_N] = {0};
int used_end[MAX_N] = {0};

// Structure to hold adjacency matrices and intervals
typedef struct {
    char matrix[MAX_N * MAX_N];
    int starts[MAX_N];
    int ends[MAX_N];
} Graph;

Graph graphs[1000];  // Adjust size as needed
int graph_count = 0;

// Check if two graphs are identical
int are_graphs_equal(Graph a, Graph b) {
    return memcmp(a.matrix, b.matrix, sizeof(a.matrix)) == 0;
}

// Check if the current graph is a duplicate
int is_duplicate(Graph g) {
    for (int i = 0; i < graph_count; i++) {
        if (are_graphs_equal(g, graphs[i])) {
            return 1;
        }
    }
    return 0;
}

// Process a valid permutation to generate the adjacency matrix and intervals
void process_permutation() {
    int starts[MAX_N], ends[MAX_N];
    for (int i = 0; i < 2 * n; i++) {
        int val = permutation[i];
        int interval = val / 2;
        if (val % 2 == 0) {
            starts[interval] = i;
        } else {
            ends[interval] = i;
        }
    }

    Graph g;
    memset(g.matrix, 0, sizeof(g.matrix));
    for (int i = 0; i < n; i++) {
        g.starts[i] = starts[i];
        g.ends[i] = ends[i];
    }

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if (starts[i] < ends[j] && starts[j] < ends[i]) {
                g.matrix[i * n + j] = 1;
                g.matrix[j * n + i] = 1;
            }
        }
    }

    if (!is_duplicate(g)) {
        graphs[graph_count++] = g;
        printf("Graph %d:\n", graph_count);
        printf("Intervals:\n");
        for (int i = 0; i < n; i++) {
            printf("Interval %d: [%d, %d]\n", i, g.starts[i], g.ends[i]);
        }
        printf("Adjacency Matrix:\n");
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                printf("%d ", g.matrix[i * n + j]);
            }
            printf("\n");
        }
        printf("\n");
    }
}

// Recursively generate valid permutations
void generate(int pos) {
    if (pos == 2 * n) {
        process_permutation();
        return;
    }
    for (int i = 0; i < n; i++) {
        if (!used_start[i] && !used_end[i]) {
            used_start[i] = 1;
            permutation[pos] = 2 * i;
            generate(pos + 1);
            used_start[i] = 0;
        }
        if (used_start[i] && !used_end[i]) {
            used_end[i] = 1;
            permutation[pos] = 2 * i + 1;
            generate(pos + 1);
            used_end[i] = 0;
        }
    }
}

int main() {
    printf("Enter n: ");
    scanf("%d", &n);
    if (n <= 0 || n > MAX_N) {
        printf("n must be between 1 and %d\n", MAX_N);
        return 1;
    }
    generate(0);
    printf("Total unique interval graphs: %d\n", graph_count);
    return 0;
}