#pragma once

#include <bitset>
#include <deque>
#include <forward_list>
#include <functional>
#include <tuple>
#include <vector>

namespace atlas
{
	constexpr uint32_t MAX_COMPONENTS = 1024;
}

using Entity = uint32_t;
using BitMask = std::bitset<atlas::MAX_COMPONENTS>;

/*********************************************************************************************************/

template<typename T>
class SparseSet
{
	using iterator = typename std::vector<T>::iterator;
	using const_iterator = typename std::vector<T>::const_iterator;

public:

	inline size_t size() const { return m_Size; }
	inline size_t capacity() const { return m_Capacity; }

	bool empty() const { return m_Size == 0; }
	void clear() { m_Size = 0; }

	void reserve(size_t cap)
	{
		if (cap > m_Capacity)
		{
			m_DenseArray.resize(cap, 0);
			m_SparseArray.resize(cap, 0);
			m_Capacity = cap;
		}
	}

	bool has(const T& val) const
	{
		return val < m_Capacity && m_SparseArray[val] < m_Size && m_DenseArray[m_SparseArray[val]] == val;
	}

	void insert(const T& val)
	{
		if (!has(val))
		{
			if (val >= m_Capacity)
				reserve(val + 1);

			m_DenseArray[m_Size] = val;
			m_SparseArray[val] = static_cast<T>(m_Size);
			++m_Size;
		}
	}

	void erase(const T& val)
	{
		if (has(val))
		{
			m_DenseArray[m_SparseArray[val]] = m_DenseArray[m_Size - 1];
			m_SparseArray[m_DenseArray[m_Size - 1]] = m_SparseArray[val];
			--m_Size;
		}
	}

	iterator begin() { return m_DenseArray.begin(); }
	iterator end() { return m_DenseArray.begin() + m_Size; }

	const_iterator begin() const { return m_DenseArray.begin(); }
	const_iterator end() const { return m_DenseArray.end() + m_Size; }

private:
	std::vector<T> m_DenseArray;
	std::vector<T> m_SparseArray;

	size_t m_Size = 0;			// current number of elements
	size_t m_Capacity = 0;		// available memory

};

/*********************************************************************************************************/

namespace atlas
{

	struct FBaseComponent;

	using ComponentCreateFunction = std::function<void()>(*)(std::vector<uint8_t>&, Entity, FBaseComponent*);
	using ComponentFreeFunction = void(*)(FBaseComponent*);

	struct FBaseComponent
	{
	public:
		//Returns a Unique Identifier for each Type
		static uint32_t RegisterComponent()
		{
			static uint32_t componentId = 0;
			return componentId++;
		}

		static BitMask GenerateBitset()
		{
			static uint32_t bit = 0;

			BitMask filter;
			filter.set(bit);
			++bit;

			return filter;
		}

		uint32_t Entity;
	};

	template<typename T>
	struct FComponent : public FBaseComponent
	{
	public:
		FComponent() = default;
		virtual std::string ToString() const { return std::string(); }

		static const uint32_t Id;
		static const BitMask Filter;
		static const size_t Size;

		static const ComponentCreateFunction Create;
		static const ComponentFreeFunction Free;

		inline friend std::ostream& operator <<(std::ostream& os, const T& component)
		{
			return os << component.ToString();
		}
	};

	template<typename T>
	const uint32_t FComponent<T>::Id(FBaseComponent::RegisterComponent());

	template<typename T>
	const BitMask FComponent<T>::Filter(FBaseComponent::GenerateBitset());

	template<typename T>
	const size_t FComponent<T>::Size(sizeof(T));

	template <typename Component>
	std::function<void()> ComponentCreate(std::vector<uint8_t>& buffer, Entity entity, FBaseComponent* comp)
	{
		size_t index = buffer.size();

		if (index == 0)
		{
			buffer.resize(Component::Size);
		}
		else
		{
			// If The Enity has already been in this array, use it's Previous Position
			if (entity < (index / Component::Size))
				index = entity * Component::Size;
			else
			{
				// Array Doubling Algorithm
				buffer.resize(index * 2);
			}
		}

		//Copy Construct
		uint8_t* memory = buffer.data();
		Component* component = new(&memory[index]) Component(*((Component*)comp));
		component->Entity = entity;

		return [component]() { component->~Component(); };
	}

