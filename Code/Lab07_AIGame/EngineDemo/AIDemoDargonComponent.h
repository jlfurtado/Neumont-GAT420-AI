#ifndef AIDEMODARGONCOMPONENT_H
#define AIDEMODARGONCOMPONENT_H

// Justin Furtado
// 5/30/2017
// AIDemoDargonComponent.h
// Demonstrates various AI Techniques 

#include "Component.h"
#include "StackFSM.h"
#include "Keyboard.h"
#include "AStarPathFollowComponent.h"
#include "SpatialComponent.h"
#include "GraphicalObjectComponent.h"

class AIDemoDargonComponent : public Engine::Component
{
public:
	bool Initialize() override;
	bool Update(float dt) override;
	void SetPlayerRef(Engine::SpatialComponent *pPlayerSpatial);
	void SetPCollectibles(Engine::LinkedList<Engine::GraphicalObject*> *pCollectibles);

private:
	static void DoNothingOnPurpose(void *pData);
	static void DoNothingOnPurpose(float dt, void *pData);
	static void StopMoving(void *pData);

	static void FlockEnter(void *pData);
	static void FlockUpdate(float dt, void *pData);
	static void FlockExit(void *pData);

	static void WanderEnter(void *pData);
	static void WanderUpdate(float dt, void *pData);

	static void SeekEnter(void *pData);
	static void SeekUpdate(float dt, void *pData);

	static void ArrivalEnter(void *pData);
	static void ArrivalUpdate(float dt, void *pData);

	static void FleeEnter(void *pData);
	static void FleeUpdate(float dt, void *pData);

	static void PursueEnter(void *pData);
	static void PursueUpdate(float dt, void *pData);

	static void PursueOffsetEnter(void *pData);
	static void PursueOffsetUpdate(float dt, void *pData);

	static void EvadeEnter(void *pData);
	static void EvadeUpdate(float dt, void *pData);

	static void EnterRandomAStar(void *pData);
	static void ExitRandomAStar(void *pData);

	static void EnterFollowAStar(void *pData);
	static void FollowAStar(float dt, void *pData);
	static void ExitFollowAStar(void *pData);

	static void ForageEnter(void *pData);
	static void ForageUpdate(float dt, void *pData);


	static void SetColor(Engine::Material *pMat, const Engine::Vec3& color);
	void FaceMoveDir();



	Engine::Keyboard m_keyboardManager;
	Engine::StackFSM m_brain;
	Engine::AStarPathFollowComponent *m_pAStarFollow;
	Engine::SpatialComponent *m_pSpatial;
	Engine::SpatialComponent *m_pPlayerSpatial{ nullptr };
	Engine::GraphicalObjectComponent *m_pGobComp;
	Engine::Vec3 m_offset;
	int m_index{ 0 };
	static const int NUM_STEERS = 8;
	static const int NUM_ASTARS = 2;
	static const int NUM_FLOCK = 1;
	static const int NUM_FUNCS = NUM_STEERS + NUM_ASTARS + NUM_FLOCK;
	static Engine::FSMPair s_AIFuncs[NUM_FUNCS];
	Engine::LinkedList<Engine::GraphicalObject*> *m_pCollectibles;
	Engine::Vec3 m_flockWeights;
	float m_speed;

};



#endif // ifndef AIDEMODARGONCOMPONENT_H