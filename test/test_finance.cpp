#include <gtest/gtest.h>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <string>
#include <unistd.h>

extern "C" {
#include "finance.h"
}

class FinanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::remove("transactions.dat");
        std::remove("savings.dat");
        std::remove("budgets.dat");
    }

    void TearDown() override {
        std::remove("transactions.dat");
        std::remove("savings.dat");
        std::remove("budgets.dat");
    }
};

TEST_F(FinanceTest, SaveAndLoadTransaction) {
    struct transaction in[1] = {};
    in[0].id = 1;
    in[0].type = 1;
    in[0].amount = 1000.0;
    snprintf(in[0].category, sizeof(in[0].category), "%s", "Salary");
    ASSERT_EQ(save(in, 1), 1);

    struct transaction *out = nullptr;
    size_t cap = 0;
    int n = 0;
    load(&out, &n, &cap);

    ASSERT_EQ(n, 1);
    ASSERT_NE(out, nullptr);
    EXPECT_EQ(out[0].id, 1);
    EXPECT_EQ(out[0].type, 1);
    EXPECT_DOUBLE_EQ(out[0].amount, 1000.0);
    EXPECT_STREQ(out[0].category, "Salary");
    EXPECT_GE(cap, static_cast<size_t>(1));
    std::free(out);
}

TEST_F(FinanceTest, LoadMissingTransactionsStartsEmpty) {
    struct transaction *out = nullptr;
    size_t cap = 0;
    int n = 5;
    load(&out, &n, &cap);
    EXPECT_EQ(n, 0);
    std::free(out);
}

TEST_F(FinanceTest, SaveAndLoadSavings) {
    ASSERT_EQ(saveSavings(100.0), 1);
    double s = 0.0;
    loadSavings(&s);
    EXPECT_DOUBLE_EQ(s, 100.0);
}

TEST_F(FinanceTest, RejectNegativeSavings) {
    EXPECT_EQ(saveSavings(-10.0), 0);
}

TEST_F(FinanceTest, RejectInfiniteSavings) {
    EXPECT_EQ(saveSavings(std::numeric_limits<double>::infinity()), 0);
}

TEST_F(FinanceTest, SaveAndLoadBudget) {
    struct budget in[1] = {};
    snprintf(in[0].category, sizeof(in[0].category), "%s", "Food");
    in[0].amount = 500.0;
    ASSERT_EQ(saveBudgets(in, 1), 1);

    struct budget out[9] = {};
    int n = 0;
    loadBudgets(out, &n);

    ASSERT_EQ(n, 1);
    EXPECT_STREQ(out[0].category, "Food");
    EXPECT_DOUBLE_EQ(out[0].amount, 500.0);
}

TEST_F(FinanceTest, AddValidIncomeTransaction) {
    struct transaction *t = nullptr;
    int n = 0;
    size_t cap = 0;

    FILE *f = tmpfile();
    ASSERT_NE(f, nullptr);
    const char input[] = "1\n1000\n1\n";
    fwrite(input, 1, sizeof(input) - 1, f);
    rewind(f);

    int old = dup(fileno(stdin));
    ASSERT_GE(old, 0);
    ASSERT_EQ(dup2(fileno(f), fileno(stdin)), fileno(stdin));
    clearerr(stdin);
    fclose(f);

    testing::internal::CaptureStdout();
    add(&t, &n, &cap);
    std::string out = testing::internal::GetCapturedStdout();

    dup2(old, fileno(stdin));
    close(old);
    clearerr(stdin);

    EXPECT_EQ(n, 1);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t[0].id, 1);
    EXPECT_EQ(t[0].type, 1);
    EXPECT_DOUBLE_EQ(t[0].amount, 1000.0);
    EXPECT_STREQ(t[0].category, "Salary");
    EXPECT_NE(out.find("Transaction added successfully"), std::string::npos);

    free(t);
}

