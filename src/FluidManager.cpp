#include "FluidManager.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace std;
using namespace vmath;
using namespace DXViewer::util;
using namespace DXViewer::xmfloat3;

FluidManager::FluidManager(int isize, int jsize, int ksize, double dx, double timeStep)
    :_isize(isize), _jsize(jsize), _ksize(ksize), _dx(dx), _timeStep(timeStep)
{
    _fluidsim = new FluidSimulation(_isize, _jsize, _ksize, _dx);
}

FluidManager::~FluidManager()
{
    delete _fluidsim;
}

TriangleMesh FluidManager::getTriangleMeshFromAABB(AABB bbox) {
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


void FluidManager::initialize()
{
    assert(fluidsim != nullptr);

    _fluidsim->setSurfaceSubdivisionLevel(2);

    double x, y, z;
    _fluidsim->getSimulationDimensions(&x, &y, &z);

    double boxWidth = (1.0 / 3.0) * x;
    double boxHeight = (1.0 / 3.0) * y;
    double boxDepth = (1.0 / 3.0) * z;
    vmath::vec3 boxPosition(0.5 * (x - boxWidth), 0.5 * (y - boxHeight), 0.5 * (z - boxDepth));
    AABB box(boxPosition, boxWidth, boxHeight, boxDepth);
    TriangleMesh boxMesh = getTriangleMeshFromAABB(box);
    MeshObject boxFluidObject(_isize, _jsize, _ksize, _dx);
    boxFluidObject.updateMeshStatic(boxMesh);
    _fluidsim->addMeshFluid(boxFluidObject);

    _fluidsim->addBodyForce(0.0, -25.0, 0.0);
    _fluidsim->initialize();

}


#pragma region Implementation
// ################################## Implementation ####################################
// Simulation methods
void FluidManager::iUpdate()
{
    int frameno = _fluidsim->getCurrentFrame();
    _fluidsim->update(_timeStep);
}

void FluidManager::iResetSimulationState(vector<ConstantBuffer>& constantBuffer)
{
}


// Mesh methods
vector<Vertex>& FluidManager::iGetVertice()
{
    _vertice.clear();
    _normal.clear();

    TriangleMesh isomesh = _fluidsim->getIsomesh();

    for (int i = 0; i < isomesh.vertices.size(); i++)
    {
        Vertex v;
        v.pos.x = isomesh.vertices[i].x;
        v.pos.y = isomesh.vertices[i].y;
        v.pos.z = isomesh.vertices[i].z;

        _normal.push_back(vec3(0.0f, 0.0f, 0.0f));

        _vertice.push_back(v);
    }


    for (int i = 0; i < isomesh.triangles.size(); i++)
    {
        int i0 = isomesh.triangles[i].tri[0];
        int i1 = isomesh.triangles[i].tri[1];
        int i2 = isomesh.triangles[i].tri[2];

        
        vec3 v0 = isomesh.vertices[i0];
        vec3 v1 = isomesh.vertices[i1];
        vec3 v2 = isomesh.vertices[i2];

        vec3 e0 = v1 - v0;
        vec3 e1 = v2 - v0;
        vec3 faceN = cross(e0, e1);

        _normal[i0] += faceN;
        _normal[i1] += faceN;
        _normal[i2] += faceN;
    }


    for (int i = 0; i < isomesh.vertices.size(); i++)
    {
        vec3 nor = normalize(_normal[i]);
        _vertice[i].nor.x = nor.x;
        _vertice[i].nor.y = nor.y;
        _vertice[i].nor.z = nor.z;
    }

    return _vertice;
}

vector<unsigned int>& FluidManager::iGetIndice()
{
    _indice.clear();
    TriangleMesh isomesh = _fluidsim->getIsomesh();

    for (int i = 0; i < isomesh.triangles.size(); i++)
    {
        for (int j = 0; j < 3; j++)
        {
            unsigned int a = static_cast<unsigned int>(isomesh.triangles[i].tri[j]);
            _indice.push_back(a);
        }
        //std::cout << "\n";
    }

    return _indice;
}

UINT FluidManager::iGetVertexBufferSize()
{
    return 1000000;
}

UINT FluidManager::iGetIndexBufferSize()
{
    return 1000000;
}

DirectX::XMINT3 FluidManager::iGetObjectCount()
{
    return { 1, 1, 1 };
}

DirectX::XMFLOAT3 FluidManager::iGetObjectSize()
{
    return XMFLOAT3(
        0.0f, 0.0f, 0.0f
    );
}

DirectX::XMFLOAT3 FluidManager::iGetObjectPositionOffset()
{
    return DirectX::XMFLOAT3(_isize * 0.06f, _jsize * 0.06f, _ksize * 0.06f);
}


// DirectX methods
void FluidManager::iCreateObject(vector<ConstantBuffer>& constantBuffer)
{
    struct ConstantBuffer objectCB;
    // Multiply by a specific value to make a stripe
    objectCB.world = transformMatrix(0.0f, 0.0f, 0.0f, 1.0f);
    objectCB.worldViewProj = transformMatrix(0.0f, 0.0f, 0.0f);
    objectCB.transInvWorld = transformMatrix(0.0f, 0.0f, 0.0f);
    objectCB.color = XMFLOAT4(0.1f, 0.25f, 0.3f, 1.0f);

    constantBuffer.push_back(objectCB);
}

void FluidManager::iUpdateConstantBuffer(vector<ConstantBuffer>& constantBuffer, int i)
{
}

void FluidManager::iDraw(ComPtr<ID3D12GraphicsCommandList>& mCommandList, int size, UINT indexCount, int i)
{
    mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mCommandList->DrawIndexedInstanced(_indice.size(), 1, 0, 0, 0);
}

void FluidManager::iSetDXApp(DX12App* dxApp)
{
    _dxapp = dxApp;
}

UINT FluidManager::iGetConstantBufferSize()
{
    return 1;
}

bool FluidManager::iIsUpdated()
{
    return _updateFlag;
}


// WndProc methods
void FluidManager::iWMCreate(HWND hwnd, HINSTANCE hInstance)
{
    CreateWindow(L"button", L"Respawn", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        85, 250, 110, 30, hwnd, reinterpret_cast<HMENU>(_COM::RESPAWN), hInstance, NULL);
    CreateWindow(L"button", _updateFlag ? L"¡«" : L"¢º", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        65, 290, 50, 25, hwnd, reinterpret_cast<HMENU>(_COM::PLAY), hInstance, NULL);
    CreateWindow(L"button", L"¡á", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        115, 290, 50, 25, hwnd, reinterpret_cast<HMENU>(_COM::STOP), hInstance, NULL);
    CreateWindow(L"button", L"¢ºl", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        165, 290, 50, 25, hwnd, reinterpret_cast<HMENU>(_COM::NEXTSTEP), hInstance, NULL);
}

void FluidManager::iWMTimer(HWND hwnd)
{
}

void FluidManager::iWMDestory(HWND hwnd)
{
}

void FluidManager::iWMHScroll(HWND hwnd, WPARAM wParam, LPARAM lParam, HINSTANCE hInstance)
{
}

void FluidManager::iWMCommand(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, HINSTANCE hInstance)
{
    switch (LOWORD(wParam))
    {
        // ### Execution buttons ###
        case static_cast<int>(_COM::PLAY) :
        {
            _updateFlag = !_updateFlag;
            SetDlgItemText(hwnd, static_cast<int>(_COM::PLAY), _updateFlag ? L"¡«" : L"¢º");

            EnableWindow(GetDlgItem(hwnd, static_cast<int>(_COM::STOP)), true);
            EnableWindow(GetDlgItem(hwnd, static_cast<int>(_COM::NEXTSTEP)), !_updateFlag);
        }
        break;
        case static_cast<int>(_COM::STOP) :
        {
            _dxapp->resetSimulationState();
            _dxapp->update();
            _dxapp->draw();
        }
        break;
        case static_cast<int>(_COM::NEXTSTEP) :
        {
            iUpdate();
            _dxapp->update();
            _dxapp->draw();
        }
        break;
        case static_cast<int>(_COM::RESPAWN) :
        {
            initialize();
            iUpdate();
            _dxapp->update();
            _dxapp->draw();

            EnableWindow(GetDlgItem(hwnd, static_cast<int>(_COM::RESPAWN)), false);
        }
        break;
    }
}

// #######################################################################################
#pragma endregion