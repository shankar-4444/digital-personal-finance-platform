#include <stdio.h>
#include <stdlib.h>
#include "finance.h"

int main()
{
    struct transaction *t = NULL;
    size_t cap = 0;
    int n = 0, c;
    double s = 0;

    load(&t, &n, &cap);
    loadSavings(&s);

    printf("Loaded %d transactions\n", n);

    while (1)
    {
        printf("\n--- Digital Personal Finance Platform ---\n");
        printf("1. Add Transaction\n");
        printf("2. View Transactions\n");
        printf("3. Financial Report\n");
        printf("4. Update Transaction\n");
        printf("5. Delete Transaction\n");
        printf("6. Search Transaction\n");
        printf("7. Filter by Category\n");
        printf("8. Set Savings\n");
        printf("9. Budget Management\n");
        printf("10. Category-wise Expense Report\n");
        printf("11. Exit\n");
        printf("Enter choice: ");

        if (!readInt(&c))
        {
            printf("Invalid input\n");
            continue;
        }

        switch (c)
        {
            case 1: add(&t, &n, &cap); break;
            case 2: view(t, n); break;
            case 3: report(t, n, s); break;
            case 4: update(t, n); break;
            case 5: del(t, &n); break;
            case 6: search(t, n); break;
            case 7: filter(t, n); break;
            case 8: setSavings(t, n, &s); break;
            case 9: budgetMenu(t, n); break;
            case 10: categoryReport(t, n); break;
            case 11:
                free(t);
                return 0;
            default: printf("Invalid choice\n");
        }
    }
}
