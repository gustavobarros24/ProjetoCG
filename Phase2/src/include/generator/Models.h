#ifndef MODELS_H
#define MODELS_H

#define _USE_MATH_DEFINES
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

/*

class Model {
public:
    std::vector<float> vertices;

    inline void addVertex(const glm::vec3 vertex) {
        vertices.push_back(vertex.x);
        vertices.push_back(vertex.y);
        vertices.push_back(vertex.z);
    }

    inline void toFile(const char* filename) {
        std::filesystem::path modelsPath = getModelsDirectory();
        std::filesystem::path filePath = modelsPath / filename;

        if (std::filesystem::exists(filePath)) {
            std::filesystem::remove(filePath);
        }

        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filePath << std::endl;
            return;
        }

        // Write vertices as x,y,z triplets
        for (size_t i = 0; i < vertices.size(); i += 3) {
            file << vertices[i] << "," << vertices[i + 1] << "," << vertices[i + 2] << std::endl;
        }

        file.close();
    }

    virtual ~Model() = default;

private:
    inline std::filesystem::path getModelsDirectory() {
        std::filesystem::path currentPath = std::filesystem::current_path();
        std::filesystem::path modelsPath = currentPath.parent_path() / "models";
        std::filesystem::create_directory(modelsPath);
        return modelsPath;
    }
};

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

                glm::vec3 v1(x, height, z);
                glm::vec3 v2(x, height, zNext);
                glm::vec3 v3(xNext, height, zNext);
                glm::vec3 v4(xNext, height, z);

                addVertex(v1);
                addVertex(v2);
                addVertex(v3);

                addVertex(v3);
                addVertex(v4);
                addVertex(v1);
            }
        }
    }
};

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
            float rowCoordNext = -offset + divLength * (iRow + 1);

            for (int iColumn = 0; iColumn < divisions; iColumn++) {

                float colCoord = -offset + divLength * iColumn;
                float colCoordNext = -offset + divLength * (iColumn + 1);

                addVertex(glm::vec3(offset, rowCoord, colCoord));
                addVertex(glm::vec3(offset, rowCoordNext, colCoord));
                addVertex(glm::vec3(offset, rowCoordNext, colCoordNext));

                addVertex(glm::vec3(offset, rowCoordNext, colCoordNext));
                addVertex(glm::vec3(offset, rowCoord, colCoordNext));
                addVertex(glm::vec3(offset, rowCoord, colCoord));
            }
        }

        // Top
        for (int iRow = 0; iRow < divisions; iRow++) {

            float rowCoord = -offset + divLength * iRow;
            float rowCoordNext = -offset + divLength * (iRow + 1);

            for (int iColumn = 0; iColumn < divisions; iColumn++) {

                float colCoord = -offset + divLength * iColumn;
                float colCoordNext = -offset + divLength * (iColumn + 1);

                addVertex(glm::vec3(rowCoord, offset, colCoord));
                addVertex(glm::vec3(rowCoord, offset, colCoordNext));
                addVertex(glm::vec3(rowCoordNext, offset, colCoordNext));

                addVertex(glm::vec3(rowCoordNext, offset, colCoordNext));
                addVertex(glm::vec3(rowCoordNext, offset, colCoord));
                addVertex(glm::vec3(rowCoord, offset, colCoord));
            }
        }

        // Front
        for (int iRow = 0; iRow < divisions; iRow++) {

            float rowCoord = -offset + divLength * iRow;
            float rowCoordNext = -offset + divLength * (iRow + 1);

            for (int iColumn = 0; iColumn < divisions; iColumn++) {

                float colCoord = -offset + divLength * iColumn;
                float colCoordNext = -offset + divLength * (iColumn + 1);

                addVertex(glm::vec3(rowCoord, colCoord, offset));
                addVertex(glm::vec3(rowCoordNext, colCoord, offset));
                addVertex(glm::vec3(rowCoord, colCoordNext, offset));

                addVertex(glm::vec3(rowCoordNext, colCoordNext, offset));
                addVertex(glm::vec3(rowCoord, colCoordNext, offset));
                addVertex(glm::vec3(rowCoordNext, colCoord, offset));
            }
        }

        // Left
        for (int iRow = 0; iRow < divisions; iRow++) {

            float rowCoord = -offset + divLength * iRow;
            float rowCoordNext = -offset + divLength * (iRow + 1);

            for (int iColumn = 0; iColumn < divisions; iColumn++) {

                float colCoord = -offset + divLength * iColumn;
                float colCoordNext = -offset + divLength * (iColumn + 1);

                addVertex(glm::vec3(-offset, rowCoord, colCoord));
                addVertex(glm::vec3(-offset, rowCoordNext, colCoordNext));
                addVertex(glm::vec3(-offset, rowCoordNext, colCoord));

                addVertex(glm::vec3(-offset, rowCoordNext, colCoordNext));
                addVertex(glm::vec3(-offset, rowCoord, colCoord));
                addVertex(glm::vec3(-offset, rowCoord, colCoordNext));
            }
        }

        // Bottom
        for (int iRow = 0; iRow < divisions; iRow++) {

            float rowCoord = -offset + divLength * iRow;
            float rowCoordNext = -offset + divLength * (iRow + 1);

            for (int iColumn = 0; iColumn < divisions; iColumn++) {

                float colCoord = -offset + divLength * iColumn;
                float colCoordNext = -offset + divLength * (iColumn + 1);

                addVertex(glm::vec3(rowCoord, -offset, colCoord));
                addVertex(glm::vec3(rowCoordNext, -offset, colCoord));
                addVertex(glm::vec3(rowCoordNext, -offset, colCoordNext));

                addVertex(glm::vec3(rowCoordNext, -offset, colCoordNext));
                addVertex(glm::vec3(rowCoord, -offset, colCoordNext));
                addVertex(glm::vec3(rowCoord, -offset, colCoord));
            }
        }

        // Back
        for (int iRow = 0; iRow < divisions; iRow++) {

            float rowCoord = -offset + divLength * iRow;
            float rowCoordNext = -offset + divLength * (iRow + 1);

            for (int iColumn = 0; iColumn < divisions; iColumn++) {

                float colCoord = -offset + divLength * iColumn;
                float colCoordNext = -offset + divLength * (iColumn + 1);

                addVertex(glm::vec3(rowCoord, colCoord, -offset));
                addVertex(glm::vec3(rowCoord, colCoordNext, -offset));
                addVertex(glm::vec3(rowCoordNext, colCoord, -offset));

                addVertex(glm::vec3(rowCoordNext, colCoord, -offset));
                addVertex(glm::vec3(rowCoord, colCoordNext, -offset));
                addVertex(glm::vec3(rowCoordNext, colCoordNext, -offset));
            }
        }
    }
};

class Sphere : public Model {
public:
    const float radius;
    const int slices;
    const int stacks;

    Sphere(float radius, int slices, int stacks)
        : radius(radius), slices(slices), stacks(stacks) {

        generateVertices();
    }

private:
    inline void generateVertices() {

        // vertical angle varying between [-pi/2, pi/2]. it snaps to stack rings.
        float phi, phiNext;
        float phiStep = M_PI / stacks;

        // horizontal angle varying between [0,2pi]. it snaps to slice rings.
        float theta, thetaNext;
        float thetaStep = (2 * M_PI) / slices;

        // The cartesian coordinates of a point P defined in a sphere are calculated using...
        // -> its phi angle -> finds the y coordinate
        // -> its theta angle -> finds the x and z coordinates using the point projection formula: r*cos(phi)

        // the x and z coordinates need further multiplication by sin(theta) or cos(theta) to obtain the axis-specific components.

        // P(
        //   r*cos(phi) * sin(theta),
        //   r*sin(phi),
        //   r*cos(phi) * cos(theta)
        // )

        float xz, xzNext, y, yNext;

        // iterate through stacks
        for (int iStack = 0; iStack < stacks; iStack++) {

            phi = M_PI / 2 - iStack * phiStep;           // since [-pi/2, pi/2]
            phiNext = M_PI / 2 - (iStack + 1) * phiStep;

            // Two points in the same stack ring share height (y) and XZ-projected distance (xz) from the origin.
            y = radius * sinf(phi);
            xz = radius * cosf(phi);
            yNext = radius * sinf(phiNext);
            xzNext = radius * cosf(phiNext);

            // iterate through quadrilateral cells, formed by a pair of stack rings intersected by a pair of slice rings
            for (int iSlice = 0; iSlice < slices; iSlice++) {

                theta = iSlice * thetaStep;
                thetaNext = (iSlice + 1) * thetaStep;

                // CCW from v1: v1 -> v2 -> v3, v3 -> v4 -> v1
                //  v1--v4
                //  | \\ |
                //  v2--v3

                glm::vec3 v1(
                    radius * cosf(phi) * sinf(theta),
                    radius * sinf(phi),
                    radius * cosf(phi) * cosf(theta)
                );
                glm::vec3 v2(                               // v1->v2 is a vertical advance, use phiNext
                    radius * cosf(phiNext) * sinf(theta),
                    radius * sinf(phiNext),
                    radius * cosf(phiNext) * cosf(theta)
                );
                glm::vec3 v3(                               // v2->v3 is a horizontal advance, use thetaNext
                    radius * cosf(phiNext) * sinf(thetaNext),
                    radius * sinf(phiNext),
                    radius * cosf(phiNext) * cosf(thetaNext)
                );
                glm::vec3 v4(                              // v3->v4 is a vertical return, use phi
                    radius * cosf(phi) * sinf(thetaNext),
                    radius * sinf(phi),
                    radius * cosf(phi) * cosf(thetaNext)
                );


                if (iStack != stacks - 1) {
                    addVertex(v1);
                    addVertex(v2);
                    addVertex(v3);
                }

                if (iStack != 0) {
                    addVertex(v3);
                    addVertex(v4);
                    addVertex(v1);
                }
            }
        }
    }
};

class Cone : public Model {
public:
    const float radius;
    const float height;
    const int slices;
    const int stacks;

    Cone(float radius, float height, int slices, int stacks)
        : radius(radius), height(height), slices(slices), stacks(stacks) {

        generateVertices();
    }

private:
    inline void generateVertices() {

        float thetaStep = 2 * M_PI / slices;
        float stackHeight = height / stacks;

        // Draw the base
        for (int iSlice = 0; iSlice < slices; iSlice++) {

            float theta = thetaStep * iSlice;
            float thetaNext = thetaStep * (iSlice + 1);

            float x1 = radius * sin(theta);
            float x2 = radius * sin(thetaNext);

            float z1 = radius * cos(theta);
            float z2 = radius * cos(thetaNext);

            // Clockwise order for base (facing downward)
            addVertex(glm::vec3(x1, 0.0f, z1));
            addVertex(glm::vec3(0.0f, 0.0f, 0.0f));
            addVertex(glm::vec3(x2, 0.0f, z2));
        }

        // Draw the sides
        for (int iStack = 0; iStack < stacks; iStack++) {

            float yBottom = iStack * stackHeight;
            float yTop = (iStack + 1) * stackHeight;
            float bottomRadius = radius * (1.0f - (float)iStack / stacks);
            float topRadius = radius * (1.0f - (float)(iStack + 1) / stacks);

            for (int iSlice = 0; iSlice < slices; iSlice++) {

                float theta = thetaStep * iSlice;
                float thetaNext = thetaStep * (iSlice + 1);

                // Bottom ring vertices
                float x1 = bottomRadius * sin(theta);
                float x2 = bottomRadius * sin(thetaNext);

                float z1 = bottomRadius * cos(theta);
                float z2 = bottomRadius * cos(thetaNext);

                // Top ring vertices
                float x3 = topRadius * sin(theta);
                float x4 = topRadius * sin(thetaNext);

                float z3 = topRadius * cos(theta);
                float z4 = topRadius * cos(thetaNext);

                // Cell vertices
                glm::vec3 bottomLeft(x1, yBottom, z1);
                glm::vec3 bottomRight(x2, yBottom, z2);
                glm::vec3 topLeft(x3, yTop, z3);
                glm::vec3 topRight(x4, yTop, z4);

                // First triangle
                addVertex(bottomLeft);
                addVertex(bottomRight);
                addVertex(topLeft);

                // Second triangle
                addVertex(topLeft);
                addVertex(bottomRight);
                addVertex(topRight);
            }
        }
    }
};

*/