TEST_F(FinanceTest, AddValidExpenseTransaction) {
    struct transaction *t = nullptr;
    int n = 0;
    size_t cap = 0;

    FILE *f = tmpfile();
    ASSERT_NE(f, nullptr);
    const char input[] = "2\n25.5\n1\n";
    fwrite(input, 1, sizeof(input) - 1, f);
    rewind(f);

    int old = dup(fileno(stdin));
    ASSERT_GE(old, 0);
    ASSERT_EQ(dup2(fileno(f), fileno(stdin)), fileno(stdin));
    clearerr(stdin);
    fclose(f);

    testing::internal::CaptureStdout();
    add(&t, &n, &cap);
    std::string out = testing::internal::GetCapturedStdout();

    dup2(old, fileno(stdin));
    close(old);
    clearerr(stdin);

    EXPECT_EQ(n, 1);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t[0].type, 2);
    EXPECT_DOUBLE_EQ(t[0].amount, 25.5);
    EXPECT_STREQ(t[0].category, "Food");
    EXPECT_NE(out.find("Transaction added successfully"), std::string::npos);

    free(t);
}

TEST_F(FinanceTest, AddRejectsZeroAmount) {
    struct transaction *t = nullptr;
    int n = 0;
    size_t cap = 0;

    FILE *f = tmpfile();
    ASSERT_NE(f, nullptr);
    const char input[] = "1\n0\n";
    fwrite(input, 1, sizeof(input) - 1, f);
    rewind(f);

    int old = dup(fileno(stdin));
    ASSERT_GE(old, 0);
    ASSERT_EQ(dup2(fileno(f), fileno(stdin)), fileno(stdin));
    clearerr(stdin);
    fclose(f);

    testing::internal::CaptureStdout();
    add(&t, &n, &cap);
    std::string out = testing::internal::GetCapturedStdout();

    dup2(old, fileno(stdin));
    close(old);
    clearerr(stdin);

    EXPECT_EQ(n, 0);
    EXPECT_EQ(t, nullptr);
    EXPECT_NE(out.find("Invalid amount"), std::string::npos);

    free(t);
}

TEST_F(FinanceTest, AddRejectsInvalidType) {
    struct transaction *t = nullptr;
    int n = 0;
    size_t cap = 0;

    FILE *f = tmpfile();
    ASSERT_NE(f, nullptr);
    const char input[] = "3\n";
    fwrite(input, 1, sizeof(input) - 1, f);
    rewind(f);

    int old = dup(fileno(stdin));
    ASSERT_GE(old, 0);
    ASSERT_EQ(dup2(fileno(f), fileno(stdin)), fileno(stdin));
    clearerr(stdin);
    fclose(f);

    testing::internal::CaptureStdout();
    add(&t, &n, &cap);
    std::string out = testing::internal::GetCapturedStdout();

    dup2(old, fileno(stdin));
    close(old);
    clearerr(stdin);

    EXPECT_EQ(n, 0);
    EXPECT_EQ(t, nullptr);
    EXPECT_NE(out.find("Invalid type"), std::string::npos);

    free(t);
}

TEST_F(FinanceTest, UpdateTransaction)
{
    struct transaction t[1] = {};
    t[0].id = 1;
    t[0].type = 1;
    t[0].amount = 1000.0;
    snprintf(t[0].category, sizeof(t[0].category), "%s", "Salary");

    FILE *f = tmpfile();
    fputs("1\n2\n500\n1\n", f);
    rewind(f);

    int old = dup(STDIN_FILENO);
    dup2(fileno(f), STDIN_FILENO);

    testing::internal::CaptureStdout();
    update(t, 1);
    std::string out = testing::internal::GetCapturedStdout();

    dup2(old, STDIN_FILENO);
    close(old);
    fclose(f);

    EXPECT_EQ(t[0].id, 1);
    EXPECT_EQ(t[0].type, 2);
    EXPECT_DOUBLE_EQ(t[0].amount, 500.0);
    EXPECT_STREQ(t[0].category, "Food");
    EXPECT_NE(out.find("Transaction updated successfully"), std::string::npos);
}

TEST_F(FinanceTest, UpdateTransactionRejectsInvalidID)
{
    struct transaction t[1] = {{1, 1, 1000.0, "Salary"}};

    FILE *f = tmpfile();
    fputs("99\n", f);
    rewind(f);

    int old = dup(STDIN_FILENO);
    dup2(fileno(f), STDIN_FILENO);

    testing::internal::CaptureStdout();
    update(t, 1);
    std::string out = testing::internal::GetCapturedStdout();

    dup2(old, STDIN_FILENO);
    close(old);
    fclose(f);

    EXPECT_DOUBLE_EQ(t[0].amount, 1000.0);
    EXPECT_NE(out.find("Transaction not found"), std::string::npos);
}

