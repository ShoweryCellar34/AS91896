#include <algorithm>
#include <cstddef>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include <raylib.h>

struct Cell {
    size_t id = 0;
    std::shared_ptr<Cell> parent;
    std::shared_ptr<Cell> child;

    size_t x = 0;
    size_t y = 0;
    std::shared_ptr<Cell> up;
    std::shared_ptr<Cell> down;
    std::shared_ptr<Cell> left;
    std::shared_ptr<Cell> right;
    short connections = 0;

    short searched = 0;

    Cell(size_t id) {
        this->id = id;
    }
};

struct Edge {
    std::shared_ptr<Cell> cellA;
    std::shared_ptr<Cell> cellB;

    Edge(std::shared_ptr<Cell>& cellA, std::shared_ptr<Cell>& cellB) {
        this->cellA = cellA;
        this->cellB = cellB;
    }
};

static int getIntInput(const std::string& name, int maxLength, int minValue) {
    std::string storage;
    std::cout << "Enter " << name << ", only characters 0-9 allowed, max characters " << maxLength << ", and minimum value " << minValue << ": " << std::flush;
    int error = 0;
    do {
        if(error) std::cout << (error == 1 ? "(too long): " : (error == 2 ? "(invalid characters): " : "(too small): ")) << "Enter " << name << ", only characters 0-9 allowed, max characters " << maxLength << ", and minimum value " << minValue << ": " << std::flush;
        std::getline(std::cin, storage);
        error = 0;
        if(storage.length() > (size_t)maxLength) {
            error = 1;
            continue;
        }
        for(char& c : storage) {
            if(c < 48 || c > 57) error = 2;
            break;
        }
        if(error) continue;
        if(std::stoi(storage) < minValue) error = 3;
    } while(error);
    return std::stoi(storage);
}

