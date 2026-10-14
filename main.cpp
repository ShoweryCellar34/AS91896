#include <algorithm>
#include <cstddef>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include <raylib.h>

// This struct is used for the cells in the maze
struct Cell {
    // For creating the maze every node supports being a member in a doubly-linked list,
    // this is purely used for creating the maze and plays no role in solving it
    std::shared_ptr<Cell> parent;
    std::shared_ptr<Cell> child;

    // The x and y position of the cell in the maze
    size_t x = 0;
    size_t y = 0;

    // The direct neighbors of the cell
    // Edges and maze walls can means most of the time not all of these are set
    std::shared_ptr<Cell> up;
    std::shared_ptr<Cell> down;
    std::shared_ptr<Cell> left;
    std::shared_ptr<Cell> right;

    // The number of neighbors the cell has,
    // computing this each time it needs to be checked is inefficient and to verbose
    short connections = 0;

    // The search status of the cell
    // 0: untouched by the search algorithm
    // 1: has been touched by the seach algorithm
    // 2: has been touched by the search algorithm and is the exit
    short touched = 0;
};

// This struct is used for the edges in the maze
struct Edge {
    // These are the edges connections
    // cellA will always be either up or to the left of cellB in the maze
    std::shared_ptr<Cell> cellA;
    std::shared_ptr<Cell> cellB;

    // A simple constructor to make setting the values of the edge easy
    Edge(std::shared_ptr<Cell>& cellA, std::shared_ptr<Cell>& cellB) {
        this->cellA = cellA;
        this->cellB = cellB;
    }
};

// This function asks the user for integer input from the terminal and enforces at set of constains,
// some from the 3 parameters accepted by this function:
// label: a std::string for a label that will be in the printed prompt that describes what the requested data is for
// maxLength: an integer that sets the max number of digits accepted
// minValue: an integer that sets the minimum value accepted
static int getIntInput(const std::string& label, int maxLength, int minValue) {
    // Create a string to store the line got from the user
    std::string storage;
    // Create a variable to store the error type from checking storage variable:
    // 0: No error, good input
    // 1: No input, user inputted nothing
    // 2: Too long, user input exceeded supplied maximum characters in length
    // 3: Invalid character(s), user input contained 1 or more characters that were not 0-9
    // 4: Too small, user input was smaller than supplied minimum value
    int error = 0;
    // Create a string to store the error text (if there is any)
    // Only errors 2, 3, and 4 get codes, 1 leaves this untouched
    // this means if the user inputs nothing they get the previous prompt
    std::string errorString;

    // Use a do while loop, with the condition if error in nonzero
    // This means that the loop will run once and if error code is set repeat until no error
    do {
        // If the error is 2, 3, 4 set the error string,
        // by leaving the error string untouched if the error is 1 the previous prompt is kept
        if(error > 1) {
            // Set the error string according to the error code
            if(error == 2) {
                errorString = "(too long): ";
            } else if(error == 3) {
                errorString = "(invalid character(s)): ";
            } else {
                errorString = "(too small): ";
            }
        };

        // Print the prompt with the error string prepended
        // The prompt contains the label, the allowed characters, and the max length and min value of the input
        std::cout << errorString << "Enter " << label << ", only characters 0-9 allowed, max characters " << maxLength << ", and minimum value " << minValue << ": " << std::flush;

        // Get the text inputted by the user before they press enter
        std::getline(std::cin, storage);

        // Set the error code to zero, this value is only set by the input checking functions
        // if there is an error, not if it passed, this means that a successful pass relies on this being 0 by default
        error = 0;

        // If the user inputted nothing set the error to 1 (no input)
        if(storage.length() == 0) {
            error = 1;
            continue;
        }

        // If the user input was too long set the error to 2 (too long)
        if(storage.length() > (size_t)maxLength) {
            error = 2;
            continue;
        }

        // If the user input contained invalid character(s) set the error to 3 (invalid input)
        for(char& c : storage) {
            if(c < 48 || c > 57) error = 3;
            break;
        }

        // If the error is set, this iteration of the loop is exited early,
        // this could be done after every check, but its needed here
        // because all errors other than too small can/will cause std::stoi (string to integer) to throw an exception if the bad input is used
        if(error) continue;

        // If the user input is less than the minimum value set the error to 4 (too small)
        if(std::stoi(storage) < minValue) error = 4;
    } while(error);

    // If this is reached its guaranteed that a valid integer has been retrieved from the user
    return std::stoi(storage);
}

