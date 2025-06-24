#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <signal.h>

#define MAX_N 30
#define MAX_C 20
#define MAX_CHECKED 100000

// Global state
int n, c = 3;
int adj[MAX_N][MAX_N];
int colors[MAX_N];
int neighbor_mask[MAX_N];
int color_mask[MAX_N];
int target_mask;
int *minimal_sets = NULL, minimal_count = 0;
volatile sig_atomic_t keep_running = 1;

void signal_handler(int sig) {
    keep_running = 0;
}

void record_minimal(int S) {
    minimal_sets = realloc(minimal_sets, sizeof(int)*(minimal_count+1));
    minimal_sets[minimal_count++] = S;
}

bool contains_any_minimal(int S) {
    for(int i=0; i<minimal_count; i++)
        if ((S & minimal_sets[i]) == minimal_sets[i])
            return true;
    return false;
}

typedef struct { int S, C, frontier, root; } State;

int compute_MCTS() {
    free(minimal_sets);
    minimal_sets = NULL;
    minimal_count = 0;

    target_mask = (1<<c)-1; // all colours
    for(int v=0; v<n; v++){
        int nm=0;
        for(int w=0; w<n; w++)
            if(adj[v][w]) nm |= 1<<w;
        neighbor_mask[v]=nm;
        color_mask[v]   = 1<<colors[v];
    }

    State *cur = malloc(n*sizeof(State));
    int cur_cnt = 0;
    for(int v=0; v<n; v++){
        cur[cur_cnt++] = (State){
            .S        = 1<<v,
            .C        = 1<<colors[v],
            .frontier = neighbor_mask[v] & ~((1<<(v+1))-1),
            .root     = v
        };
    }

    for (int k = 1; k <= n && cur_cnt > 0; k++) {
        bool *seen = calloc(1, sizeof(bool)*(1<<n));
        State *nxt = NULL;
        int    nxt_cnt = 0;
        for (int i = 0; i < cur_cnt; i++) {
            State s = cur[i];
            if (contains_any_minimal(s.S)) continue;
            if (s.C == target_mask) {
                record_minimal(s.S);
                continue;
            }
            for (int u = s.root + 1; u < n; u++) {
                if (!(s.frontier & (1<<u))) continue;
                State t = { 
                    .S        = s.S | (1<<u),
                    .C        = s.C | color_mask[u],
                    .root     = s.root,
                    .frontier = (s.frontier | neighbor_mask[u]) & ~(s.S | (1<<u)) & ~((1<<(s.root+1)) - 1)
                };
                if (!seen[t.S]) {
                    seen[t.S] = true;
                    nxt = realloc(nxt, sizeof(State)*(nxt_cnt+1));
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

bool is_valid_coloring(int *coloring) {
    bool has_color[3] = {false, false, false};
    for(int i=0; i<n; i++) {
        has_color[coloring[i]] = true;
    }
    return has_color[0] && has_color[1] && has_color[2];
}

int compare_int(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

bool next_coloring(int *color, int n, int k) {
    for(int i=n-1; i>=0; i--) {
        if(color[i] < k-1) {
            color[i]++;
            return true;
        }
        color[i] = 0;
    }
    return false;
}

int main() {
    signal(SIGINT, signal_handler);
    scanf("%d", &n);
    for (int i=0; i < n; i++) {
        for (int j=0; j < n; j++) {
            scanf("%d", &adj[i][j]);
        }
    }

    long long total_colorings = 1;
    for(int i=0; i<n; i++) total_colorings *= 3;

    int *mcts_counts = malloc(MAX_CHECKED * sizeof(int));
    int **used_colorings = malloc(MAX_CHECKED * sizeof(int*));
    for(int i=0; i<MAX_CHECKED; i++) used_colorings[i] = malloc(n * sizeof(int));
    int num_checked = 0;

    if(total_colorings <= MAX_CHECKED) {
        int *coloring = calloc(n, sizeof(int));
        while(keep_running) {
            if(is_valid_coloring(coloring)) {
                for(int i=0; i<n; i++) colors[i] = coloring[i];
                int mcts = compute_MCTS();
                mcts_counts[num_checked] = mcts;
                memcpy(used_colorings[num_checked], coloring, n*sizeof(int));
                num_checked++;
                if(num_checked % 100 == 0) {
                    printf("Checked %d colorings so far\n", num_checked);
                }
            }
            if(!next_coloring(coloring, n, c)) break;
        }
        free(coloring);
    } else {
        srand(time(NULL));
        while(num_checked < MAX_CHECKED && keep_running) {
            int new_coloring[MAX_N];
            int perm[3] = {0,1,2};
            for(int i=0; i<3; i++) {
                int j = rand() % (3 - i) + i;
                int temp = perm[i];
                perm[i] = perm[j];
                perm[j] = temp;
            }
            for(int i=0; i<3; i++) {
                new_coloring[i] = perm[i];
            }
            for(int i=3; i<n; i++) {
                new_coloring[i] = rand() % 3;
            }
            for(int i=0; i<n; i++) {
                int j = rand() % n;
                int temp = new_coloring[i];
                new_coloring[i] = new_coloring[j];
                new_coloring[j] = temp;
            }
            if(!is_valid_coloring(new_coloring)) continue;
            bool is_new = true;
            for(int j=0; j<num_checked; j++) {
                if(memcmp(new_coloring, used_colorings[j], n*sizeof(int)) == 0) {
                    is_new = false;
                    break;
                }
            }
            if(is_new) {
                memcpy(used_colorings[num_checked], new_coloring, n*sizeof(int));
                for(int i=0; i<n; i++) colors[i] = new_coloring[i];
                int mcts = compute_MCTS();
                mcts_counts[num_checked] = mcts;
                num_checked++;
                if(num_checked % 100 == 0) {
                    printf("Checked %d colorings so far\n", num_checked);
                }
            }
        }
    }

    if(num_checked > 0) {
        qsort(mcts_counts, num_checked, sizeof(int), compare_int);
        int min_mcts = mcts_counts[0];
        int max_mcts = mcts_counts[num_checked-1];
        double sum = 0;
        for(int i=0; i<num_checked; i++) sum += mcts_counts[i];
        double mean_mcts = sum / num_checked;
        int median_mcts;
        if(num_checked % 2 == 1) {
            median_mcts = mcts_counts[num_checked/2];
        } else {
            median_mcts = (mcts_counts[num_checked/2 - 1] + mcts_counts[num_checked/2]) / 2;
        }
        printf("Number of different colorings checked: %d\n", num_checked);
        printf("Min number of MCTS: %d\n", min_mcts);
        printf("Max number of MCTS: %d\n", max_mcts);
        printf("Mean number of MCTS: %.2f\n", mean_mcts);
        printf("Median number of MCTS: %d\n", median_mcts);
    } else {
        printf("No colorings checked.\n");
    }

    for(int i=0; i<MAX_CHECKED; i++) free(used_colorings[i]);
    free(used_colorings);
    free(mcts_counts);
    return 0;
}