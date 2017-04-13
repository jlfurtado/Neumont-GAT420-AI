#include "StarComp.h"
#include "SpatialComponent.h"


StarComp::StarComp()
{
}


StarComp::~StarComp()
{
}

bool StarComp::Initialize()
{
	return true;
}

bool StarComp::Update(float dt)
{
	Engine::SpatialComponent *pSpatial = GetSiblingComponent<Engine::SpatialComponent>();
	
	if (pSpatial != nullptr)
	{
		pSpatial->Translate(Engine::Vec3(0, 10, 0) *dt);
	}

	return true;
}
