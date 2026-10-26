"""
Student Record Search System - (Binary Search Tree)
=====================================================================
Each student record has:
    * enrolment ID  -> BST key (unique integer)
    * name
    * department / programme

Implements:
  * BST insertion (duplicates rejected with a message)
  * Search by enrolment ID
  * Deletion: leaf, one-child and two-child cases
  * Inorder / Preorder / Postorder traversals (Inorder = sorted order)
  * Tree height and sideways tree display
  * Comparison of balanced-looking vs sorted insertion order
  * Built-in test cases

Run:
    python student_bst.py          -> interactive menu
    python student_bst.py --test   -> run test cases only
"""

import sys

CASE_LEAF = "Leaf node (no children)"
CASE_ONE = "Node with one child"
CASE_TWO = "Node with two children (replaced by inorder successor)"


class Student:
    """A student record. enrolment_id is the unique BST key."""

    def __init__(self, enrolment_id, name, department):
        self.enrolment_id = enrolment_id
        self.name = name
        self.department = department

    def __str__(self):
        return f"ID: {self.enrolment_id:<6} Name: {self.name:<16} Dept: {self.department}"


class Node:
    """A BST node. The key is copied from the student's enrolment ID."""

    def __init__(self, student):
        self.key = student.enrolment_id
        self.student = student
        self.left = None
        self.right = None


class StudentBST:
    def __init__(self):
        self.root = None
        self.size = 0
        self.comparisons = 0          # key comparisons made (for analysis)
        self.last_delete_case = ""    # which deletion case was used last

    # ---------------- Insertion ----------------
    def insert(self, student):
        """Insert a student. Returns False if the enrolment ID already exists."""
        if self.root is None:
            self.root = Node(student)
            self.size += 1
            return True

        current = self.root
        while True:
            self.comparisons += 1
            if student.enrolment_id == current.key:
                return False                       # duplicate -> reject
            if student.enrolment_id < current.key:
                if current.left is None:
                    current.left = Node(student)
                    break
                current = current.left
            else:
                if current.right is None:
                    current.right = Node(student)
                    break
                current = current.right
        self.size += 1
        return True

    # ---------------- Search ----------------
    def search(self, enrolment_id):
        """Return the Student with this ID, or None if not found."""
        current = self.root
        while current is not None:
            self.comparisons += 1
            if enrolment_id == current.key:
                return current.student
            current = current.left if enrolment_id < current.key else current.right
        return None

    # ---------------- Deletion ----------------
    def delete(self, enrolment_id):
        """Delete the record with this ID. Returns True if a record was deleted."""
        self.root, deleted = self._delete(self.root, enrolment_id)
        if deleted:
            self.size -= 1
        return deleted

    def _delete(self, node, key):
        """Recursive helper. Returns (new subtree root, deleted flag)."""
        if node is None:
            return None, False

        if key < node.key:
            node.left, deleted = self._delete(node.left, key)
            return node, deleted
        if key > node.key:
            node.right, deleted = self._delete(node.right, key)
            return node, deleted

        # ---- key found ----
        # Case 1: leaf node -> simply remove it
        if node.left is None and node.right is None:
            self.last_delete_case = CASE_LEAF
            return None, True

        # Case 2: one child -> the child takes this node's place
        if node.left is None or node.right is None:
            self.last_delete_case = CASE_ONE
            return (node.right if node.left is None else node.left), True

        # Case 3: two children -> copy the inorder successor (smallest key
        # in the right subtree) into this node, then delete that successor
        successor = node.right
        while successor.left is not None:
            successor = successor.left
        node.key = successor.key
        node.student = successor.student
        node.right, _ = self._delete(node.right, successor.key)
        self.last_delete_case = CASE_TWO       # set after the recursive call
        return node, True

    # ---------------- Traversals ----------------
    def inorder(self):
        """Left, Root, Right -> records in ascending enrolment-ID order."""
        result = []
        self._inorder(self.root, result)
        return result

    def _inorder(self, node, result):
        if node is not None:
            self._inorder(node.left, result)
            result.append(node.student)
            self._inorder(node.right, result)

    def preorder(self):
        """Root, Left, Right."""
        result = []
        self._preorder(self.root, result)
        return result

    def _preorder(self, node, result):
        if node is not None:
            result.append(node.student)
            self._preorder(node.left, result)
            self._preorder(node.right, result)

    def postorder(self):
        """Left, Right, Root."""
        result = []
        self._postorder(self.root, result)
        return result

    def _postorder(self, node, result):
        if node is not None:
            self._postorder(node.left, result)
            self._postorder(node.right, result)
            result.append(node.student)

    # ---------------- Height ----------------
    def height(self):
        """Number of levels in the tree (empty tree = 0)."""
        return self._height(self.root)

    def _height(self, node):
        if node is None:
            return 0
        return 1 + max(self._height(node.left), self._height(node.right))