namespace generateVertices {

    std::vector<float> plane(float length, int divisions, float height = 0.0f) {
        std::vector<float> vertices;

        const float halfLength = length / 2.0f;
        const float step = length / divisions;

        for (int row = 0; row < divisions; row++) {
            float z1 = -halfLength + row * step;
            float z2 = -halfLength + (row + 1) * step;

            for (int col = 0; col < divisions; col++) {
                float x1 = -halfLength + col * step;
                float x2 = -halfLength + (col + 1) * step;

                vertices.insert(vertices.end(), {
                    // first tri
                    x1, height, z1,
                    x1, height, z2,
                    x2, height, z2,
                    // second tri
                    x2, height, z2,
                    x2, height, z1,
                    x1, height, z1
                    });
            }
        }
        return vertices;
    }

    std::vector<float> box(float length, int divisions) {
        std::vector<float> vertices;

        const float halfLength = length / 2.0f;
        const float step = length / divisions;

        // Generate each face (right, top, front, left, bottom, back)
        for (int face = 0; face < 6; face++) {
            bool isXFace = (face == 0 || face == 3);
            bool isYFace = (face == 1 || face == 4);
            bool isNegative = (face >= 3);

            for (int row = 0; row < divisions; row++) {
                float a1 = -halfLength + row * step;
                float a2 = -halfLength + (row + 1) * step;

                for (int col = 0; col < divisions; col++) {
                    float b1 = -halfLength + col * step;
                    float b2 = -halfLength + (col + 1) * step;

                    float faceCoord = isNegative ? -halfLength : halfLength;
                    glm::vec3 v1, v2, v3, v4;

                    if (isXFace) {
                        v1 = { faceCoord, a1, b1 };
                        v2 = { faceCoord, a2, b1 };
                        v3 = { faceCoord, a2, b2 };
                        v4 = { faceCoord, a1, b2 };
                    }
                    else if (isYFace) {
                        v1 = { a1, faceCoord, b1 };
                        v2 = { a1, faceCoord, b2 };
                        v3 = { a2, faceCoord, b2 };
                        v4 = { a2, faceCoord, b1 };
                    }
                    else { // Z face
                        v1 = { a1, b1, faceCoord };
                        v2 = { a2, b1, faceCoord };
                        v3 = { a2, b2, faceCoord };
                        v4 = { a1, b2, faceCoord };
                    }

                    if (isNegative) { // cw (back)
                        vertices.insert(vertices.end(), {
                            v1.x, v1.y, v1.z,
                            v3.x, v3.y, v3.z,
                            v2.x, v2.y, v2.z,

                            v1.x, v1.y, v1.z,
                            v4.x, v4.y, v4.z,
                            v3.x, v3.y, v3.z
                            });
                    }
                    else { // ccw (front)
                        vertices.insert(vertices.end(), {
                            v1.x, v1.y, v1.z,
                            v2.x, v2.y, v2.z,
                            v3.x, v3.y, v3.z,

                            v1.x, v1.y, v1.z,
                            v3.x, v3.y, v3.z,
                            v4.x, v4.y, v4.z
                            });
                    }
                }
            }
        }
        return vertices;
    }

