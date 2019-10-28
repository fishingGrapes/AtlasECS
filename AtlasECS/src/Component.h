#pragma once

#include <bitset>
#include <vector>
#include <iostream>



namespace atlas
{
	constexpr uint32_t MAX_COMPONENTS = 1024;

	using Entity = uint32_t;
	using BitMask = std::bitset<atlas::MAX_COMPONENTS>;

	struct FBaseComponent;

	using ComponentCreateFunction = std::function<void( )>( * )( std::vector<uint8_t>&, Entity, FBaseComponent* );
	using ComponentFreeFunction = void( * )( FBaseComponent* );

	struct FBaseComponent
	{
	public:
		//Returns a Unique Identifier for each Type
		static uint32_t RegisterComponent( )
		{
			static uint32_t componentId = 0;
			return componentId++;
		}

		static BitMask GenerateBitset( )
		{
			static uint32_t bit = 0;

			BitMask filter;
			filter.set( bit );
			++bit;

			return filter;
		}

		uint32_t Entity;
	};

	template<typename T>
	struct FComponent : public FBaseComponent
	{
	public:
		FComponent( ) = default;
		virtual std::string ToString( ) const
		{
			return std::string( );
		}

		static const uint32_t Id;
		static const BitMask Filter;
		static const size_t Size;

		static const ComponentCreateFunction Create;
		static const ComponentFreeFunction Free;

		inline friend std::ostream& operator <<( std::ostream& os, const T& component )
		{
			return os << component.ToString( );
		}
	};

	template<typename T>
	const uint32_t FComponent<T>::Id( FBaseComponent::RegisterComponent( ) );

	template<typename T>
	const BitMask FComponent<T>::Filter( FBaseComponent::GenerateBitset( ) );

	template<typename T>
	const size_t FComponent<T>::Size( sizeof( T ) );

	template <typename Component>
	std::function<void( )> ComponentCreate( std::vector<uint8_t>& buffer, Entity entity, FBaseComponent* comp )
	{
		size_t index = buffer.size( );

		if (index == 0)
		{
			buffer.resize( Component::Size );
		}
		else
		{
			// If The Enity has already been in this array, use it's Previous Position
			if (entity < ( index / Component::Size ))
				index = entity * Component::Size;
			else
			{
				// Array Doubling Algorithm
				buffer.resize( index * 2 );
			}
		}

		uint8_t* memory = buffer.data( );
		Component* component = new( &memory[ index ] ) Component( *( ( Component*) comp ) );
		component->Entity = entity;

		// This is returned for Component Deletion when an Entity is Destroyed.
		return [ component ] ( )
		{
			component->~Component( );
		};
	}

	template <typename Component>
	void ComponentFree( FBaseComponent* comp )
	{
		Component* component = ( Component*) comp;
		component->~Component( );
	}

	template<typename T>
	const ComponentCreateFunction FComponent<T>::Create( ComponentCreate<T> );

	template<typename T>
	const ComponentFreeFunction FComponent<T>::Free( ComponentFree<T> );
}