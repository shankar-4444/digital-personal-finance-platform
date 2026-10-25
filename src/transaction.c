#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <math.h>
#include "finance.h"

static int reserveTransactions(struct transaction **t, size_t *cap, size_t need)
{
    size_t nc;
    struct transaction *p;

    if (t == NULL || cap == NULL || need > MAX_TRANSACTIONS)
        return 0;

    if (need <= *cap)
        return 1;

    nc = (*cap == 0) ? 10 : *cap;

    while (nc < need)
    {
        if (nc > SIZE_MAX / 2)
        {
            nc = need;
            break;
        }
        nc *= 2;
    }

    if (nc > MAX_TRANSACTIONS)
        nc = MAX_TRANSACTIONS;

    if (nc < need || nc > SIZE_MAX / sizeof(struct transaction))
        return 0;

    p = realloc(*t, nc * sizeof(struct transaction));
    if (p == NULL)
        return 0;

    *t = p;
    *cap = nc;
    return 1;
}

int readInt(int *x)
{
    char s[128], *p;
    long v;

    if (fgets(s, sizeof(s), stdin) == NULL)
        return 0;

    if (strchr(s, '\n') == NULL)
    {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        return 0;
    }

    errno = 0;
    v = strtol(s, &p, 10);

    while (*p == ' ' || *p == '\t' || *p == '\r')
        p++;

    if (p == s || *p != '\n' || errno == ERANGE ||
        v < INT_MIN || v > INT_MAX)
        return 0;

    *x = (int)v;
    return 1;
}

int readDouble(double *x)
{
    char s[128], *p;
    double v;

    if (fgets(s, sizeof(s), stdin) == NULL)
        return 0;

    if (strchr(s, '\n') == NULL)
    {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        return 0;
    }

    errno = 0;
    v = strtod(s, &p);

    while (*p == ' ' || *p == '\t' || *p == '\r')
        p++;

    if (p == s || *p != '\n' || errno == ERANGE || !isfinite(v))
        return 0;

    *x = v;
    return 1;
}

int cmp(char a[], char b[])
{
    int i = 0;

    while (a[i] && b[i])
    {
        if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i]))
            return 0;
        i++;
    }

    return a[i] == b[i];
}

int cat(int type, char c[])
{
    char *a[] = {"Salary", "Business", "Freelance", "Investment", "Other Income"};
    char *b[] = {"Food", "Travel", "Shopping", "Bills", "Rent", "Healthcare", "Education", "Entertainment", "Other Expense"};
    int i, n;

    if (type == 1)
    {
        n = 5;
        for (i = 0; i < n; i++)
            printf("%d. %s\n", i + 1, a[i]);
    }
    else
    {
        n = 9;
        for (i = 0; i < n; i++)
            printf("%d. %s\n", i + 1, b[i]);
    }

    printf("Choice: ");

    if (readInt(&i) != 1 || i < 1 || i > n)
    {
        printf("Invalid category\n");
        return 0;
    }

    if (type == 1)
        snprintf(c, 30, "%s", a[i - 1]);
    else
        snprintf(c, 30, "%s", b[i - 1]);

    return 1;
}

void add(struct transaction **t, int *n, size_t *cap)
{
    struct transaction x = {0};
    int i, m = 0;

    if (t == NULL || n == NULL || cap == NULL || *n < 0 ||
        *n >= MAX_TRANSACTIONS)
    {
        printf("Transaction limit reached or invalid storage\n");
        return;
    }

    for (i = 0; i < *n; i++)
        if ((*t)[i].id > m)
            m = (*t)[i].id;

    if (m == INT_MAX)
    {
        printf("Transaction ID limit reached\n");
        return;
    }
    x.id = m + 1;

    printf("1. Income\n2. Expense\nChoice: ");
    if (readInt(&x.type) != 1 || (x.type != 1 && x.type != 2))
    {
        printf("Invalid type\n");
        return;
    }

    printf("Enter amount: ");
    if (readDouble(&x.amount) != 1 || x.amount <= 0)
    {
        printf("Invalid amount\n");
        return;
    }

    printf("Select category:\n");
    if (!cat(x.type, x.category))
        return;

    if (!reserveTransactions(t, cap, (size_t)*n + 1))
    {
        printf("Unable to allocate memory for transaction\n");
        return;
    }

    memcpy(&(*t)[*n], &x, sizeof(struct transaction));
    (*n)++;

    if (!save(*t, *n))
    {
        (*n)--;
        return;
    }

    printf("Transaction added successfully. ID: %d\n", x.id);
}

