#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <omp.h>

class Cell
{
    public:

    int value;
    std::pair<int,int> position;  // position of the cell
    std::vector<int> domain ; //first index is the size of domain 
    std::vector<Cell*> biggerThan;  // list of cells that are smaller than this
    std::vector<Cell*> smallerThan;  // list of cells that are bigger than this
    std::vector<Cell*> column;  // list of cells that are in the same column with this
    std::vector<Cell*> row;  // list of cells that are in the same row with this


    Cell(int size, int val, std::pair<int,int> pos)
    {
        domain.push_back(size);  // now we have the size (range of possible values) stored in memory
        for(int i = 1; i <= size ; i ++)
            domain.push_back(1);  // we are going to store 1 for each domain candidate
            
        value = val;  // actual value of the cell
        position = pos;  // position of the cell, stored as pair
    }
};

class Puzzle
{
    public:

    int domainRange;
    int unprocessedCellCount;
    std::vector<std::vector< Cell > > CellMap;
    std::vector<std::pair<int,int> > emptyCells;
    bool deadEnd;

    void solutionNextStep(Cell &focus, std::vector<Cell>& modifiedCells, bool forward)
    {
        if(forward)
        {
            modifiedCells.clear();  // we are in a safe state, reset the modified cells memory

            // cells that are related to our focus cell, we should prune their domains with respect to our focus cell's value

            // for the smaller Cells
            for(int index = 0; index < focus.biggerThan.size(); index ++)  // iterating through every cell in the biggerThan list
            {   
                Cell temp = *focus.biggerThan[index];
                modifiedCells.push_back(temp);

                for(int number = focus.value; number <= domainRange; number ++)  // eliminate all numbers between [focusValue - maxValue] from the target cells possibleValues, since they should be smaller than our focus cell
                {

                    if(temp.domain[number] != 0)  // if this possible value is currently available
                    {
                        focus.biggerThan[index]->domain[number] = 0;  // make it unavailable
                        focus.biggerThan[index]->domain[0]--;  // decrease the size of possibleValues

                        if (focus.biggerThan[index]->domain[0] == 0)  // if any cells possible value amount drops to 0
                        {
                          deadEnd = true; 
                          return;
                        }
                    }
                }
            }
        
            // for the bigger Cells
            for(int index = 0; index < focus.smallerThan.size(); index ++)  // iterating through every cell in the smallerThan list
            {
                Cell temp = *focus.smallerThan[index];
                modifiedCells.push_back(temp);
                for(int number = 1; number <= focus.value; number ++) // eliminate all numbers between [1 - focusValue] from the target cells possibleValues, since they should be greater than our focus cell
                {
                    if(temp.domain[number] != 0 )  // if this possible value is currently available
                    {
                        focus.smallerThan[index]->domain[number] = 0;  // make it unavailable
                        focus.smallerThan[index]->domain[0]--;  // decrease the size of possibleValues

                        if(focus.smallerThan[index]->domain[0] == 0)  // if any cells possible value amount drops to 0
                        {
                          deadEnd = true;  
                          return;
                        }
                    }

                }
            }


            // a number cannot be used again in the same row or column 
            for(int index = 0; index < focus.row.size(); index++) 
            {
                // domain pruning on column
                if(focus.row[index]->domain[focus.value] != 0)  // if value to prune is still flagged as avalible
                {   
                    Cell temp = *focus.row[index];  // create a copy of the "to be modified" cell
                    modifiedCells.push_back(temp);  // store the backup in the modifiedCells
                    focus.row[index]->domain[focus.value] = 0;  // set the flag of value to be pruned as not available
                    focus.row[index]->domain[0]--;  // decrease the total possible values by one for that cell

                    if(focus.row[index]->domain[0] == 0)  // if any cells possible value amount drops to 0
                    {
                        deadEnd = true;  
                        return; 
                    }
                }
                
                // domain pruning on row
                if(focus.column[index]->domain[focus.value] != 0)  // if value to prune is still flagged as avalible
                {   
                    Cell temp = *focus.column[index];  // create a copy of the "to be modified" cell
                    modifiedCells.push_back(temp);  // store the backup in the modifiedCells
                    focus.column[index]->domain[focus.value] = 0;  // set the flag of value to be pruned as not available
                    focus.column[index]->domain[0]--;  // decrease the total possible values by one for that cell
                    
                    if(focus.column[index]->domain[0] == 0)  // if any cells possible value amount drops to 0
                    {
                        deadEnd = true;  
                        return;
                    }
                }
            }

        }

        else  // we need to roll-back
        {
            deadEnd = false;  // clear the dead end flag
            for(int index = 0; index < modifiedCells.size(); index++)  // roll-back modified cells to previous state
            {
                CellMap[modifiedCells[index].position.first][modifiedCells[index].position.second] = modifiedCells[index];
            }
        }
    }

