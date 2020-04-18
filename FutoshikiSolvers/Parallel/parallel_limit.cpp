#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <omp.h>

int domainRange;
bool solutionNotFound = true;
int counter;
int limit;

//USE THIS FUNCTION WHILE CHECKING FINAL RESULT
//YOU MAY FURTHER OPTIMIZE THIS FUNCTION TO USE FOR CALCULATION
bool solved(int** &matrix, std::vector<std::pair<std::pair<int, int>,std::pair<int,int> > > constraints, int size)
{
    for(int i = 0; i < constraints.size(); i++)
    {
        if(matrix[constraints[i].first.first][constraints[i].first.second] < matrix[constraints[i].second.first][constraints[i].second.second])
            return false;
    }


    std::vector<int> rows;
    std::vector<int> cols;
    for(int rc = 0; rc < size; rc++)
    {
        for(int s = 0; s < size; s++)
        {
            rows.push_back(matrix[rc][s]);
            cols.push_back(matrix[s][rc]);
        }

        std::sort(rows.begin(), rows.end());
        std::sort(cols.begin(), cols.end());

        if((rows[0] == -1) || (cols[0] == -1))
            return false;

        for(int i = 0; i < size-1; i++)
        {
            if((rows[i] == rows[i+1]) || (cols[i] == cols[i+1]))
            {
                return false;
            }
        }

        rows.clear();
        cols.clear();
    }

    return true;

}