    std::vector<float> sphere(float radius, int stacks, int slices) {
        std::vector<float> vertices;

        for (int stack = 0; stack < stacks; ++stack) {
            float pitch1 = M_PI / 2 - static_cast<float>(stack) / stacks * M_PI;
            float pitch2 = M_PI / 2 - static_cast<float>(stack + 1) / stacks * M_PI;

            for (int slice = 0; slice < slices; ++slice) {
                float yaw1 = static_cast<float>(slice) / slices * 2 * M_PI;
                float yaw2 = static_cast<float>(slice + 1) / slices * 2 * M_PI;

                // Compute the 4 vertices of the quad (on the sphere) using the new conversion
                glm::vec3 t1 = radius * glm::vec3(cos(pitch1) * sin(yaw1), sin(pitch1), cos(pitch1) * cos(yaw1));
                glm::vec3 t2 = radius * glm::vec3(cos(pitch1) * sin(yaw2), sin(pitch1), cos(pitch1) * cos(yaw2));
                glm::vec3 b1 = radius * glm::vec3(cos(pitch2) * sin(yaw1), sin(pitch2), cos(pitch2) * cos(yaw1));
                glm::vec3 b2 = radius * glm::vec3(cos(pitch2) * sin(yaw2), sin(pitch2), cos(pitch2) * cos(yaw2));

                // lower tri
                vertices.insert(vertices.end(), {
                    t1.x, t1.y, t1.z,
                    b1.x, b1.y, b1.z,
                    b2.x, b2.y, b2.z,
                    });

                // upper tri
                if (stack != 0) {
                    vertices.insert(vertices.end(), {
                        b2.x, b2.y, b2.z,
                        t2.x, t2.y, t2.z,
                        t1.x, t1.y, t1.z
                        });
                }
            }
        }

        return vertices;
    }

