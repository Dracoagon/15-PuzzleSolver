// Drake Taylor, CS 482 - 1001, HW2 - Program to solve a given 15-puzzle.
//

#include <iostream>
#include <iomanip>
#include <cmath>
#include <unordered_map>
#include <fstream>

using namespace std;

int stepCounter = 1; //global used while printing solution

enum class direction {LEFT, RIGHT, UP, DOWN, NA}; //increase readability when selecting movement directions. NA is used as a placeholder to denote no direction.

struct Point { //struct to store a point on a grid
    int x;
    int y;
};

class PuzzleGrid { //class that stores all relevant game information
    int grid[4][4]; //the puzzle board.
    Point active_piece; //keeps track of where the blank is so we don't have to constantly search for it
    direction lastMove; //keeps track of the last move made to prevent redundant pairs of move (ex: up into down is illegal)
    int heuristicVal; //the heuristic value of the current board
public:
    PuzzleGrid();
    PuzzleGrid(int is[4][4]);
    PuzzleGrid(PuzzleGrid p, direction d);
    void operator=(const PuzzleGrid& p);
    bool operator==(const PuzzleGrid& p) const;
    void PrintGrid(ofstream& ofile);
    Point getCursor();
    int getCell(int x, int y);
    int getHeuristic();
    bool checkMove(direction d);
    bool move(direction d);
private:
    int calculateHeuristic();
    bool moveLeft();
    bool moveRight();
    bool moveUp();
    bool moveDown();
};

//blank constructor for puzzle grid. Begins grid in goal state.
PuzzleGrid::PuzzleGrid() {
    grid[0][0] = 1;
    grid[0][1] = 2;
    grid[0][2] = 3;
    grid[0][3] = 4;
    grid[1][0] = 5;
    grid[1][1] = 6;
    grid[1][2] = 7;
    grid[1][3] = 8;
    grid[2][0] = 9;
    grid[2][1] = 10;
    grid[2][2] = 11;
    grid[2][3] = 12;
    grid[3][0] = 13;
    grid[3][1] = 14;
    grid[3][2] = 15;
    grid[3][3] = 16;
    active_piece.x = 3;
    active_piece.y = 3;
    lastMove = direction::NA;
    heuristicVal = 0;
}

//constructor for puzzle grid using an initial state. Assumes initial state is a legal board.
PuzzleGrid::PuzzleGrid(int is[4][4]) {
    active_piece.x = 0;
    active_piece.y = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            grid[i][j] = is[i][j];
            //set active piece to wherever the blank is
            if (is[i][j] == 16) {
                active_piece.x = i;
                active_piece.y = j;
            }
        }
    }
    lastMove = direction::NA;
    heuristicVal = calculateHeuristic();
}

//construct a copy of the puzzle grid and then move in a direction. Assumes given move is legal.
PuzzleGrid::PuzzleGrid(PuzzleGrid p, direction d) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            this->grid[i][j] = p.getCell(i, j);
        }
    }
    this->active_piece.x = p.active_piece.x;
    this->active_piece.y = p.active_piece.y;
    this->heuristicVal = p.getHeuristic();
    this->move(d);
}

//return the value of a piece at the given location. Used in above constructor.
int PuzzleGrid::getCell(int x, int y) {
    return grid[x][y];
}

//Overloaded = definition for PuzzleGrid objects
void PuzzleGrid::operator=(const PuzzleGrid& p) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            this->grid[i][j] = p.grid[i][j];
        }
    }
    this->heuristicVal = p.heuristicVal;
    this->lastMove = p.lastMove;
    this->active_piece.x = p.active_piece.x;
    this->active_piece.y = p.active_piece.y;
}

//Overloaded == definition for PuzzleGrid objects. For our purposes, only the contents of the grid matter for this.
bool PuzzleGrid::operator==(const PuzzleGrid& p) const {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (this->grid[i][j] != p.grid[i][j]) {
                return false;
            }
        }
    }
    return true;
}

//prints current grid configuration onto command line
void PuzzleGrid::PrintGrid(ofstream& ofile) {
    ofile << "Step " << stepCounter << ": ";
    if (lastMove == direction::DOWN) {
        ofile << "Move Down" << endl;
    }
    if (lastMove == direction::LEFT) {
        ofile << "Move Left" << endl;
    }
    if (lastMove == direction::RIGHT) {
        ofile << "Move Right" << endl;
    }
    if (lastMove == direction::UP) {
        ofile << "Move Up" << endl;
    }
    if (lastMove == direction::NA) {
        ofile << "Initial State" << endl;
    }
    ofile << "------------------------------" << endl;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (grid[i][j] == 16) {
                ofile << "| [] |\t";
            }
            else {
                ofile << "| " << grid[i][j] << " |\t";
            }
        }
        ofile << endl;
    }
    ofile << "------------------------------" << endl << endl;
    stepCounter++;
}