TEST_F(FinanceTest, DeleteTransaction)
{
    struct transaction t[2] = {};
    t[0].id = 1;
    t[0].type = 1;
    t[0].amount = 1000.0;
    snprintf(t[0].category, sizeof(t[0].category), "%s", "Salary");
    t[1].id = 2;
    t[1].type = 2;
    t[1].amount = 100.0;
    snprintf(t[1].category, sizeof(t[1].category), "%s", "Food");
    int n = 2;

    FILE *f = tmpfile();
    fputs("1\n", f);
    rewind(f);

    int old = dup(STDIN_FILENO);
    dup2(fileno(f), STDIN_FILENO);

    testing::internal::CaptureStdout();
    del(t, &n);
    std::string out = testing::internal::GetCapturedStdout();

    dup2(old, STDIN_FILENO);
    close(old);
    fclose(f);

    EXPECT_EQ(n, 1);
    EXPECT_EQ(t[0].id, 2);
    EXPECT_DOUBLE_EQ(t[0].amount, 100.0);
    EXPECT_NE(out.find("Transaction deleted successfully"), std::string::npos);
}

TEST_F(FinanceTest, DeleteTransactionRejectsInvalidID)
{
    struct transaction t[1] = {{1, 1, 1000.0, "Salary"}};
    int n = 1;

    FILE *f = tmpfile();
    fputs("99\n", f);
    rewind(f);

    int old = dup(STDIN_FILENO);
    dup2(fileno(f), STDIN_FILENO);

    testing::internal::CaptureStdout();
    del(t, &n);
    std::string out = testing::internal::GetCapturedStdout();

    dup2(old, STDIN_FILENO);
    close(old);
    fclose(f);

    EXPECT_EQ(n, 1);
    EXPECT_DOUBLE_EQ(t[0].amount, 1000.0);
    EXPECT_NE(out.find("Transaction not found"), std::string::npos);
}

TEST_F(FinanceTest, SearchExistingTransaction)
{
    struct transaction t[1] = {{1, 1, 1000.0, "Salary"}};

    FILE *f = tmpfile();
    fputs("1\n", f);
    rewind(f);

    int old = dup(STDIN_FILENO);
    dup2(fileno(f), STDIN_FILENO);

    testing::internal::CaptureStdout();
    search(t, 1);
    std::string out = testing::internal::GetCapturedStdout();

    dup2(old, STDIN_FILENO);
    close(old);
    fclose(f);

    EXPECT_NE(out.find("Transaction Found"), std::string::npos);
    EXPECT_NE(out.find("Salary"), std::string::npos);
    EXPECT_NE(out.find("1000.00"), std::string::npos);
}

TEST_F(FinanceTest, SearchMissingTransaction)
{
    struct transaction t[1] = {{1, 1, 1000.0, "Salary"}};

    FILE *f = tmpfile();
    fputs("99\n", f);
    rewind(f);

    int old = dup(STDIN_FILENO);
    dup2(fileno(f), STDIN_FILENO);

    testing::internal::CaptureStdout();
    search(t, 1);
    std::string out = testing::internal::GetCapturedStdout();

    dup2(old, STDIN_FILENO);
    close(old);
    fclose(f);

    EXPECT_NE(out.find("Transaction not found"), std::string::npos);
}

TEST_F(FinanceTest, FilterTransactionsByCategory)
{
    struct transaction t[2] = {
        {1, 1, 1000.0, "Salary"},
        {2, 2, 100.0, "Food"}
    };

    FILE *f = tmpfile();
    fputs("2\n1\n", f);
    rewind(f);

    int old = dup(STDIN_FILENO);
    dup2(fileno(f), STDIN_FILENO);

    testing::internal::CaptureStdout();
    filter(t, 2);
    std::string out = testing::internal::GetCapturedStdout();

    dup2(old, STDIN_FILENO);
    close(old);
    fclose(f);

    EXPECT_NE(out.find("Food"), std::string::npos);
    EXPECT_NE(out.find("100.00"), std::string::npos);
    EXPECT_EQ(out.find("Salary"), std::string::npos);
}

