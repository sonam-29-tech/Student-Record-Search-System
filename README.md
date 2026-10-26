# Student Record Search System (BST, C)
A program that models campus locations as a graph. It uses an adjacency list and BFS to find the shortest route between two places, DFS to list reachable locations, and detects disconnected places.
Each student record has a unique enrolment ID (the BST key), a name, and a department.

## Features
- Insert records and reject duplicate IDs with a message
- Search by enrolment ID
- Delete: leaf node, one-child node, and two-child node
- Inorder (sorted), preorder, and postorder traversals
- Load 10 sample records with one menu option
- 4 test cases

## Compile and Run
```bash
gcc -Wall -std=c99 -o student_bst student_bst.c
./student_bst
```
On Windows (MinGW): `gcc -Wall -std=c99 -o student_bst.exe student_bst.c`

## Sample Data
Load option 7 inserts these IDs in this order:
1005, 1002, 1008, 1001, 1004, 1007, 1010, 1003, 1006, 1009

This gives the tree:
```text
            1005
          /      \
       1002      1008
      /    \    /    \
    1001  1004 1007  1010
           /         /
        1003      1009
```
(1006 is the right child of 1007's left... see note below.)

Note: 1006 is inserted as the left child of 1007.

## Sample Output

Option 4 (inorder, sorted):
```text
ID: 1001   Name: Neha Gupta      Dept: CSE
ID: 1002   Name: Priya Singh     Dept: ECE
ID: 1003   Name: Ananya Roy      Dept: BBA
...
ID: 1010   Name: Vikram Joshi    Dept: CE
```

Option 5 (preorder):
```text
1005, 1002, 1001, 1004, 1003, 1008, 1007, 1006, 1010, 1009
```

Option 6 (postorder):
```text
1001, 1003, 1004, 1002, 1006, 1007, 1009, 1010, 1008, 1005
```

## Deletion Cases

| Case | Example | What happens |
|------|---------|--------------|
| Leaf | Delete 1009 | Node is removed |
| One child | Delete 1010 (its only child is 1009) | Child 1009 takes its place |
| Two children | Delete 1002 | Replaced by the smallest ID in its right subtree (1003), then 1003 is removed from there |

## Test Cases

| Test | Scenario | Expected | Actual | Result |
|------|----------|----------|--------|--------|
| 1 | Delete leaf (20) | 30, 40, 50, 60, 70, 80 | 30, 40, 50, 60, 70, 80 | PASS |
| 2 | Delete one-child node (20, child 15) | 15, 30, 40, 50, 60, 70, 80 | 15, 30, 40, 50, 60, 70, 80 | PASS |
| 3 | Delete two-child node (30) | 20, 40, 50, 60, 70, 80 | 20, 40, 50, 60, 70, 80 | PASS |
| 4 | Duplicate ID (30) | Rejected | Rejected | PASS |

## Complexity

| Operation | Average | Worst (skewed tree) |
|-----------|---------|---------------------|
| Search, insert, delete | O(log n) | O(n) |
| Inorder, preorder, postorder | O(n) | O(n) |

Space is O(n) for the records. Recursive calls use O(h) stack space, where h is the tree height.

## Why a BST Can Become O(n)
If IDs are inserted in sorted order (1, 2, 3, ...), each new ID becomes the right child of the previous one. The tree turns into a straight chain, so search behaves like a linked list. An AVL tree fixes this by rotating nodes after each insert or delete to keep the height at O(log n).


