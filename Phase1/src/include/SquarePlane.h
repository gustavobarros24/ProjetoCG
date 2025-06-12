#ifndef SQUAREPLANE_H
#define SQUAREPLANE_H

#include "Model.h"

class SquarePlane : public Model {
public:
    const float length;
    const int divisions;
    const float height;

    SquarePlane(float length, int divisions, float height = 0.0)
        : length(length), divisions(divisions), height(height) {

        generateVertices();
    }

private:
    inline void generateVertices() {

        // To center the plane, offset from origin by -(length/2) in both X and Z
        float offset = length / 2;
        float divLength = length / divisions;

        float z, zNext;
        float x, xNext;

        // iterate row by row, from top to bottom (i.e. Z increases)
        for (int iRow = 0; iRow < divisions; iRow++) {
            
            z = -offset + divLength * iRow;
            zNext = -offset + divLength * (iRow + 1);

            // iterate column by column under a specific row (i.e. X increases)
            for (int iColumn = 0; iColumn < divisions; iColumn++) {
                
                x = -offset + divLength * iColumn;
                xNext = -offset + divLength * (iColumn + 1);

                //  v1--v4
                //  | \\ |
                //  v2--v3

                Vec3 v1(x,     height, z);
                Vec3 v2(x,     height, zNext);
                Vec3 v3(xNext, height, zNext);
                Vec3 v4(xNext, height, z);

                vertices.push_back(v1);
                vertices.push_back(v2);
                vertices.push_back(v3);

                vertices.push_back(v3);
                vertices.push_back(v4);
                vertices.push_back(v1);
            }
        }
    }
};

#endif