TEST_F(FinanceTest, FilterRejectsInvalidType)
{
    struct transaction t[1] = {{1, 1, 1000.0, "Salary"}};

    FILE *f = tmpfile();
    fputs("3\n", f);
    rewind(f);

    int old = dup(STDIN_FILENO);
    dup2(fileno(f), STDIN_FILENO);

    testing::internal::CaptureStdout();
    filter(t, 1);
    std::string out = testing::internal::GetCapturedStdout();

    dup2(old, STDIN_FILENO);
    close(old);
    fclose(f);

    EXPECT_NE(out.find("Invalid type"), std::string::npos);
}

TEST_F(FinanceTest, CreateBudget)
{
    struct transaction t[1] = {};
    FILE *f = tmpfile();
    fputs("1\n1\n500\n3\n", f);
    rewind(f);

    int old = dup(STDIN_FILENO);
    dup2(fileno(f), STDIN_FILENO);

    testing::internal::CaptureStdout();
    budgetMenu(t, 0);
    std::string out = testing::internal::GetCapturedStdout();

    dup2(old, STDIN_FILENO);
    close(old);
    fclose(f);

    struct budget b[9] = {};
    int n = 0;
    loadBudgets(b, &n);

    ASSERT_EQ(n, 1);
    EXPECT_STREQ(b[0].category, "Food");
    EXPECT_DOUBLE_EQ(b[0].amount, 500.0);
    EXPECT_NE(out.find("Budget saved"), std::string::npos);
}

TEST_F(FinanceTest, RejectInvalidBudgetAmount)
{
    struct transaction t[1] = {};
    FILE *f = tmpfile();
    fputs("1\n1\n0\n3\n", f);
    rewind(f);

    int old = dup(STDIN_FILENO);
    dup2(fileno(f), STDIN_FILENO);

    testing::internal::CaptureStdout();
    budgetMenu(t, 0);
    std::string out = testing::internal::GetCapturedStdout();

    dup2(old, STDIN_FILENO);
    close(old);
    fclose(f);

    struct budget b[9] = {};
    int n = 0;
    loadBudgets(b, &n);

    EXPECT_EQ(n, 0);
    EXPECT_NE(out.find("Invalid amount"), std::string::npos);
}

TEST_F(FinanceTest, ViewBudgetAndSpentAmount)
{
    struct transaction t[2] = {
        {1, 2, 100.0, "Food"},
        {2, 2, 50.0, "Food"}
    };

    struct budget b[9] = {{"Food", 200.0}};
    ASSERT_EQ(saveBudgets(b, 1), 1);

    FILE *f = tmpfile();
    fputs("2\n3\n", f);
    rewind(f);

    int old = dup(STDIN_FILENO);
    dup2(fileno(f), STDIN_FILENO);

    testing::internal::CaptureStdout();
    budgetMenu(t, 2);
    std::string out = testing::internal::GetCapturedStdout();

    dup2(old, STDIN_FILENO);
    close(old);
    fclose(f);

    EXPECT_NE(out.find("Food"), std::string::npos);
    EXPECT_NE(out.find("200.00"), std::string::npos);
    EXPECT_NE(out.find("150.00"), std::string::npos);
    EXPECT_NE(out.find("50.00"), std::string::npos);
}

TEST_F(FinanceTest, FinancialReportCalculations)
{
    struct transaction t[3] = {
        {1, 1, 1000.0, "Salary"},
        {2, 2, 200.0, "Food"},
        {3, 2, 100.0, "Travel"}
    };

    testing::internal::CaptureStdout();
    report(t, 3, 300.0);
    std::string out = testing::internal::GetCapturedStdout();

    EXPECT_NE(out.find("Total Income: 1000.00"), std::string::npos);
    EXPECT_NE(out.find("Total Expenses: 300.00"), std::string::npos);
    EXPECT_NE(out.find("Account Balance: 700.00"), std::string::npos);
    EXPECT_NE(out.find("Savings: 300.00"), std::string::npos);
    EXPECT_NE(out.find("Balance After Savings: 400.00"), std::string::npos);
}