//get position of the blank piece
Point PuzzleGrid::getCursor() {
    return active_piece;
}

//get heuristic value of current board
int PuzzleGrid::getHeuristic() {
    return heuristicVal;
}


/*
calculate the heuristic value for the current board.
heuristic value = sum of number of moves each piece is from goal position, with pieces whose
goal position is on a higher row weighted higher. 
*/
int PuzzleGrid::calculateHeuristic() {
    int goalVal = 1;
    int weight = 4; //weight of highest-row tiles = 4 per move
    int newHeuristic = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            //check if the piece is in its goal position
            if (grid[i][j] != goalVal) {
                int m = 0, n = 0;
                int dist = 0;
                //if not, find where it is
                while (grid[m][n] != goalVal) {
                    n++;
                    //we have to make sure we dont skip tiles on the right edge
                    if (n == 3 && grid[m][n] != goalVal) {
                        m++;
                        n = 0;
                    }
                }
                //add the distance from the pieces current position to its goal position to heuristic value
                //multiplying by weight as necessary
                newHeuristic += (abs(i - m) + abs(j - n)) * weight;
            }
            goalVal++;
        }
        //decrease weight of pieces as we move down rows
        weight--;
    }
    return newHeuristic;
}

//check if a move is legal without commiting the move.
//call with d = the direction you want to move
bool PuzzleGrid::checkMove(direction d) {
    //check legality by seeing if piece would move off the board or undo the last move
    if (d == direction::DOWN && (active_piece.x == 3 || lastMove == direction::UP)) {
        return false;
    }
    if (d == direction::UP && (active_piece.x == 0 || lastMove == direction::DOWN)) {
        return false;
    }
    if (d == direction::LEFT && (active_piece.y == 0 || lastMove == direction::RIGHT)) {
        return false;
    }
    if (d == direction::RIGHT && (active_piece.y == 3 || lastMove == direction::LEFT)) {
        return false;
    }
    //NA isn't a real direction, so always return false.
    if (d == direction::NA) {
        return false;
    }
    return true;
}

//move in a specified direction
bool PuzzleGrid::move(direction d) {
    if (d == direction::DOWN) {
        //if the move fails, return a failure.
        if (moveDown() == false) {
            return false;
        }
    }
    if (d == direction::UP) {
        if (moveUp() == false) {
            return false;
        }
    }
    if (d == direction::LEFT) {
        if (moveLeft() == false) {
            return false;
        }
    }
    if (d == direction::RIGHT) {
        if (moveRight() == false) {
            return false;
        }
    }
    //NA isn't a real direction, so always fail.
    if (d == direction::NA) {
        return false;
    }
    return true;
}

//moves the blank to the left.
bool PuzzleGrid::moveLeft() {
    //return that this is an illegal move if the last move was right or if blank is on leftmost edge
    if (lastMove == direction::RIGHT || active_piece.y == 0) {
        return false;
    }
    else {
        //swap blank with piece at target destination
        int temp = grid[active_piece.x][active_piece.y-1];
        grid[active_piece.x][active_piece.y-1] = grid[active_piece.x][active_piece.y];
        grid[active_piece.x][active_piece.y] = temp;
        //update values accordingly
        lastMove = direction::LEFT;
        active_piece.y = active_piece.y - 1;
        heuristicVal = calculateHeuristic();
        return true;
    }
}

//moves the blank to the right.
bool PuzzleGrid::moveRight() {
    //return that this is an illegal move if the last move was left or if blank is on rightmost edge
    if (lastMove == direction::LEFT || active_piece.y == 3) {
        return false;
    }
    else {
        int temp = grid[active_piece.x][active_piece.y + 1];
        grid[active_piece.x][active_piece.y + 1] = grid[active_piece.x][active_piece.y];
        grid[active_piece.x][active_piece.y] = temp;
        lastMove = direction::RIGHT;
        active_piece.y = active_piece.y + 1;
        heuristicVal = calculateHeuristic();
        return true;
    }
}

//moves the blank up.
bool PuzzleGrid::moveUp() {
    //return that this is an illegal move if the last move was down or if blank is on uppermost edge
    if (lastMove == direction::DOWN || active_piece.x == 0) {
        return false;
    }
    else {
        int temp = grid[active_piece.x-1][active_piece.y];
        grid[active_piece.x-1][active_piece.y] = grid[active_piece.x][active_piece.y];
        grid[active_piece.x][active_piece.y] = temp;
        lastMove = direction::UP;
        active_piece.x = active_piece.x - 1;
        heuristicVal = calculateHeuristic();
        return true;
    }
}

