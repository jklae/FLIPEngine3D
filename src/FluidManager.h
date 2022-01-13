#pragma once
#include <fstream>
#include "engine/fluidsimulation.h"
#include "engine/stopwatch.h"
#include "Win32App.h" // This includes ISimulation.h

class FluidManager : public ISimulation
{
public:
	FluidManager(int isize, int jsize, int ksize, double dx, double timeStep);
	~FluidManager() override;

	void initialize();

#pragma region Implementation
	// ################################## Implementation ####################################
	// Simulation methods
	void iUpdate() override;
	void iResetSimulationState(std::vector<ConstantBuffer>& constantBuffer) override;

	// Mesh methods
	std::vector<Vertex>& iGetVertice() override;
	std::vector<unsigned int>& iGetIndice() override;
	UINT iGetVertexBufferSize() override;
	UINT iGetIndexBufferSize() override;
	DirectX::XMINT3 iGetObjectCount() override;
	DirectX::XMFLOAT3 iGetObjectSize() override;
	DirectX::XMFLOAT3 iGetObjectPositionOffset() override;

	// DirectX methods
	void iCreateObject(std::vector<ConstantBuffer>& constantBuffer) override;
	void iUpdateConstantBuffer(std::vector<ConstantBuffer>& constantBuffer, int i) override;
	void iDraw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& mCommandList, int size, UINT indexCount, int i) override;
	void iSetDXApp(DX12App* dxApp) override;
	UINT iGetConstantBufferSize() override;
	bool iIsUpdated() override;

	// WndProc methods
	void iWMCreate(HWND hwnd, HINSTANCE hInstance) override;
	void iWMCommand(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, HINSTANCE hInstance) override;
	void iWMHScroll(HWND hwnd, WPARAM wParam, LPARAM lParam, HINSTANCE hInstance) override;
	void iWMTimer(HWND hwnd) override;
	void iWMDestory(HWND hwnd) override;
	// #######################################################################################
#pragma endregion

private:
	enum class _COM
	{
		PLAY, STOP, NEXTSTEP,
		TIME_TEXT, FRAME_TEXT
	};

	DX12App* _dxapp;
	bool _updateFlag = true;

	int _isize, _jsize, _ksize;
	double _dx;
	double _timeStep;

    std::vector<Vertex> _vertice;
    std::vector<vmath::vec3> _normal;
    std::vector<unsigned int> _indice;

    TriangleMesh getTriangleMeshFromAABB(AABB bbox);
	FluidSimulation* _fluidsim;

	clock_t _simTime = 0;
	int _simFrame = 0;
};

