// System Headers
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Project Headers
#include <projectData.h>

// Create aliases for datatypes used so they can be switched if needed

// The datatype that each cell in the maze contains
typedef uint16_t MAZE_CELL_DATA;

// The datatype used for the maze cell dimensions
typedef uint8_t MAZE_CELL_DIMENSION;

// The datatype used for an iterator into the maze cells
// needs to be able to contain the max value supported by MAZE_DIMENSION squared
typedef uint16_t MAZE_CELL_ITERATOR;     

// The datatype used for an iterator into the maze edges
// The number of edges exceeds the number of cells, so this needs to have a bigger capacity than MAZE_CELL_ITERATOR
typedef uint32_t MAZE_EDGE_ITERATOR;

typedef struct {
    MAZE_CELL_ITERATOR cell1;
    MAZE_CELL_ITERATOR cell2;
} MazeEdge;

typedef struct {
    struct {
        MAZE_CELL_DATA*     data;
        MAZE_CELL_ITERATOR  count;
        MAZE_CELL_DIMENSION width;
        MAZE_CELL_DIMENSION height;
    } cells;
    struct {
        MazeEdge*          data;
        MAZE_EDGE_ITERATOR count;
        MAZE_CELL_ITERATOR horizontalCount;
        MAZE_CELL_ITERATOR verticalCount;
    } edges;
} Maze;

Maze MazeCreate(MAZE_CELL_DIMENSION width, MAZE_CELL_DIMENSION height) {
    assert(width || height);
    Maze maze = {
        .cells.data            = calloc(width * height, sizeof(MAZE_CELL_DATA)),
        .cells.count           = width * height,
        .cells.width           = width,
        .cells.height          = height,
        .edges.data            = calloc(((width - 1) * height) + ((height - 1) * width), sizeof(MazeEdge)),
        .edges.count           = ((width - 1) * height) + ((height - 1) * width),
        .edges.horizontalCount = (height - 1) * width,
        .edges.verticalCount   = (width - 1) * height
    };
    assert(maze.cells.data);
    assert(maze.edges.data);
    return maze;
}
void MazeDestroy(Maze* maze) {
    assert(maze);
    assert(maze->cells.data);
    assert(maze->edges.data);

    free(maze->cells.data);
    maze->cells.data   = NULL;
    maze->cells.count  = 0;
    maze->cells.width  = 0;
    maze->cells.height = 0;
    free(maze->edges.data);
    maze->edges.data            = NULL;
    maze->edges.count           = 0;
    maze->edges.horizontalCount = 0;
    maze->edges.verticalCount   = 0;
}

MAZE_CELL_DATA MazeGetCellAt(Maze* maze, MAZE_CELL_DIMENSION x, MAZE_CELL_DIMENSION y) {
    assert(maze);
    assert(maze->cells.data);
    assert(maze->edges.data);
    assert(x < maze->cells.width);
    assert(y < maze->cells.height);
    return *(maze->cells.data + (maze->cells.width * y + x));
}
void MazeSetCellAt(Maze* maze, MAZE_CELL_DATA data, MAZE_CELL_DIMENSION x, MAZE_CELL_DIMENSION y) {
    assert(maze);
    assert(maze->cells.data);
    assert(maze->edges.data);
    assert(x < maze->cells.width);
    assert(y < maze->cells.height);
    *(maze->cells.data + (maze->cells.width * y + x)) = data;
}

MAZE_CELL_ITERATOR MazeGetCellIterator(Maze* maze, MAZE_CELL_DIMENSION x, MAZE_CELL_DIMENSION y) {
    assert(maze);
    assert(maze->cells.data);
    assert(maze->edges.data);
    assert(x < maze->cells.width);
    assert(y < maze->cells.height);
    return maze->cells.width * y + x;
}

typedef struct {
    MAZE_CELL_DIMENSION x;
    MAZE_CELL_DIMENSION y;
} MazeCellPosition;

MazeCellPosition MazeGetCellPosition(Maze* maze, MAZE_CELL_ITERATOR i) {
    assert(maze);
    assert(maze->cells.data);
    assert(maze->edges.data);
    assert(i < maze->cells.count);
    return (MazeCellPosition){
        .x = i % maze->cells.width,
        .y = (MAZE_CELL_DIMENSION)(i / maze->cells.width)
    };
}

int main(int argc, char* argv[]) {
    // Output project info and argument count
    printf("Project Name:    %s\nProject Version: %s\nArg Count:       %i\n", PROJECT_NAME, PROJECT_VERSION, argc);

    // Output arguments
    for(int i = 0; i < argc; i++) {
        printf("Arg %i:           %s\n", i, argv[i]);
    }

    Maze maze = MazeCreate(3, 5);

    MAZE_EDGE_ITERATOR currentMazeEdge = 0;
    for(MAZE_CELL_DIMENSION y = 0; y < maze.cells.height; y++) {
        for(MAZE_CELL_DIMENSION x = 0; x < maze.cells.width; x++) {
            maze.cells.data[MazeGetCellIterator(&maze, x, y)] = MazeGetCellIterator(&maze, x, y);

            if((x + 1) < maze.cells.width) {
                maze.edges.data[currentMazeEdge].cell1   = MazeGetCellIterator(&maze, x, y);
                maze.edges.data[currentMazeEdge++].cell2 = MazeGetCellIterator(&maze, x + 1, y);
            }
        }
    }
    for(MAZE_CELL_DIMENSION x = 0; x < maze.cells.width; x++) {
        for(MAZE_CELL_DIMENSION y = 0; y < maze.cells.height; y++) {
            if((y + 1) < maze.cells.height) {
                maze.edges.data[currentMazeEdge].cell1   = MazeGetCellIterator(&maze, x, y);
                maze.edges.data[currentMazeEdge++].cell2 = MazeGetCellIterator(&maze, x, y + 1);
            }
        }
    }

    // Destroy resources
    MazeDestroy(&maze);

    // Exit with success
    return EXIT_SUCCESS;
}
