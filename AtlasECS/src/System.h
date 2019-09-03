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
		CSystem(std::shared_ptr<CWorld> world)
		{
			world->AddOnComponentAddedFunction(
				[this](uint32_t entity, const BitMask& entityMask, const BitMask& componentMask)
			{
				// If the Entity contains a Type thats is in the Exclusion Mask,
				//  then return as we're not interested in those types.
				if ((entityMask & m_ExclusionMask_Any) == entityMask)
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
				if ((entityMask & m_ExclusionMask_Any) == entityMask)
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
		BitMask m_ExclusionMask_Any;

		template <typename ... T>
		void MatchEntitiesWith()
		{
			// NOTE: This is a Fold Expression.
			m_InclusionMask |= (T::Filter | ...);
		}

		template <typename ... T>
		void ExcludeEntitiesWithAnyOf()
		{
			m_ExclusionMask_Any |= (T::Filter | ...);
		}
	};
}