# ----------------------------------------------------------------------
# Helper functions
# ----------------------------------------------------------------------
SAMPLE_RECORDS = [
    (1005, "Aarav Sharma", "CSE"),
    (1002, "Priya Singh", "ECE"),
    (1008, "Rohan Mehta", "ME"),
    (1001, "Neha Gupta", "CSE"),
    (1004, "Arjun Verma", "IT"),
    (1007, "Simran Kaur", "CSE"),
    (1010, "Vikram Joshi", "CE"),
    (1003, "Ananya Roy", "BBA"),
    (1006, "Karan Malhotra", "ECE"),
    (1009, "Ishita Das", "IT"),
]


def show_structure(node, depth=0, label="Root"):
    """Prints the tree sideways (right subtree on top) to visualise its shape."""
    if node is None:
        return
    show_structure(node.right, depth + 1, "R")
    print("    " * depth + f"{label}:{node.key}")
    show_structure(node.left, depth + 1, "L")


def balanced_order(sorted_keys):
    """Insertion order that gives a balanced tree: middle element first, recursively."""
    order = []

    def add_middle(lo, hi):
        if lo > hi:
            return
        mid = (lo + hi) // 2
        order.append(sorted_keys[mid])
        add_middle(lo, mid - 1)
        add_middle(mid + 1, hi)

    add_middle(0, len(sorted_keys) - 1)
    return order


def build_tree(keys):
    """Builds a BST by inserting the given IDs in order."""
    tree = StudentBST()
    for key in keys:
        tree.insert(Student(key, f"Student {key}", "CSE"))
    return tree


def keys_of(students):
    return [s.enrolment_id for s in students]


def compare_insertion_orders(n=15):
    """Compares comparisons and height for balanced-looking vs sorted insertion."""
    sorted_keys = list(range(1, n + 1))
    balanced = balanced_order(sorted_keys)
    print(f"\nBalanced-looking insertion order: {balanced}")
    print(f"Sorted insertion order          : {sorted_keys}")
    print(f"\n{'Insertion order':<20}| {'Comparisons':>12} | {'Height':>6}")
    print("-" * 44)
    for label, order in (("Balanced-looking", balanced), ("Sorted (ascending)", sorted_keys)):
        tree = build_tree(order)
        print(f"{label:<20}| {tree.comparisons:>12} | {tree.height():>6}")


def read_int(prompt):
    try:
        return int(input(prompt).strip())
    except ValueError:
        print("  !! Please enter a valid whole number.")
        return None


def print_students(title, students):
    print(f"\n{title}")
    if not students:
        print("  (no records)")
    for s in students:
        print("  " + str(s))


def handle_insert(tree):
    eid = read_int("Enrolment ID: ")
    if eid is None:
        return
    name = input("Student name: ").strip()
    dept = input("Programme / Department: ").strip()
    if not name or not dept:
        print("  !! Name and department cannot be empty.")
        return
    if tree.insert(Student(eid, name, dept)):
        print(f"  Record {eid} inserted successfully.")
    else:
        print(f"  !! Duplicate rejected: enrolment ID {eid} already exists.")


def load_samples(tree):
    added = 0
    for eid, name, dept in SAMPLE_RECORDS:
        if tree.insert(Student(eid, name, dept)):
            added += 1
    print(f"  {added} sample record(s) loaded "
          f"({len(SAMPLE_RECORDS) - added} skipped as duplicates).")