    // this function initializes the futoshiki instance, fills the board with Cells and links the cells with respective relations
    void create_CellMap(int** &matrix, std::vector<std::pair<std::pair<int, int>,std::pair<int,int> > > constraints, int size)
    {
      domainRange = size;  // getting domain range
      unprocessedCellCount = domainRange*domainRange;  // counter for processed cells
      deadEnd = false;  // flag for dead end

      std::vector<std::pair<int,int> > initial_Cells;  // empty vector to hold initial filled cells
      std::vector<Cell> tempCellList;  // temporary list to hold cells, it's not important

      // initializing the CellMap 
      for(int row = 0 ; row < domainRange ; row ++)
      { 
        tempCellList.clear();  // clear the dummy before the inner loop
        for(int col = 0 ; col < domainRange ; col ++ )
        { 
          Cell new_Cell(domainRange, matrix[row][col], std::make_pair(row,col));  // creating a new cell
          tempCellList.push_back(new_Cell);  // temporarily store it into dummy
          
          if(matrix[row][col] != -1)
            initial_Cells.push_back(std::make_pair(row,col));  // if it's a filled cell, store it in the initial

          else
            emptyCells.push_back(std::make_pair(row, col));  // if it's an empty cell, store it in the empty
        } 
      
        CellMap.push_back(tempCellList);  // store all cells in the CellMap
      }
      // end of initiazation

      for(int index = 0; index < constraints.size(); index ++)
      {
        // get the bigger and the smaller cell from constraints
        Cell* bigger = & CellMap[constraints[index].first.first][constraints[index].first.second]; 
        Cell* smaller = & CellMap[constraints[index].second.first][constraints[index].second.second];
        
        bigger->biggerThan.push_back(smaller);            
        if(bigger->domain[1] != 0)  // if the bigger cell's possible min value, is not set as 0(unavailable)
        {
            bigger->domain[1] = 0;  // make it unavalible, since bigger cell can't have 0 as value
            bigger->domain[0]--;  // reduce the total number of domains of bigger cell
        }


        smaller->smallerThan.push_back(bigger);

        if(smaller->domain[domainRange] != 0)  // if the smaller cell's possible max value, is not set as 0(unavailable)
        {
            smaller->domain[domainRange] = 0;  // make it unavalible, since bigger cell can't have the max value
            smaller->domain[0]--;  // reduce the total number of domains of smaller cell
        }
      }

      // binding nodes to each other
      for(int r = 0; r < domainRange; r ++)  // to iterate through every single element in the matrix
      {
          for(int c = 0; c < domainRange; c ++)  // to iterate through every single element in the matrix
          {
              for(int row = 0; row < domainRange; row++)  // to iterate through every single element in the row of that cell
                  if(row != r)
                      CellMap[r][c].column.push_back(&CellMap[row][c]);  // establishing column link between cells

              for(int col = 0; col < domainRange; col++)  // to iterate through every single element in the column of that cell
                  if(col != c)
                      CellMap[r][c].row.push_back(&CellMap[r][col]);  // establishing row link between cells
          }
      }

      
      for(int index = 0; index < initial_Cells.size(); index ++)
      {
          solutionNextStep(CellMap[initial_Cells[index].first][initial_Cells[index].second], tempCellList, true);  // 1st parameter: the cell we are focusing on, 2nd parameter: a temporary list (we are going to clear it anyway, so it's not important), 3rd parameter: necessary bool control for backtracking
          unprocessedCellCount--;  // we processed this cell, thus reduce from from unprocessed cells
      }

    }

    // finalize the algorithm by filling the actual matrix by the values we have found in CellMap
    void matrixCallback(int** &matrix)
    {
        for(int row = 0 ; row < domainRange ; row ++)
            for(int col = 0 ; col < domainRange ; col ++ )
                matrix[row][col] = CellMap[row][col].value;
    }

