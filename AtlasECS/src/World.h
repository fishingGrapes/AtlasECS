#pragma once

#include "Component.h"
#include "SparseSet.h"

#include <deque>
#include <functional>
#include <variant>


namespace atlas 
{

	class CWorld
	{
		using ComponentChangedFunction =
			std::function<void(Entity e, const BitMask& entityMask, const BitMask& compMask)>;

	public:

		CWorld(const size_t initialEntities)
		{
			m_LargestEntityIndex = 0;

			m_Entities.reserve(initialEntities);
			m_EntityMasks.reserve(initialEntities);


			m_ComponentBuffers.reserve(MAX_COMPONENTS);
			m_ValidComponents.resize(MAX_COMPONENTS, 0);

			for (size_t i = 0; i < m_ComponentBuffers.capacity(); i++)
			{
				m_ComponentBuffers.push_back(std::vector<uint8_t>());
			}
		}

		Entity CreateEntity()
		{
			Entity entity;

			if (m_DeletedEntities.size() <= 0)
			{
				// Assign a New Entity Identifier
				entity = m_LargestEntityIndex;
				++m_LargestEntityIndex;
				m_EntityMasks.push_back(0);
			}
			else
			{
				// Recycle Entity Id from deleted Entities
				entity = m_DeletedEntities.front();
				m_DeletedEntities.pop_front();
			}

			m_Entities.insert(entity);
			return entity;
		}

		template<typename... Components>
		Entity CreateEntity(Components... components)
		{
			Entity entity = this->CreateEntity();
			m_EntityMasks[entity].reset();

			this->AddComponent_Recurse(entity, components...);

			return entity;
		}

		void DestroyEntity(Entity entity)
		{
			// This can be used to recycle Entities
			m_DeletedEntities.push_back(entity);
			m_Entities.erase(entity);

			BitMask mask = m_EntityMasks[entity];

			for (auto& fn : m_ComponentRemoveFunctions)
			{
				// The Third parameter is unnecessary
				fn(entity, mask, mask);
			}

			m_EntityMasks[entity].reset();
		}

		template<typename... Components>
		void AddComponent(Entity entity, Components... components)
		{
			this->AddComponent_Recurse(entity, components...);
		}

		// Remove a Single Component from the Entity.
		template<typename T>
		void RemoveComponent(Entity entity)
		{
			// Actually Removing the Component
			// Destroys the Component Object in Memory
			uint32_t id = T::Id;

			size_t index = entity * T::Size;
			uint8_t* memory = m_ComponentBuffers[id].data();

			T* component = (T*)(&memory[index]);
			T::Free(component);

			m_ValidComponents.insert(m_ValidComponents.begin() + id, m_ValidComponents[id] - 1);

			// Entity now has the Component bit set to 0.
			BitMask entityMask = m_EntityMasks[entity];

			for (auto& fn : m_ComponentRemoveFunctions)
			{
				fn(entity, entityMask, T::Filter);
			}

			m_EntityMasks[entity] &= (~T::Filter);
		}

		template <typename T>
		std::tuple<T* const, size_t> GetComponentsOfType()
		{
			uint8_t* memory = m_ComponentBuffers[T::Id].data();
			T* const buffer = reinterpret_cast<T* const>(memory);
			size_t size = m_ValidComponents[T::Id];
			return  { buffer, size };
		}

		// TODO: Maybe move these into Private and add CSystem as Friend Function

		void AddOnComponentAddedFunction(ComponentChangedFunction fn)
		{
			m_ComponentAddFunctions.push_back(fn);
		}

		void AddOnComponentRemovedFunction(ComponentChangedFunction fn)
		{
			m_ComponentRemoveFunctions.push_back(fn);
		}


	private:

		Entity m_LargestEntityIndex;
		SparseSet<Entity> m_Entities;
		std::deque<Entity> m_DeletedEntities;

		// TODO: Space Complexity is not the best
		std::vector<BitMask> m_EntityMasks;

		std::vector<ComponentChangedFunction> m_ComponentAddFunctions;
		std::vector<ComponentChangedFunction> m_ComponentRemoveFunctions;

		std::vector<std::vector<uint8_t>> m_ComponentBuffers;
		std::vector<size_t> m_ValidComponents;


		// Recursive Add Components
		template<typename First, typename... Rest>
		void AddComponent_Recurse(Entity entity, First first, Rest... rest)
		{
			this->AddComponent_Single<First>(entity, first);
			AddComponent_Recurse(entity, rest...);
		}
		void AddComponent_Recurse(Entity entity) {}

		// Add a Single Component to the Entity.
		template<typename T>
		void AddComponent_Single(Entity entity, T component)
		{

			// Actually Setting Component
			uint32_t id = T::Id;
			T::Create(m_ComponentBuffers[id], entity, &component);
			m_ValidComponents.insert(m_ValidComponents.begin() + id, m_ValidComponents[id] + 1);

			// Entity now has the Component bit set to 1.
			m_EntityMasks[entity] |= T::Filter;
			BitMask entityMask = m_EntityMasks[entity];

			for (auto& fn : m_ComponentAddFunctions)
			{
				fn(entity, entityMask, T::Filter);
			}
		}
	};
}