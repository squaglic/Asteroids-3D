#pragma warning( disable : 4996 ) 

#include <cstdlib>
#include <vector>
#include <iostream>
#include <string>
#include "GX.h"
#include "GX TexturedFace3D.h"
#include "GX Objects3D.h"



struct GameData99nuits
{

	GX::Camera Cam;

	Object3D Arbre;

	Object3D Triangle;

	float heightMap[20][10];

	vector<V2> TreePos;

};
GameData99nuits G;



void demoForestRender(bool isInPause)
{
	//////////////////////////////////////////////////
	// intialisation de la scène



	GX::select3DMode(Color::Gray, G.Cam, 1200, 700);

	////////////////////////////////////////////////
	//
	// terrain
	 
	Object3D Cub = Object3D::CreateCube(10, Color::ColorFromHex(0x8B5A2B)); // marron terre
	Cub.setPivot(1, 3, 3);
	for (int ix = 0 ; ix  < 20 ; ix++)
	for (int iz = 0 ; iz  < 10 ; iz++)
	{ 
		GX::BeginLocalFrame();
		
		int x = ix * 10  -100;
		int z = iz * -10;
		float h = G.heightMap[ix][iz];
		GX::Translate(V3(x,h,z));
		TexturedFace T(V3(0,0,0), V3(10,0,0), V3(0,0,-10), false, ".\\assets\\tree\\herbe.png");
		T.Draw();
		GX::Translate(V3(0,-0.01,0));
		Cub.Draw(true,false,false);

		GX::EndLocalFrame();
	}
	

	/////////////////////////////////////////////////
	//
	//   Dessin des arbres

	for(V2 V : G.TreePos)
	{ 
		GX::BeginLocalFrame();
		float x = V.x * 10 - 100;
		float z = V.y * -10;
		int ix = int(V.x);
		int iz = int(V.y);
		float h = G.heightMap[ix][iz];
		GX::Translate(V3(x, h , z));
		G.Arbre.Draw(true, false, false);
		GX::EndLocalFrame();
	}

	/////////////////////////////////////////////////
	//
	//   Dessin du triangle qui suit le joueur

	GX::BeginLocalFrame();
	GX::Translate(V3(G.Cam.pos.x, 0, G.Cam.pos.z));
	G.Triangle.Draw(true, false, false);
	GX::EndLocalFrame();
	 
	////////////////////////////////////////
	// 
	//     affichage de la GUI

	GX::ActivateUImode();
	GX::drawStringFontMono(V3(50, 10, 0), "Forest - Move with ZQSD", 60, 3, Color::Black);
}

void demoForestLogic(float dT)
{
	float walkSpeed = 6;
	// G.Cam.pos.z -= walkSpeed * dT;
 
	// rotations
	float turnSpeed = 80;
	if (GX::isKeyPressed(Key::W)) G.Cam.yaw -= turnSpeed * dT;
	if (GX::isKeyPressed(Key::X)) G.Cam.yaw += turnSpeed * dT;

	// walk in the plane

	float rad = G.Cam.yaw * PI / 180.0f;
	V3 forward(sin(rad), 0, -cos(rad));
	V3 left(forward.z, 0, -forward.x);
	V3 right(-forward.z, 0, forward.x);


	if (GX::isKeyPressed(Key::Z)) G.Cam.pos = G.Cam.pos + walkSpeed * dT * forward;
	if (GX::isKeyPressed(Key::S)) G.Cam.pos = G.Cam.pos - walkSpeed * dT * forward;

	if (GX::isKeyPressed(Key::Q)) G.Cam.pos = G.Cam.pos + walkSpeed * dT * left;
	if (GX::isKeyPressed(Key::D)) G.Cam.pos = G.Cam.pos + walkSpeed * dT * right;
}



void demoForestInit()
{
	G.Cam.pos = V3(0, 20, 0);		// Caméra placée à 50m de haut
	G.Cam.pitch = 0;		// regard vers le bas

	G.Arbre = Object3D::LoadFromPLYInAssetFolder(".\\assets\\tree\\toon-pine-tree.ply");
	G.Arbre.forceSize(6);      // 3m de haut
	G.Arbre.setPivot(2, 1, 2); // pivot mis au milieu au pied de larbre
	G.Arbre.moveVertices(V3(0, 0.4, 0)); // manque le tronc, on sureleve l'arbre

	// Création d'un triangle rouge au sol
	G.Triangle = Object3D::CreateCube(15, Color::ColorFromHex(0xFF0000));
	G.Triangle.forceSize(15);
	G.Triangle.setPivot(7.5f, 1, 7.5f);

	for (int ix = 0; ix < 20; ix++)
		for (int iz = 0; iz < 10; iz++)
		{
			float h = (2.0f * rand()) / RAND_MAX - 1.0f;  // dans -1 1
			G.heightMap[ix][iz] = h;
			 
		}

	for (int i = 0 ; i < 100 ; i++)
	{
		float x = (20.0f * rand()) / RAND_MAX;
		float z =   10.0f * rand() / RAND_MAX;      
		G.TreePos.push_back(V2(x, z));
	}
	 
}














