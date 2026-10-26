/*
 * ===================================================================
 *  Student Record Search System - DSA Mini Project (BST) - C version
 * ===================================================================
 *  Each student record has:
 *      enrolmentId  -> BST key (unique integer)
 *      name
 *      department / programme
 *
 *  Implements:
 *    - BST insertion (duplicate enrolment IDs are rejected)
 *    - Search by enrolment ID (with comparison count)
 *    - Deletion: leaf, one-child and two-child cases
 *    - Inorder / Preorder / Postorder traversals (Inorder = sorted order)
 *    - Tree height and sideways tree display
 *    - Balanced-looking vs sorted insertion comparison
 *    - Built-in test cases
 *
 *  Compile:  gcc -Wall -std=c99 -o student_bst student_bst.c
 *  Run:      ./student_bst          (interactive menu)
 *            ./student_bst --test   (test cases only)
 * ===================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NAME_MAX_LEN   50
#define DEPT_MAX_LEN   30
#define INPUT_LEN      64
#define TEXT_LEN       512
#define EXPERIMENT_N   15     /* size used for the balanced vs sorted comparison */
#define NUM_TESTS      7

#define CASE_LEAF "Leaf node (no children)"
#define CASE_ONE  "Node with one child"
#define CASE_TWO  "Node with two children (replaced by inorder successor)"

/* ------------------------------------------------------------------
 * Data types
 * ------------------------------------------------------------------ */
typedef struct {
    int  enrolmentId;                 /* BST key - must be unique */
    char name[NAME_MAX_LEN];
    char department[DEPT_MAX_LEN];
} Student;

typedef struct Node {
    Student data;                     /* data.enrolmentId is the key */
    struct Node *left;
    struct Node *right;
} Node;

typedef struct {
    Node *root;
    int   size;                       /* number of records in the tree */
    long  comparisons;                /* key comparisons made (for analysis) */
    char  lastDeleteCase[80];         /* which deletion case was used last */
} BST;

typedef enum { INORDER, PREORDER, POSTORDER } Order;

/* ------------------------------------------------------------------
 * Small helpers
 * ------------------------------------------------------------------ */
static void *xmalloc(size_t bytes) {
    void *p = malloc(bytes);
    if (p == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    return p;
}

static Student make_student(int id, const char *name, const char *dept) {
    Student s;
    s.enrolmentId = id;
    snprintf(s.name, sizeof s.name, "%s", name);
    snprintf(s.department, sizeof s.department, "%s", dept);
    return s;
}

static void print_student(const Student *s) {
    printf("ID: %-6d Name: %-16s Dept: %s\n", s->enrolmentId, s->name, s->department);
}

/* Reads one line from stdin and removes the trailing newline. */
static int read_line(char *buf, size_t size) {
    if (fgets(buf, (int)size, stdin) == NULL)
        return 0;
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    } else {
        /* line was longer than the buffer: discard the rest of it */
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }
    }
    return 1;
}

/* Reads a whole number. Returns 1 on success, 0 if the input was invalid. */
static int read_int(const char *prompt, int *out) {
    char line[INPUT_LEN];
    char *end;

    printf("%s", prompt);
    if (!read_line(line, sizeof line))
        return 0;

    long v = strtol(line, &end, 10);
    if (end == line) {
        printf("  !! Please enter a valid whole number.\n");
        return 0;
    }
    while (isspace((unsigned char)*end))
        end++;
    if (*end != '\0') {
        printf("  !! Please enter a valid whole number.\n");
        return 0;
    }
    *out = (int)v;
    return 1;
}

/* ------------------------------------------------------------------
 * BST core operations
 * ------------------------------------------------------------------ */
static void bst_init(BST *t) {
    t->root = NULL;
    t->size = 0;
    t->comparisons = 0;
    t->lastDeleteCase[0] = '\0';
}

static Node *new_node(Student s) {
    Node *n = xmalloc(sizeof(Node));
    n->data = s;
    n->left = NULL;
    n->right = NULL;
    return n;
}