	template <typename Component>
	void ComponentFree(FBaseComponent* comp)
	{
		Component* component = (Component*)comp;
		component->~Component();
	}

	template<typename T>
	const ComponentCreateFunction FComponent<T>::Create(ComponentCreate<T>);

	template<typename T>
	const ComponentFreeFunction FComponent<T>::Free(ComponentFree<T>);


	/*********************************************************************************************************/

	class CWorld
	{

		using ComponentChangedFunction = std::function<void(Entity e, const BitMask& entityMask, const BitMask& compMask)>;
		using ComponentDestroyFunction = std::function<void()>;

	public:

		CWorld(const size_t initialEntities)
		{
			m_LargestEntityIndex = 0;

			m_Entities.reserve(initialEntities);
			m_EntityMasks.reserve(initialEntities);

			//The following two sets would work together
			m_ContainedComponentIDs.resize(initialEntities, std::forward_list<uint32_t>());
			m_ComponentDestroyFunctions.resize(initialEntities);
			for (size_t i = 0; i < initialEntities; ++i)
			{
				m_ComponentDestroyFunctions[i].resize(MAX_COMPONENTS);
			}

			std::pair<uint32_t, size_t> initialPair = std::make_pair(0, 0);
			m_ValidComponents.resize(MAX_COMPONENTS, initialPair);

			m_ComponentBuffers.reserve(MAX_COMPONENTS);
			for (size_t i = 0; i < MAX_COMPONENTS; ++i)
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
				m_EntityMasks.push_back(0);

				entity = m_LargestEntityIndex;
				++m_LargestEntityIndex;

				// Per Entity Data must be resized when the Limit is Touched
				if (entity >= m_EntityMasks.capacity())
				{
					m_EntityMasks.resize(entity * 2);
					m_ContainedComponentIDs.resize(entity * 2, std::forward_list<uint32_t>());
					m_ComponentDestroyFunctions.resize(entity * 2);
				}
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

			if (!m_Entities.has(entity))
			{
				std::cout << "ATLAS_ECS :: Entity is Not Alive." << std::endl;
				return;
			}


			// This can be used to recycle Entities
			m_DeletedEntities.push_back(entity);
			m_Entities.erase(entity);



			// It updates the Number of valid Components which might be useful
			std::forward_list<uint32_t>& componentIDs = m_ContainedComponentIDs[entity];
			for (auto itr = componentIDs.begin(); itr != componentIDs.end(); ++itr)
			{
				std::pair<uint32_t, size_t> oldPair = m_ValidComponents[*itr];
				std::pair<uint32_t, size_t> newPair = std::make_pair(oldPair.first - 1, oldPair.second);
				m_ValidComponents[*itr] = newPair;

				//Call the Free Function of Each Component Instance
				(m_ComponentDestroyFunctions[entity][*itr])();
			}
			// Clear all the ComponentIDs associated with this Entity
			m_ContainedComponentIDs[entity].clear();



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
			// If the Entity does'nt contain this Component do Nothing
			if ((m_EntityMasks[entity] & T::Filter) != T::Filter)
			{
				std::cout << "ATLAS_ECS :: Entity Doesn't Contain Component." << std::endl;
				return;
			}

			// Actually Removing the Component
			// Destroys the Component Object in Memory
			uint32_t id = T::Id;

			size_t index = entity * T::Size;
			uint8_t* memory = m_ComponentBuffers[id].data();

			T* component = (T*)(&memory[index]);
			T::Free(component);

			// Remove the Id from the Entity's ComponetIDs List
			m_ContainedComponentIDs[entity].remove(id);

			std::pair<uint32_t, size_t> newPair = std::make_pair(m_ValidComponents[id].first - 1, T::Size);
			m_ValidComponents[id] = newPair;

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
			uint32_t id = T::Id;

			uint8_t* memory = m_ComponentBuffers[id].data();
			T* const buffer = reinterpret_cast<T* const>(memory);
			size_t size = m_ValidComponents[id].first;

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

		/****************************** Per Entity Variables ************************/

		SparseSet<Entity> m_Entities;
		std::deque<Entity> m_DeletedEntities;
		// TODO: Space Complexity is not the best
		std::vector<BitMask> m_EntityMasks;
		// This is a map of Entites and their Valid Components
		std::vector<std::forward_list<uint32_t>> m_ContainedComponentIDs;
		//This is a map of Entities and its Respective ComponentDestry Functions
		std::vector<std::vector<ComponentDestroyFunction>> m_ComponentDestroyFunctions;

		/****************************** Per Component Variables ************************/

		// This is a map of ComponentIDs and a Pair of 
		// <No. of Valid Components of The Type, Size of the ComponentType >
		std::vector<std::pair<uint32_t, size_t>> m_ValidComponents;
		// This a map of ComponentIDs and the Array of Instances of that ComponentType
		std::vector<std::vector<uint8_t>> m_ComponentBuffers;


		std::vector<ComponentChangedFunction> m_ComponentAddFunctions;
		std::vector<ComponentChangedFunction> m_ComponentRemoveFunctions;


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
			// If this Entity Already Contains thiss Component, Do nothing
			if ((m_EntityMasks[entity] & T::Filter) == T::Filter)
			{
				std::cout << "ATLAS_ECS :: Entity Already Contains Component." << std::endl;
				return;
			}

			// Actually Setting Component
			uint32_t id = T::Id;
			auto destroyFn = T::Create(m_ComponentBuffers[id], entity, &component);

			// The Entity now conatins this Component
			m_ContainedComponentIDs[entity].push_front(id);
			// Set the Component's free Function
			m_ComponentDestroyFunctions[entity][id] = std::move(destroyFn);
			// The Current Number of Instances of this Components in the World
			std::pair<uint32_t, size_t> newPair = std::make_pair(m_ValidComponents[id].first + 1, T::Size);
			m_ValidComponents[id] = newPair;

			// Entity now has the Component bit set to 1.
			m_EntityMasks[entity] |= T::Filter;
			BitMask entityMask = m_EntityMasks[entity];

			for (auto& fn : m_ComponentAddFunctions)
			{
				fn(entity, entityMask, T::Filter);
			}
		}

	};



