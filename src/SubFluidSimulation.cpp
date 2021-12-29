#include "SubFluidSimulation.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace std;
using namespace vmath;
using namespace DXViewer::util;

SubFluidSimulation::SubFluidSimulation(int isize, int jsize, int ksize, double dx, double timeStep)
    :FluidSimulation(isize, jsize, ksize, dx), _timeStep(timeStep)
{
}

SubFluidSimulation::~SubFluidSimulation()
{
}

void SubFluidSimulation::writeSurfaceMesh(int frameno) {
    std::ostringstream ss;
    ss << frameno;
    std::string frameString = ss.str();
    //frameString.insert(frameString.begin(), 6 - frameString.size(), '0');
    std::string filepath = "ply/fluid_" + frameString + ".ply";

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


#pragma region Implementation
// ################################## Implementation ####################################
// Simulation methods
void SubFluidSimulation::iUpdate()
{
    int frameno = getCurrentFrame();
    FluidSimulation::update(_timeStep);
    writeSurfaceMesh(frameno);
}

void SubFluidSimulation::iResetSimulationState(vector<ConstantBuffer>& constantBuffer)
{
}


// Mesh methods
vector<Vertex>& SubFluidSimulation::iGetVertice()
{
    _vertice.clear();
    _normal.clear();

    for (int i = 0; i < _isomesh.vertices.size(); i++)
    {
        Vertex v;
        v.pos.x = _isomesh.vertices[i].x;
        v.pos.y = _isomesh.vertices[i].y;
        v.pos.z = _isomesh.vertices[i].z;

        _normal.push_back(vec3(0.0f, 0.0f, 0.0f));

        _vertice.push_back(v);
    }


    for (int i = 0; i < _isomesh.triangles.size(); i++)
    {
        int i0 = _isomesh.triangles[i].tri[0];
        int i1 = _isomesh.triangles[i].tri[1];
        int i2 = _isomesh.triangles[i].tri[2];

        
        vec3 v0 = _isomesh.vertices[i0];
        vec3 v1 = _isomesh.vertices[i1];
        vec3 v2 = _isomesh.vertices[i2];

        vec3 e0 = v1 - v0;
        vec3 e1 = v2 - v0;
        vec3 faceN = cross(e0, e1);

        _normal[i0] += faceN;
        _normal[i1] += faceN;
        _normal[i2] += faceN;
    }


    for (int i = 0; i < _isomesh.vertices.size(); i++)
    {
        vec3 nor = normalize(_normal[i]);
        _vertice[i].nor.x = nor.x;
        _vertice[i].nor.y = nor.y;
        _vertice[i].nor.z = nor.z;
    }

    return _vertice;
}

vector<unsigned int>& SubFluidSimulation::iGetIndice()
{
    _indice.clear();

    for (int i = 0; i < _isomesh.triangles.size(); i++)
    {
        for (int j = 0; j < 3; j++)
        {
            unsigned int a = static_cast<unsigned int>(_isomesh.triangles[i].tri[j]);
            _indice.push_back(a);
        }
        //std::cout << "\n";
    }

    return _indice;
}

UINT SubFluidSimulation::iGetVertexBufferSize()
{
    return 1000000;
}

UINT SubFluidSimulation::iGetIndexBufferSize()
{
    return 1000000;
}


// DirectX methods
void SubFluidSimulation::iCreateObject(vector<ConstantBuffer>& constantBuffer)
{
    struct ConstantBuffer objectCB;
    // Multiply by a specific value to make a stripe
    objectCB.world = transformMatrix(0.0f, 0.0f, 0.0f, 1.0f);
    objectCB.worldViewProj = transformMatrix(0.0f, 0.0f, 0.0f);
    objectCB.color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    constantBuffer.push_back(objectCB);
}

void SubFluidSimulation::iUpdateConstantBuffer(vector<ConstantBuffer>& constantBuffer, int i)
{
}

void SubFluidSimulation::iDraw(ComPtr<ID3D12GraphicsCommandList>& mCommandList, int size, UINT indexCount, int i)
{
    mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mCommandList->DrawIndexedInstanced(_indice.size(), 1, 0, 0, 0);
}

void SubFluidSimulation::iSetDXApp(DX12App* dxApp)
{
}

UINT SubFluidSimulation::iGetConstantBufferSize()
{
    return 1;
}

DirectX::XMINT2 SubFluidSimulation::iGetDomainSize()
{
    return { 5, 5 };
}


// WndProc methods
void SubFluidSimulation::iWMCreate(HWND hwnd, HINSTANCE hInstance)
{
}

void SubFluidSimulation::iWMTimer(HWND hwnd)
{
}

void SubFluidSimulation::iWMDestory(HWND hwnd)
{
}

void SubFluidSimulation::iWMHScroll(HWND hwnd, WPARAM wParam, LPARAM lParam, HINSTANCE hInstance)
{
}

void SubFluidSimulation::iWMCommand(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, HINSTANCE hInstance)
{
}


// Win32 methods
bool SubFluidSimulation::iGetUpdateFlag()
{
    return true;
}
// #######################################################################################
#pragma endregion