void view(struct transaction t[], int n)
{
    int i;

    if (n == 0)
    {
        printf("No transactions found\n");
        return;
    }

    printf("\nID\tType\tAmount\tCategory\n");
    for (i = 0; i < n; i++)
        printf("%d\t%s\t%.2f\t%s\n", t[i].id,
               t[i].type == 1 ? "Income" : "Expense",
               t[i].amount, t[i].category);
}

void update(struct transaction t[], int n)
{
    int id, i, type;
    double amount;
    char c[30];

    printf("Enter transaction ID to update: ");
    if (readInt(&id) != 1)
    {
        printf("Invalid ID\n");
        return;
    }

    for (i = 0; i < n && t[i].id != id; i++);
    if (i == n)
    {
        printf("Transaction not found\n");
        return;
    }

    printf("1. Income\n2. Expense\nChoice: ");
    if (readInt(&type) != 1 || (type != 1 && type != 2))
    {
        printf("Invalid type\n");
        return;
    }

    printf("Enter new amount: ");
    if (readDouble(&amount) != 1 || amount <= 0)
    {
        printf("Invalid amount\n");
        return;
    }

    printf("Select category:\n");
    if (!cat(type, c))
        return;

    struct transaction old = t[i];
    t[i].type = type;
    t[i].amount = amount;
    snprintf(t[i].category, sizeof(t[i].category), "%s", c);

    if (!save(t, n))
    {
        t[i] = old;
        return;
    }

    printf("Transaction updated successfully\n");
}

void del(struct transaction t[], int *n)
{
    int id, i, j;
    struct transaction *old;

    printf("Enter transaction ID to delete: ");
    if (readInt(&id) != 1)
    {
        printf("Invalid ID\n");
        return;
    }

    for (i = 0; i < *n && t[i].id != id; i++);
    if (i == *n)
    {
        printf("Transaction not found\n");
        return;
    }

    if ((size_t)*n > SIZE_MAX / sizeof(struct transaction))
    {
        printf("Unable to back up transactions\n");
        return;
    }

    old = malloc((size_t)*n * sizeof(struct transaction));
    if (old == NULL)
    {
        printf("Unable to back up transactions\n");
        return;
    }

    for (j = 0; j < *n; j++)
        old[j] = t[j];

    for (j = i; j < *n - 1; j++)
        t[j] = t[j + 1];

    (*n)--;

    if (!save(t, *n))
    {
        for (j = 0; j < *n + 1; j++)
            t[j] = old[j];
        (*n)++;
        free(old);
        return;
    }

    free(old);
    printf("Transaction deleted successfully\n");
}

void search(struct transaction t[], int n)
{
    int id, i;

    printf("Enter transaction ID: ");
    if (readInt(&id) != 1)
    {
        printf("Invalid ID\n");
        return;
    }

    for (i = 0; i < n; i++)
        if (t[i].id == id)
        {
            printf("Transaction Found\n");
            printf("ID: %d\nType: %s\nAmount: %.2f\nCategory: %s\n",
                   t[i].id, t[i].type == 1 ? "Income" : "Expense",
                   t[i].amount, t[i].category);
            return;
        }

    printf("Transaction not found\n");
}