/* Insert. Returns 1 if inserted, 0 if the enrolment ID already exists. */
static int bst_insert(BST *t, Student s) {
    Node **cur = &t->root;               /* pointer to the link we may fill */

    while (*cur != NULL) {
        t->comparisons++;
        if (s.enrolmentId == (*cur)->data.enrolmentId)
            return 0;                    /* duplicate -> reject */
        if (s.enrolmentId < (*cur)->data.enrolmentId)
            cur = &(*cur)->left;
        else
            cur = &(*cur)->right;
    }

    *cur = new_node(s);                  /* found an empty spot: attach here */
    t->size++;
    return 1;
}

/* Search. Returns a pointer to the student, or NULL if not found. */
static Student *bst_search(BST *t, int id) {
    Node *cur = t->root;
    while (cur != NULL) {
        t->comparisons++;
        if (id == cur->data.enrolmentId)
            return &cur->data;
        cur = (id < cur->data.enrolmentId) ? cur->left : cur->right;
    }
    return NULL;
}

/*
 * Recursive delete helper. 'link' points to the pointer that holds the
 * current subtree root, so the parent's child pointer can be updated.
 * Returns 1 if a node was deleted, 0 if the key was not found.
 */
static int delete_node(BST *t, Node **link, int id) {
    Node *node = *link;
    if (node == NULL)
        return 0;

    t->comparisons++;
    if (id < node->data.enrolmentId)
        return delete_node(t, &node->left, id);
    if (id > node->data.enrolmentId)
        return delete_node(t, &node->right, id);

    /* ---- key found ---- */
    if (node->left == NULL && node->right == NULL) {
        /* Case 1: leaf node -> remove it */
        strcpy(t->lastDeleteCase, CASE_LEAF);
        free(node);
        *link = NULL;
    } else if (node->left == NULL || node->right == NULL) {
        /* Case 2: one child -> the child takes this node's place */
        Node *child = (node->left != NULL) ? node->left : node->right;
        strcpy(t->lastDeleteCase, CASE_ONE);
        free(node);
        *link = child;
    } else {
        /* Case 3: two children -> copy the inorder successor
           (smallest key in the right subtree) into this node,
           then delete that successor from the right subtree. */
        Node *succ = node->right;
        while (succ->left != NULL)
            succ = succ->left;

        node->data = succ->data;
        delete_node(t, &node->right, succ->data.enrolmentId);
        strcpy(t->lastDeleteCase, CASE_TWO);   /* set after the recursive call */
    }
    return 1;
}

/* Public delete. Returns 1 if a record was deleted. */
static int bst_delete(BST *t, int id) {
    int ok = delete_node(t, &t->root, id);
    if (ok)
        t->size--;
    return ok;
}

/* Frees every node in the subtree. */
static void free_nodes(Node *n) {
    if (n == NULL)
        return;
    free_nodes(n->left);
    free_nodes(n->right);
    free(n);
}

static void bst_free(BST *t) {
    free_nodes(t->root);
    t->root = NULL;
    t->size = 0;
}

/* Height = number of levels (empty tree = 0). */
static int tree_height(const Node *n) {
    if (n == NULL)
        return 0;
    int l = tree_height(n->left);
    int r = tree_height(n->right);
    return 1 + (l > r ? l : r);
}

/*
 * One traversal function for all three orders.
 *   PREORDER : visit node, then left, then right
 *   INORDER  : left, then visit node, then right   (sorted order)
 *   POSTORDER: left, then right, then visit node
 */
static void collect(const Node *n, Order order, Student out[], int *count) {
    if (n == NULL)
        return;
    if (order == PREORDER)
        out[(*count)++] = n->data;
    collect(n->left, order, out, count);
    if (order == INORDER)
        out[(*count)++] = n->data;
    collect(n->right, order, out, count);
    if (order == POSTORDER)
        out[(*count)++] = n->data;
}

/* Prints the tree sideways: right subtree on top, left subtree below. */
static void show_structure(const Node *n, int depth, const char *label) {
    if (n == NULL)
        return;
    show_structure(n->right, depth + 1, "R");
    for (int i = 0; i < depth; i++)
        printf("    ");
    printf("%s:%d\n", label, n->data.enrolmentId);
    show_structure(n->left, depth + 1, "L");
}

