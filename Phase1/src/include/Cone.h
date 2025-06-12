#ifndef CONE_H
#define CONE_H

#include "Model.h"

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
            vertices.push_back(Vec3(x1, 0.0f, z1));
            vertices.push_back(Vec3(0.0f, 0.0f, 0.0f));
            vertices.push_back(Vec3(x2, 0.0f, z2));
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
                Vec3 bottomLeft(x1, yBottom, z1);
                Vec3 bottomRight(x2, yBottom, z2);
                Vec3 topLeft(x3, yTop, z3);
                Vec3 topRight(x4, yTop, z4);

                // First triangle
                vertices.push_back(bottomLeft);
                vertices.push_back(bottomRight);
                vertices.push_back(topLeft);

                // Second triangle
                vertices.push_back(topLeft);
                vertices.push_back(bottomRight);
                vertices.push_back(topRight);
            }
        }
    }
};


#endif