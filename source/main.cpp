#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <ftxui/component/app.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/screen.hpp>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include <ftxui/ftxui.hpp>

typedef std::uint16_t MAZE_DIMENSION;
typedef std::uint32_t MAZE_CELL_ITERATOR;
typedef std::uint64_t MAZE_EDGE_ITERATOR;

static_assert(sizeof(MAZE_DIMENSION) <= sizeof(std::uintmax_t),     "MAZE_DIMENSION cannot be larger than std::uintmax_t");
static_assert(sizeof(MAZE_CELL_ITERATOR) <= sizeof(std::uintmax_t), "MAZE_CELL_ITERATOR cannot be larger than std::uintmax_t");
static_assert(sizeof(MAZE_EDGE_ITERATOR) <= sizeof(std::uintmax_t), "MAZE_EDGE_ITERATOR cannot be larger than std::uintmax_t");

static_assert(std::numeric_limits<MAZE_CELL_ITERATOR>::max() >= ((size_t)std::numeric_limits<MAZE_DIMENSION>::max() * std::numeric_limits<MAZE_DIMENSION>::max()),
    "MAZE_CELL_ITERATOR must be able to hold the max of MAZE_DIMENSION squared");
static_assert(std::numeric_limits<MAZE_EDGE_ITERATOR>::max() >= ((size_t)std::numeric_limits<MAZE_CELL_ITERATOR>::max() * 2),
    "MAZE_EDGE_ITERATOR must be able to hold the max of MAZE_CELL_ITERATOR times 2");

struct Cell {
    Cell*              root     = nullptr;
    std::vector<Cell*> children = {};
    MAZE_DIMENSION     x        = 0,
                       y        = 0;

    Cell() {};
    Cell(Cell* root, MAZE_DIMENSION x, MAZE_DIMENSION y): root(root), x(x), y(y) {
        if(this->root) this->root->children.push_back(this);
    };
};

struct MazeEdge {
    Cell* cell1 = nullptr;
    Cell* cell2 = nullptr;

    MazeEdge() {};
    MazeEdge(Cell* cell1, Cell* cell2): cell1(cell1), cell2(cell2) {};

    bool operator==(const MazeEdge& other) const {
        bool cell1Same = false, cell2Same = false;
        if(this->cell1 == other.cell1) {
            cell1Same = true;
        }
        if(this->cell2 == other.cell2) {
            cell2Same = true;
        }
        return cell1Same && cell2Same;
    }
};

struct Maze {
    std::vector<Cell>     cells    = {};
    Cell                  cellTree = Cell();
    std::vector<MazeEdge> edges    = {};
    MAZE_DIMENSION        width    = 0,
                          height   = 0;

    Maze(MAZE_DIMENSION width, MAZE_DIMENSION height): width(width), height(height) {
        // Reserve space for the cells and edges, this is optional, but it does reduce memory allocations
        this->cells.reserve((MAZE_CELL_ITERATOR)this->width * this->height);
        this->cellTree.children.reserve((MAZE_CELL_ITERATOR)this->width * this->height);
        this->edges.resize((MAZE_CELL_ITERATOR)this->width * this->height * 2 - this->width - this->height);

        for(MAZE_CELL_ITERATOR i = 0; i < (MAZE_CELL_ITERATOR)this->width * this->height; i++) {
            this->cells.emplace_back(&this->cellTree, i % this->width, i / this->width);
        }
        MAZE_EDGE_ITERATOR currentMazeEdge = 0;
        for(MAZE_DIMENSION y = 0; y < this->height; y++) {
            for(MAZE_DIMENSION x = 0; x < this->width; x++) {
                if((x + 1) < this->width) {
                    this->edges[currentMazeEdge].cell1   = &this->cells[(MAZE_CELL_ITERATOR)this->width * y + x];
                    this->edges[currentMazeEdge++].cell2 = &this->cells[(MAZE_CELL_ITERATOR)this->width * y + x + 1];
                }
            }
        }
        for(MAZE_DIMENSION x = 0; x < this->width; x++) {
            for(MAZE_DIMENSION y = 0; y < this->height; y++) {
                if((y + 1) < this->height) {
                    this->edges[currentMazeEdge].cell1   = &this->cells[(MAZE_CELL_ITERATOR)this->width * y + x];
                    this->edges[currentMazeEdge++].cell2 = &this->cells[(MAZE_CELL_ITERATOR)this->width * (y + 1) + x];
                }
            }
        }
    };
};

