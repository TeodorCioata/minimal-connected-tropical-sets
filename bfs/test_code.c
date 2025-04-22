#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_N 30
#define MAX_C 20

// --- global data ----------------
int n, c;
int adj[MAX_N][MAX_N];
int colors[MAX_N];

int neighbor_mask[MAX_N];
int color_mask[MAX_N];
int target_mask;

// dynamic storage for found minimal sets:
int *minimal_sets = NULL;
int  minimal_count = 0;

// record a new minimal set
void record_minimal(int S) {
    minimal_sets = realloc(minimal_sets, sizeof(int)*(minimal_count+1));
    minimal_sets[minimal_count++] = S;
}
// test if M ⊆ S
bool is_subset(int M, int S) {
    return (S & M) == M;
}

// print a bitmask as a vertex set
void print_set(int S) {
    printf("{ ");
    for (int i = 0; i < n; i++)
        if (S & (1<<i)) printf("%d ", i);
    puts("}");
}

// one state in our BFS‐by‐size
typedef struct {
    int set_mask;      // which vertices
    int col_mask;      // which colors
    int frontier;      // which neighbors we may add
    int last;          // last added vertex (to enforce order)
} State;

int main(){
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

    // --- Precompute masks ---
    target_mask = (1<<c) - 1;
    for(int v=0; v<n; v++){
        int nm = 0;
        for(int w=0; w<n; w++)
            if(adj[v][w]) nm |= (1<<w);
        neighbor_mask[v] = nm;
        color_mask[v]    = (1<<colors[v]);
    }

    // --- Initialize level k=1 (single vertices) ---
    State *cur = malloc(n * sizeof(State));
    int  cur_cnt = 0;
    for(int v=0; v<n; v++){
        cur[cur_cnt++] = (State){
            .set_mask = 1<<v,
            .col_mask = 1<<colors[v],
            .frontier = neighbor_mask[v] & ~((1<<(v+1)) - 1),
            .last     = v
        };
    }

    // --- BFS by size k = 1..n ---
    for(int k=1; k<=n && cur_cnt>0; k++){
        State *next = NULL;
        int     nxt_cnt = 0;

        for(int i=0; i<cur_cnt; i++){
            State s = cur[i];

            // (a) prune if it contains any existing minimal set
            bool skip = false;
            for(int m=0; m<minimal_count; m++){
                if(is_subset(minimal_sets[m], s.set_mask)){
                    skip = true; 
                    break;
                }
            }
            if(skip) continue;

            // (b) if tropical, it must be minimal at this size—
            //     record & do NOT generate its supersets
            if(s.col_mask == target_mask){
                print_set(s.set_mask);
                record_minimal(s.set_mask);
                continue;
            }

            // (c) otherwise, grow by one vertex for size k+1
            for(int u = 0; u < n; u++){
                if(!(s.frontier & (1<<u))) continue;
                int S2   = s.set_mask | (1<<u);
                int C2   = s.col_mask | color_mask[u];
                int last = u;
                int F2   = (s.frontier | neighbor_mask[u])
                           & ~S2
                           & ~((1<<(u+1)) - 1);

                next = realloc(next, sizeof(State)*(nxt_cnt+1));
                next[nxt_cnt++] = (State){ S2, C2, F2, last };
            }
        }

        free(cur);
        cur     = next;
        cur_cnt = nxt_cnt;
    }

    printf("Number of minimal connected tropical sets = %d\n", minimal_count);

    free(cur);
    free(minimal_sets);
    return 0;
}
