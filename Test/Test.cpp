// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "Interface.h"
#include <iostream>

int main()
{
    std::cout << "Hello World!\n";
	int number = get_scene_poly_count("..//Assets//BattleMageRun.fbx");
	std::cout << number;
	int cool = export_simple_mesh("..//Assets//BattleMageRun.fbx","..//Assets//BattleMageMesh.bin");
	int iscool = export_material("..//Assets//BattleMageRun.fbx", "..//Assets//BattleMageMesh.mat");
	int testout = export_bindpose("..//Assets//BattleMageRun.fbx", "..//Assets//BattleMagebind.bin");
	int testAnim = export_animation("..//Assets//BattleMageRun.fbx", "..//Assets//BattleMageRun.anim");

	system("pause");
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