# ----------------------------------------------------------------------
# Test cases
# ----------------------------------------------------------------------
def run_tests():
    print("\n=== Test Cases ===")
    results = []

    def check(title, expected, actual):
        ok = (expected == actual)
        results.append(ok)
        print(f"\nTest {len(results)}: {title}")
        print(f"  Expected : {expected}")
        print(f"  Actual   : {actual}")
        print(f"  Result   : {'PASS' if ok else 'FAIL'}")

    # Test 1: delete a LEAF node
    t = build_tree([50, 30, 70, 20, 40, 60, 80])
    deleted = t.delete(20)
    check("Delete a leaf node (20)",
          (True, [30, 40, 50, 60, 70, 80], CASE_LEAF),
          (deleted, keys_of(t.inorder()), t.last_delete_case))

    # Test 2: delete a node with ONE child
    t = build_tree([50, 30, 70, 20, 40, 60, 80, 15])   # 20 has only a left child (15)
    deleted = t.delete(20)
    check("Delete a node with one child (20)",
          (True, [15, 30, 40, 50, 60, 70, 80], CASE_ONE),
          (deleted, keys_of(t.inorder()), t.last_delete_case))

    # Test 3: delete a node with TWO children
    t = build_tree([50, 30, 70, 20, 40, 60, 80])
    deleted = t.delete(30)                              # children 20 and 40
    check("Delete a node with two children (30)",
          (True, [20, 40, 50, 60, 70, 80], CASE_TWO),
          (deleted, keys_of(t.inorder()), t.last_delete_case))

    # Test 4: duplicate enrolment ID is rejected
    t = build_tree([50, 30, 70])
    inserted = t.insert(Student(30, "Duplicate", "CSE"))
    check("Reject duplicate enrolment ID (30)",
          (False, 3),
          (inserted, t.size))

    # Test 5: successful search
    t = build_tree([50, 30, 70])
    s = t.search(70)
    check("Successful search (70)",
          "Student 70",
          s.name if s else None)

    # Test 6: unsuccessful search
    t = build_tree([50, 30, 70])
    s = t.search(99)
    check("Unsuccessful search (99)",
          None,
          s.name if s else None)

    # Test 7: inorder traversal gives sorted order
    t = build_tree([50, 30, 70, 20, 40, 60, 80])
    check("Inorder traversal is sorted",
          [20, 30, 40, 50, 60, 70, 80],
          keys_of(t.inorder()))

    print(f"\n{sum(results)}/{len(results)} test cases passed.")


# ----------------------------------------------------------------------
# Menu
# ----------------------------------------------------------------------
MENU = """
========  STUDENT RECORD SEARCH SYSTEM (BST)  ========
 1. Insert a student record
 2. Search by enrolment ID
 3. Delete a student record
 4. Display records - Inorder (sorted by ID)
 5. Display records - Preorder
 6. Display records - Postorder
 7. Show tree size, height and structure
 8. Load 10 sample records
 9. Compare balanced vs sorted insertion
10. Run test cases
 0. Exit
"""


def main():
    if "--test" in sys.argv:
        run_tests()
        return

    tree = StudentBST()
    while True:
        print(MENU)
        choice = input("Enter your choice: ").strip()

        if choice == "1":
            handle_insert(tree)

        elif choice == "2":
            eid = read_int("Enter enrolment ID to search: ")
            if eid is not None:
                before = tree.comparisons
                student = tree.search(eid)
                if student:
                    print(f"\nFound: {student}")
                else:
                    print(f"\nNo record found for enrolment ID {eid}.")
                print(f"Comparisons used: {tree.comparisons - before}")

        elif choice == "3":
            eid = read_int("Enter enrolment ID to delete: ")
            if eid is not None:
                if tree.delete(eid):
                    print(f"  Deleted record {eid}. Case used: {tree.last_delete_case}")
                else:
                    print(f"  No record with ID {eid} to delete.")

        elif choice == "4":
            print_students("Records in sorted order (Inorder):", tree.inorder())

        elif choice == "5":
            print_students("Preorder traversal:", tree.preorder())

        elif choice == "6":
            print_students("Postorder traversal:", tree.postorder())

        elif choice == "7":
            print(f"\nTotal records : {tree.size}")
            print(f"Tree height   : {tree.height()}")
            print("Tree structure (right subtree shown on top):")
            show_structure(tree.root)

        elif choice == "8":
            load_samples(tree)

        elif choice == "9":
            compare_insertion_orders()

        elif choice == "10":
            run_tests()

        elif choice == "0":
            print("Goodbye!")
            break

        else:
            print("Invalid choice, try again.")


if __name__ == "__main__":
    main()
