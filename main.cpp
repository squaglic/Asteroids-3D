#include "GX.h"

void Demo2DRender(bool isInPause);
void Demo2DLogic(float deltaT);
void Demo2DInit();
 

void demo3DRender(bool isInPause);		void demo3DLogic(float dT);			void demo3DInit();
void demoForestRender(bool isInPause); void demoForestLogic(float dT);	void demoForestInit();



int main(int argc, char* argv[])
{
	GX::RegisterGame(demoForestRender, demoForestLogic, demoForestInit,3);
	


	int HeighPix = 800;   // hauteur de la fenêtre d'application
	int WidthPix = 600;   // largeur de la fenêtre d'application

	// crée la fenêtre de l'application
	GX::initWindow(V2(WidthPix, HeighPix), V2(200, 0), string("GXD DEMO"));
	 

	// lance l'application 
	GX::initGX();


	// aucun code ici
	return 0;
}