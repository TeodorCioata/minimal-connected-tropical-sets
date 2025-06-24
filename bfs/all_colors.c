#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_N 30

typedef struct { int S, C, frontier, root; } State;

bool contains_any_minimal(int S, int *minimal_sets, int minimal_count) {
    for(int i = 0; i < minimal_count; i++)
        if((S & minimal_sets[i]) == minimal_sets[i])
            return true;
    return false;
}

int compute_minimal_tropical(int n, int c, int adj[MAX_N][MAX_N], int colors[MAX_N]) {
    int target_mask = (1 << c) - 1;
    int neighbor_mask[MAX_N];
    int color_mask[MAX_N];
    for(int v = 0; v < n; v++) {
        int nm = 0;
        for(int w = 0; w < n; w++)
            if(adj[v][w]) nm |= 1 << w;
        neighbor_mask[v] = nm;
        color_mask[v] = 1 << colors[v];
    }
    int *minimal_sets = NULL;
    int minimal_count = 0;
    State *cur = malloc(n * sizeof(State));
    int cur_cnt = 0;
    for(int v = 0; v < n; v++) {
        cur[cur_cnt++] = (State){
            .S = 1 << v,
            .C = 1 << colors[v],
            .frontier = neighbor_mask[v] & ~((1 << (v + 1)) - 1),
            .root = v
        };
    }
    for(int k = 1; k <= n && cur_cnt > 0; k++) {
        bool *seen = calloc(1, sizeof(bool) * (1 << n));
        State *nxt = NULL;
        int nxt_cnt = 0;
        for(int i = 0; i < cur_cnt; i++) {
            State s = cur[i];
            if(contains_any_minimal(s.S, minimal_sets, minimal_count)) continue;
            if(s.C == target_mask) {
                minimal_sets = realloc(minimal_sets, sizeof(int) * (minimal_count + 1));
                minimal_sets[minimal_count++] = s.S;
                continue;
            }
            for(int u = s.root + 1; u < n; u++) {
                if(!(s.frontier & (1 << u))) continue;
                State t = {
                    .S = s.S | (1 << u),
                    .C = s.C | color_mask[u],
                    .root = s.root,
                    .frontier = (s.frontier | neighbor_mask[u]) & ~(s.S | (1 << u)) & ~((1 << (s.root + 1)) - 1)
                };
                if(!seen[t.S]) {
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
    free(minimal_sets);
    return minimal_count;
}

int main() {
    int N, n;
    scanf("%d%d", &N, &n);
    int max[MAX_N] = {0};
    for(int g = 0; g < N; g++) {
        int adj[MAX_N][MAX_N];
        for(int i = 0; i < n; i++)
            for(int j = 0; j < n; j++)
                scanf("%d", &adj[i][j]);
        for(int k = 1; k <= n; k++) {
            int max_count = 0;
            int colors[MAX_N] = {0};
            while(true) {
                uint32_t used = 0;
                for(int i = 0; i < n; i++) used |= (1 << colors[i]);
                if(__builtin_popcount(used) == k) {
                    int count = compute_minimal_tropical(n, k, adj, colors);
                    if(count > max_count) max_count = count;
                }
                int i = n - 1;
                while(i >= 0 && colors[i] == k - 1) {
                    colors[i] = 0;
                    i--;
                }
                if(i < 0) break;
                colors[i]++;
            }
            printf("%d ", max_count);
            if (max_count > max[k]) max[k] = max_count;
        }
        printf("\n");
    }

    printf("Max values over all graphs:\n");
    for (int i = 1; i <= n; i++) {
        printf("Colors = %d => %d sets\n", i, max[i]);
    }
    return 0;
}