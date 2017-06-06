#include "AIDemoDargonComponent.h"
#include "GameLogger.h"
#include "Flocker.h"
#include "SteeringBehaviors.h"
#include "MathUtility.h"
#include "MyWindow.h"
#include "Mesh.h"

// Justin Furtado
// 5/30/2017
// AIDemoDargonComponent.cpp
// Demonstrates various AI Techniques 

const char *const AIDemoDargonKeys = "YTV0123456789";

Engine::FSMPair AIDemoDargonComponent::s_AIFuncs[NUM_FUNCS] = {
	Engine::FSMPair(AIDemoDargonComponent::EnterRandomAStar, AIDemoDargonComponent::DoNothingOnPurpose, AIDemoDargonComponent::ExitRandomAStar, nullptr),
	Engine::FSMPair(AIDemoDargonComponent::EnterFollowAStar, AIDemoDargonComponent::FollowAStar, AIDemoDargonComponent::ExitFollowAStar, nullptr),
	Engine::FSMPair(AIDemoDargonComponent::SeekEnter, AIDemoDargonComponent::SeekUpdate, AIDemoDargonComponent::StopMoving, nullptr),
	Engine::FSMPair(AIDemoDargonComponent::ArrivalEnter, AIDemoDargonComponent::ArrivalUpdate, AIDemoDargonComponent::StopMoving, nullptr),
	Engine::FSMPair(AIDemoDargonComponent::PursueOffsetEnter, AIDemoDargonComponent::PursueOffsetUpdate, AIDemoDargonComponent::StopMoving, nullptr),
	Engine::FSMPair(AIDemoDargonComponent::PursueEnter, AIDemoDargonComponent::PursueUpdate, AIDemoDargonComponent::StopMoving, nullptr),
	Engine::FSMPair(AIDemoDargonComponent::FleeEnter, AIDemoDargonComponent::FleeUpdate, AIDemoDargonComponent::StopMoving, nullptr),
	Engine::FSMPair(AIDemoDargonComponent::EvadeEnter, AIDemoDargonComponent::EvadeUpdate, AIDemoDargonComponent::StopMoving, nullptr),
	Engine::FSMPair(AIDemoDargonComponent::WanderEnter, AIDemoDargonComponent::WanderUpdate, AIDemoDargonComponent::StopMoving, nullptr),
	Engine::FSMPair(AIDemoDargonComponent::ForageEnter, AIDemoDargonComponent::ForageUpdate, AIDemoDargonComponent::StopMoving, nullptr),
	Engine::FSMPair(AIDemoDargonComponent::FlockEnter, AIDemoDargonComponent::FlockUpdate, AIDemoDargonComponent::FlockExit, nullptr)
};