TEST_F(FinanceTest, CategoryReportCalculatesExpenses)
{
    struct transaction t[3] = {
        {1, 1, 1000.0, "Salary"},
        {2, 2, 100.0, "Food"},
        {3, 2, 50.0, "Food"}
    };

    testing::internal::CaptureStdout();
    categoryReport(t, 3);
    std::string out = testing::internal::GetCapturedStdout();

    EXPECT_NE(out.find("Food"), std::string::npos);
    EXPECT_NE(out.find("150.00"), std::string::npos);
    EXPECT_EQ(out.find("Salary"), std::string::npos);
}

TEST_F(FinanceTest, SetSavingsRejectsAmountAboveBalance)
{
    struct transaction t[2] = {
        {1, 1, 1000.0, "Salary"},
        {2, 2, 200.0, "Food"}
    };
    double s = 100.0;

    FILE *f = tmpfile();
    fputs("900\n", f);
    rewind(f);

    int old = dup(STDIN_FILENO);
    dup2(fileno(f), STDIN_FILENO);

    testing::internal::CaptureStdout();
    setSavings(t, 2, &s);
    std::string out = testing::internal::GetCapturedStdout();

    dup2(old, STDIN_FILENO);
    close(old);
    fclose(f);

    EXPECT_DOUBLE_EQ(s, 100.0);
    EXPECT_NE(out.find("Invalid savings amount"), std::string::npos);
}

TEST_F(FinanceTest, AddRejectsInvalidCategory)
{
    struct transaction *t = nullptr;
    size_t cap = 0;
    int n = 0;

    FILE *f = tmpfile();
    fputs("1\n1000\n6\n", f);
    rewind(f);

    int old = dup(STDIN_FILENO);
    dup2(fileno(f), STDIN_FILENO);

    testing::internal::CaptureStdout();
    add(&t, &n, &cap);
    std::string out = testing::internal::GetCapturedStdout();

    dup2(old, STDIN_FILENO);
    close(old);
    fclose(f);

    EXPECT_EQ(n, 0);
    EXPECT_NE(out.find("Invalid category"), std::string::npos);
    free(t);
}

TEST_F(FinanceTest, SetSavingsRejectsNegativeAmount)
{
    struct transaction t[1] = {{1, 1, 1000.0, "Salary"}};
    double s = 100.0;

    FILE *f = tmpfile();
    fputs("-10\n", f);
    rewind(f);

    int old = dup(STDIN_FILENO);
    dup2(fileno(f), STDIN_FILENO);

    testing::internal::CaptureStdout();
    setSavings(t, 1, &s);
    std::string out = testing::internal::GetCapturedStdout();

    dup2(old, STDIN_FILENO);
    close(old);
    fclose(f);

    EXPECT_DOUBLE_EQ(s, 100.0);
    EXPECT_NE(out.find("Invalid savings amount"), std::string::npos);
}

TEST_F(FinanceTest, BudgetExceededWarning)
{
    struct transaction t[1] = {{1, 2, 250.0, "Food"}};
    struct budget b[9] = {{"Food", 200.0}};
    ASSERT_EQ(saveBudgets(b, 1), 1);

    FILE *f = tmpfile();
    fputs("2\n3\n", f);
    rewind(f);

    int old = dup(STDIN_FILENO);
    dup2(fileno(f), STDIN_FILENO);

    testing::internal::CaptureStdout();
    budgetMenu(t, 1);
    std::string out = testing::internal::GetCapturedStdout();

    dup2(old, STDIN_FILENO);
    close(old);
    fclose(f);

    EXPECT_NE(out.find("Budget exceeded"), std::string::npos);
    EXPECT_NE(out.find("250.00"), std::string::npos);
}

TEST_F(FinanceTest, EmptyReports)
{
    struct transaction t[1] = {};

    testing::internal::CaptureStdout();
    report(t, 0, 0.0);
    categoryReport(t, 0);
    std::string out = testing::internal::GetCapturedStdout();

    EXPECT_NE(out.find("Total Income: 0.00"), std::string::npos);
    EXPECT_NE(out.find("Total Expenses: 0.00"), std::string::npos);
    EXPECT_NE(out.find("No expenses found"), std::string::npos);
}

TEST_F(FinanceTest, LoadRejectsInvalidTransactionCount)
{
    FILE *f = fopen("transactions.dat", "wb");
    int n = -1;
    ASSERT_NE(f, nullptr);
    fwrite(&n, sizeof(int), 1, f);
    fclose(f);

    struct transaction *t = nullptr;
    size_t cap = 0;
    n = 5;

    testing::internal::CaptureStdout();
    load(&t, &n, &cap);
    std::string out = testing::internal::GetCapturedStdout();

    EXPECT_EQ(n, 0);
    EXPECT_NE(out.find("Invalid transaction file"), std::string::npos);
    free(t);
}

