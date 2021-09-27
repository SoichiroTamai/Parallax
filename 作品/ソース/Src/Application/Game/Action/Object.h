#pragma once
#include "../GameObject.h"

class Object : public GameObject
{
public:
	virtual void Deserialize(const json11::Json& jsonObj) override;
	//virtual void Update() override;
};