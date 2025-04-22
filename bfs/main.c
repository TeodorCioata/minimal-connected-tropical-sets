#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_N 30
#define MAX_C 20

// global state:
int n, c;
int adj[MAX_N][MAX_N];
int colors[MAX_N];
int neighbor_mask[MAX_N];
int color_mask[MAX_N];
int target_mask;

// store found minimal sets
int *minimal_sets = NULL, minimal_count = 0;

void record_minimal(int S) {
    minimal_sets = realloc(minimal_sets, sizeof(int)*(minimal_count+1));
    minimal_sets[minimal_count++] = S;
}

bool contains_any_minimal(int S) {
    for(int i=0;i<minimal_count;i++)
        if ((S & minimal_sets[i]) == minimal_sets[i])
            return true;
    return false;
}

void print_set(int S) {
    printf("{ ");
    for(int i=0;i<n;i++)
      if (S&(1<<i)) printf("%d ", i);
    puts("}");
}

typedef struct { int S, C, frontier, root; } State;

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

    // 1) precompute masks
    target_mask = (1<<c)-1; // all colours
    for(int v=0; v<n; v++){
        int nm=0;
        for(int w=0; w<n; w++)
            if(adj[v][w]) nm |= 1<<w;
        neighbor_mask[v]=nm;
        color_mask[v]   = 1<<colors[v];
    }

    // 2) initialize size‐1 states
    State *cur = malloc(n*sizeof(State)), *nxt=NULL;
    int cur_cnt = 0, nxt_cnt;
    for(int v=0; v<n; v++){
        cur[cur_cnt++] = (State){
        .S        = 1<<v,
        .C        = 1<<colors[v],
        .frontier = neighbor_mask[v] & ~((1<<(v+1))-1), // only take neighbors greater than root
        .root     = v
        };
    }

    // 3) BFS‐by‐size k=1..n
    for (int k = 1; k <= n && cur_cnt > 0; k++) {
        // --- allocate per‐level “seen” container ---
        bool *seen = calloc(1, sizeof(bool)*(1<<n));
        State *nxt = NULL; // next states to be checked
        int    nxt_cnt = 0;
    
        for (int i = 0; i < cur_cnt; i++) {
            State s = cur[i];
            // prune supersets of known minimals
            if (contains_any_minimal(s.S)) continue;
            // if tropical => record & skip
            if (s.C == target_mask) {
                print_set(s.S);
                record_minimal(s.S);
                continue;
            }
            // otherwise, grow by adding a frontier node
            for (int u = s.root + 1; u < n; u++) {
                if (!(s.frontier & (1<<u))) continue;
                State t = { 
                  .S        = s.S | (1<<u),
                  .C        = s.C | color_mask[u],
                  .root     = s.root,
                  .frontier = (s.frontier | neighbor_mask[u])
                               & ~(s.S | (1<<u)) //remove node from frontier
                               & ~((1<<(s.root+1)) - 1)
                };
                // **dedupe by mask**
                if (!seen[t.S]) {
                    seen[t.S] = true;
                    nxt = realloc(nxt, sizeof(State)*(nxt_cnt+1));
                    nxt[nxt_cnt++] = t;
                }
            }
        }
    
        free(cur);
        free(seen);
        cur     = nxt; // current checking states becomes next checking steates
        cur_cnt = nxt_cnt;
    }

    printf("Number of minimal connected tropical sets = %d\n", minimal_count);

    free(cur);
    free(minimal_sets);
    return 0;
}