TEST_F(FinanceTest, LoadRejectsTruncatedTransactionFile)
{
    FILE *f = fopen("transactions.dat", "wb");
    int n = 1;
    ASSERT_NE(f, nullptr);
    fwrite(&n, sizeof(int), 1, f);
    fclose(f);

    struct transaction *t = nullptr;
    size_t cap = 0;
    n = 0;

    testing::internal::CaptureStdout();
    load(&t, &n, &cap);
    std::string out = testing::internal::GetCapturedStdout();

    EXPECT_EQ(n, 0);
    EXPECT_NE(out.find("Unable to load transactions"), std::string::npos);
    free(t);
}

TEST_F(FinanceTest, LoadRejectsUnexpectedTrailingData)
{
    struct transaction in[1] = {};
    in[0].id = 1;
    in[0].type = 1;
    in[0].amount = 1000.0;
    snprintf(in[0].category, sizeof(in[0].category), "%s", "Salary");
    ASSERT_EQ(save(in, 1), 1);

    FILE *f = fopen("transactions.dat", "ab");
    ASSERT_NE(f, nullptr);
    fputc('X', f);
    fclose(f);

    struct transaction *t = nullptr;
    size_t cap = 0;
    int n = 0;

    testing::internal::CaptureStdout();
    load(&t, &n, &cap);
    std::string out = testing::internal::GetCapturedStdout();

    EXPECT_EQ(n, 0);
    EXPECT_NE(out.find("Unexpected data in transaction file"), std::string::npos);
    free(t);
}

TEST_F(FinanceTest, LoadSavingsRejectsTruncatedFile)
{
    FILE *f = fopen("savings.dat", "wb");
    ASSERT_NE(f, nullptr);
    fputc('X', f);
    fclose(f);

    double s = 100.0;

    testing::internal::CaptureStdout();
    loadSavings(&s);
    std::string out = testing::internal::GetCapturedStdout();

    EXPECT_DOUBLE_EQ(s, 0.0);
    EXPECT_NE(out.find("Invalid savings file"), std::string::npos);
}

TEST_F(FinanceTest, LoadSavingsRejectsTrailingData)
{
    ASSERT_EQ(saveSavings(100.0), 1);

    FILE *f = fopen("savings.dat", "ab");
    ASSERT_NE(f, nullptr);
    fputc('X', f);
    fclose(f);

    double s = 0.0;

    testing::internal::CaptureStdout();
    loadSavings(&s);
    std::string out = testing::internal::GetCapturedStdout();

    EXPECT_DOUBLE_EQ(s, 0.0);
    EXPECT_NE(out.find("Invalid savings file"), std::string::npos);
}

TEST_F(FinanceTest, LoadBudgetsRejectsInvalidCount)
{
    FILE *f = fopen("budgets.dat", "wb");
    ASSERT_NE(f, nullptr);
    int n = 10;
    fwrite(&n, sizeof(int), 1, f);
    fclose(f);

    struct budget b[9] = {};
    n = 5;

    testing::internal::CaptureStdout();
    loadBudgets(b, &n);
    std::string out = testing::internal::GetCapturedStdout();

    EXPECT_EQ(n, 0);
    EXPECT_NE(out.find("Invalid budget file"), std::string::npos);
}

TEST_F(FinanceTest, LoadBudgetsRejectsTrailingData)
{
    struct budget in[1] = {};
    snprintf(in[0].category, sizeof(in[0].category), "%s", "Food");
    in[0].amount = 500.0;
    ASSERT_EQ(saveBudgets(in, 1), 1);

    FILE *f = fopen("budgets.dat", "ab");
    ASSERT_NE(f, nullptr);
    fputc('X', f);
    fclose(f);

    struct budget b[9] = {};
    int n = 0;

    testing::internal::CaptureStdout();
    loadBudgets(b, &n);
    std::string out = testing::internal::GetCapturedStdout();

    EXPECT_EQ(n, 0);
    EXPECT_NE(out.find("Unexpected data in budget file"), std::string::npos);
}
