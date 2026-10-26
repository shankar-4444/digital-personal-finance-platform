#ifndef FINANCE_H
#define FINANCE_H

#include <stddef.h>
#include <time.h>

#define MAX_TRANSACTIONS 100000

struct transaction
{
    int id;
    int type;
    double amount;
    char category[30];
    char timestamp[20];
};

struct budget
{
    char category[30];
    double amount;
};

void add(struct transaction **t, int *n, size_t *cap);
void view(struct transaction t[], int n);
void report(struct transaction t[], int n, double s);
int save(struct transaction t[], int n);
void load(struct transaction **t, int *n, size_t *cap);
void update(struct transaction t[], int n);
void del(struct transaction t[], int *n);
void search(struct transaction t[], int n);
void filter(struct transaction t[], int n);
void setSavings(struct transaction t[], int n, double *s);
void loadSavings(double *s);
int saveSavings(double s);
void budgetMenu(struct transaction t[], int n);
void loadBudgets(struct budget b[], int *m);
int saveBudgets(struct budget b[], int m);
void categoryReport(struct transaction t[], int n);
void logActivity(const char *action, int id);
int readInt(int *x);
int readDouble(double *x);

#endif