/* Builds an insertion order that gives a balanced tree (middle element first). */
static void balanced_order(const int sorted[], int lo, int hi, int out[], int *count) {
    if (lo > hi)
        return;
    int mid = (lo + hi) / 2;
    out[(*count)++] = sorted[mid];
    balanced_order(sorted, lo, mid - 1, out, count);
    balanced_order(sorted, mid + 1, hi, out, count);
}

/* Builds a BST by inserting the given IDs in order. */
static void build_tree(BST *t, const int keys[], int n) {
    bst_init(t);
    for (int i = 0; i < n; i++) {
        char name[NAME_MAX_LEN];
        snprintf(name, sizeof name, "Student %d", keys[i]);
        bst_insert(t, make_student(keys[i], name, "CSE"));
    }
}

/* ------------------------------------------------------------------
 * Display helpers
 * ------------------------------------------------------------------ */
static void print_students(const char *title, const Student arr[], int n) {
    printf("\n%s\n", title);
    if (n == 0)
        printf("  (no records)\n");
    for (int i = 0; i < n; i++) {
        printf("  ");
        print_student(&arr[i]);
    }
}

/* Collects one traversal into a temporary array and prints it. */
static void show_traversal(const BST *t, Order order, const char *title) {
    Student *arr = xmalloc(sizeof(Student) * (size_t)(t->size + 1));
    int n = 0;
    collect(t->root, order, arr, &n);
    print_students(title, arr, n);
    free(arr);
}

static void print_int_list(const int a[], int n) {
    for (int i = 0; i < n; i++)
        printf("%d%s", a[i], i < n - 1 ? ", " : "");
    printf("\n");
}

/* Writes IDs as "30, 40, 50" (used by the test cases). */
static void ids_to_text(const Student arr[], int n, char *buf, size_t size) {
    size_t used = 0;
    buf[0] = '\0';
    for (int i = 0; i < n && used < size - 1; i++)
        used += (size_t)snprintf(buf + used, size - used, "%s%d",
                                 i ? ", " : "", arr[i].enrolmentId);
}

/* Writes the inorder sequence of IDs as text. */
static void inorder_text(const BST *t, char *buf, size_t size) {
    Student *arr = xmalloc(sizeof(Student) * (size_t)(t->size + 1));
    int n = 0;
    collect(t->root, INORDER, arr, &n);
    ids_to_text(arr, n, buf, size);
    free(arr);
}

/* ------------------------------------------------------------------
 * Sample data, loading, and experiments
 * ------------------------------------------------------------------ */
static const Student SAMPLE_RECORDS[] = {
    {1005, "Aarav Sharma",    "CSE"},
    {1002, "Priya Singh",     "ECE"},
    {1008, "Rohan Mehta",     "ME"},
    {1001, "Neha Gupta",      "CSE"},
    {1004, "Arjun Verma",     "IT"},
    {1007, "Simran Kaur",     "CSE"},
    {1010, "Vikram Joshi",    "CE"},
    {1003, "Ananya Roy",      "BBA"},
    {1006, "Karan Malhotra",  "ECE"},
    {1009, "Ishita Das",      "IT"},
};
#define NUM_SAMPLES ((int)(sizeof(SAMPLE_RECORDS) / sizeof(SAMPLE_RECORDS[0])))

static void load_samples(BST *t) {
    int added = 0;
    for (int i = 0; i < NUM_SAMPLES; i++)
        if (bst_insert(t, SAMPLE_RECORDS[i]))
            added++;
    printf("  %d sample record(s) loaded (%d skipped as duplicates).\n",
           added, NUM_SAMPLES - added);
}

