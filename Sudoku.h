#ifndef SUDOKO_H
#define SUDOKO_H

/*
Your name and Student id: A00961904 Eric Wu

Algorithm & data structure used
- dancing links
- cicular link list
- stack
- gauss elimination (sort of)

The sudoku problem can be seen as an exact cover. In my implementation, I frist create 
a 2 dimension array of size 729 * 324. 729 is the number of the all possible values in
a 9 * 9 sudoku cell. 324 is the total number of constraitns to fullfilled. The constraints
can be devided into 4 group - 
    1) each cell must have 1 and only one value                     81
    2) each row must consist of value from 1 - 9 and no duplicate   81 +
    3) each col must consist of value from 1 - 9 and no duplicate   81 +
    4) each box must consist of value from 1 - 9 and no duplicate   81 +
                                                                 =  324

To avoid the need to maintian a huge table of only few values (sparse matrix), I implemented
an alogrithm known as "dancing link" for the sudoku solver. 
The figure illustrate the data structure
root <-> [A] <-> [B] <-> [C] <-> root
          |       |       |
          |       |       |
          |       |       |
entry1   [1] <---------> [1]
          |       |       |
entry2    |      [1] <-> [1]
          |       |
entry3   [1] <-> [1]

Lastly, each of the constraint node are linked together in a circular fasion. The same
goes for nodes in each of the entry.
*/

#include <vector>
#include <stack>
#include <sstream>

// #include "lib/Node.h"
#include <iostream>
#include <string>
#include <unistd.h>

using namespace std;

template <typename T>
class Node
{
public:
    Node<T> *left = NULL;
    Node<T> *right = NULL;
    Node<T> *up = NULL;
    Node<T> *bottom = NULL;

    T data;

    void horiz_cover()
    {
        if (left != NULL)
            left->right = right;
        if (right != NULL)
            right->left = left;
    }

    void horiz_uncover()
    {
        if (left != NULL)
            left->right = this;
        if (right != NULL)
            right->left = this;
    }

    void vert_cover()
    {
        if (up != NULL)
            up->bottom = bottom;
        if (bottom != NULL)
            bottom->up = up;
    }

    void vert_uncover()
    {
        if (up != NULL)
            up->bottom = this;
        if (bottom != NULL)
            bottom->up = this;
    }

    Node(T data)
    {
        this->data = data;
    };
    ~Node(){};
};

typedef struct constraint Constraint;
typedef struct entry Entry;

struct entry
{
    int cell[2];
    int value;
    Node<Constraint> *col_header;
};
struct constraint : Entry
{
};

class SudokuSolver
{
private:
    bool solved = false;
    int puzzle[9][9]{};
    int puzzle_solution[9][9]{};
    stack<Node<Entry> *> solution_stack;

    // 81 (cell constraints) + 81 (row constraints) + 81 (col constraints) + 81 (block constraints) = 324
    Node<Constraint> *root;
    vector<Node<Constraint> *> col_headers;
    void _init_contraints_header()
    {
        root = new Node<Constraint>({});
        for (int i = 0; i < 324; i++)
            col_headers.push_back(new Node<Constraint>({}));
        for (int i = 0; i < 324 - 1; i++)
        {
            col_headers[i]->right = col_headers[i + 1];
            col_headers[i + 1]->left = col_headers[i];
        }

        //  root <-> [A] <-> [B] <-> [C] <-> [D] <-> root
        col_headers[324 - 1]->right = root;
        root->left = col_headers[324 - 1];
        root->right = col_headers[0];
        col_headers[0]->left = root;
    }

    // 9 (rows) * 9 (col) * 9 (1 - 9 values) = 729, 729 * 324
    void _init_entry_rows()
    {
        // Sparse matrix for showing constraitns each entry fullfilled
        Node<Entry> *matrix[729][324]{NULL};
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                for (int k = 0; k < 9; k++)
                {
                    Entry entry = {{i, j}, k + 1, NULL};
                    Node<Entry> *cell_node = new Node<Entry>(entry);
                    Node<Entry> *row_node = new Node<Entry>(entry);
                    Node<Entry> *col_node = new Node<Entry>(entry);
                    Node<Entry> *block_node = new Node<Entry>(entry);

                    matrix[i * 81 + j * 9 + k][0 + (i * 9 + j)] = cell_node;                    // cell constraints
                    matrix[i * 81 + j * 9 + k][81 + (i * 9 + k)] = row_node;                    // row constraints
                    matrix[i * 81 + j * 9 + k][162 + (j * 9 + k)] = col_node;                   // col constraints
                    matrix[i * 81 + j * 9 + k][243 + (i / 3 + j / 3 * 3) * 9 + k] = block_node; // block constraints

                    cell_node->right = row_node;
                    row_node->left = cell_node;
                    row_node->right = col_node;
                    col_node->left = row_node;
                    col_node->right = block_node;
                    block_node->left = col_node;
                    block_node->right = cell_node;
                    cell_node->left = block_node;
                }

        // Link entry to constraint header
        for (int i = 0; i < 324; i++)                                 // root <-> [A] <-> [B] <-> [C] <-> root
        {                                                             //
            Node<Constraint> *header = col_headers[i], *tmp = header; // entry1   [1] <---------> [1]
            for (int j = 0; j < 729; j++)                             //
            {                                                         // entry2           [1] <-> [1]
                if (matrix[j][i] != NULL)                             //
                {                                                     // entry3   [1] <-> [1]
                    matrix[j][i]->data.col_header = header;           //
                    tmp->bottom = (Node<Constraint> *)(matrix[j][i]);
                    matrix[j][i]->up = (Node<Entry> *)tmp;
                    tmp = (Node<Constraint> *)(matrix[j][i]);
                }
            }
        }
    }

