#include <iostream>

#include "include/AtlasECS.hpp"

#include "TestComponents.h"
#include "TestSystem.h"

int main( )
{
	// Create an ECS World/Context. There can be Multiple Worlds for Physics, 
	// Rendering, Networking and So on.
	// The Parameter takes the Initial Number of Entities to reserve Memory.
	std::shared_ptr<atlas::CWorld> world = std::make_shared<atlas::CWorld>( 100 );

	// Systems derive from CSystem and can selectively access 
	// for a Combination of Components.
	CTestSystem testSystem( world );

	using Entity = uint32_t;

	// Entities can be Created using the CreateEntity Function
	// Entity IDs are unique to each world.

	std::cout << "Creating e2" << std::endl;

	Entity e2 = world->CreateEntity<FPositionComponent>( std::move( FPositionComponent( 1, 2, 2 ) ) );
	world->AddComponent<FNameComponent>( e2, std::move( FNameComponent( "Hello e2" ) ) );
	std::cout << "Creating e1" << std::endl;

	Entity e1 = world->CreateEntity<FNameComponent>( std::move( FNameComponent( "Hello e1" ) ) );

	FNameComponent* comp = world->GetComponent<FNameComponent>( e2 );
	std::cout << comp->Name << std::endl;

	std::cout << "Wait" << std::endl;
	
	world->DestroyEntity( e1 );
	world->DestroyEntity( e2 );

	world->RemoveComponent<FNameComponent>( e1 );
	world->RemoveComponent<FNameComponent>( e2 );

	std::getchar( );
	return 0;
}

