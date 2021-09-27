#include "Object.h"

void Object::Deserialize(const json11::Json& jsonObj)
{
	GameObject::Deserialize(jsonObj);
}

//void Object::Update(){}