void filter(struct transaction t[], int n)
{
    char c[30];
    int i, found = 0, type;

    printf("1. Income\n2. Expense\nChoice: ");
    if (readInt(&type) != 1 || (type != 1 && type != 2))
    {
        printf("Invalid type\n");
        return;
    }

    printf("Select category:\n");
    if (!cat(type, c))
        return;

    for (i = 0; i < n; i++)
        if (cmp(t[i].category, c))
        {
            if (!found)
                printf("\nID\tType\tAmount\tCategory\n");
            printf("%d\t%s\t%.2f\t%s\n", t[i].id,
                   t[i].type == 1 ? "Income" : "Expense",
                   t[i].amount, t[i].category);
            found = 1;
        }

    if (!found)
        printf("No transactions found in this category\n");
}

int save(struct transaction t[], int n)
{
    FILE *f;

    if (n < 0 || n > MAX_TRANSACTIONS || (n > 0 && t == NULL))
    {
        printf("Invalid transaction data\n");
        return 0;
    }

    f = fopen("transactions.dat", "wb");
    if (f == NULL)
    {
        printf("Unable to save transactions\n");
        return 0;
    }

    if (fwrite(&n, sizeof(int), 1, f) != 1 ||
        (n > 0 && fwrite(t, sizeof(struct transaction), (size_t)n, f) != (size_t)n))
    {
        printf("Error saving transactions\n");
        fclose(f);
        return 0;
    }

    if (fclose(f) != 0)
    {
        printf("Error closing transaction file\n");
        return 0;
    }

    return 1;
}

void load(struct transaction **t, int *n, size_t *cap)
{
    FILE *f;
    struct transaction *x = NULL;
    int c, i, j, k;
    int ok = 1;

    if (t == NULL || n == NULL || cap == NULL)
        return;

    *n = 0;
    f = fopen("transactions.dat", "rb");
    if (f == NULL)
        return;

    if (fread(&c, sizeof(int), 1, f) != 1 || c < 0 || c > MAX_TRANSACTIONS)
    {
        printf("Invalid transaction file or record count exceeds limit\n");
        fclose(f);
        return;
    }

    if (c > 0)
    {
        if ((size_t)c > SIZE_MAX / sizeof(struct transaction))
        {
            printf("Transaction file is too large\n");
            fclose(f);
            return;
        }

        x = malloc((size_t)c * sizeof(struct transaction));
        if (x == NULL)
        {
            printf("Unable to allocate memory while loading transactions\n");
            fclose(f);
            return;
        }

        if (fread(x, sizeof(struct transaction), (size_t)c, f) != (size_t)c)
        {
            printf("Unable to load transactions\n");
            free(x);
            fclose(f);
            return;
        }
    }

    for (i = 0; i < c && ok; i++)
    {
        if (x[i].id <= 0 || (x[i].type != 1 && x[i].type != 2) ||
            !isfinite(x[i].amount) || x[i].amount <= 0 ||
            memchr(x[i].category, '\0', sizeof(x[i].category)) == NULL)
        {
            printf("Invalid transaction data\n");
            ok = 0;
            break;
        }

        for (j = 0; j < i; j++)
            if (x[i].id == x[j].id)
            {
                printf("Duplicate transaction ID\n");
                ok = 0;
                break;
            }

        for (k = 0; k < (int)sizeof(x[i].category) &&
                    x[i].category[k] != '\0'; k++)
            if (!isprint((unsigned char)x[i].category[k]))
            {
                printf("Invalid transaction category data\n");
                ok = 0;
                break;
            }
    }

    if (ok && fgetc(f) != EOF)
    {
        printf("Unexpected data in transaction file\n");
        ok = 0;
    }

    if (fclose(f) != 0)
        ok = 0;

    if (!ok)
    {
        free(x);
        return;
    }

    if (!reserveTransactions(t, cap, (size_t)c))
    {
        printf("Unable to allocate memory for loaded transactions\n");
        free(x);
        return;
    }

    if (c > 0)
        memcpy(*t, x, (size_t)c * sizeof(struct transaction));

    *n = c;
    free(x);
}
