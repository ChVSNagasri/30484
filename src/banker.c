
#include <stdio.h>
#include <stdbool.h>

#include "banker.h"

#define PROCESS_COUNT 5
#define RESOURCE_COUNT 3

void banker_command(void)
{
    int allocation[PROCESS_COUNT][RESOURCE_COUNT] = {
        {0, 1, 0},
        {2, 0, 0},
        {3, 0, 2},
        {2, 1, 1},
        {0, 0, 2}
    };

    int max[PROCESS_COUNT][RESOURCE_COUNT] = {
        {7, 5, 3},
        {3, 2, 2},
        {9, 0, 2},
        {2, 2, 2},
        {4, 3, 3}
    };

    int available[RESOURCE_COUNT] = {3, 3, 2};
    int need[PROCESS_COUNT][RESOURCE_COUNT];
    int work[RESOURCE_COUNT];
    int safe_sequence[PROCESS_COUNT];
    bool finish[PROCESS_COUNT] = {false};
    int completed = 0;

    printf("\n===== BANKER'S ALGORITHM =====\n");

    for (int j = 0; j < RESOURCE_COUNT; j++) {
        work[j] = available[j];
    }

    printf("\nNeed Matrix:\n");
    printf("Process\tA\tB\tC\n");

    for (int i = 0; i < PROCESS_COUNT; i++) {
        printf("P%d\t", i);

        for (int j = 0; j < RESOURCE_COUNT; j++) {
            need[i][j] = max[i][j] - allocation[i][j];
            printf("%d\t", need[i][j]);
        }

        printf("\n");
    }

    while (completed < PROCESS_COUNT) {
        bool found = false;

        for (int i = 0; i < PROCESS_COUNT; i++) {
            if (finish[i]) {
                continue;
            }

            bool can_finish = true;

            for (int j = 0; j < RESOURCE_COUNT; j++) {
                if (need[i][j] > work[j]) {
                    can_finish = false;
                    break;
                }
            }

            if (can_finish) {
                for (int j = 0; j < RESOURCE_COUNT; j++) {
                    work[j] += allocation[i][j];
                }

                safe_sequence[completed++] = i;
                finish[i] = true;
                found = true;
            }
        }

        if (!found) {
            break;
        }
    }

    if (completed == PROCESS_COUNT) {
        printf("\nSystem is in a SAFE state.\n");
        printf("Safe Sequence: ");

        for (int i = 0; i < PROCESS_COUNT; i++) {
            printf("P%d", safe_sequence[i]);

            if (i < PROCESS_COUNT - 1) {
                printf(" -> ");
            }
        }

        printf("\n");
    } else {
        printf("\nSystem is in an UNSAFE state.\n");
        printf("No safe sequence exists.\n");
    }

    printf("==============================\n\n");
}
