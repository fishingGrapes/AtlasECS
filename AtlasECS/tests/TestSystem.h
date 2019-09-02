#pragma once

#include "include/AtlasECS.hpp"

class CTestSystem : public atlas::CSystem
{
public:
	CTestSystem(std::shared_ptr<atlas::CWorld> world)
		:CSystem(world)
	{
		// Selectively Include and Exclude Components 
		// using these Functions
		this->Match<FPositionComponent>();
		this->Exclude<FStaticMeshComponent>();
	}
};