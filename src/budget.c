
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "finance.h"


void loadBudgets(struct budget b[], int *m)
{
    FILE *f = fopen("budgets.dat", "rb");
    struct budget x[9];
    int c, i, j;

    *m = 0;

    if (f == NULL)
        return;

    if (fread(&c, sizeof(int), 1, f) != 1 || c < 0 || c > 9)
    {
        printf("Invalid budget file\n");
        fclose(f);
        return;
    }

    if (c > 0 && fread(x, sizeof x[0], c, f) != (size_t)c)
    {
        printf("Unable to load budgets\n");
        fclose(f);
        return;
    }

    for (i = 0; i < c; i++)
    {
        if (memchr(x[i].category, '\0', sizeof(x[i].category)) == NULL ||
            !isfinite(x[i].amount) || x[i].amount <= 0)
        {
            printf("Invalid budget data\n");
            fclose(f);
            return;
        }

        for (j = 0; j < i; j++)
        {
            if (strcmp(x[i].category, x[j].category) == 0)
            {
                printf("Duplicate budget category\n");
                fclose(f);
                return;
            }
        }
    }

    if (fgetc(f) != EOF)
    {
        printf("Unexpected data in budget file\n");
        fclose(f);
        return;
    }

    fclose(f);

    for (i = 0; i < c; i++)
        b[i] = x[i];

    *m = c;
}



int saveBudgets(struct budget b[], int m)
{
    FILE *f = fopen("budgets.dat", "wb");

    if (f == NULL)
    {
        printf("Unable to save budgets\n");
        return 0;
    }

    if (fwrite(&m, sizeof(int), 1, f) != 1 ||
        (m > 0 && fwrite(b, sizeof b[0], m, f) != (size_t)m))
    {
        printf("Error saving budgets\n");
        fclose(f);
        return 0;
    }

    if (fclose(f) != 0)
    {
        printf("Error closing budget file\n");
        return 0;
    }

    return 1;
}


void budgetMenu(struct transaction t[], int n)
{
    struct budget b[9] = {0}, old[9] = {0};
    char *c[] = {"Food", "Travel", "Shopping", "Bills", "Rent",
                 "Healthcare", "Education", "Entertainment", "Other Expense"};
    int m = 0, ch, i, j, found;
    double x, spent;

    loadBudgets(b, &m);

    while (1)
    {
        printf("\n--- Budget Management ---\n");
        printf("1. Create or Update Budget\n");
        printf("2. View Budgets\n");
        printf("3. Exit\n");
        printf("Enter choice: ");

        if (!readInt(&ch))
        {
            printf("Invalid input\n");
            continue;
        }

        if (ch == 3)
            return;

        if (ch == 1)
        {
            for (i = 0; i < 9; i++)
                printf("%d. %s\n", i + 1, c[i]);

            printf("Select category: ");

            if (!readInt(&ch) || ch < 1 || ch > 9)
            {
                printf("Invalid category\n");
                continue;
            }

            printf("Enter budget amount: ");

            if (!readDouble(&x) || x <= 0)
            {
                printf("Invalid amount\n");
                continue;
            }
            for (i = 0; i < m; i++)
                old[i] = b[i];

            int oldm = m;

            found = 0;

            for (i = 0; i < m; i++)
            {
                if (strcmp(b[i].category, c[ch - 1]) == 0)
                {
                    b[i].amount = x;
                    found = 1;
                    break;
                }
            }

            if (!found && m < 9)
            {
                strcpy(b[m].category, c[ch - 1]);
                b[m].amount = x;
                m++;
            }

            if (saveBudgets(b, m))
            {
                printf("Budget saved\n");
            }
            else
            {
                for (i = 0; i < oldm; i++)
                    b[i] = old[i];

                m = oldm;
                printf("Budget was not saved\n");
            }
        }
        else if (ch == 2)
        {
            if (m == 0)
            {
                printf("No budgets found\n");
                continue;
            }

            printf("\nCategory\tBudget\tSpent\tRemaining\n");

            for (i = 0; i < m; i++)
            {
                spent = 0;

                for (j = 0; j < n; j++)
                {
                    if (t[j].type == 2 &&
                        strcmp(t[j].category, b[i].category) == 0)
                        spent += t[j].amount;
                }

                printf("%s\t%.2f\t%.2f\t%.2f",
                       b[i].category, b[i].amount, spent,
                       b[i].amount - spent);

                if (spent > b[i].amount)
                    printf(" (Budget exceeded)");

                printf("\n");
            }
        }
        else
        {
            printf("Invalid choice\n");
        }
    }
}