    std::vector<float> cone(float radius, float height, int slices, int stacks) {
        std::vector<float> vertices;

        const float stackHeight = height / stacks;

        for (int stack = 0; stack < stacks; stack++) {
            // bottom/top stack ycoords
            float yb = stack * stackHeight;
            float yt = (stack + 1) * stackHeight;

            // bottom/top stack radii
            float rb = radius * (1.0f - (float)stack / stacks);
            float rt = radius * (1.0f - (float)(stack + 1) / stacks);

            for (int slice = 0; slice < slices; slice++) {
                float yaw1 = static_cast<float>(slice) / slices * 2 * M_PI;
                float yaw2 = static_cast<float>(slice + 1) / slices * 2 * M_PI;

                // (cw) base slice -> done in a single stack
                if (stack != stacks - 1) {

                    glm::vec3 c1(radius * sinf(yaw1), 0.0f, radius * cosf(yaw1));
                    glm::vec3 c2(radius * sinf(yaw2), 0.0f, radius * cosf(yaw2));

                    vertices.insert(vertices.end(), {
                        c1.x, c1.y, c1.z,
                        0.0f, 0.0f, 0.0f,
                        c2.x, c2.y, c2.z,
                        });
                }

                glm::vec3 t1(rt * sinf(yaw1), yt, rt * cosf(yaw1));
                glm::vec3 t2(rt * sinf(yaw2), yt, rt * cosf(yaw2));
                glm::vec3 b1(rb * sinf(yaw1), yb, rb * cosf(yaw1));
                glm::vec3 b2(rb * sinf(yaw2), yb, rb * cosf(yaw2));

                // lateral lower tri
                vertices.insert(vertices.end(), {
                    b1.x, b1.y, b1.z,
                    b2.x, b2.y, b2.z,
                    t1.x, t1.y, t1.z,
                    });

                // lateral upper tri
                if (stack != stacks - 1) {
                    vertices.insert(vertices.end(), {
                        t1.x, t1.y, t1.z,
                        b2.x, b2.y, b2.z,
                        t2.x, t2.y, t2.z
                        });
                }
            }
        }
        return vertices;
    }

