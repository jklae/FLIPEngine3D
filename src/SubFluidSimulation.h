#pragma once
#include "engine/fluidsimulation.h"
#include "engine/stopwatch.h"
#include "../ext/DXViewer/src/ISimulation.h"
#include <fstream>

class SubFluidSimulation : public FluidSimulation, public ISimulation
{
public:
    TriangleMesh isomesh2;

    SubFluidSimulation(int isize, int jsize, int ksize, double dx);
    ~SubFluidSimulation();

    void _outputSurfaceMeshThread(std::vector<vmath::vec3>* particles,
        MeshLevelSet* solidSDF) override;

    void writeSurfaceMesh(int frameno);
    TriangleMesh getTriangleMeshFromAABB(AABB bbox);

    void initialize() override;

    void IUpdate(double timestep) override;
    std::vector<float> IGetVertice() override;
    std::vector<unsigned int> IGetIndice() override;
};

