#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_N 25  // Max number of vertices
#define MAX_C 15  // Max number of colors

int n, c; // number of vertices and number of colors
int adj[MAX_N][MAX_N];
int colors[MAX_N];
int minimal_count;

// Check if subset (given as bitmask) is connected
bool is_connected(int subset) {
    bool visited[MAX_N] = {0};
    int queue[MAX_N], front = 0, back = 0;

    // Find a starting vertex in the subset
    int start = -1;
    for (int i = 0; i < n; i++) {
        if (subset & (1 << i)) {
            start = i;
            break;
        }
    }
    if (start == -1) return false; // empty set

    visited[start] = true;
    queue[back++] = start;

    while (front < back) {
        int u = queue[front++];
        for (int v = 0; v < n; v++) {
            if ((subset & (1 << v)) && adj[u][v] && !visited[v]) {
                visited[v] = true;
                queue[back++] = v;
            }
        }
    }

    // Check if all vertices in the subset were visited
    for (int i = 0; i < n; i++) {
        if ((subset & (1 << i)) && !visited[i]) {
            return false;
        }
    }

    return true;
}

// Check if subset is tropical (contains all colors)
bool is_tropical(int subset) {
    bool found[MAX_C] = {0};
    int count = 0;

    for (int i = 0; i < n; i++) {
        if (subset & (1 << i)) {
            int col = colors[i];
            if (!found[col]) {
                found[col] = true;
                count++;
            }
        }
    }

    return count == c;
}

// Check if subset is minimal connected tropical
bool is_minimal(int subset) {
    // Remove a node and check if it stays connected and tropical
    for (int i = 0; i < n; i++) {
        if (subset & (1 << i)) {
            int smaller = subset & ~(1 << i);
            if (is_connected(smaller) && is_tropical(smaller)) {
                return false;
            }
        }
    }

    return true;
}

// Print a subset
void print_subset(int subset) {
    minimal_count++;
    printf("{ ");
    for (int i = 0; i < n; i++) {
        if (subset & (1 << i)) {
            printf("%d ", i);
        }
    }
    printf("}\n");
}

// Main function to find all minimal connected tropical sets
void find_minimal_connected_tropical_sets() {
    int total = 1 << n; //2^n subsets
    minimal_count = 0;
    for (int subset = 1; subset < total; subset++) {
        if (is_connected(subset) && is_tropical(subset) && is_minimal(subset)) {
            print_subset(subset);
        }
    }
}

int main() {
    scanf("%d%d", &n, &c);
    for (int i=0; i < n; i++) {
        for (int j=0; j < n; j++) {
            scanf("%d", &adj[i][j]);
        }
    }
    printf("Enter colours\n");
    for (int i=0; i < n; i++) {
        scanf("%d", &colors[i]);
    }

    find_minimal_connected_tropical_sets();

    printf("Number of minimal connected tropical sets = %d\n", minimal_count);

    return 0;
}