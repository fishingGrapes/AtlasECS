#include <iostream>

#include "include/AtlasECS.hpp"

#include "TestComponents.h"
#include "TestSystem.h"

int main()
{
	// Create an ECS World/Context. There can be Multiple Worlds for Physics, 
	// Rendering, Networking and So on.
	// The Parameter takes the Initial Number of Entities to reserve Memory.
	std::shared_ptr<atlas::CWorld> world = std::make_shared<atlas::CWorld>(100);
	
	// Systems derive from CSystem and can selectively access 
	// for a Combination of Components.
	CTestSystem testSystem(world);
	
	using Entity = uint32_t;

	// Entities can be Created using the CreateEntity Function
	// Entity IDs are unique to each world.

	Entity e2 = world->CreateEntity<FPositionComponent>(std::move(FPositionComponent(1, 2, 2)));
	world->AddComponent<FNameComponent>(e2, std::move(FNameComponent("Hello")));

	std::cout << "Wait" << std::endl;
	world->DestroyEntity(e2);

	world->RemoveComponent<FNameComponent>(e2);

	std::getchar();
	return 0;
}

