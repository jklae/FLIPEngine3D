#include "../ext/DXViewer/src/Win32App.h"
#include "SubFluidSimulation.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
    int isize = 20;
    int jsize = 20;
    int ksize = 20;
    double dx = 0.125;
    double timestep = 1.0 / 30.0;

    SubFluidSimulation* fluidsim = new SubFluidSimulation(isize, jsize, ksize, dx);
    fluidsim->initialize();

    DX12App* dxapp = new DX12App();
    dxapp->SetSimulation(fluidsim, timestep);

    Win32App winApp(800, 600);
    winApp.Initialize(hInstance);

    winApp.InitDirectX(dxapp);

    return winApp.Run();
}