int main(void) {
    // Get the maze dimensions from the user, the user can enter any value from 2-99 for either dimensions
    size_t mazeWidth = (size_t)getIntInput("maze width", 2, 2);
    size_t mazeHeight = (size_t)getIntInput("maze height", 2, 2);

    // Get the max dimensions of the window that displays the maze from the user
    // The maze window will fill this rectangle described here without going past these values
    float windowWidthLimit = (float)getIntInput("max window width", 4, 400);
    float windowHeightLimit = (float)getIntInput("max window height", 4, 400);

    // Get the target number of maze solver steps per second from the user,
    // this is used as the target frames per second for the window
    int stepRate = getIntInput("the target number of maze solver steps per second", 5, 1);

    // If the window width is the scaling limit for the maze use it at the scale ratio, and use the height otherwise
    float windowScale = (float)mazeWidth / (float)mazeHeight > 1.0f ? windowWidthLimit / (float)mazeWidth : windowHeightLimit / (float)mazeHeight;

    // Create the window to display the maze using the values got from the user
    InitWindow((int)((float)mazeWidth * windowScale), (int)((float)mazeHeight * windowScale), (std::to_string(mazeWidth) + "x" + std::to_string(mazeHeight) + " Maze").c_str());
    // Set the target frames per second for the window,
    // The maze solver is in the render loop, so this effectively sets it speed
    SetTargetFPS(stepRate);

    // This vector contains every cell in the maze in the order they were created
    // This is great for finding a cell by its x and y values as the equation "width * y + x" give the iterator into this vector
    std::vector<std::shared_ptr<Cell>> cells;
    // Reserve space in the vectors to reduce vector resize operations
    cells.reserve(mazeWidth * mazeHeight);
    // This for loop creates a cell in cells and sets its x and y values
    // Throughout this code shared pointers are used for all cells,
    // this is done because it greatly simplifies the code by removing ownership management and reducing memory error risk
    for(size_t i = 0; i < mazeWidth * mazeHeight; i++) {
        cells.push_back(std::make_shared<Cell>());
        cells[i]->x = i % mazeWidth;
        cells[i]->y = i / mazeWidth;
    }

    // Create a vector of edges and reserve the space for them
    std::vector<Edge> edges;
    edges.reserve(mazeWidth * mazeHeight * 2 - mazeWidth - mazeHeight);

    // Create variable to store the previously processed cell
    std::shared_ptr<Cell> prevCell;
    // Two nested for loops, the outer iterates over every horizontal position in maze,
    // and the inner for every vertical position, this effectively iterates over every cell in the maze
    // This generates horizontal edges (the edge is horizontal, not what is joins)
    for(size_t x = 0; x < mazeWidth; x++) {
        for(size_t y = 0; y < mazeHeight; y++) {
            // If the prevCell is not null create an edge between prevCell and the current one
            if(prevCell) edges.emplace_back(prevCell, cells[mazeWidth * y + x]);
            // Set prevCell to the current one for the next iteration
            prevCell = cells[mazeWidth * y + x];
        }
        // When a column is complete set prevCell to nullptr,
        // this is so an edge between the bottom of one column and the top of another
        prevCell = nullptr;
    }
    // Do the same thing as the last two loops but the vertical and horizontal loops are switch
    // This generates vertical edges
    for(size_t y = 0; y < mazeHeight; y++) {
        for(size_t x = 0; x < mazeWidth; x++) {
            if(prevCell) {
                edges.emplace_back(prevCell, cells[mazeWidth * y + x]);
            }
            prevCell = cells[mazeWidth * y + x];
        }
        prevCell = nullptr;
    }

    // Create a randomDevice and use it to initialize a generator that is used to shuffle the edges vector
    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());
    std::shuffle(edges.begin(), edges.end(), generator);

    // Create a vector to store edges selected to be part of the finished maze and reserve space
    std::vector<Edge> mazeEdges;
    mazeEdges.reserve(mazeWidth * mazeHeight - mazeWidth - (mazeHeight - 1));
    // Loop over every edge in edges
    for(Edge& edge : edges) {
        // Create boolean to store the results of the check if their in the same family
        // The family checks only set it if they detect the same family, so sameFamily is false by default
        bool sameFamily = false;

        // Check if amy of the parent of cellA are cellB, and if to set sameFamily to true
        for(std::shared_ptr<Cell> cell = edge.cellA; cell; cell = cell->parent) {
            if(cell == edge.cellB) sameFamily = true;
        }
        // Same as above but check the children,
        // and store the foot of the cellA doubly-linked list for later use
        std::shared_ptr<Cell> foot;
        for(std::shared_ptr<Cell> cell = edge.cellA; cell; cell = cell->child) {
            if(cell == edge.cellB) sameFamily = true;
            foot = cell;
        }

        // If sameFamily is set to true, add the edge to mazeEdges,
        // this is done because it does not block a cell from being accessed and can be a part of the maze
        if(sameFamily) {
            mazeEdges.emplace_back(edge.cellA, edge.cellB);
        } else {
            // If sameFamily is false then start the proccess of removing it

            // Get the head of the cellB doubly-link list
            std::shared_ptr<Cell> head;
            for(std::shared_ptr<Cell> cell = edge.cellB; cell; cell = cell->parent) {
                head = cell;
            }

            // Set foot as heads parent and parent as foots child
            foot->child = head;
            head->parent = foot;

            // Detect if the edge is horizontal or vertical
            if(edge.cellA->x == edge.cellB->x) {
                // Set the down member of cellA to cellB,
                // cellA will always be up or left as an effect of edge generation
                edge.cellA->down = edge.cellB;
                // increment the connections member of cellA
                // There are other edges that may increment this, so don't set it directly
                edge.cellA->connections++;

                // Do the same thing for cellB, but set the up not down member
                edge.cellB->up = edge.cellA;
                edge.cellB->connections++;
            } else {
                // Same thing as above, but set the left and right members instead
                edge.cellA->right = edge.cellB;
                edge.cellA->connections++;
                edge.cellB->left = edge.cellA;
                edge.cellB->connections++;
            }
        }
    }

    // This is the drawing loop and the maze solving loop at the same time for simplicity
    // While the window should not close continue this loop
    while(!WindowShouldClose()) {
        // Start drawing using raylib
        BeginDrawing();

        // Clear the screen
        ClearBackground(WHITE);

        // Iterate over every edge in mazeEdges and draw it
        for(const Edge& edge : mazeEdges) {
            float centerX = (float)(edge.cellA->x + edge.cellB->x) / 2.0f;
            float centerY = (float)(edge.cellA->y + edge.cellB->y) / 2.0f;

            if(centerX == (float)edge.cellA->x) {
                DrawLine((int)(windowScale * (centerX)), (int)(windowScale * (centerY + 0.5f)), (int)(windowScale * (centerX + 1.0f)), (int)(windowScale * (centerY + 0.5f)), BLACK);
            } else {
                DrawLine((int)(windowScale * (centerX + 0.5f)), (int)(windowScale * (centerY)), (int)(windowScale * (centerX + 0.5f)), (int)(windowScale * (centerY + 1.0f)), BLACK);
            }
        }

        // Iterate over every cell in cells and if it is touched status is nonzero draw a circle,
        // red for touched, and green for exit found
        for(const std::shared_ptr<Cell>& cell : cells) {
            float centerX = (float)cell->x + 0.5f;
            float centerY = (float)cell->y + 0.5f;

            if(cell->touched == 1) DrawCircle((int)(centerX * windowScale), (int)(centerY * windowScale), ((float)mazeWidth * windowScale) / (float)(mazeWidth * 2) / 2, RED);
            if(cell->touched == 2) DrawCircle((int)(centerX * windowScale), (int)(centerY * windowScale), ((float)mazeWidth * windowScale) / (float)(mazeWidth * 2) / 2, GREEN);
        }

        // Print a message that tells the user they can press the escape key to close the window
        DrawText("Press Esc to close", 0, 0, 40, BLUE);

        // End drawing using raylib
        EndDrawing();

        // The maze solver

        // Store the current cell being worked on, and set the starting point to the top left
        static std::shared_ptr<Cell> currentCell = cells[0];
        // A vector used as a stack where all the discovered junction cells with more than 1 unexplored route
        static std::vector<std::shared_ptr<Cell>> junctionStack;
        // If the current cell is the start force it to be a junction,
        // if it is not a junction then an edge case where it is not the end of a path,
        // and not a real junction, and the correct path is not the first route taken
        // results in when it inevitably needs to go back to the starting cell to take another route
        // it cannot as there is no junction to go back to unless the start is registered as a junction
        if(currentCell == cells[0]) {
            junctionStack.push_back(currentCell);
            currentCell->connections++;
        }
        // If the current cell is the last cell (bottom right),
        // then the exit has been found and its touched status gets set to 2 (finish found),
        // if the current cell is not the end then continue the search
        if(currentCell == cells[cells.size() - 1]) {
            currentCell->touched = 2;
        } else {
            // If the current cell is a junction and it is untouched then add it to the junction stack
            // If the current cell is a junction but it is touched if all its routes except 1 have been explored
            // then remove it from the junction stack
            if(currentCell->connections > 2) {
                if(!currentCell->touched) {
                    junctionStack.push_back(currentCell);
                } else {
                    short options = 0;
                    if(currentCell->up && !currentCell->up->touched) options++;
                    if(currentCell->down && !currentCell->down->touched) options++;
                    if(currentCell->left && !currentCell->left->touched) options++;
                    if(currentCell->right && !currentCell->right->touched) options++;
                    if(options == 1) junctionStack.pop_back();
                }
            }
            // Set the current cell to touched so it is not counted as a route to explore
            currentCell->touched = 1;
            // Check the current cells direct neighbors and move to them if they exist and are untouched,
            // the checks are done in the order up, down, left, and right
            // If the end is reached and there are no valid options then go back to the most recent junction in the stack
            if(currentCell->up && !currentCell->up->touched) {
                currentCell = currentCell->up;
            } else if(currentCell->down && !currentCell->down->touched) {
                currentCell = currentCell->down;
            } else if(currentCell->left && !currentCell->left->touched) {
                currentCell = currentCell->left;
            } else if(currentCell->right && !currentCell->right->touched) {
                currentCell = currentCell->right;
            } else {
                currentCell = junctionStack[junctionStack.size() - 1];
            }
        }
    }

    // Clean exit
    return 0;
}
