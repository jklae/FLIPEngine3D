#include "SubFluidSimulation.h"


SubFluidSimulation::SubFluidSimulation(int isize, int jsize, int ksize, double dx)
    :FluidSimulation(isize, jsize, ksize, dx)
{
}

SubFluidSimulation::~SubFluidSimulation()
{
}

void SubFluidSimulation::_outputSurfaceMeshThread(std::vector<vmath::vec3>* particles,
    MeshLevelSet* solidSDF)
{
    if (!_isSurfaceMeshReconstructionEnabled) { return; }

    _logfile.logString(_logfile.getTime() + " BEGIN       Generate Surface Mesh");

    StopWatch t;
    t.start();

    _applyMeshingVolumeToSDF(solidSDF);
    _filterParticlesOutsideMeshingVolume(particles);

    TriangleMesh isomesh, previewmesh;
    _polygonizeOutputSurface(isomesh, previewmesh, particles, solidSDF);
    delete particles;
    delete solidSDF;

    isomesh.removeMinimumTriangleCountPolyhedra(_minimumSurfacePolyhedronTriangleCount);

    _removeMeshNearDomain(isomesh);
    _smoothSurfaceMesh(isomesh);
    _smoothSurfaceMesh(previewmesh);
    _invertContactNormals(isomesh);

    if (_isSurfaceMotionBlurEnabled) {
        TriangleMesh blurData;
        blurData.vertices.reserve(isomesh.vertices.size());
        double dt = _currentFrameDeltaTime;
        for (size_t i = 0; i < isomesh.vertices.size(); i++) {
            vmath::vec3 p = isomesh.vertices[i];
            vmath::vec3 t = _MACVelocity.evaluateVelocityAtPositionLinear(p) * _domainScale * dt;
            blurData.vertices.push_back(t);
        }

        _getTriangleMeshFileData(blurData, _outputData.surfaceBlurData);
        _outputData.frameData.surfaceblur.enabled = 1;
        _outputData.frameData.surfaceblur.vertices = (int)blurData.vertices.size();
        _outputData.frameData.surfaceblur.triangles = (int)blurData.triangles.size();
        _outputData.frameData.surfaceblur.bytes = (unsigned int)_outputData.surfaceBlurData.size();
    }

    vmath::vec3 scale(_domainScale, _domainScale, _domainScale);
    isomesh.scale(scale);
    isomesh.translate(_domainOffset);
    previewmesh.scale(scale);
    previewmesh.translate(_domainOffset);

    _getTriangleMeshFileData(isomesh, _outputData.surfaceData);

    _outputData.frameData.surface.enabled = 1;
    _outputData.frameData.surface.vertices = (int)isomesh.vertices.size();
    _outputData.frameData.surface.triangles = (int)isomesh.triangles.size();
    _outputData.frameData.surface.bytes = (unsigned int)_outputData.surfaceData.size();

    if (_isPreviewSurfaceMeshEnabled) {
        _getTriangleMeshFileData(previewmesh, _outputData.surfacePreviewData);
        _outputData.frameData.preview.enabled = 1;
        _outputData.frameData.preview.vertices = (int)previewmesh.vertices.size();
        _outputData.frameData.preview.triangles = (int)previewmesh.triangles.size();
        _outputData.frameData.preview.bytes = (unsigned int)_outputData.surfacePreviewData.size();
    }

    t.stop();
    _timingData.outputMeshSimulationData += t.getTime();

    _logfile.logString(_logfile.getTime() + " COMPLETE    Generate Surface Mesh");

    isomesh2 = isomesh;

}

void SubFluidSimulation::IUpdate(double timestep)
{
    FluidSimulation::update(timestep);
}
