#include "SubFluidSimulation.h"

using namespace std;

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


void SubFluidSimulation::writeSurfaceMesh(int frameno) {
    std::ostringstream ss;
    ss << frameno;
    std::string frameString = ss.str();
    //frameString.insert(frameString.begin(), 6 - frameString.size(), '0');
    std::string filepath = "C:/tmp/fluid_" + frameString + ".ply";

    std::vector<char>* data = getSurfaceData();
    std::ofstream ply(filepath.c_str(), std::ios::out | std::ios::binary);
    ply.write(data->data(), data->size());
    ply.close();
}

TriangleMesh SubFluidSimulation::getTriangleMeshFromAABB(AABB bbox) {
    vmath::vec3 p = bbox.position;
    std::vector<vmath::vec3> verts{
        vmath::vec3(p.x, p.y, p.z),
        vmath::vec3(p.x + bbox.width, p.y, p.z),
        vmath::vec3(p.x + bbox.width, p.y, p.z + bbox.depth),
        vmath::vec3(p.x, p.y, p.z + bbox.depth),
        vmath::vec3(p.x, p.y + bbox.height, p.z),
        vmath::vec3(p.x + bbox.width, p.y + bbox.height, p.z),
        vmath::vec3(p.x + bbox.width, p.y + bbox.height, p.z + bbox.depth),
        vmath::vec3(p.x, p.y + bbox.height, p.z + bbox.depth)
    };

    std::vector<Triangle> tris{
        Triangle(0, 1, 2), Triangle(0, 2, 3), Triangle(4, 7, 6), Triangle(4, 6, 5),
        Triangle(0, 3, 7), Triangle(0, 7, 4), Triangle(1, 5, 6), Triangle(1, 6, 2),
        Triangle(0, 4, 5), Triangle(0, 5, 1), Triangle(3, 2, 6), Triangle(3, 6, 7)
    };

    TriangleMesh m;
    m.vertices = verts;
    m.triangles = tris;

    return m;
}


void SubFluidSimulation::initialize()
{
    setSurfaceSubdivisionLevel(2);

    double x, y, z;
    getSimulationDimensions(&x, &y, &z);

    double boxWidth = (1.0 / 3.0) * x;
    double boxHeight = (1.0 / 3.0) * y;
    double boxDepth = (1.0 / 3.0) * z;
    vmath::vec3 boxPosition(0.5 * (x - boxWidth), 0.5 * (y - boxHeight), 0.5 * (z - boxDepth));
    AABB box(boxPosition, boxWidth, boxHeight, boxDepth);
    TriangleMesh boxMesh = getTriangleMeshFromAABB(box);
    MeshObject boxFluidObject(_isize, _jsize, _ksize, _dx);
    boxFluidObject.updateMeshStatic(boxMesh);
    addMeshFluid(boxFluidObject);

    addBodyForce(0.0, -25.0, 0.0);
    FluidSimulation::initialize();

}




void SubFluidSimulation::IUpdate(double timestep)
{
    int frameno = getCurrentFrame();
    FluidSimulation::update(timestep);
    writeSurfaceMesh(frameno);
}

vector<float> SubFluidSimulation::IGetVertice()
{
    vector<float> vertice;
    vertice.clear();

    for (int i = 0; i < isomesh2.vertices.size(); i++)
    {
        vertice.push_back(isomesh2.vertices[i].x);
        vertice.push_back(isomesh2.vertices[i].y);
        vertice.push_back(isomesh2.vertices[i].z);
    }

    return vertice;
}

vector<unsigned int> SubFluidSimulation::IGetIndice()
{
    vector<unsigned int> indice;
    indice.clear();

    for (int i = 0; i < isomesh2.triangles.size(); i++)
    {
        for (int j = 0; j < 3; j++)
        {
            unsigned int a = static_cast<unsigned int>(isomesh2.triangles[i].tri[j]);
            indice.push_back(a);
        }
        //std::cout << "\n";
    }

    return indice;
}
