// Test code that combines all your implementation from Questions 1 to 3 in Assignment 2
// compile and run with:
//          g++ -std=c++11 COMP8042A2Test.cpp -o COMP8042A2Test && ./COMP8042A2Test
#include <iostream>
#include <vector>
#include "Graph.h"
#include "CuckooHashing.h"
#include "Sudoku.h"

using namespace std;

int main()
{
    int sudoku[9][9] = {
        {8, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 3, 6, 0, 0, 0, 0, 0},
        {0, 7, 0, 0, 9, 0, 2, 0, 0},
        {0, 5, 0, 0, 0, 7, 0, 0, 0},
        {0, 0, 0, 0, 4, 5, 7, 0, 0},
        {0, 0, 0, 1, 0, 0, 0, 3, 0},
        {0, 0, 1, 0, 0, 0, 0, 6, 8},
        {0, 0, 8, 5, 0, 0, 0, 1, 0},
        {0, 9, 0, 0, 0, 0, 4, 0, 0}};

    SudokuSolver solver(sudoku);
    if (solver.Solve())
        cout << solver.Print();
    else
        cout << "Solution Does Not Exist!" << endl;
    return 0;
}