/* Compares a balanced-looking insertion order with a sorted one. */
static void compare_insertion_orders(void) {
    int sorted[EXPERIMENT_N];
    int balanced[EXPERIMENT_N];
    int count = 0;

    for (int i = 0; i < EXPERIMENT_N; i++)
        sorted[i] = i + 1;
    balanced_order(sorted, 0, EXPERIMENT_N - 1, balanced, &count);

    printf("\nBalanced-looking insertion order: ");
    print_int_list(balanced, EXPERIMENT_N);
    printf("Sorted insertion order          : ");
    print_int_list(sorted, EXPERIMENT_N);

    printf("\n%-20s| %12s | %6s\n", "Insertion order", "Comparisons", "Height");
    for (int i = 0; i < 44; i++)
        putchar('-');
    printf("\n");

    const char *labels[] = {"Balanced-looking", "Sorted (ascending)"};
    const int *orders[] = {balanced, sorted};
    for (int k = 0; k < 2; k++) {
        BST t;
        build_tree(&t, orders[k], EXPERIMENT_N);
        printf("%-20s| %12ld | %6d\n", labels[k], t.comparisons, tree_height(t.root));
        bst_free(&t);
    }
}

/* ------------------------------------------------------------------
 * Interactive handlers
 * ------------------------------------------------------------------ */
static void handle_insert(BST *t) {
    int id;
    char name[NAME_MAX_LEN];
    char dept[DEPT_MAX_LEN];

    if (!read_int("Enrolment ID: ", &id))
        return;
    printf("Student name: ");
    if (!read_line(name, sizeof name))
        return;
    printf("Programme / Department: ");
    if (!read_line(dept, sizeof dept))
        return;

    if (name[0] == '\0' || dept[0] == '\0') {
        printf("  !! Name and department cannot be empty.\n");
        return;
    }

    if (bst_insert(t, make_student(id, name, dept)))
        printf("  Record %d inserted successfully.\n", id);
    else
        printf("  !! Duplicate rejected: enrolment ID %d already exists.\n", id);
}

/* ------------------------------------------------------------------
 * Test cases
 * ------------------------------------------------------------------ */
static int report_test(int number, const char *title,
                       const char *expected, const char *actual) {
    int ok = (strcmp(expected, actual) == 0);
    printf("\nTest %d: %s\n", number, title);
    printf("  Expected : %s\n", expected);
    printf("  Actual   : %s\n", actual);
    printf("  Result   : %s\n", ok ? "PASS" : "FAIL");
    return ok;
}

/* Builds a tree from keys, deletes one ID, and checks the result. */
static int test_delete(int number, const char *title, const int keys[], int n,
                       int idToDelete, const char *expInorder, const char *expCase) {
    BST t;
    char text[TEXT_LEN], expected[TEXT_LEN], actual[TEXT_LEN];

    build_tree(&t, keys, n);
    int deleted = bst_delete(&t, idToDelete);
    inorder_text(&t, text, sizeof text);

    snprintf(expected, sizeof expected,
             "deleted=yes, inorder=[%s], case=%s", expInorder, expCase);
    snprintf(actual, sizeof actual,
             "deleted=%s, inorder=[%s], case=%s",
             deleted ? "yes" : "no", text, t.lastDeleteCase);

    bst_free(&t);
    return report_test(number, title, expected, actual);
}

static void run_tests(void) {
    BST t;
    char text[TEXT_LEN], expected[TEXT_LEN], actual[TEXT_LEN];
    int keys1[] = {50, 30, 70, 20, 40, 60, 80};
    int keys2[] = {50, 30, 70, 20, 40, 60, 80, 15};
    int keys3[] = {50, 30, 70};
    int passed = 0;

    printf("\n=== Test Cases ===\n");

    /* Tests 1-3: the three deletion cases */
    passed += test_delete(1, "Delete a leaf node (20)", keys1, 7, 20,
                          "30, 40, 50, 60, 70, 80", CASE_LEAF);
    passed += test_delete(2, "Delete a node with one child (20, child 15)", keys2, 8, 20,
                          "15, 30, 40, 50, 60, 70, 80", CASE_ONE);
    passed += test_delete(3, "Delete a node with two children (30)", keys1, 7, 30,
                          "20, 40, 50, 60, 70, 80", CASE_TWO);

    /* Test 4: duplicate enrolment ID is rejected */
    build_tree(&t, keys3, 3);
    int inserted = bst_insert(&t, make_student(30, "Duplicate", "CSE"));
    snprintf(expected, sizeof expected, "rejected, size=3");
    snprintf(actual, sizeof actual, "%s, size=%d",
             inserted ? "inserted" : "rejected", t.size);
    passed += report_test(4, "Reject duplicate enrolment ID (30)", expected, actual);
    bst_free(&t);

    /* Test 5: successful search */
    build_tree(&t, keys3, 3);
    Student *s = bst_search(&t, 70);
    passed += report_test(5, "Successful search (70)", "Student 70",
                          s ? s->name : "not found");
    bst_free(&t);

    /* Test 6: unsuccessful search */
    build_tree(&t, keys3, 3);
    s = bst_search(&t, 99);
    passed += report_test(6, "Unsuccessful search (99)", "not found",
                          s ? "found" : "not found");
    bst_free(&t);

    /* Test 7: inorder traversal gives sorted order */
    build_tree(&t, keys1, 7);
    inorder_text(&t, text, sizeof text);
    passed += report_test(7, "Inorder traversal is sorted",
                          "20, 30, 40, 50, 60, 70, 80", text);
    bst_free(&t);

    printf("\n%d/%d test cases passed.\n", passed, NUM_TESTS);
}

