#ifndef SPHERE_H
#define SPHERE_H

#include "Model.h"

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

                Vec3 v1 = Vec3(
                    radius * cosf(phi) * sinf(theta),
                    radius * sinf(phi),
                    radius * cosf(phi) * cosf(theta)
                );
                Vec3 v2 = Vec3(                               // v1->v2 is a vertical advance, use phiNext
                    radius * cosf(phiNext) * sinf(theta),
                    radius * sinf(phiNext),
                    radius * cosf(phiNext) * cosf(theta)
                );
                Vec3 v3 = Vec3(                               // v2->v3 is a horizontal advance, use thetaNext
                    radius * cosf(phiNext) * sinf(thetaNext),
                    radius * sinf(phiNext),
                    radius * cosf(phiNext) * cosf(thetaNext)
                );
                Vec3 v4 = Vec3(                              // v3->v4 is a vertical return, use phi
                    radius * cosf(phi) * sinf(thetaNext),
                    radius * sinf(phi),
                    radius * cosf(phi) * cosf(thetaNext)
                );


                if (iStack != stacks - 1) {
                    vertices.push_back(v1);
                    vertices.push_back(v2);
                    vertices.push_back(v3);
                }

                if (iStack != 0) {
                    vertices.push_back(v3);
                    vertices.push_back(v4);
                    vertices.push_back(v1);
                }
            }
        }
    }
};


#endif