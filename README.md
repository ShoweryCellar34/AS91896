This is my submission for NCEA Standard AS91896, it is a program that upon launch will request the user to enter 5 integers:
| Order | Description                         | Internal Name    | Allowed Characters | Max Characters | Minimum Value |
|:------|:------------------------------------|:-----------------|:-------------------|:---------------|:--------------|
|1      | Maze width in cells                 |mazeWidth         |0-9                 |2               |2              |
|2      | Maze height in cells                |mazeHeight        |0-9                 |2               |2              |
|3      | Maximum window width in pixels      |windowWidthLimit  |0-9                 |4               |400            |
|4      | Maximum window height in pixels     |windowHeightLimit |0-9                 |4               |400            |
|5      | Target maze solver steps per second |stepRate          |0-9                 |5               |1              |

After these inputs are retrieved from the user a window will open with the aspect ratio mazeWidth:mazeHeight uniformly scaled to fill a windowWidthLimit by windowHeightLimit box but not go past its boundaries.

Then the window will display the maze and the solver will immediately start with stepRate steps per second, this is configured by setting the windows target frames per second. The window can be closed by pressing Escape or closing by normal methods.

The algorithm used to generate the maze is Randomized Kruskal's Algorithm, it works like this:
1. Put every edge directly between two cells into a list.
2. Shuffle the list of edges randomly.
3. Pick the next edge in the list (starting at edge 0), if the cells on the edge are not connected, destroy the edge, if they are connected leave the edge alone.
4. Repeat step 3 until every edge has been processed.

The algorithm used to solve the maze is Depth First Search, this is how it works:
1. Pick a starting cell and make it the current cell.
2. If the current cell is the finish terminate the search.
3. If the current cell is a junction, and it is unmarked, add it to the junction stack and skip to step 5.
4. If the current cell is a junction and there are less than 2 unmarked neighbors pop a junction (current cell) off the junction stack.
5. Mark the current cell.
6. Set current cell to an unmarked neighbor, if there are none set current cell to the most recent junction.
7. Jump to step 2.