bool AIDemoDargonComponent::Initialize()
{
	// add keys we will need
	if (!m_keyboardManager.AddKeys(AIDemoDargonKeys) || !m_keyboardManager.AddKey(VK_SHIFT))
	{
		// log error if we can't find the keys
		Engine::GameLogger::Log(Engine::MessageType::cError, "AIDemoDargonComponent [%s] on [%s] failed to initialize successfully! Could not add keys [%s]!\n", this->GetName(), this->m_owner->GetName(), AIDemoDargonKeys);
		return false;
	}

	// grab the a spatial component so our brain can talk to it
	m_pSpatial = GetSiblingComponent<Engine::SpatialComponent>();
	if (!m_pSpatial)
	{
		// if we can't find it log an error
		Engine::GameLogger::Log(Engine::MessageType::cError, "AIDemoDargonComponent [%s] on [%s] failed to initialize successfully! Could not find m_pSpatial!\n", this->GetName(), this->m_owner->GetName());
		return false;
	}

	// grab the a star component so our brain can talk to it
	m_pAStarFollow = GetSiblingComponent<Engine::AStarPathFollowComponent>();
	if (!m_pAStarFollow)
	{
		// if we can't find it log an error
		Engine::GameLogger::Log(Engine::MessageType::cError, "AIDemoDargonComponent [%s] on [%s] failed to initialize successfully! Could not find AStarFollowComponent!\n", this->GetName(), this->m_owner->GetName());
		return false;
	}

	// grab the a star component so our brain can talk to it
	m_pGobComp = GetSiblingComponent<Engine::GraphicalObjectComponent>();
	if (!m_pGobComp)
	{
		// if we can't find it log an error
		Engine::GameLogger::Log(Engine::MessageType::cError, "AIDemoDargonComponent [%s] on [%s] failed to initialize successfully! Could not find GraphicalObjectComponent!\n", this->GetName(), this->m_owner->GetName());
		return false;
	}

	// start a-thinking!
	Engine::FSMPair pair = s_AIFuncs[0];
	m_brain.Push(pair.m_enter, pair.m_update, pair.m_exit, this);

	InitOffset();

	//m_flockWeights = Engine::MathUtility::Rand(Engine::Vec3(1, 0, 0), Engine::Vec3(1, 0, 0));
	m_flockWeights = Engine::MathUtility::Rand(Engine::Vec3(0.4f, 0.4f, 0.4f), Engine::Vec3(0.6f, 0.6f, 0.6f));
	m_speed = Engine::MathUtility::Rand(30.0f, 70.0f);
	m_pAStarFollow->SetSpeed(m_speed);

	// log success
	Engine::GameLogger::Log(Engine::MessageType::Info, "AIDemoDargonComponent [%s] on [%s] initialized successfully!\n", this->GetName(), this->m_owner->GetName());
	return true;
}

bool AIDemoDargonComponent::Update(float dt)
{
	// think
	m_brain.Update(dt);
	m_keyboardManager.Update(dt);

	if (m_keyboardManager.KeyIsUp(VK_SHIFT))
	{
		// change brain state if necessary
		for (int i = 0; i < 10; ++i)
		{
			if (m_keyboardManager.KeyWasPressed('0' + i))
			{
				m_index = i;
				Engine::FSMPair pair = s_AIFuncs[m_index];
				m_brain.Push(pair.m_enter, pair.m_update, pair.m_exit, this);
			}
		}

		if (m_keyboardManager.KeyWasPressed('Y'))
		{
			m_index = 10;
			Engine::FSMPair pair = s_AIFuncs[m_index];
			m_brain.Push(pair.m_enter, pair.m_update, pair.m_exit, this);
		}
		else if (m_keyboardManager.KeyWasPressed('T'))
		{
			m_index = Engine::MathUtility::Rand(0, NUM_FUNCS);
			Engine::FSMPair pair = s_AIFuncs[m_index];
			m_brain.Push(pair.m_enter, pair.m_update, pair.m_exit, this);
		}
		else if (m_keyboardManager.KeyWasPressed('V'))
		{
			m_brain.Pop();
		}
	}

	return true;
}

void AIDemoDargonComponent::SetPlayerRef(Engine::SpatialComponent * pPlayerSpatial)
{
	m_pPlayerSpatial = pPlayerSpatial;
}

void AIDemoDargonComponent::SetPCollectibles(Engine::LinkedList<Engine::GraphicalObject*>* pCollectibles)
{
	m_pCollectibles = pCollectibles;
}

void AIDemoDargonComponent::SetFormationGobPtr(Engine::GraphicalObject * pFormationGob)
{
	m_pFormationGob = pFormationGob;
}

void AIDemoDargonComponent::DoNothingOnPurpose(void * /*pData*/)
{
	// does nothing - ON PURPOSE :D
}

void AIDemoDargonComponent::DoNothingOnPurpose(float /*dt*/, void * /*pData*/)
{
	// does nothing - ON PURPOSE :D
}

void AIDemoDargonComponent::WanderEnter(void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	pComp->m_pSpatial->SetVelocity(Engine::MathUtility::GetRandSphereEdgeVec(pComp->m_speed));
	SetColor(pComp->m_pFormationGob->GetMatPtr(), Engine::Vec3(0.5f, 0.5f, 0.5f));
}

void AIDemoDargonComponent::StopMoving(void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	pComp->m_pSpatial->SetVelocity(Engine::Vec3(0.0f));
}

