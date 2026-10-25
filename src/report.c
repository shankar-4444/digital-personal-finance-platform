
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "finance.h"

void report(struct transaction t[], int n, double s)
{
    int i;
    double a = 0, b = 0;

    for (i = 0; i < n; i++)
    {
        if (t[i].type == 1)
            a += t[i].amount;
        else if (t[i].type == 2)
            b += t[i].amount;
    }

    printf("\n----- Financial Report -----\n");
    printf("Total Income: %.2f\n", a);
    printf("Total Expenses: %.2f\n", b);
    printf("Account Balance: %.2f\n", a - b);
    printf("Savings: %.2f\n", s);
    printf("Balance After Savings: %.2f\n", a - b - s);
}

void setSavings(struct transaction t[], int n, double *s)
{
    int i;
    double a = 0, b = 0, x;

    for (i = 0; i < n; i++)
    {
        if (t[i].type == 1)
            a += t[i].amount;
        else if (t[i].type == 2)
            b += t[i].amount;
    }

    
    printf("Enter savings amount: ");

    if (!readDouble(&x) || x < 0 || x > a - b)
    {
        printf("Invalid savings amount\n");
        return;
    }


    if (!saveSavings(x))
        return;

    *s = x;

    printf("Savings updated successfully\n");
}


int saveSavings(double s)
{
    FILE *f;

    if (!isfinite(s) || s < 0)
        return 0;

    f = fopen("savings.dat", "wb");

    if (f == NULL)
    {
        printf("Unable to save savings\n");
        return 0;
    }

    if (fwrite(&s, sizeof(double), 1, f) != 1)
    {
        printf("Error saving savings\n");
        fclose(f);
        return 0;
    }

    if (fclose(f) != 0)
    {
        printf("Error closing savings file\n");
        return 0;
    }

    return 1;
}


void loadSavings(double *s)
{
    FILE *f = fopen("savings.dat", "rb");
    double x;

    *s = 0;

    if (f == NULL)
        return;

    if (fread(&x, sizeof(double), 1, f) != 1 ||
        !isfinite(x) || x < 0 || fgetc(f) != EOF)
    {
        printf("Invalid savings file\n");
        fclose(f);
        return;
    }

    fclose(f);
    *s = x;
}

void categoryReport(struct transaction t[], int n)
{
    char *c[] = {"Food", "Travel", "Shopping", "Bills", "Rent",
                 "Healthcare", "Education", "Entertainment", "Other Expense"};
    int i, j, found = 0;
    double x;

    printf("\n--- Category-wise Expense Report ---\n");
    printf("Category\tTotal Expenses\n");

    for (i = 0; i < 9; i++)
    {
        x = 0;

        for (j = 0; j < n; j++)
        {
            if (t[j].type == 2 &&
                strcmp(t[j].category, c[i]) == 0)
            {
                x += t[j].amount;
                found = 1;
            }
        }

        if (x > 0)
            printf("%s\t%.2f\n", c[i], x);
    }

    if (!found)
        printf("No expenses found\n");
}
