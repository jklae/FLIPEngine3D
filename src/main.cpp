// Console window is displayed in debug mode.
#ifdef _DEBUG
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif

#include "FluidManager.h" // This includes Win32App.h

using namespace DXViewer::xmint3;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
    int isize = 30;
    int jsize = 30;
    int ksize = 30;
    double dx = 0.125;
    double timestep = FPS30_D;

    // Simulation init
    FluidManager* fluidsim = new FluidManager(isize, jsize, ksize, dx, timestep);
    fluidsim->initialize();

    // DirectX init
    DX12App* dxapp = new DX12App();
    int maxSize = max_element(DirectX::XMINT3(isize, jsize, ksize));
    dxapp->setCameraProperties(
        PROJ::PERSPECTIVE,
        maxSize * 0.25f, -0.3f, -0.3f);
   /* dxapp->setCameraProperties(
        PROJ::ORTHOGRAPHIC,
        maxSize * 0.00035f, 
        maxSize * 0.2f, -0.3f, -0.3f);*/
    dxapp->setBackgroundColor(DirectX::Colors::LightSlateGray);
    dxapp->setLightPosition(1.0f, 1.0f, 1.0f);

    // Window init
    Win32App winApp(500, 500);
    winApp.setWinName(L"3D Fluid Simulation");
    winApp.setWinOffset(400, 200);
    winApp.initialize(hInstance, dxapp, fluidsim);

    return winApp.run();
}