//moves the blank down.
bool PuzzleGrid::moveDown() {
    //return that this is an illegal move if the last move was down or if blank is on uppermost edge
    if (lastMove == direction::UP || active_piece.x == 3) {
        return false;
    }
    else {
        int temp = grid[active_piece.x + 1][active_piece.y];
        grid[active_piece.x + 1][active_piece.y] = grid[active_piece.x][active_piece.y];
        grid[active_piece.x][active_piece.y] = temp;
        lastMove = direction::DOWN;
        active_piece.x = active_piece.x + 1;
        heuristicVal = calculateHeuristic();
        return true;
    }
}

struct searchNode { // a node on the search tree
    PuzzleGrid puzzle;
    int parentName = -1;
    int depth = 0;
    int myHval = 0; // more easily accessible variable for the puzzle grid's heuristic value
    int nodeName = 0;
    bool operator==(const searchNode sn) const { //comparison operator for search nodes. Used for compatability with unordered_map
    //since each node will have a unique name, we can simply check if the names are equal to determint node equality.
        if (this->nodeName == sn.nodeName) {
            return true;
        }
        else {
            return false;
        }
    }
    void operator=(const searchNode& sn) { //equality operator. Default was causing issues so I added an explicit definition here.
        this->depth = sn.depth;
        this->myHval = sn.myHval;
        this->puzzle = sn.puzzle;
        this->nodeName = sn.nodeName;
        this->parentName = sn.parentName;
    }
};

class searchNodeHash { //a very simple hash function for our search nodes. Since no two nodes will have the same name, we can simply use the name as a hash.
public:
    size_t operator()(const searchNode& sn) const {
        return sn.nodeName;
    }
};


class AStarSearch { //class that performs an A* search.
    unordered_map<int, searchNode> openSet; //set of nodes that can be expanded
    unordered_map<int, searchNode> closedSet; //set of nodes that have been expanded
    searchNode activeNode; //node being considered for expansion
    int nodeNumber; //give a unique number to each node
public:
    AStarSearch();
    AStarSearch(int is[4][4]);
    void expandActiveNode();
    void selectNode();
    bool getSolvedStatus();
    void printSolution(searchNode sn, ofstream& ofile);
    void printTechnicalData(ofstream& ofile);
    searchNode getActiveNode();
};

//Construct an A* search tree assuming the initial state = goal state. Mainly for debugging.
AStarSearch::AStarSearch() {
    nodeNumber = 0;
    PuzzleGrid p;
    activeNode.puzzle = p;
    activeNode.depth = 0;
    activeNode.myHval = activeNode.puzzle.getHeuristic();
    activeNode.parentName = -1; //use parent name = -1 to mark the root node
    activeNode.nodeName = nodeNumber;
    openSet.insert(make_pair(activeNode.nodeName, activeNode));
}

//Begin an A* search tree with a specified initial state, given as a 4x4 integer array.
AStarSearch::AStarSearch(int is[4][4]) {
    nodeNumber = 0;
    PuzzleGrid p(is);
    activeNode.puzzle = p;
    activeNode.depth = 0;
    activeNode.myHval = activeNode.puzzle.getHeuristic();
    activeNode.parentName = -1; 
    activeNode.nodeName = nodeNumber;
    openSet.insert(make_pair(activeNode.nodeName, activeNode));
}

