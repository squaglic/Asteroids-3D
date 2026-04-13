#include "GX.h"

void Demo2DRender(bool isInPause);
void Demo2DLogic(float deltaT);
void Demo2DInit();

void demo3DRender(bool isInPause);      void demo3DLogic(float dT);         void demo3DInit();
void demoForestRender(bool isInPause);  void demoForestLogic(float dT);     void demoForestInit();

int main(int argc, char* argv[])
{
    // On garde les noms de fonctions existants, mais la scène "forest"
    // devient maintenant notre jeu 3D inspiré de Asteroids.
    GX::RegisterGame(demoForestRender, demoForestLogic, demoForestInit, 3);

    int HeightPix = 800;
    int WidthPix = 1200;

    GX::initWindow(V2(WidthPix, HeightPix), V2(120, 40), string("GX - Asteroids 3D"));
    GX::initGX();
    return 0;
}