int main(int argc, char* argv[]) {
    std::vector<std::string> arguments(argv, argv + argc);

    MAZE_DIMENSION mazeWidth = 3, mazeHeight = 3;

    // {
    //     using namespace ftxui;

    //     auto screen = App::TerminalOutput();

    //     std::string mazeWidthString;
    //     std::string mazeHeightString;

    //     std::string widthStatusString;
    //     std::string heightStatusString;

    //     MAZE_DIMENSION minWidth  = 2;
    //     MAZE_DIMENSION minHeight = 2;

    //     InputOption mazeWidthOption;
    //     mazeWidthOption.multiline = false;
    //     Component mazeWidthInput = Input(&mazeWidthString, &widthStatusString, mazeWidthOption);
    //     mazeWidthInput |= CatchEvent([&](Event event) {
    //         if(event.is_character() && !std::isdigit(event.character()[0])) return true;
    //         return false;
    //     });

    //     InputOption mazeHeightOption;
    //     mazeHeightOption.multiline = false;
    //     Component mazeHeightInput = Input(&mazeHeightString, &heightStatusString, mazeHeightOption);
    //     mazeHeightInput |= CatchEvent([&](Event event) {
    //         if(event.is_character() && !std::isdigit(event.character()[0])) return true;
    //         return false;
    //     });

    //     auto submitButton = Button("Submit", [&](){
    //         bool invalidInput = false;

    //         if(mazeWidthString == "") {
    //             widthStatusString = "Width must not be empty";
    //             invalidInput = true;
    //         } if((MAZE_DIMENSION)std::stoll(mazeWidthString) < minWidth) {
    //             mazeWidthString   = "";
    //             widthStatusString = "Width must not be less than " + std::to_string(minWidth) + "";
    //             invalidInput = true;
    //         } else if(mazeHeightString == "") {
    //             heightStatusString = "Height must not be empty";
    //             invalidInput = true;
    //         } else if((MAZE_DIMENSION)std::stoll(mazeHeightString) < minHeight) {
    //             mazeHeightString   = "";
    //             heightStatusString = "Height must not be less than " + std::to_string(minHeight) + "";
    //             invalidInput = true;
    //         }

    //         if(!invalidInput) screen.ExitLoopClosure()();
    //     });
    //     auto clearButton  = Button("Clear", [&](){
    //         mazeWidthString    = "";
    //         mazeHeightString   = "";
    //         widthStatusString  = "Cleared width";
    //         heightStatusString = "Cleared height";
    //     });

    //     // The component tree:
    //     auto component = Container::Vertical({
    //         mazeWidthInput,
    //         mazeHeightInput,
    //         submitButton,
    //         clearButton
    //     });

    //     // Tweak how the component tree is rendered:
    //     auto renderer = Renderer(component, [&] {
    //     return vbox({
    //                 hbox(text("(MAX: " + std::to_string((MAZE_DIMENSION)-1) + " MIN: " + std::to_string(minWidth)  + ") Maze Width:  "), mazeWidthInput->Render()),
    //                 hbox(text("(MAX: " + std::to_string((MAZE_DIMENSION)-1) + " MIN: " + std::to_string(minHeight) + ") Maze height: "), mazeHeightInput->Render()),
    //                 separator(),
    //                 vbox(submitButton->Render(), clearButton->Render()),
    //             }) |
    //             border;
    //     });

    //     screen.Loop(renderer);

    //     mazeWidth  = (MAZE_DIMENSION)std::stoll(mazeWidthString);
    //     mazeHeight = (MAZE_DIMENSION)std::stoll(mazeHeightString);
    // }

    Maze maze(mazeWidth, mazeHeight);

    // This is for debugging, it prints each cells in the maze grids iterators in a grid shape, then prints the list of edges following this template "iterator1|iterator2 "

    // std::vector<Node*> nodePtrs;
    // nodePtrs.reserve(maze.cells.size());
    // for(MAZE_CELL_ITERATOR i = 0; i < maze.cells.size(); i++) {
    //     nodePtrs.push_back(&maze.cells[i]);
    // }

    // for(MAZE_DIMENSION y = 0; y < maze.height; y++) {
    //     for(MAZE_DIMENSION x = 0; x < maze.width; x++) {
    //         std::cout << std::find(nodePtrs.begin(), nodePtrs.end(), &maze.cells[maze.width * y + x]) - nodePtrs.begin() << ' ';
    //     }
    //     std::cout << std::endl;
    // }
    // for(MazeEdge& edge : maze.edges) {
    //     std::cout << std::find(nodePtrs.begin(), nodePtrs.end(), edge.cell1) - nodePtrs.begin() << '|' << std::find(nodePtrs.begin(), nodePtrs.end(), edge.cell2) - nodePtrs.begin() << ' ';
    // }

    // Copy a pointer of each maze edge to a queue of edges to be processed
    // this means an edge can be processed and not removed with no risk of it being reprocessed
    std::vector<MazeEdge*> waitingEdges;
    waitingEdges.reserve(maze.edges.size());
    for(MAZE_CELL_ITERATOR i = 0; i < maze.edges.size(); i++) {
        waitingEdges.push_back(&maze.edges[i]);
    }

    // Shuffle the order of the waiting edges queue, this makes Kruskal's algorithm random
    std::random_device randomDevice;
    std::mt19937       randomProvider(randomDevice());
    std::ranges::shuffle(waitingEdges, randomProvider);

    // Debug print all cell addresses in the cell tree before processing
    // for(MAZE_CELL_ITERATOR i = 0; i < maze.cellTree.children.size(); i++) {
    //     std::cout << maze.cellTree.children[i] << std::endl;
    // }
    // std::cout << std::endl;

    while(maze.cellTree.children.size() > 1) {
        MazeEdge* edge = waitingEdges[waitingEdges.size() - 1];
        waitingEdges.pop_back();

        bool separateTrees = true;

        std::vector<Cell*> cell1Parents;
        Cell* nextParent = edge->cell1;
        while(nextParent->root) {
            cell1Parents.push_back(nextParent);
            nextParent = nextParent->root;
        }
        std::vector<Cell*> cell2Parents;
        nextParent = edge->cell2;
        while(nextParent->root) {
            cell2Parents.push_back(nextParent);
            nextParent = nextParent->root;
        }
        nextParent = edge->cell2;
        for(MAZE_CELL_ITERATOR i = 0; i < cell2Parents.size(); i++) {
            if(std::find(cell1Parents.begin(), cell1Parents.end(), cell2Parents[i]) != cell1Parents.end()) separateTrees = false;
        }

        if(separateTrees) {
            cell1Parents[cell1Parents.size() - 1]->children.push_back(cell2Parents[cell2Parents.size() - 1]);
            cell2Parents[cell2Parents.size() - 1]->root = cell1Parents[cell1Parents.size() - 1];
            // Debug print the memory address of the cell we want to remove from the cell tree root
            // std::cout << cell2Parents[cell2Parents.size() - 1] << std::endl;
            assert(std::erase(maze.cellTree.children, cell2Parents[cell2Parents.size() - 1]));
            assert(std::erase(maze.edges, *edge));
        }
        std::vector<std::string> mazeText(maze.height + (maze.height - 1));
        for(MAZE_DIMENSION y = 0; y < maze.height + (maze.height - 1); y++) {
            mazeText[y].assign(maze.width + (maze.width - 1), '*');
            if(y % 2) {
                for(MAZE_DIMENSION x = 0; x < mazeText[y].size(); x++) {
                    mazeText[y][x] = x % 2 ? '@' : '*';
                }
            }
        }
        for(MAZE_EDGE_ITERATOR i = 0; i < maze.edges.size(); i++) {
            char edgeChar = '.';
            if(maze.edges[i].cell1->y == maze.edges[i].cell2->y) {
                edgeChar = '|';
            } else {
                edgeChar = '-';
            }
            MAZE_DIMENSION edgeX = (((maze.edges[i].cell1->x + 1) * 2 - 2) + ((maze.edges[i].cell2->x + 1) * 2 - 2)) / 2,
                        edgeY = (((maze.edges[i].cell1->y + 1) * 2 - 2) + ((maze.edges[i].cell2->y + 1) * 2 - 2)) / 2;
            mazeText[edgeY][edgeX] = edgeChar;
        }

        std::string mazeTextBuffer(mazeText[0].size() * 2 + 4, '-');
        std::cout << mazeTextBuffer << std::endl;
        for(MAZE_DIMENSION y = 0; y < mazeText.size(); y++) {
            std::cout << "||";
            for(MAZE_DIMENSION x = 0; x < mazeText[y].size(); x++) {
                std::cout << mazeText[y][x] << mazeText[y][x];
            }
            std::cout << "||" << std::endl;
        }
        std::cout << mazeTextBuffer << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    // std::string mazeTextBuffer(mazeText[0].size() * 2 + 4, '-');
    // std::cout << mazeTextBuffer << std::endl;
    // for(MAZE_DIMENSION y = 0; y < mazeText.size(); y++) {
    //     std::cout << "||";
    //     for(MAZE_DIMENSION x = 0; x < mazeText[y].size(); x++) {
    //         std::cout << mazeText[y][x] << mazeText[y][x];
    //     }
    //     std::cout << "||" << std::endl;
    // }
    // std::cout << mazeTextBuffer << std::endl;

    std::cout << std::endl;
}