void AIDemoDargonComponent::FlockEnter(void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	Engine::Flocker::AddToFlock(pComp->m_pSpatial);
	SetColor(pComp->m_pFormationGob->GetMatPtr(), Engine::Vec3(0.75f, 0.25f, 0.75f));
}


void AIDemoDargonComponent::FlockUpdate(float /*dt*/, void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	Engine::Flocker::Flock(pComp->m_pSpatial, pComp->m_flockWeights.GetX(), pComp->m_flockWeights.GetY(), pComp->m_flockWeights.GetZ(), pComp->m_speed);
	pComp->FaceMoveDir();
}

void AIDemoDargonComponent::FlockExit(void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	Engine::Flocker::RemoveFromFlock(pComp->m_pSpatial);
	AIDemoDargonComponent::StopMoving(pData);
}

void AIDemoDargonComponent::SeekEnter(void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	SetColor(pComp->m_pFormationGob->GetMatPtr(), Engine::Vec3(0.0f, 0.0f, 1.0f));
}

void AIDemoDargonComponent::SeekUpdate(float /*dt*/, void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	Engine::SteeringBehaviors::Seek(pComp->m_pSpatial, pComp->m_pPlayerSpatial, pComp->m_speed);
	pComp->FaceMoveDir();
}

void AIDemoDargonComponent::ArrivalEnter(void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	SetColor(pComp->m_pFormationGob->GetMatPtr(), Engine::Vec3(0.0f, 1.0f, 1.0f));
}

void AIDemoDargonComponent::ArrivalUpdate(float /*dt*/, void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	Engine::SteeringBehaviors::Arrival(pComp->m_pSpatial, pComp->m_pPlayerSpatial, pComp->m_speed, 25.0f);
	pComp->FaceMoveDir();
}

void AIDemoDargonComponent::FleeEnter(void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	SetColor(pComp->m_pFormationGob->GetMatPtr(), Engine::Vec3(1.0f, 0.0f, 1.0f));
}

void AIDemoDargonComponent::FleeUpdate(float /*dt*/, void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	Engine::SteeringBehaviors::Flee(pComp->m_pSpatial, pComp->m_pPlayerSpatial, pComp->m_speed);
	pComp->FaceMoveDir();
}

void AIDemoDargonComponent::PursueEnter(void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	SetColor(pComp->m_pFormationGob->GetMatPtr(), Engine::Vec3(1.0f, 1.0f, 1.0f));
}

void AIDemoDargonComponent::PursueUpdate(float dt, void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	Engine::SteeringBehaviors::Pursue(pComp->m_pSpatial, pComp->m_pPlayerSpatial, dt, pComp->m_speed);
	pComp->FaceMoveDir();
}

void AIDemoDargonComponent::PursueOffsetEnter(void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	SetColor(pComp->m_pFormationGob->GetMatPtr(), Engine::Vec3(0.25, 0.5f, 1.0f));
}

void AIDemoDargonComponent::PursueOffsetUpdate(float dt, void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	Engine::Mat4 rot = Engine::Mat4::AxisRotation(pComp->m_pPlayerSpatial->GetForward(), pComp->m_pPlayerSpatial->GetUp());
	Engine::SteeringBehaviors::OffsetPursuitArrival(pComp->m_pSpatial, pComp->m_pPlayerSpatial, dt, pComp->m_speed, 25.0f, rot * pComp->m_offset);
	pComp->FaceMoveDir();
}

void AIDemoDargonComponent::WanderUpdate(float /*dt*/, void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	Engine::SteeringBehaviors::Wander(pComp->m_pSpatial, pComp->m_speed, 1.0f, 5.0f);
	pComp->FaceMoveDir();
}

void AIDemoDargonComponent::EvadeEnter(void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	SetColor(pComp->m_pFormationGob->GetMatPtr(), Engine::Vec3(1.0f, 0.75f, 0.0f));
}

void AIDemoDargonComponent::EvadeUpdate(float dt, void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	Engine::SteeringBehaviors::Evade(pComp->m_pSpatial, pComp->m_pPlayerSpatial, dt, 50.0f);
	pComp->FaceMoveDir();
}

