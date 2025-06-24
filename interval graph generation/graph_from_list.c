#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int size, n;

void process_permutation(int* intervals) {
    int starts[size], ends[size];
    for (int i = 0; i < size; i++) {
        starts[i] = -1;
        ends[i] = -1;
    }

    for (int i = 0; i < size * 2; i++) {
        if (starts[intervals[i]] == -1) {
            starts[intervals[i]] = i;
        } else {
            ends[intervals[i]] = i;
        }
    }

    int matrix[size][size];

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            matrix[i][j] = 0;
        }
    }

    for (int i = 0; i < size; i++) {
        for (int j = i + 1; j < size; j++) {
            if (fmax(starts[i], starts[j]) < fmin(ends[i], ends[j])) {
                matrix[i][j] = 1;
                matrix[j][i] = 1;
            }
        }
    }

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

int main() {
    scanf("%d%d", &size, &n);

    for (int i = 0; i < n; i++) {
        int intervals[size * 2];
        for (int j = 0; j < size * 2; j++) {
            scanf("%d", &intervals[j]);
            intervals[j]--;
        }
        process_permutation(intervals);
    }
}