	/*********************************************************************************************************/


	/// <summary>
	/// Systems contain  Logic and process Entities that have a Matching Set of Components
	/// Each System has a Bitmask that it has to Check against to when components added or removed.
	/// </summary>
	class CSystem
	{

	public:
		// TODO: This might take in an InitalCapacity, 
		// which is user Dependent.
		CSystem(std::shared_ptr<CWorld> world)
		{
			world->AddOnComponentAddedFunction(
				[this](uint32_t entity, const BitMask& entityMask, const BitMask& componentMask)
			{
				// If the Entity contains a Type thats is in the Exclusion Mask,
				//  then return as we're not interested in those types.
				if ((entityMask & m_ExclusionMask) == entityMask)
					return;

				// If the Entity does not contain the Type in the Inclusion Mask,
				// then return as We're not interested in other types.
				if ((componentMask & m_InclusionMask) == componentMask)
				{
					m_MatchingEntities.insert(entity);
				}
			});


			world->AddOnComponentRemovedFunction(
				[this](uint32_t entity, const BitMask& entityMask, const BitMask& componentMask)
			{
				if ((entityMask & m_ExclusionMask) == entityMask)
					return;

				if ((componentMask & m_InclusionMask) == componentMask)
				{
					m_MatchingEntities.erase(entity);
				}

			});
		}

	protected:

		SparseSet<uint32_t> m_MatchingEntities;
		BitMask m_InclusionMask;
		BitMask m_ExclusionMask;

		template <typename ... T>
		void Match()
		{
			// NOTE: This is a Fold Expression.
			m_InclusionMask |= (T::Filter | ...);
		}

		template <typename ... T>
		void Exclude()
		{
			m_ExclusionMask |= (T::Filter | ...);
		}
	};

	/*********************************************************************************************************/

}