void AIDemoDargonComponent::EnterRandomAStar(void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	pComp->m_pAStarFollow->Enable();
	pComp->m_pAStarFollow->SetSpeed(pComp->m_speed);
	pComp->m_pAStarFollow->SetRandomTargetNode(true); 
	SetColor(pComp->m_pFormationGob->GetMatPtr(), Engine::Vec3(1.0f, 1.0f, 0.0f));
}

void AIDemoDargonComponent::EnterFollowAStar(void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	pComp->m_pAStarFollow->Enable();
	pComp->m_pAStarFollow->SetRandomTargetNode(false);
	pComp->m_pAStarFollow->SetSpeed(pComp->m_speed);
	pComp->m_pAStarFollow->ForceRecalc(pComp->m_pSpatial->GetPosition());
	SetColor(pComp->m_pFormationGob->GetMatPtr(), Engine::Vec3(1.0f, 0.0f, 0.0f));
}

void AIDemoDargonComponent::ExitRandomAStar(void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	pComp->m_pAStarFollow->Enable(false);
	pComp->m_pSpatial->SetVelocity(Engine::Vec3(0.0f));
}

void AIDemoDargonComponent::ExitFollowAStar(void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	pComp->m_pAStarFollow->Enable(false);
	pComp->m_pSpatial->SetVelocity(Engine::Vec3(0.0f));
}

void AIDemoDargonComponent::ForageEnter(void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	SetColor(pComp->m_pFormationGob->GetMatPtr(), Engine::Vec3(0.75f, 0.5f, 0.2f));
}

void AIDemoDargonComponent::ForageUpdate(float /*dt*/, void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);
	Engine::SteeringBehaviors::Forage(pComp->m_pSpatial, pComp->m_speed, 1.0f, 25.0f, 100.0f, 5.0f, pComp->m_pCollectibles);
	pComp->FaceMoveDir();
}

void AIDemoDargonComponent::FollowAStar(float /*dt*/, void * pData)
{
	AIDemoDargonComponent *pComp = reinterpret_cast<AIDemoDargonComponent *>(pData);

	pComp->m_pAStarFollow->SetFollowPos(pComp->m_pPlayerSpatial->GetPosition());
}

void AIDemoDargonComponent::SetColor(Engine::Material * pMat, const Engine::Vec3 & color)
{
	pMat->m_ambientReflectivity = color * 0.1f;
	pMat->m_diffuseReflectivity = color *0.7f;
	pMat->m_specularReflectivity = color * 0.1f;
}

const Engine::Vec3 PLUS_Y(0.0f, 1.0f, 0.0f);
void AIDemoDargonComponent::FaceMoveDir()
{
	Engine::Vec3 vel = m_pSpatial->GetVelocity();
	if (vel.LengthSquared() > 0.01f) 
	{
		Engine::Vec3 forward = vel.Normalize();
		m_pGobComp->GetGraphicalObject()->SetRotMat(Engine::Mat4::AxisRotation(forward, forward.Cross(PLUS_Y).Cross(forward).Normalize()));
		m_pGobComp->GetGraphicalObject()->CalcFullTransform();
	}
	
}

void AIDemoDargonComponent::InitOffset()
{
	int index = Engine::MathUtility::Rand(0, (int)m_pFormationGob->GetMeshPointer()->GetVertexCount());
	Engine::Vec3 pos1 = *reinterpret_cast<Engine::Vec3*>(m_pFormationGob->GetMeshPointer()->GetPointerToVertexAt(index));
	int indexTwo = Engine::MathUtility::Clamp(index % 3 == 0 ? index + 1 : index - 1, 0, m_pFormationGob->GetMeshPointer()->GetVertexCount() - 1);
	Engine::Vec3 pos2 = *reinterpret_cast<Engine::Vec3*>(m_pFormationGob->GetMeshPointer()->GetPointerToVertexAt(indexTwo));
	m_offset = pos1.Lerp(pos2, Engine::MathUtility::Rand(0.0f, 1.0f));
	m_offset = m_offset * 20.0f;
}

