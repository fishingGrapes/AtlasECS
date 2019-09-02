#pragma once

#include "include/AtlasECS.hpp"

#include <sstream>
#include <string>


struct FPositionComponent : atlas::FComponent<FPositionComponent>
{
public:

	FPositionComponent(float_t x, float_t y, float_t z)
		:x(x), y(y), z(z){}

	FPositionComponent(const FPositionComponent& other)
		:x(other.x), y(other.y), z(other.z){}

	FPositionComponent(FPositionComponent&& other)
		:x(other.x), y(other.y), z(other.z)
	{
		other.x = 0;
		other.y = 0;
		other.z = 0;
	}

	~FPositionComponent() { std::cout << "Position Destroyed " << std::endl; }

	virtual std::string ToString() const override
	{
		std::stringstream ss;
		ss << "(" << x << ", " << y << ", " << z << ")";
		return ss.str();
	}

	float_t x;
	float_t y;
	float_t z;
};


struct FNameComponent : atlas::FComponent<FNameComponent>
{
public:

	FNameComponent(std::string name)
		: Name(name){}

	FNameComponent(const FNameComponent& other)
		: Name(other.Name) {}

	FNameComponent(FNameComponent&& other)
		: Name(other.Name)
	{
		other.Name = "";
	}
	~FNameComponent() { std::cout << "Name Destroyed " << std::endl; }

	virtual std::string ToString() const override { return Name; }

	std::string Name;
};



struct FStaticMeshComponent : atlas::FComponent<FStaticMeshComponent>
{

};