void finalize(int*** &wholeMatrix, std::vector<std::pair<std::pair<int, int>,std::pair<int,int> > > &constraints)
{   // reformatting the matrix and checking if the solution is true
    int** matrix = new int*[domainRange];
    for(int i = 0; i < domainRange; i++)
    {
        matrix[i] = new int[domainRange];
        for(int j = 0; j < domainRange; j++)
        {
            matrix[i][j] = wholeMatrix[i][j][domainRange+1];
        }
    }

    // ONLY FOR DEBUG **** DELETE HERE AFTER DEBUG ****
    // checking if solution is true
    if(solved(matrix, constraints, domainRange))
    {
        for(int i = 0; i < domainRange; i++)
        {
            for(int j = 0; j < domainRange; j++)
            {
                std::cout << matrix[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }
    else{
        std::cout << ":)";
        for(int i = 0; i < domainRange; i++)
        {
            for(int j = 0; j < domainRange; j++)
            {
                std::cout << matrix[i][j] << " ";
            }
            std::cout << std::endl;
        }

    }
    // ONLY FOR DEBUG **** DELETE HERE AFTER DEBUG ****
    // checking if solution is true

}


bool solutionNextStep(int*** &wholeMatrix, int row, int col, std::vector<std::pair<std::pair<int, int>,std::pair<int,int> > > &constraints, int & unprocessedCell)
{   // pruning the matrix
    if(unprocessedCell == 0)  // if nothing left to process
    { 
        solutionNotFound = false;
        finalize(wholeMatrix, constraints);
    }
    else
    {
        int targetRow;
        int targetCol;
        int value = wholeMatrix[row][col][domainRange + 1];  // to quickly access the value of the cell

        // pruning for constraints
        for(int index = 0; index < constraints.size(); index ++)  // iterating through every cell in the constraints
        {  // first is bigger, second is smaller
            
            if(row == constraints[index].first.first && col == constraints[index].first.second)  // given cell is bigger
            {  // so need to prune smaller cell's possibilities

                for(int number = value; number <= domainRange; number++)  
                {  // eliminate all numbers between [focusValue - maxValue] from the target cells possibleValues, since they should be smaller than our focus cell

                    targetRow = constraints[index].second.first;
                    targetCol = constraints[index].second.second;
                    
                    if(wholeMatrix[targetRow][targetCol][number] != 0)  // reaching to the smaller cell's target value 
                    {  // if this possible value is currently available

                        wholeMatrix[targetRow][targetCol][number] = 0;  // make it unavailable
                        wholeMatrix[targetRow][targetCol][0]--;  // decrease the size of possibleValues

                        if (wholeMatrix[targetRow][targetCol][0] == 0)  // if any cells possible value amount drops to 0
                        {
                            return false;
                        }
                    }
                }
            } 

            else if(row == constraints[index].second.first && col == constraints[index].second.second)  // given cell is smaller
            {  // so need to prune bigger cell's possibilities

                for(int number = 1; number <= value; number++)  
                {  // eliminate all numbers between [1 - focusValue] from the target cells possibleValues, since they should be bigger than our focus cell

                    targetRow = constraints[index].first.first;
                    targetCol = constraints[index].first.second;
                    
                    if(wholeMatrix[targetRow][targetCol][number] != 0)  // reaching to the smaller cell's target value 
                    {  // if this possible value is currently available

                        wholeMatrix[targetRow][targetCol][number] = 0;  // make it unavailable
                        wholeMatrix[targetRow][targetCol][0]--;  // decrease the size of possibleValues

                        if (wholeMatrix[targetRow][targetCol][0] == 0)  // if any cells possible value amount drops to 0
                        {
                            return false;
                        }
                    }
                }
            } 
        }

        
        for(int index = 0; index < domainRange; index++)  // a number cannot be used again in the same row or column
        {
            // domain pruning on column
            if(wholeMatrix[row][index][value] != 0)  // if value to prune is still flagged as avalible
            {   
                wholeMatrix[row][index][value] = 0;  // set the flag of value to be pruned as not available
                wholeMatrix[row][index][0]--;  // decrease the total possible values by one for that cell

                if(wholeMatrix[row][index][0] == 0 && index != col)  // if any cells other than focus cells possible value amount drops to 0
                {  
                    return false; 
                }
            }
            
            
            // domain pruning on row
            if(wholeMatrix[index][col][value] != 0)  // if value to prune is still flagged as avalible
            {   
                wholeMatrix[index][col][value] = 0;  // set the flag of value to be pruned as not available
                wholeMatrix[index][col][0]--;  // decrease the total possible values by one for that cell
                
                if(wholeMatrix[index][col][0] == 0 && index != row)  // if any cells possible value amount drops to 0
                { 
                    return false;
                }
            }
            
        }
        wholeMatrix[row][col][0] += 1;  // it should preserve it's own possibility, otherwise algorithm does not work
        wholeMatrix[row][col][value] = 1;

        unprocessedCell--;

        for(int i = 0; i < domainRange; i++)
        {
            for(int j = 0; j < domainRange; j++)
            {
                if(wholeMatrix[i][j][0] == 1 && wholeMatrix[i][j][domainRange+1] == -1)  // if this cell has only 1 option left, no need to branch out for it
                {
                    for(int value = 1; value <= domainRange; value++ )  // iterating through it's possible values
                    {
                        if(wholeMatrix[i][j][value] == 1)  // we need to check if the flag is set to 1(available)
                        {
                            wholeMatrix[i][j][domainRange+1] = value;  // setting the only possible value to the cell
                            solutionNextStep(wholeMatrix, i, j, constraints, unprocessedCell);  // next pruning for this cell
                        }
                    }
                }
            }
        }
    }
    return true;
}



void initialization(int** &matrix, std::vector<std::pair<std::pair<int, int>,std::pair<int,int> > > &constraints, int*** &wholeMatrix, int &unprocessedCellCount)
{   // this function initializes the futoshiki instance
    unprocessedCellCount = domainRange*domainRange;  // counter for processed cells

    wholeMatrix = new int**[domainRange];  // creation of rows
    for(int i = 0; i < domainRange; i++)  // iteating through every row
    {
        wholeMatrix[i] = new int *[domainRange];  // creation of columns
        for(int j = 0; j < domainRange; j++)  // iteating through every row
        {
            wholeMatrix[i][j] = new int[domainRange + 2];  // creation of wholeMatrix, with the size being first index, cell value being the last index
            for(int k = 1; k <= domainRange; k++) 
                wholeMatrix[i][j][k] = 1;  // if the value is equal to 1, it means that index(k) is a possible value for that cell ([i][j])
            wholeMatrix[i][j][0] = domainRange;  // first index of the  list is the total number of possible values available
            wholeMatrix[i][j][domainRange+1] = matrix[i][j];  // last index of the list is the actual cell value
        }
    }


    for(int index = 0; index < constraints.size(); index ++)  // iterate through the constraints
    {
        // get the bigger cell from constraints
        if(wholeMatrix[constraints[index].first.first][constraints[index].first.second][1] == 1)  // multiple constraints can affect the same cell
        {
            wholeMatrix[constraints[index].first.first][constraints[index].first.second][1] = 0;  // make the bigger cell's smallest value unavailable
            wholeMatrix[constraints[index].first.first][constraints[index].first.second][0]--;  // reduce the total number of domains of bigger cell
        }


        // get the smaller cell from constraints
        if(wholeMatrix[constraints[index].second.first][constraints[index].second.second][domainRange] == 1)
        {
            wholeMatrix[constraints[index].second.first][constraints[index].second.second][domainRange] = 0;  // make the smaller cell's biggest value unavailable
            wholeMatrix[constraints[index].second.first][constraints[index].second.second][0]--;  // reduce the total number of domains of smaller cell
        }
    }

    for(int row = 0 ; row < domainRange ; row ++)
    { 
        for(int col = 0 ; col < domainRange ; col ++ )
        { 
            if(matrix[row][col] != -1){  // if the cell is not empty
                solutionNextStep(wholeMatrix, row, col, constraints, unprocessedCellCount);
            }
        }
    }
}


void findBestCandidate(int*** &wholeMatrix, std::vector<std::pair<std::pair<int, int>,std::pair<int,int> > > &constraints, int &unprocessedCellCount)
{   // pick the Cell that has the minumum domain number and biggest affect to its neighbours (by calculating it's relation to neighbours)
    int valueIndex = domainRange + 1;
    if(unprocessedCellCount == 0)  // if nothing left to process
    { 
        solutionNotFound = false;
        finalize(wholeMatrix, constraints);
    }
    else
    {
        int focusCol;  // to store the column index of the focus cell
        int focusRow;  // to store the row index of the focus cell
        int minSize = domainRange;  // to store the focus cell's possibility size
        std::vector<std::pair<int,int>> minDomain; 
        

        for(int row = 0; row < domainRange; row++)  // iterating through matrix
        {
            for(int col = 0; col < domainRange; col++)
            {
                if(wholeMatrix[row][col][valueIndex] == -1 && wholeMatrix[row][col][0] < minSize)  // if this cell is empty and new minimum
                {  // and it's the current minimum cell that has the smallest possibility set
                    minDomain.clear();  // clear the stack
                    minDomain.push_back(std::make_pair(row, col));  // insert the new minimum to the stack
                    minSize = wholeMatrix[row][col][0];  // update the current minimum value
                }
                else if(wholeMatrix[row][col][valueIndex] == -1 && wholeMatrix[row][col][0] == minSize)  // if this cell is empty and has same amount of possibilities
                {
                    minDomain.push_back(std::make_pair(row, col));  // if
                }
            }
        }

        int amount_relatedPossibleValues_sum = 2*domainRange*domainRange;  // domainRange x 2 (column and row for a cell),  x domainRange again (since every cell can have maximum of domainRange of possible values)
        int temp_amount_relatedPossibleValues_sum;  // temporary integer to store sum of the related possible values for a cell

        for(int index = 0; index < minDomain.size(); index++)  // iterate through all minDomain candidates
        {
            temp_amount_relatedPossibleValues_sum = 0;  // initializing the value for sum of the related possible values for that cell
            for(int i = 0; i < domainRange; i++)  // iterate through all other cells in the row and column
            {
                if(wholeMatrix[minDomain[index].first][i][domainRange+1] == -1)
                    temp_amount_relatedPossibleValues_sum += wholeMatrix[minDomain[index].first][i][0];  // all the empty cells in this column, sum their domain size (since this cell will affect them)
                if(wholeMatrix[minDomain[index].second][i][domainRange+1] == -1)
                    temp_amount_relatedPossibleValues_sum += wholeMatrix[i][minDomain[index].second][0];  // all the empty cells in this row, sum their domain size (since this cell will affect them)
            }

            if(temp_amount_relatedPossibleValues_sum < amount_relatedPossibleValues_sum)  // select the cell with the minimum sum, which has the maximum effect to the overall instance of futoshiki
            {
                amount_relatedPossibleValues_sum = temp_amount_relatedPossibleValues_sum;  // update the current minimum value
                focusRow = minDomain[index].first;  // get the position of the found cell
                focusCol = minDomain[index].second;
            }
        }



        for(int value = 1; value <= domainRange && solutionNotFound; value++ )  // to search which values are available
        {
            
            if(wholeMatrix[focusRow][focusCol][value] == 1)  // we need to check if the flag is set to 1(available)
            {
                //std::cout << "this is the value:" << value << std::endl;
                int*** newMatrix;
                int newUnprocessed = unprocessedCellCount;
                newMatrix = new int**[domainRange];  // creation of rows
                for(int i = 0; i < domainRange; i++)  // iteating through every row
                {
                    newMatrix[i] = new int *[domainRange];  // creation of columns
                    for(int j = 0; j < domainRange; j++)  // iteating through every row
                    {
                        newMatrix[i][j] = new int[domainRange + 2];  // creation of newMatrix, with the size being first index, cell value being the last index
                        for(int k = 0; k < domainRange + 2; k++) 
                            newMatrix[i][j][k] = wholeMatrix[i][j][k];  // if the value is equal to 1, it means that index(k) is a possible value for that cell ([i][j])
                    }
                }
            
                newMatrix[focusRow][focusCol][valueIndex] = value;  // setting this value to the cell
#pragma omp task if(counter < limit)
{
                counter++;
                if(solutionNextStep(newMatrix, focusRow, focusCol, constraints, newUnprocessed)){  // prune the matrix and check if creates a dead end
                    findBestCandidate(newMatrix, constraints, newUnprocessed);  // if no cell left to process
                }   
}
            }
        }       
    }  
}   


void solve(int** &matrix, std::vector<std::pair<std::pair<int, int>,std::pair<int,int> > > constraints, int size)
{
    counter = 0; // current amount of tasks
    int *** wholeMatrix;  // holy mADAFaka 
    int unprocessedCellCount;
    initialization(matrix, constraints, wholeMatrix, unprocessedCellCount); 

#pragma omp parallel
    {
    #pragma omp single
        {
    limit = omp_get_num_threads();
    std::cout << "***************" << std::endl << "TOTAL THREAD NUM: " << limit << std::endl << std::endl;
    findBestCandidate(wholeMatrix, constraints, unprocessedCellCount);
        }
    }

}



//DON'T CHANGE THIS FUNCTION
void get_constraints(std::vector<std::pair<std::pair<int,int>,std::pair<int,int> > > &constraints, std::ifstream &file)
{
    int ctr = 1;
    std::string constraint;
    while(!file.eof())
    {
        std::getline(file, constraint);
        if(constraint != "")
        {
            std::cout << "Constraint " << ctr++ << ": " << constraint << std::endl;
            std::stringstream ss(constraint);
            int val1, val2, val3, val4; // Coordinate (val1, val2) > (val3, val4)
            ss >> val1 >> val2 >> val3 >> val4;
            constraints.push_back(std::pair<std::pair<int,int>, std::pair<int,int> >(std::pair<int,int>(val1-1,val2-1),std::pair<int,int>(val3-1,val4-1)));
        }
    }
}


//DON'T CHANGE THIS FUNCTION
void read_matrix(int** &matrix, std::ifstream &file, int size)
{
    matrix = new int*[size];

    for(int i = 0; i < size; i++)
    {
        matrix[i] = new int[size];
    }

    int val;
    for(int i = 0; i < size; i++)
    {
        for(int j = 0; j < size; j++)
        {
            file >> val;
            matrix[i][j] = val;
        }
    }
}



int main(int argc, char** argv)
{
    std::string filename(argv[1]);

    std::ifstream file;
    file.open(filename.c_str());


    file >> domainRange;
    std::cout << "Size: " << domainRange << std::endl;

    int** matrix;

    read_matrix(matrix, file, domainRange);

    std::vector<std::pair<std::pair<int,int>,std::pair<int,int> > > constraints;
    get_constraints(constraints, file);

    for(int i = 0 ; i < domainRange; i++)
    {
        for(int j = 0; j < domainRange; j++)
        {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }

    //Measure only solution function's execution time

    double start = omp_get_wtime();
    solve(matrix, constraints, domainRange);
    double duration = omp_get_wtime() - start;
    std::cout << "Time: " << duration << std::endl;
    

    //DELETE//
    for(int i = 0; i < domainRange; i++)
    {
        delete matrix[i];
    }

    delete[] matrix;
    //DELETE//

    return 0;
}
