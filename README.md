## AtlasECS
An Implementation of Entity Component Systems Pattern leveraging Sparse Sets and Bit Fields for Efficiency and Performacne.


#### Features
* A Dead Simple System and API, where Entities are just integers.
* Smart and Fast Iterations by Storing Matching Entities in a Sparse Sets.
* Safety Measures to avoid Adding Duplicate Components to Entities and Removing non-existent Components from Entities.
* Automatic Deallocation of Component Resources on Entity Deletion - No need to worry about Components once Added.
* Storing Per-System Component Arrays and Updation on Add/Remove Componenets.
* Components are Stored in contiguous Memory for Cache Efficiency.


#### Usage
Include the Single Header `AtlasECS.hpp` in your Project. Following is an Example of how to use the Library:

```
//Include this File
#include "include/AtlasECS.hpp"

#include "TestComponents.h"
#include "TestSystem.h"

#include <iostream>


int main()
{
	// Create an ECS World/Context. There can be Multiple Worlds for Physics, 
	// Rendering, Networking and So on.
	// The Parameter takes the Initial Number of Entities to reserve Memory.
	// Choose a Large Number to avoid Memory Moving Around.
	std::shared_ptr<atlas::CWorld> world = std::make_shared<atlas::CWorld>(1000);
	
	// Systems derive from CSystem and can selectively access 
	// for a Combination of Components.
	CTestSystem testSystem(world);	

	// Entities can be Created using the CreateEntity Function
	// Entity IDs can repeat across Worlds, but is unique within a World.
	Entity e1 = world->CreateEntity();

	// Components can be Added to an Entity using the Following Syntax.
	world->AddComponent<FNameComponent>(e2, std::move(FNameComponent("Entity 1")));

	// Multiple Components can be added with the Same AddComponent Call
	world->AddComponent<FPositionComponent, FRotationComponent>(e2, FPositionComponent(0,0,0), FRotationComponent(30,85,90));

	// Or Entities can be Created with Components, as Shown Below
	Entity e2 = world->CreateEntity<FNameComponent, FPositionComponent, FRotationComponent>
		(FNameComponent("Entity 2"),FPositionComponent(1,1,1), FRotationComponent(0,0,0));

	// A Component can be Removed using the `RemoveComponent` Function
	world->RemoveComponent<FPositionComponent>(e2);

	// Destroying an Entity would destroy all the Components associated with it.
	world->DestroyEntity(e2);

	// Get Components of type would return an Array of the Components of the Passed Type
	// and the Number of **Valid** Components in the Array.
	auto[array, count] = world->GetComponentsOfType<FNameComponent>();

	return 0;
}
```

#### References
* [ECS Back and Forth by skypjack](https://skypjack.github.io)
* [Data Oriented Design and C++ by Mike Acton](https://www.youtube.com/watch?v=rX0ItVEVjHc)
* [Game Programming by thebennybox](https://www.youtube.com/watch?v=Y6he35HfDmA&list=PLEETnX-uPtBUrfzE3Dxy3PWyApnW6YEMm&index=6&t=0s)
* [The The ID Lookup Table by bitsquid](http://bitsquid.blogspot.com/2011/09/managing-decoupling-part-4-id-lookup.html)
* [Curiously Recurring Template pattern by oneproduct](https://www.youtube.com/watch?v=7-nHdQjSRe0)
