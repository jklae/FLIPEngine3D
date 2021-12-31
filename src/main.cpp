// Console window is displayed in debug mode.
#ifdef _DEBUG
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif

#include "SubFluidSimulation.h" // This includes Win32App.h

using namespace DXViewer::xmint3;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
    int isize = 60;
    int jsize = 60;
    int ksize = 60;
    double dx = 0.125;
    double timestep = 1.0 / 30.0;

    SubFluidSimulation* fluidsim = new SubFluidSimulation(isize, jsize, ksize, dx, timestep);
    fluidsim->initialize();

    // DirectX init
    DX12App* dxapp = new DX12App();
    int maxSize = max_element(DirectX::XMINT3(isize, jsize, ksize));
    dxapp->setCameraProperties(
        PROJ::PERSPECTIVE,
        0.0f, maxSize * 0.25f,
        -0.3f, -0.3f);
    /*dxapp->setCameraProperties(
        PROJ::ORTHOGRAPHIC,
        maxSize * 0.0002f, maxSize * 0.2f,
        -0.3f, -0.3f);*/
    dxapp->setBackgroundColor(DirectX::Colors::LightSlateGray);

    // Window init
    Win32App winApp(800, 800);
    winApp.initialize(hInstance, dxapp, fluidsim);

    return winApp.run();
}