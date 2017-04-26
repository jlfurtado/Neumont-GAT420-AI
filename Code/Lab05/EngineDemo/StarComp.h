#pragma once

#include "Component.h"

class StarComp : public Engine::Component
{
public:
	StarComp();
	~StarComp();

	bool Initialize() override;
	bool Update(float dt) override;
};