int main(void) {

    size_t width = (size_t)getIntInput("maze width", 2, 2);
    size_t height = (size_t)getIntInput("maze height", 2, 2);

    float windowWidthLimit = (float)getIntInput("max window width", 4, 200);
    float windowHeightLimit = (float)getIntInput("max window height", 4, 200);
    float windowScale = (float)width / (float)height > 1.0f ? windowWidthLimit / (float)width : windowHeightLimit / (float)height;
    int stepRate = getIntInput("the target number of maze solver steps per second", 5, 1);

    InitWindow((int)((float)width * windowScale), (int)((float)height * windowScale), (std::to_string(width) + "x" + std::to_string(height) + " Maze").c_str());
    SetTargetFPS(stepRate);

    std::vector<std::shared_ptr<Cell>> cellFamilies;
    std::vector<std::shared_ptr<Cell>> cells;
    cellFamilies.reserve(width * height);
    cells.resize(width * height);
    for(size_t i = 0; i < width * height; i++) {
        cellFamilies.push_back(std::make_shared<Cell>(i));
        cellFamilies[i]->x = i % width;
        cellFamilies[i]->y = i / width;
        cells[i] = cellFamilies[i];
    }

    std::vector<Edge> edges;
    edges.reserve(width * height * 2 - width - height);

    std::shared_ptr<Cell> prevCell;
    for(size_t x = 0; x < width; x++) {
        for(size_t y = 0; y < height; y++) {
            if(prevCell) {
                edges.emplace_back(prevCell, cellFamilies[width * y + x]);
            }
            prevCell = cellFamilies[width * y + x];
        }
        prevCell = nullptr;
    }
    for(size_t y = 0; y < height; y++) {
        for(size_t x = 0; x < width; x++) {
            if(prevCell) {
                edges.emplace_back(prevCell, cellFamilies[width * y + x]);
            }
            prevCell = cellFamilies[width * y + x];
        }
        prevCell = nullptr;
    }

    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());
    std::shuffle(edges.begin(), edges.end(), generator);

    std::vector<Edge> mazeEdges;
    mazeEdges.reserve(width * height * 2 - width - height);
    for(Edge& edge : edges) {
        bool sameFamily = false;
        for(std::shared_ptr<Cell> cell = edge.cellA; cell; cell = cell->parent) {
            if(cell == edge.cellB) sameFamily = true;
        }
        std::shared_ptr<Cell> foot;
        for(std::shared_ptr<Cell> cell = edge.cellA; cell; cell = cell->child) {
            if(cell == edge.cellB) sameFamily = true;
            foot = cell;
        }
        if(sameFamily) {
            mazeEdges.emplace_back(edge.cellA, edge.cellB);
        } else {
            std::shared_ptr<Cell> head;
            for(std::shared_ptr<Cell> cell = edge.cellB; cell; cell = cell->parent) {
                head = cell;
            }
            foot->child = head;
            head->parent = foot;
            if(std::erase_if(cellFamilies, [&](std::shared_ptr<Cell> cell) { return cell == head; }) != 1) {
                throw std::runtime_error("Bad Error :(");
            }

            if(edge.cellA->x == edge.cellB->x) {
                edge.cellA->down = edge.cellB;
                edge.cellA->connections++;
                edge.cellB->up = edge.cellA;
                edge.cellB->connections++;
            } else {
                edge.cellA->right = edge.cellB;
                edge.cellA->connections++;
                edge.cellB->left = edge.cellA;
                edge.cellB->connections++;
            }
        }
    }

    while(!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(WHITE);

        for(const Edge& edge : mazeEdges) {
            size_t cellAX = edge.cellA->id % width;
            size_t cellAY = edge.cellA->id / width;
            size_t cellBX = edge.cellB->id % width;
            size_t cellBY = edge.cellB->id / width;

            float centerX = (float)(cellAX + cellBX) / 2.0f;
            float centerY = (float)(cellAY + cellBY) / 2.0f;

            if(centerX == (float)cellAX) {
                DrawLine((int)(windowScale * (centerX)), (int)(windowScale * (centerY + 0.5f)), (int)(windowScale * (centerX + 1.0f)), (int)(windowScale * (centerY + 0.5f)), BLACK);
            } else {
                DrawLine((int)(windowScale * (centerX + 0.5f)), (int)(windowScale * (centerY)), (int)(windowScale * (centerX + 0.5f)), (int)(windowScale * (centerY + 1.0f)), BLACK);
            }
        }

        for(const std::shared_ptr<Cell>& cell : cells) {
            float centerX = (float)cell->x + 0.5f;
            float centerY = (float)cell->y + 0.5f;

            if(cell->searched == 1) DrawCircle((int)(centerX * windowScale), (int)(centerY * windowScale), 6.0f, RED);
            if(cell->searched == 2) DrawCircle((int)(centerX * windowScale), (int)(centerY * windowScale), 6.0f, GREEN);
        }

        DrawText("Press Esc to close", 0, 0, 24, PURPLE);

        EndDrawing();

        static std::shared_ptr<Cell> currentCell = cells[0];
        static std::vector<std::shared_ptr<Cell>> junctionStack;
        if(currentCell == cells[0]) {
            junctionStack.emplace_back();
            junctionStack[junctionStack.size() - 1] = currentCell;
            currentCell->connections++;
        }
        if(currentCell == cells[cells.size() - 1]) {
            currentCell->searched = 2;
        } else {
            if(currentCell->connections > 2) {
                if(!currentCell->searched) {
                    junctionStack.emplace_back();
                    junctionStack[junctionStack.size() - 1] = currentCell;
                } else {
                    short options = 0;
                    if(currentCell->up && !currentCell->up->searched) options++;
                    if(currentCell->down && !currentCell->down->searched) options++;
                    if(currentCell->left && !currentCell->left->searched) options++;
                    if(currentCell->right && !currentCell->right->searched) options++;
                    if(options == 1) junctionStack.pop_back();
                }
            }
            currentCell->searched = 1;
            if(currentCell->up && !currentCell->up->searched) {
                currentCell = currentCell->up;
            } else if(currentCell->down && !currentCell->down->searched) {
                currentCell = currentCell->down;
            } else if(currentCell->left && !currentCell->left->searched) {
                currentCell = currentCell->left;
            } else if(currentCell->right && !currentCell->right->searched) {
                currentCell = currentCell->right;
            } else {
                currentCell = junctionStack[junctionStack.size() - 1];
            }
        }
    }

    return 0;
}
