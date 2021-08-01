#pragma once
#include "engine/fluidsimulation.h"
#include "engine/stopwatch.h"
#include "../ext/DXViewer/src/ISimulation.h"

class SubFluidSimulation : public FluidSimulation, ISimulation
{
public:
    TriangleMesh isomesh2;

    SubFluidSimulation(int isize, int jsize, int ksize, double dx);
    ~SubFluidSimulation();

    void _outputSurfaceMeshThread(std::vector<vmath::vec3>* particles,
        MeshLevelSet* solidSDF);

    void IUpdate(double timestep) override;
    std::vector<float> IGetVertice() override;
};

