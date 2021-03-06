#include "Goal_EscapeFromTarget.h"
#include "Goal_SeekToPosition.h"
#include "Goal_MoveToPosition.h"
#include "Goal_GetItem.h"
#include "Goal_HuntTarget.h"
#include "Goal_DodgeSideToSide.h"
#include "..\Raven_ObjectEnumerations.h"
#include "../Raven_Bot.h"






void Goal_EscapeFromTarget::Activate()
{
	m_iStatus = active;

	RemoveAllSubgoals();

	m_vector = m_pOwner->GetTargetBot()->Pos() - m_pOwner->Pos();
	m_vector = Vec2DNormalize(m_vector) * 20;
	switch((int)RandInRange(0, 3))
	{
	case 0:
		m_vector.x = -m_vector.x;
		break;
	case 1:
		m_vector.y = -m_vector.y;
		break;
	default:
		break;
	}

	//AddSubgoal(new Goal_MoveToPosition(m_pOwner, m_pOwner->Pos() - m_vector));
	if(m_pOwner->canWalkTo(m_pOwner->Pos() - m_vector))
		AddSubgoal(new Goal_SeekToPosition(m_pOwner, m_pOwner->Pos() - m_vector));
	//AddSubgoal(new Goal_GetItem(m_pOwner, type_health));
}


int Goal_EscapeFromTarget::Process()
{
	ActivateIfInactive();

	m_iStatus = ProcessSubgoals();

	ReactivateIfFailed();

	return m_iStatus;
}