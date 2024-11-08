#ifndef GRID_H
#define GRID_H

#include <QVector3D>
#include <vector>

struct Cell {
    bool hasMesh;
    gameObject mesh;
};

class Grid {
public:
    Grid();

    Grid(int X, int Y, int Z,QVector3D bbMin);

    Cell& getCell(int x, int y, int z);
    void setMesh(GameObject gameobject,int x,int y,int z);

    uint getCellIndex(int x, int y, int z) const;

    QVector3D BBmin;
    QVector3D BBmax;
    int resX;
    int resY;
    int resZ;
    std::vector<Cell> cells;
};

#endif // GRID_H