    Node<Entry> *_find_entry(const int &i, const int &j, const int &value)
    {
        Node<Constraint> *col = root->right;
        Node<Entry> *node;
        while (col != root)
        {
            node = (Node<Entry> *)(col->bottom);
            if (node != NULL)

                while (node->bottom != NULL)
                {
                    if (node->data.cell[0] == i && node->data.cell[1] == j && node->data.value == value)
                        return node;
                    node = node->bottom;
                }
            col = col->right;
        }
        return NULL;
    }

    void _cover(Node<Constraint> *col)
    {
        col->horiz_cover();

        Node<Entry> *entry_node = (Node<Entry> *)(col->bottom);
        Node<Entry> *temp;
        while (entry_node != NULL)
        {
            temp = entry_node->right;
            for (int i = 0; i < 3; i++)
            {
                temp->vert_cover();
                temp = temp->right;
            }
            entry_node = entry_node->bottom;
        }
    }

    void _uncover(Node<Constraint> *col)
    {
        col->horiz_uncover();

        Node<Entry> *entry_node = (Node<Entry> *)(col->bottom);
        Node<Entry> *temp;
        while (entry_node != NULL)
        {
            temp = entry_node->right;
            for (int i = 0; i < 3; i++)
            {
                temp->vert_uncover();
                temp = temp->right;
            }
            entry_node = entry_node->bottom;
        }
    }

    void _remove_existed_entry()
    {
        Node<Entry> *node;
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                if (puzzle[i][j] != 0)
                {
                    if ((node = _find_entry(i, j, puzzle[i][j])) != NULL)
                    {
                        solution_stack.push(node);
                        for (int i = 0; i < 4; i++)
                        {
                            _cover(node->data.col_header);
                            node = node->right;
                        }
                    }
                    else
                        cout << "Error" << endl;
                }
    }

    Node<Constraint> *_constraint_with_min_entries()
    {
        int min_entries = 729; // arbitrary value
        Node<Constraint> *temp, *result = NULL;
        Node<Entry> *node;
        temp = root->right;
        while (temp != root)
        {
            int count = 0;
            node = (Node<Entry> *)temp->bottom;
            if (node != NULL)
            {
                while (node != NULL)
                {
                    count++;
                    node = node->bottom;
                }
                if (count < min_entries)
                {
                    min_entries = count;
                    result = temp;
                }
            }
            temp = temp->right;
        }
        return result;
    }

    void _apply_solution()
    {
        Node<Entry> *entry;
        stack<Node<Entry> *> stack = solution_stack;
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
            {
                puzzle_solution[i][j] = puzzle[i][j];
            }

        while (!stack.empty())
        {
            entry = stack.top();
            puzzle_solution[entry->data.cell[0]][entry->data.cell[1]] = entry->data.value;
            stack.pop();
        }
    }

public:
    bool Solve()
    {
        if (root->right == root)
        {
            _apply_solution();
            return true;
        }
        else
        {
            Node<Constraint> *col = _constraint_with_min_entries();
            Node<Entry> *node, *temp;

            // no entry left to select while there are still constraint exist
            if (col == NULL)
                return false;

            node = (Node<Entry> *)(col->bottom);
            while (node != NULL)
            {
                solution_stack.push(node);
                temp = node;
                for (int i = 0; i < 4; i++)
                {
                    _cover(temp->data.col_header);
                    temp = temp->right;
                }

                // check if solved
                // uncover the covered constraint and its associated node entries
                if (!(solved = Solve()))
                {
                    temp = node;
                    for (int i = 0; i < 4; i++)
                    {
                        _uncover(temp->data.col_header);
                        temp = temp->right;
                    }
                    solution_stack.pop();
                }
                node = node->bottom;
            }
            return solved;
        }
    }

    string Print()
    {
        ostringstream os;
        for (auto &i : puzzle_solution)
        {
            for (int j : i)
                os << j << "  ";
            os << endl;
        }
        return os.str();
    }

    explicit SudokuSolver(int (&p)[9][9])
    {
        for (int i = 0; i < 9; ++i)
            for (int j = 0; j < 9; ++j)
                puzzle[i][j] = p[i][j];

        _init_contraints_header();
        _init_entry_rows();
        _remove_existed_entry();
    }
    ~SudokuSolver(){};
};

#endif //SUDOKU_SOLVER_H