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
	Entity e1 = world->CreateEntity<FPositionComponent, FNameComponent>
		(FPositionComponent(1, 2, 3), FNameComponent("Hello World!"));

	Entity e2 = world->CreateEntity();
	world->AddComponent<FPositionComponent>(e2, FPositionComponent(4, 5, 6));

	world->RemoveComponent<FNameComponent>(e2);
	world->AddComponent<FPositionComponent>(e1, FPositionComponent(0,0,0));

	world->DestroyEntity(e2);
	world->DestroyEntity(e2);
	e2 = world->CreateEntity();
	std::cout << e2 << std::endl;
	world->DestroyEntity(e2);
	world->DestroyEntity(e2);

	std::getchar();
	return 0;
}