    std::vector<float> tube(float iradius, float oradius, float height, int slices) {

        std::vector<float> vertices;

        const float halfHeight = height * 0.5f;
        const float yawStep = 2.0f * M_PI / slices;

        // Generate top and bottom caps
        for (int slice = 0; slice < slices; slice++) {
            float yaw1 = slice * yawStep;
            float yaw2 = yaw1 + yawStep;

            // outer/inner top/bottom 1/2
            glm::vec3 ot1(oradius * sinf(yaw1), halfHeight, oradius * cosf(yaw1));
            glm::vec3 ot2(oradius * sinf(yaw2), halfHeight, oradius * cosf(yaw2));

            glm::vec3 it1(iradius * sinf(yaw1), halfHeight, iradius * cosf(yaw1));
            glm::vec3 it2(iradius * sinf(yaw2), halfHeight, iradius * cosf(yaw2));

            glm::vec3 ob1(oradius * sinf(yaw1), -halfHeight, oradius * cosf(yaw1));
            glm::vec3 ob2(oradius * sinf(yaw2), -halfHeight, oradius * cosf(yaw2));

            glm::vec3 ib1(iradius * sinf(yaw1), -halfHeight, iradius * cosf(yaw1));
            glm::vec3 ib2(iradius * sinf(yaw2), -halfHeight, iradius * cosf(yaw2));


            vertices.insert(vertices.end(), {

                //// (ccw) top
                // outer tri
                ot1.x, ot1.y, ot1.z,
                ot2.x, ot2.y, ot2.z,
                it1.x, it1.y, it1.z,
                // inner tri
                it1.x, it1.y, it1.z,
                ot2.x, ot2.y, ot2.z,
                it2.x, it2.y, it2.z,

                //// (cw) bottom
                // outer tri
                ob2.x, ob2.y, ob2.z,
                ob1.x, ob1.y, ob1.z,
                ib1.x, ib1.y, ib1.z,
                // inner tri
                ib1.x, ib1.y, ib1.z,
                ib2.x, ib2.y, ib2.z,
                ob2.x, ob2.y, ob2.z,

                //// (ccw) outer lateral
                // lower tri
                ob1.x, ob1.y, ob1.z,
                ob2.x, ob2.y, ob2.z,
                ot2.x, ot2.y, ot2.z,
                // upper tri
                ob1.x, ob1.y, ob1.z,
                ot2.x, ot2.y, ot2.z,
                ot1.x, ot1.y, ot1.z,

                //// (cw) inner lateral
                // lower tri 
                ib1.x, ib1.y, ib1.z,
                it2.x, it2.y, it2.z,
                ib2.x, ib2.y, ib2.z,
                // upper tri
                ib1.x, ib1.y, ib1.z,
                it1.x, it1.y, it1.z,
                it2.x, it2.y, it2.z,
                });
        }
        return vertices;
    }
}

namespace modelFiles {

    inline std::filesystem::path getDirectory(const std::string& customPath = "") {
        if (!customPath.empty()) {
            std::filesystem::create_directories(customPath);
            return customPath;
        }

        auto path = std::filesystem::current_path().parent_path() / "models";
        std::filesystem::create_directories(path);
        return path;
    }

    inline bool save(const std::vector<float>& vertices, const std::string& filename, const std::string& directory = "") {
        const auto path = directory.empty() ? getDirectory() / filename : std::filesystem::path(directory) / filename;

        if (std::filesystem::exists(path)) {
            std::filesystem::remove(path);
        }

        std::ofstream file(path);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << path << std::endl;
            return false;
        }

        for (size_t i = 0; i < vertices.size(); i += 3) {
            file << vertices[i] << "," << vertices[i + 1] << "," << vertices[i + 2] << "\n";
        }

        return true;
    }

}

#endif