/* ------------------------------------------------------------------
 * Menu and main
 * ------------------------------------------------------------------ */
static const char *MENU =
    "\n========  STUDENT RECORD SEARCH SYSTEM (BST)  ========\n"
    " 1. Insert a student record\n"
    " 2. Search by enrolment ID\n"
    " 3. Delete a student record\n"
    " 4. Display records - Inorder (sorted by ID)\n"
    " 5. Display records - Preorder\n"
    " 6. Display records - Postorder\n"
    " 7. Show tree size, height and structure\n"
    " 8. Load 10 sample records\n"
    " 9. Compare balanced vs sorted insertion\n"
    "10. Run test cases\n"
    " 0. Exit\n";

int main(int argc, char *argv[]) {
    BST t;
    char choice[INPUT_LEN];
    int value;

    if (argc > 1 && strcmp(argv[1], "--test") == 0) {
        run_tests();
        return 0;
    }

    bst_init(&t);
    while (1) {
        printf("%s", MENU);
        printf("Enter your choice: ");
        if (!read_line(choice, sizeof choice))
            break;

        if (strcmp(choice, "1") == 0) {
            handle_insert(&t);

        } else if (strcmp(choice, "2") == 0) {
            if (read_int("Enter enrolment ID to search: ", &value)) {
                long before = t.comparisons;
                Student *s = bst_search(&t, value);
                if (s != NULL) {
                    printf("\nFound: ");
                    print_student(s);
                } else {
                    printf("\nNo record found for enrolment ID %d.\n", value);
                }
                printf("Comparisons used: %ld\n", t.comparisons - before);
            }

        } else if (strcmp(choice, "3") == 0) {
            if (read_int("Enter enrolment ID to delete: ", &value)) {
                if (bst_delete(&t, value))
                    printf("  Deleted record %d. Case used: %s\n", value, t.lastDeleteCase);
                else
                    printf("  No record with ID %d to delete.\n", value);
            }

        } else if (strcmp(choice, "4") == 0) {
            show_traversal(&t, INORDER, "Records in sorted order (Inorder):");

        } else if (strcmp(choice, "5") == 0) {
            show_traversal(&t, PREORDER, "Preorder traversal:");

        } else if (strcmp(choice, "6") == 0) {
            show_traversal(&t, POSTORDER, "Postorder traversal:");

        } else if (strcmp(choice, "7") == 0) {
            printf("\nTotal records : %d\n", t.size);
            printf("Tree height   : %d\n", tree_height(t.root));
            printf("Tree structure (right subtree shown on top):\n");
            show_structure(t.root, 0, "Root");

        } else if (strcmp(choice, "8") == 0) {
            load_samples(&t);

        } else if (strcmp(choice, "9") == 0) {
            compare_insertion_orders();

        } else if (strcmp(choice, "10") == 0) {
            run_tests();

        } else if (strcmp(choice, "0") == 0) {
            printf("Goodbye!\n");
            break;

        } else {
            printf("Invalid choice, try again.\n");
        }
    }

    bst_free(&t);
    return 0;
}
