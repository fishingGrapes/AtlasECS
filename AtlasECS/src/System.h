#pragma once

#include "Component.h"

#include "SparseSet.h"
#include "World.h"

namespace atlas
{
	/// <summary>
	/// Systems contain  Logic and process Entities that have a Matching Set of Components
	/// Each System has a Bitmask that it has to Check against to when components added or removed.
	/// </summary>
	class CSystem
	{

	public:
		// TODO: This might take in an InitalCapacity, 
		// which is user Dependent.
		CSystem( std::shared_ptr<CWorld> world )
			:m_World( world )
		{
			m_World->AddOnComponentAddedFunction( [ this ] ( uint32_t entity, const BitMask& entityMask, const BitMask& componentMask )
			{
				// If the Entity contains a Type thats is in the Exclusion Mask,
				//  then return as we're not interested in those types.

				// Atlease One Component must be Set to be Excluded.
				if (( entityMask & m_ExclusionMask_Any ).any( ))
					return;

				// All Components must be Set to be Excluded.
				if (( entityMask & m_ExclusionMask_All ) == entityMask)
					return;

				// If the Entity does not contain the Type in the Inclusion Mask,
				// then return as We're not interested in other types.
				if (( componentMask & m_InclusionMask ) == componentMask)
				{
					m_MatchingEntities.insert( entity );
				}
			} );


			m_World->AddOnComponentRemovedFunction( [ this ] ( uint32_t entity, const BitMask& entityMask, const BitMask& componentMask )
			{
				if (( entityMask & m_ExclusionMask_Any ).any( ))
					return;

				if (( entityMask & m_ExclusionMask_All ) == entityMask)
					return;

				if (( componentMask & m_InclusionMask ) == componentMask)
				{
					m_MatchingEntities.erase( entity );
				}

			} );

		}

	protected:

		std::shared_ptr<CWorld> m_World;
		SparseSet<uint32_t> m_MatchingEntities;

		BitMask m_InclusionMask;
		BitMask m_ExclusionMask_Any;
		BitMask m_ExclusionMask_All;

		void UpdateMatchingEntities( )
		{
			Entity entityID = -1;
			auto entityMasks = m_World->GetEntityMasks( );
			for (BitMask& mask : entityMasks)
			{
				++entityID;

				// If any Bit returns true in the Exclusion Mask
				// then Return.
				if (( mask & m_ExclusionMask_Any ).any( ))
					continue;

				if (( mask & m_ExclusionMask_All ) == mask)
					continue;

				// If the Entity does not contain the Type in the Inclusion Mask,
				// then return as We're not interested in other types.
				if (( mask & m_InclusionMask ) == m_InclusionMask)
				{
					if (m_World->IsEntityAlive( entityID ))
						m_MatchingEntities.insert( entityID );
				}

			}
		}

		template <typename ... T>
		void MatchEntitiesWith( )
		{
			// NOTE: This is a Fold Expression.
			m_InclusionMask |= ( T::Filter | ... );
		}

		template <typename ... T>
		void ExcludeEntitiesWithAnyOf( )
		{
			m_ExclusionMask_Any |= ( T::Filter | ... );
		}

		template <typename ... T>
		void ExcludeEntitiesWithAllOf( )
		{
			m_ExclusionMask_All |= ( T::Filter | ... );
		}
	};

}