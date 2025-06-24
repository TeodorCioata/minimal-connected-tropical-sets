#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <signal.h>
#include <string.h>

#define MAX_N 30
#define MAX_C 3
#define MAX_START 196
#define MAX_LENGTH 14

// Global variables
int n, c = 3;
int adj[MAX_N][MAX_N];
int colors[MAX_N];
int neighbor_mask[MAX_N];
int color_mask[MAX_N];
int target_mask;
int *minimal_sets = NULL, minimal_count = 0;
volatile sig_atomic_t keep_running = 1;

// Signal handler for interruption
void signal_handler(int sig) {
    keep_running = 0;
}

// Record a minimal set
void record_minimal(int S) {
    minimal_sets = realloc(minimal_sets, sizeof(int) * (minimal_count + 1));
    minimal_sets[minimal_count++] = S;
}

// Check if S contains any minimal set
bool contains_any_minimal(int S) {
    for (int i = 0; i < minimal_count; i++)
        if ((S & minimal_sets[i]) == minimal_sets[i])
            return true;
    return false;
}

// State for BFS in MCTS computation
typedef struct { int S, C, frontier, root; } State;

// Compute the number of minimal connected tropical sets
int compute_MCTS() {
    free(minimal_sets);
    minimal_sets = NULL;
    minimal_count = 0;

    target_mask = (1 << c) - 1; // Mask for all colors, e.g., 7 for c=3
    for (int v = 0; v < n; v++) {
        int nm = 0;
        for (int w = 0; w < n; w++)
            if (adj[v][w]) nm |= 1 << w;
        neighbor_mask[v] = nm;
        color_mask[v] = 1 << colors[v];
    }

    State *cur = malloc(n * sizeof(State));
    int cur_cnt = 0;
    for (int v = 0; v < n; v++) {
        cur[cur_cnt++] = (State){
            .S = 1 << v,
            .C = 1 << colors[v],
            .frontier = neighbor_mask[v] & ~((1 << (v + 1)) - 1),
            .root = v
        };
    }

    for (int k = 1; k <= n && cur_cnt > 0; k++) {
        bool *seen = calloc(1, sizeof(bool) * (1 << n));
        State *nxt = NULL;
        int nxt_cnt = 0;
        for (int i = 0; i < cur_cnt; i++) {
            State s = cur[i];
            if (contains_any_minimal(s.S)) continue;
            if (s.C == target_mask) {
                record_minimal(s.S);
                continue;
            }
            for (int u = s.root + 1; u < n; u++) {
                if (!(s.frontier & (1 << u))) continue;
                State t = {
                    .S = s.S | (1 << u),
                    .C = s.C | color_mask[u],
                    .root = s.root,
                    .frontier = (s.frontier | neighbor_mask[u]) & ~(s.S | (1 << u)) & ~((1 << (s.root + 1)) - 1)
                };
                if (!seen[t.S]) {
                    seen[t.S] = true;
                    nxt = realloc(nxt, sizeof(State) * (nxt_cnt + 1));
                    nxt[nxt_cnt++] = t;
                }
            }
        }
        free(cur);
        free(seen);
        cur = nxt;
        cur_cnt = nxt_cnt;
    }
    free(cur);
    return minimal_count;
}

// Check if the graph is connected using BFS
bool is_connected(int adj[MAX_N][MAX_N], int n) {
    bool visited[MAX_N] = {false};
    int queue[MAX_N];
    int front = 0, rear = 0;
    queue[rear++] = 0;
    visited[0] = true;
    while (front < rear) {
        int v = queue[front++];
        for (int w = 0; w < n; w++) {
            if (adj[v][w] && !visited[w]) {
                queue[rear++] = w;
                visited[w] = true;
            }
        }
    }
    for (int i = 0; i < n; i++) {
        if (!visited[i]) return false;
    }
    return true;
}

// Comparison function for qsort
int compare_int(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

int main() {
    // Set up signal handler
    signal(SIGINT, signal_handler);

    // Read number of nodes
    scanf("%d", &n);
    if (n < 1 || n > MAX_N) {
        printf("Invalid n. Must be between 1 and %d.\n", MAX_N);
        return 1;
    }

    // Initialize random seed
    srand(time(NULL));

    // Initialize array to store MCTS counts
    int capacity = 1000;
    int *mcts_counts = malloc(capacity * sizeof(int));
    int count = 0;

    // Main loop
    while (keep_running) {
        // Generate random intervals
        int start[MAX_N], end[MAX_N];
        for (int i = 0; i < n; i++) {
            start[i] = rand() % MAX_START;
            int length = 1 + rand() % MAX_LENGTH;
            end[i] = start[i] + length;
        }

        // Compute adjacency matrix based on interval overlaps
        memset(adj, 0, sizeof(adj));
        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                if (start[i] < end[j] && start[j] < end[i]) {
                    adj[i][j] = adj[j][i] = 1;
                }
            }
        }

        // Check if the graph is connected
        if (!is_connected(adj, n)) continue;

        // Generate random coloring with 3 colors
        if (n >= c) {
            // Ensure all 3 colors are used
            int perm[c];
            for (int i = 0; i < c; i++) perm[i] = i;
            // Shuffle the first 3 colors
            for (int i = 0; i < c; i++) {
                int j = i + rand() % (c - i);
                int temp = perm[i];
                perm[i] = perm[j];
                perm[j] = temp;
            }
            // Assign colors to first 3 nodes
            for (int i = 0; i < c; i++) colors[i] = perm[i];
            // Assign random colors to remaining nodes
            for (int i = c; i < n; i++) colors[i] = rand() % c;
            // Shuffle all colors
            for (int i = 0; i < n; i++) {
                int j = rand() % n;
                int temp = colors[i];
                colors[i] = colors[j];
                colors[j] = temp;
            }

            // Compute MCTS count
            int mcts = compute_MCTS();

            // Store the count
            if (count >= capacity) {
                capacity *= 2;
                mcts_counts = realloc(mcts_counts, capacity * sizeof(int));
            }
            mcts_counts[count] = mcts;
            count++;

            // Periodic update
            if (count % 100 == 0) {
                printf("Checked %d graphs so far\n", count);
            }
        }
        // If n < 3, skip since all 3 colors can't be used
    }

    // Compute and display statistics
    if (count > 0) {
        qsort(mcts_counts, count, sizeof(int), compare_int);
        int min_mcts = mcts_counts[0];
        int max_mcts = mcts_counts[count - 1];
        double sum = 0;
        for (int i = 0; i < count; i++) sum += mcts_counts[i];
        double mean_mcts = sum / count;
        int median_mcts;
        if (count % 2 == 1) {
            median_mcts = mcts_counts[count / 2];
        } else {
            median_mcts = (mcts_counts[count / 2 - 1] + mcts_counts[count / 2]) / 2;
        }

        printf("Total number of graphs checked: %d\n", count);
        printf("Min number of MCTS: %d\n", min_mcts);
        printf("Max number of MCTS: %d\n", max_mcts);
        printf("Mean number of MCTS: %.2f\n", mean_mcts);
        printf("Median number of MCTS: %d\n", median_mcts);
    } else {
        printf("No graphs checked.\n");
    }

    // Clean up
    free(mcts_counts);
    if (minimal_sets) free(minimal_sets);
    return 0;
}