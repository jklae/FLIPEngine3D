// Console window is displayed in debug mode.
#ifdef _DEBUG
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif

#include "SubFluidSimulation.h" // This includes Win32App.h


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
    int isize = 40;
    int jsize = 40;
    int ksize = 40;
    double dx = 0.125;
    double timestep = 1.0 / 30.0;

    SubFluidSimulation* fluidsim = new SubFluidSimulation(isize, jsize, ksize, dx, timestep);
    fluidsim->initialize();

    // DirectX init
    DX12App* dxapp = new DX12App();
    dxapp->setProjectionType(PROJ::ORTHOGRAPHIC);
    dxapp->setBackgroundColor(DirectX::Colors::LightSlateGray);

    // Window init
    Win32App winApp(800, 800);
    winApp.initialize(hInstance, dxapp, fluidsim);

    return winApp.run();
}