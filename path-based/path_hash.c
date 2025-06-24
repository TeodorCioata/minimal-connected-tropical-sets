#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define HASH_SIZE 10007

typedef struct {
    int* nodes;    // Array of adjacent node indices
    int size;      // Number of adjacent nodes
    int capacity;  // Allocated capacity of nodes array
} AdjList;

typedef struct {
    int n;         // Number of nodes
    AdjList* adj;  // Array of adjacency lists
} Graph;

typedef struct HashEntry {
    char* set_str;
    struct HashEntry* next;
} HashEntry;

HashEntry* hash_table[HASH_SIZE] = {NULL};

unsigned long hash_function(char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % HASH_SIZE;
}

bool is_set_seen(char* set_str) {
    unsigned long h = hash_function(set_str);
    HashEntry* entry = hash_table[h];
    while (entry != NULL) {
        if (strcmp(entry->set_str, set_str) == 0) {
            return true;
        }
        entry = entry->next;
    }
    return false;
}

void add_set(char* set_str) {
    unsigned long h = hash_function(set_str);
    HashEntry* new_entry = (HashEntry*)malloc(sizeof(HashEntry));
    new_entry->set_str = (char*)malloc(strlen(set_str) + 1);
    strcpy(new_entry->set_str, set_str);
    new_entry->next = hash_table[h];
    hash_table[h] = new_entry;
}

// Initialize graph with n nodes
Graph* create_graph(int n) {
    Graph* g = (Graph*)malloc(sizeof(Graph));
    g->n = n;
    g->adj = (AdjList*)malloc(n * sizeof(AdjList));
    for (int i = 0; i < n; i++) {
        g->adj[i].size = 0;
        g->adj[i].capacity = 4; // Initial capacity
        g->adj[i].nodes = (int*)malloc(g->adj[i].capacity * sizeof(int));
    }
    return g;
}

// Add an edge to the graph
void add_edge(Graph* g, int u, int v) {
    AdjList* list = &g->adj[u];
    if (list->size >= list->capacity) {
        list->capacity *= 2;
        list->nodes = (int*)realloc(list->nodes, list->capacity * sizeof(int));
    }
    list->nodes[list->size++] = v;
}

// Check if there is an edge between u and v
bool has_edge(Graph* g, int u, int v) {
    AdjList* list = &g->adj[u];
    for (int i = 0; i < list->size; i++) {
        if (list->nodes[i] == v) return true;
    }
    return false;
}

// Free graph memory
void free_graph(Graph* g) {
    for (int i = 0; i < g->n; i++) {
        free(g->adj[i].nodes);
    }
    free(g->adj);
    free(g);
}

int compare_int(const void* a, const void* b) {
    return *(int*)a - *(int*)b;
}

int MCTS(int x, bool* A, int t, Graph* g, int* color, int path_size, int* path) {
    path[path_size - 1] = x;

    if (has_edge(g, x, t) && path_size >= 2) {
        int set_nodes[path_size + 1];
        for (int i = 0; i < path_size; i++) {
            set_nodes[i] = path[i];
        }
        set_nodes[path_size] = t;
        qsort(set_nodes, path_size + 1, sizeof(int), compare_int);
        char set_str[1000];
        int pos = 0;
        for (int i = 0; i < path_size + 1; i++) {
            pos += sprintf(set_str + pos, "%d,", set_nodes[i]);
        }
        set_str[pos - 1] = '\0';
        if (!is_set_seen(set_str)) {
            add_set(set_str);
            return 1;
        }
        return 0;
    }

    int C[g->n];
    int c_size = 0;
    for (int i = 0; i < g->adj[x].size; i++) {
        int y = g->adj[x].nodes[i];
        if (A[y]) {
            C[c_size++] = y;
        }
    }

    if (c_size == 0) return 0;

    int count = 0;
    bool* A_new = (bool*)malloc(g->n * sizeof(bool));
    for (int i = 0; i < c_size; i++) {
        int y = C[i];
        for (int j = 0; j < g->n; j++) A_new[j] = A[j];
        for (int j = 0; j < c_size; j++) A_new[C[j]] = false;
        count += MCTS(y, A_new, t, g, color, path_size + 1, path);
    }
    free(A_new);
    return count;
}

int main() {
    int n, m;
    scanf("%d %d", &n, &m);
    Graph* g = create_graph(n);
    for (int i = 0; i < m; i++) {
        int u, v;
        scanf("%d %d", &u, &v);
        add_edge(g, u, v);
        add_edge(g, v, u);
    }
    int* color = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        scanf("%d", &color[i]);
    }

    for (int i = 0; i < HASH_SIZE; i++) {
        hash_table[i] = NULL;
    }

    long long total = 0;
    bool* A = (bool*)malloc(n * sizeof(bool));
    int* path = (int*)malloc(n * sizeof(int));

    for (int s = 0; s < n; s++) {
        for (int t = s + 1; t < n; t++) {
            if (color[s] != color[t]) {
                int through_color = 6 - color[s] - color[t];
                for (int y = 0; y < n; y++) {
                    A[y] = (color[y] == through_color);
                }
                total += MCTS(s, A, t, g, color, 1, path);
            }
        }
    }

    printf("%lld\n", total);

    for (int i = 0; i < HASH_SIZE; i++) {
        HashEntry* entry = hash_table[i];
        while (entry != NULL) {
            HashEntry* next = entry->next;
            free(entry->set_str);
            free(entry);
            entry = next;
        }
    }

    free(path);
    free(A);
    free(color);
    free_graph(g);
    return 0;
}