//Expand the currently selected node
void AStarSearch::expandActiveNode() {
    int parentNode = activeNode.nodeName; //mark the current node as the parent for all children produced in this step
    closedSet.insert(make_pair(activeNode.nodeName, activeNode)); //since the node is goind to be expanded, we can call it closed
    openSet.erase(activeNode.nodeName); //since the node is now closed, remove it from the open set.
    /* check what moves are possible. 
        For every possible move, create a new node corresponding to it and add it to the open set.
    */
    if (activeNode.puzzle.checkMove(direction::DOWN)) {
        nodeNumber++; //give the produced node a new name
        searchNode s1;
        PuzzleGrid p1(activeNode.puzzle, direction::DOWN);
        s1.puzzle = p1;
        s1.parentName = parentNode;
        s1.depth = activeNode.depth + 1;
        s1.myHval = s1.puzzle.getHeuristic();
        s1.nodeName = nodeNumber;
        openSet.insert(make_pair(s1.nodeName, s1)); //insert newly produced node into open set
    }
    if (activeNode.puzzle.checkMove(direction::LEFT)) {
        nodeNumber++;
        searchNode s2;
        PuzzleGrid p2(activeNode.puzzle, direction::LEFT);
        s2.puzzle = p2;
        s2.parentName = parentNode;
        s2.depth = activeNode.depth + 1;
        s2.myHval = s2.puzzle.getHeuristic();
        s2.nodeName = nodeNumber;
        openSet.insert(make_pair(s2.nodeName, s2));
    }
    if (activeNode.puzzle.checkMove(direction::RIGHT)) {
        nodeNumber++;
        searchNode s3;
        PuzzleGrid p3(activeNode.puzzle, direction::RIGHT);
        s3.puzzle = p3;
        s3.parentName = parentNode;
        s3.depth = activeNode.depth + 1;
        s3.myHval = s3.puzzle.getHeuristic();
        s3.nodeName = nodeNumber;
        openSet.insert(make_pair(s3.nodeName, s3));
    }
    if (activeNode.puzzle.checkMove(direction::UP)) {
        nodeNumber++;
        searchNode s4;
        PuzzleGrid p4(activeNode.puzzle, direction::UP);
        s4.puzzle = p4;
        s4.parentName = parentNode;
        s4.depth = activeNode.depth + 1;
        s4.myHval = s4.puzzle.getHeuristic();
        s4.nodeName = nodeNumber;
        openSet.insert(make_pair(s4.nodeName, s4));
    }
}

//choose the next node to expand from the open set
void AStarSearch::selectNode() {
    int minCost; //find the minimum cost (= depth + heuristic value) node to be the new active node.
    unordered_map<int, searchNode>::const_iterator itr;
    itr = openSet.begin();
    minCost = itr->second.depth + itr->second.myHval; //initialize to the first element in the open set.
    activeNode = itr->second;
    for (itr = openSet.begin(); itr != openSet.end(); itr++) {
        if ((itr->second.depth + itr->second.myHval) < minCost) {
            minCost = itr->second.depth + itr->second.myHval;
            activeNode = itr->second;
        }
    }
}

//check if the active node has a heuristic value of 0, meaning it is a goal state and the puzzle is solved.
bool AStarSearch::getSolvedStatus() {
    if (activeNode.myHval == 0) {
        cout << "the puzzle has been solved! Printing solution (look for 'solution.txt' in your directory!)" << endl;
        return true;
    }
    return false;
}

//print a solution path for the puzzle recursivley. Assumes the puzzle is solved.
void AStarSearch::printSolution(searchNode sn, ofstream& ofile) {
    if (sn.parentName != -1) { //recurse until the root node is reached
        //look for the parent node in our closed set using the parent's name as a search key
        unordered_map<int, searchNode>::const_iterator itr = closedSet.find(sn.parentName);
        if (itr == closedSet.end()) {
            //This should never run, but older builds sometimes set the parent to a node in the open set. Better safe than sorry.
            itr = openSet.find(sn.parentName);
            cout << "could not find a step. Attempting to recover..." << endl;
            if (itr == openSet.end()) {
                cout << "something went wrong while printing solution" << endl;
            }
            else {
                printSolution(itr->second, ofile);
            }
        }
        else {
            printSolution(itr->second, ofile);
        }
    }
    sn.puzzle.PrintGrid(ofile);
}

//print number of nodes created by the search
void AStarSearch::printTechnicalData(ofstream& ofile) {
    ofile << "Number of open nodes: " << openSet.size() << endl;
    ofile << "Number of closed nodes: " << closedSet.size() << endl;
    ofile << "Number of total nodes: " << openSet.size() + closedSet.size() << endl;
}

//returns the active node in the search
searchNode AStarSearch::getActiveNode() {
    return activeNode;
}

int main()
{
    ofstream myfile("solution.txt");
    if (myfile.is_open()) {
        //set initial state and run a search.
        int initial_state[4][4] = { {16, 15, 14, 13},{12, 11, 10, 9},{8, 7, 6, 5},{4, 3, 2, 1} };
        AStarSearch solver(initial_state);
        cout << "beginning search..." << endl;
        int i = 0; //debugger to see how many runs the search is taking
        while (solver.getSolvedStatus() == false && i <= 10000) {
            solver.expandActiveNode();
            //i++;
            //cout << "performed " << i << " expansions." << endl;
            solver.selectNode();
        }
        if (i > 10000) { //optional to terminate long searches
            cout << "search was terminated early. Showing preliminary results." << endl;
        }
        solver.printSolution(solver.getActiveNode(), myfile);
        solver.printTechnicalData(myfile);
        myfile.close();
        cout << "Completed execution." << endl;
    }
    else { // error handling in case solution file cannot be created
        cout << "could not open an output file..." << endl;
    }
    return 0;
}