    // pick the Cell that has the minumum domain number and biggest affect to its neighbours (by calculating it's relation to neighbours)
    bool findBestCandidate()
    {
        if(unprocessedCellCount == 0)  // if nothing left to process
        { 
            return true;  // means we are done
        }
        else 
        {
            std::vector<int> possible_values;  // local copy for possible values
            std::vector<int> indices;  // local variable for storing the indices of found cells
            std::vector<Cell> modified_Cells;  // local copy for modified Cells
            std::vector<std::pair<int,int> > backup;  // backup for emptyCells, in case of we reach a dead-end and have to roll-back
            std::vector<std::pair<int,int> > minDomain;  // to store the cells, that has the minimum amount of possible values
            
            
            minDomain.push_back(CellMap[emptyCells[0].first][emptyCells[0].second].position);  // initializing minDomain for the comparison at line 290
            indices.push_back(0);  // storing the indices of the Cells in the minDomain

            for(int index = 1; index < emptyCells.size(); index++)  // iterating through all the emptyCells
            {
                if (CellMap[emptyCells[index].first][emptyCells[index].second].domain[0] == CellMap[minDomain[0].first][minDomain[0].second].domain[0] )  // if the domain size of the new cell is equal to the minimum domain amount in our minDomain list
                {
                    minDomain.push_back(CellMap[emptyCells[index].first][emptyCells[index].second].position);  // store this cell in the minDomain queue aswell
                    indices.push_back(index);  // store it's index
                    
                }

                else if(CellMap[emptyCells[index].first][emptyCells[index].second].domain[0] < CellMap[minDomain[0].first][minDomain[0].second].domain[0] )  // if we found a new minimum domain size
                {
                    minDomain.clear();  // reset the queue
                    indices.clear();  // reset the indices aswell
                    minDomain.push_back(CellMap[emptyCells[index].first][emptyCells[index].second].position);  // insert the new minimum into the queue
                    indices.push_back(index);  // store the new minimum cell's index 
                }
            }

            int amount_relatedPossibleValues_sum = 2*domainRange*domainRange;  // domainRange x 2 (column and row for a cell),  x domainRange again (since every cell can have maximum of domainRange of possible values)
            int temp_amount_relatedPossibleValues_sum;  // temporary integer to store sum of the related possible values for a cell
            std::pair<int,int> focusCell_pos;  // to store the to be found cell's position
            int focusCell_index;  // to store it's index
            

            for(int index = 0; index < minDomain.size(); index++)  // iterate through all minDomain candidates
            {
                Cell* tempCell = &CellMap[minDomain[index].first][minDomain[index].second];  // temporary cell to store current minDomain candidate
                temp_amount_relatedPossibleValues_sum = 0;  // initializing the value for sum of the related possible values for that cell
                for(int i = 0; i< tempCell->column.size();i++)  // iterate through all other cells in the row and column
                {
                    temp_amount_relatedPossibleValues_sum += tempCell->column[i]->domain[0];  // all the empty cells in this column, sum their domain size (since this cell will affect them)
                    temp_amount_relatedPossibleValues_sum += tempCell->row[i]->domain[0];  // all the empty cells in this row, sum their domain size (since this cell will affect them)
                }

                if(temp_amount_relatedPossibleValues_sum < amount_relatedPossibleValues_sum)  // select the cell with the minimum sum, which has the maximum effect to the overall instance of futoshiki
                {
                    amount_relatedPossibleValues_sum = temp_amount_relatedPossibleValues_sum;  // update the current minimum value
                    focusCell_pos = minDomain[index];  // store the position of the found cell
                    focusCell_index = indices[index];  // store the index of the found cell
                }
            }


            Cell *focus = &CellMap[focusCell_pos.first][focusCell_pos.second];  // selecting the cell with the minimum power
            backup = emptyCells;  // store the emptyCells, because we are going to modify it

            possible_values = focus->domain;  // get the possible values
            emptyCells.erase(emptyCells.begin() + focusCell_index);

            for(int index = 1; index < possible_values.size(); index++ )  // since possible_values[0] is the size of the domain (remember domain in the Cell Class)
            {
                if(possible_values[index] == 1)  // remember we got possible values from the domain, so we need to check if the flas is set to 1(available)
                {
                    modified_Cells.clear();
                    focus->value = index;  // try the first value
                    
                    solutionNextStep(*focus, modified_Cells, true);  // continue to the puzzle
                    unprocessedCellCount--;  // assume we have processed this cell

                    if(!deadEnd)  // if no dead end
                        if(findBestCandidate())  // if no cell left to process
                            return true;  // means we are done

                    solutionNextStep(*focus, modified_Cells, false);  // if not, reverse the last step
                    
                    focus->value = -1;  // set the cell as empty
                    unprocessedCellCount++;  // increase the unprocessed cell count
                }
                
            }

            emptyCells = backup;  // reverse emptyCells from the backup 
            return false;  // return false for roll-back (line 341)
                  
        }  
    }   
};

void solve(int** &matrix, std::vector<std::pair<std::pair<int, int>,std::pair<int,int> > > constraints, int size)
{
    Puzzle obj;
    obj.create_CellMap(matrix,constraints, size);  // YOUR
    std::cout << "cellMap created" << std::endl;

    obj.findBestCandidate();  // MOM
    std::cout << "min_max function done" << std::endl;

    obj.matrixCallback(matrix);  // XDDDD
    std::cout << "callback done" << std::endl;
}


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
    int size;

    file >> size;
    std::cout << "Size: " << size << std::endl;

    int** matrix;

    read_matrix(matrix, file, size);

    std::vector<std::pair<std::pair<int,int>,std::pair<int,int> > > constraints;
    get_constraints(constraints, file);

    for(int i = 0 ; i < size; i++)
    {
        for(int j = 0; j < size; j++)
        {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }

    //Measure only solution function's execution time

    double start = omp_get_wtime();
    solve(matrix, constraints, size);
    double duration = omp_get_wtime() - start;
    std::cout << "Time: " << duration << std::endl;
    std::cout << "True: " << solved(matrix, constraints, size) << std::endl;
    
    for(int i = 0 ; i < size; i++)
    {
        for(int j = 0; j < size; j++)
        {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }

    //DELETE//
    for(int i = 0; i < size; i++)
    {
        delete matrix[i];
    }

    delete[] matrix;
    //DELETE//

    return 0;
}