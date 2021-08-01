#pragma once
#include <fstream>
#include "engine/fluidsimulation.h"
#include "engine/stopwatch.h"
#include "../ext/DXViewer/src/Win32App.h" // This includes ISimulation.h

class SubFluidSimulation : public FluidSimulation, public ISimulation
{
private:
    TriangleMesh isomesh2;
    void _outputSurfaceMeshThread(std::vector<vmath::vec3>* particles, MeshLevelSet* solidSDF) override;

    // The ones originally in main.cpp
    void writeSurfaceMesh(int frameno);
    TriangleMesh getTriangleMeshFromAABB(AABB bbox);


public:
    SubFluidSimulation(int isize, int jsize, int ksize, double dx);
    ~SubFluidSimulation();

    void initialize() override;

    // Functions for virtual class
    void IUpdate(double timestep) override;
    std::vector<Vertex> IGetVertice() override;
    std::vector<unsigned int> IGetIndice() override;
};

