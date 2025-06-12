#ifndef SQUAREBOX_H
#define SQUAREBOX_H

#include "Model.h"

class SquareBox : public Model {
public:
    const float length;
    const int divisions;
    
    SquareBox(float length, int divisions)
        : length(length), divisions(divisions) {
        
        generateVertices();
    }

private:
    inline void generateVertices() {

        float offset = length / 2;
        float divLength = length / divisions;

        // A square box or cube is composed of 6 square faces:
        // -> Right & Left, where X is +/- offset
        // -> Top & Bottom, where Y is +/- offset
        // -> Front & Back, where Z is +/- offset

        // Right, Top and Front are front-facing: defined counterclockwise
        // others are back-facing: defined clockwise

        // Right
        for (int iRow = 0; iRow < divisions; iRow++) {

            float rowCoord = -offset + divLength * iRow;
            float rowCoordNext = -offset + divLength * (iRow+1);

            for (int iColumn = 0; iColumn < divisions; iColumn++) {
                
                float colCoord = -offset + divLength * iColumn;
                float colCoordNext = -offset + divLength * (iColumn+1);

                vertices.push_back(Vec3(offset, rowCoord,     colCoord));
                vertices.push_back(Vec3(offset, rowCoordNext, colCoord));
                vertices.push_back(Vec3(offset, rowCoordNext, colCoordNext));

                vertices.push_back(Vec3(offset, rowCoordNext, colCoordNext));
                vertices.push_back(Vec3(offset, rowCoord,     colCoordNext));
                vertices.push_back(Vec3(offset, rowCoord,     colCoord));
            }
        }

        // Top
        for (int iRow = 0; iRow < divisions; iRow++) {

            float rowCoord = -offset + divLength * iRow;
            float rowCoordNext = -offset + divLength * (iRow + 1);

            for (int iColumn = 0; iColumn < divisions; iColumn++) {

                float colCoord = -offset + divLength * iColumn;
                float colCoordNext = -offset + divLength * (iColumn + 1);

                vertices.push_back(Vec3(rowCoord,     offset, colCoord));
                vertices.push_back(Vec3(rowCoord, offset, colCoordNext));
                vertices.push_back(Vec3(rowCoordNext, offset, colCoordNext));

                vertices.push_back(Vec3(rowCoordNext, offset, colCoordNext));
                vertices.push_back(Vec3(rowCoordNext, offset, colCoord));
                vertices.push_back(Vec3(rowCoord,     offset, colCoord));
            }
        }

        // Front
        for (int iRow = 0; iRow < divisions; iRow++) {

            float rowCoord = -offset + divLength * iRow;
            float rowCoordNext = -offset + divLength * (iRow + 1);

            for (int iColumn = 0; iColumn < divisions; iColumn++) {

                float colCoord = -offset + divLength * iColumn;
                float colCoordNext = -offset + divLength * (iColumn + 1);

                vertices.push_back(Vec3(rowCoord,     colCoord,     offset));
                vertices.push_back(Vec3(rowCoordNext, colCoord,     offset));
                vertices.push_back(Vec3(rowCoord,     colCoordNext, offset));

                vertices.push_back(Vec3(rowCoordNext, colCoordNext, offset));
                vertices.push_back(Vec3(rowCoord,     colCoordNext, offset));
                vertices.push_back(Vec3(rowCoordNext, colCoord,     offset));
            }
        }

        // Left
        for (int iRow = 0; iRow < divisions; iRow++) {

            float rowCoord = -offset + divLength * iRow;
            float rowCoordNext = -offset + divLength * (iRow + 1);

            for (int iColumn = 0; iColumn < divisions; iColumn++) {

                float colCoord = -offset + divLength * iColumn;
                float colCoordNext = -offset + divLength * (iColumn + 1);

                vertices.push_back(Vec3(-offset, rowCoord,     colCoord));
                vertices.push_back(Vec3(-offset, rowCoordNext, colCoordNext));
                vertices.push_back(Vec3(-offset, rowCoordNext, colCoord));

                vertices.push_back(Vec3(-offset, rowCoordNext, colCoordNext));
                vertices.push_back(Vec3(-offset, rowCoord,     colCoord));
                vertices.push_back(Vec3(-offset, rowCoord,     colCoordNext));
            }
        }

        // Bottom
        for (int iRow = 0; iRow < divisions; iRow++) {

            float rowCoord = -offset + divLength * iRow;
            float rowCoordNext = -offset + divLength * (iRow + 1);

            for (int iColumn = 0; iColumn < divisions; iColumn++) {

                float colCoord = -offset + divLength * iColumn;
                float colCoordNext = -offset + divLength * (iColumn + 1);

                vertices.push_back(Vec3(rowCoord,     -offset, colCoord));
                vertices.push_back(Vec3(rowCoordNext, -offset, colCoord));
                vertices.push_back(Vec3(rowCoordNext, -offset, colCoordNext));

                vertices.push_back(Vec3(rowCoordNext, -offset, colCoordNext));
                vertices.push_back(Vec3(rowCoord,     -offset, colCoordNext));
                vertices.push_back(Vec3(rowCoord,     -offset, colCoord));
            }
        }

        // Back
        for (int iRow = 0; iRow < divisions; iRow++) {

            float rowCoord = -offset + divLength * iRow;
            float rowCoordNext = -offset + divLength * (iRow + 1);

            for (int iColumn = 0; iColumn < divisions; iColumn++) {

                float colCoord = -offset + divLength * iColumn;
                float colCoordNext = -offset + divLength * (iColumn + 1);

                vertices.push_back(Vec3(rowCoord,     colCoord,     -offset));
                vertices.push_back(Vec3(rowCoord,     colCoordNext, -offset));
                vertices.push_back(Vec3(rowCoordNext, colCoord,     -offset));

                vertices.push_back(Vec3(rowCoordNext, colCoord,     -offset));
                vertices.push_back(Vec3(rowCoord,     colCoordNext, -offset));
                vertices.push_back(Vec3(rowCoordNext, colCoordNext, -offset));
            }
        }
    }
};

#endif
