#include "stdafx.h"
#include "WorldState.h"
#include <Exam_HelperStructs.h>

#include "EBlackboard.h"

WorldState::WorldState()
{
}

WorldState::~WorldState()
{
}

void WorldState::UpdateWorldState(Elite::Blackboard& blackboard) //Intensive function and performance overhead of GOAP
{
	WorldStates worldState{};

	std::vector<EntityInfo*> enemies{};
	std::vector<EntityInfo*> items{};
	std::vector<EntityInfo*> purges{};

#pragma region FOV entity checks

	blackboard.GetData("EnemiesInFOV", enemies);
	if(!enemies.empty())
	{
		worldState.seeEnemy = true;
	}

	blackboard.GetData("ItemsInFOV", items);
	if (!items.empty())
	{
		worldState.seeItem = true;
	}

	blackboard.GetData("PurgesInFOV", purges);
	if (!purges.empty())
	{
		worldState.seePurge = true;
	}
	
	//for (auto& e : blackboard.GetData())
	//{
	//	switch (e.Type)
	//	{
	//	case eEntityType::ENEMY:
	//		worldState.seeEnemy = true;
	//		enemies.push_back(&e);
	//		break;
	//	case eEntityType::ITEM:
	//		worldState.seeItem = true;
	//		items.push_back(&e);
	//		break;
	//	case eEntityType::PURGEZONE:
	//		worldState.seePurge = true;
	//		break;
	//	default:
	//		std::cout << "Check visible entity switch, something's wrong there\n";
	//	}
	//}
#pragma endregion

#pragma region LookAt checks and range check for items
	AgentInfo pAgent{};
	blackboard.GetData("Agent", pAgent);

	if (!enemies.empty())
	{
		EntityInfo target = *enemies[0];
		//LookAt
		const Elite::Vector2 desiredVector = Elite::Vector2(target.Location - pAgent.Position);
		const Elite::Vector2 lookVector{ std::cosf(pAgent.Orientation), std::sinf(pAgent.Orientation) };

		if (fabsf(Elite::AngleBetween(lookVector, desiredVector)) < 0.1f)
		{
			worldState.lookingAt = true;
		}
	}
	else if (!items.empty())
	{
		EntityInfo target = *items[0];
		//LookAt
		const Elite::Vector2 desiredVector = Elite::Vector2(target.Location - pAgent.Position);
		const Elite::Vector2 lookVector{ std::cosf(pAgent.Orientation), std::sinf(pAgent.Orientation) };

		if (fabsf(Elite::AngleBetween(lookVector, desiredVector)) < 0.1f)
		{
			worldState.lookingAt = true;
		}

		if (target.Location.DistanceSquared(pAgent.Position) <= pAgent.GrabRange * pAgent.GrabRange)
		{
			worldState.inPickupRange = true;
		}
	}
#pragma endregion

#pragma region Agent checks
	if (pAgent.Health <= 3.f)
	{
		worldState.lowHP = true;
	}
	if (pAgent.Energy <= 3.f)
	{
		worldState.lowEnergy = true;
	}
#pragma endregion

	worldState.newHouse = NewHouseInFOV(blackboard);
	worldState.movingToHouse = ShouldMoveToHouse(blackboard);
	worldState.agentInHouse = AgentInHouse(blackboard);
	worldState.entranceSet = m_WorldStates.entranceSet;
	worldState.leavingHouse = m_WorldStates.leavingHouse;

	m_WorldStates = worldState;
}

bool WorldState::NewHouseInFOV(Elite::Blackboard& blackboard)
{
	std::vector<HouseInfo> pHousesInFOV{};
	std::vector<std::pair<HouseInfo, float>> pFoundHouses{};

	if (!blackboard.GetData("HousesInFOV", pHousesInFOV))
	{
		return false;
	}

	if (pHousesInFOV.empty())
	{
		return false;
	}

	if (!blackboard.GetData("FoundHouses", pFoundHouses))
	{
		return false;
	}

	//std::cout << pHousesInFOV[0].Center << '\n';

	if(pHousesInFOV[0].Center.x < 20.f)
	{
		//std::cout << "hello there \n";
	}

	//std::cout << pFoundHouses.size() << '\n';

	std::vector<HouseInfo> foundHouseInfos{};

	for (const auto house : pFoundHouses) //Extract house infos of all houses
	{
		foundHouseInfos.push_back(house.first);
	}

	for (const auto& houseToCheck : foundHouseInfos)
	{
		//std::cout << houseToCheck.Center << '\n';
		if (pHousesInFOV[0].Center == houseToCheck.Center) //If house isn't already found
		{
			return false;
		}
	}

	pFoundHouses.push_back({ pHousesInFOV[0], 300.f });
	blackboard.ChangeData("FoundHouses", pFoundHouses);
	//std::cout << "Changed data\n";
	return true;
}

bool WorldState::ShouldMoveToHouse(Elite::Blackboard& blackboard)
{
	std::vector<std::pair<HouseInfo, float>> pFoundHouses{};
	HouseInfo targetHouse{};
	Elite::Vector2 target{};
	std::vector<HouseInfo> pHousesInFOV{};
	float dt{};

	if (!blackboard.GetData("FoundHouses", pFoundHouses))
	{
		return false;
	}

	if (!blackboard.GetData("TargetHouse", targetHouse))
	{
		return false;
	}

	if (!blackboard.GetData("Target", target))
	{
		return false;
	}

	if (!blackboard.GetData("dt", dt))
	{
		return false;
	}

	//if (!pBlackboard->GetData("MovingToHouse", movingToHouse))
	//{
	//	return false;
	//}

	//std::cout << pFoundHouses.size() << '\n';

	for (auto& house : pFoundHouses)
	{
		house.second += dt;
	}
	blackboard.ChangeData("FoundHouses", pFoundHouses);

	if (!m_WorldStates.movingToHouse && !m_WorldStates.agentInHouse)
	{
		const auto it = std::find_if(pFoundHouses.begin(), pFoundHouses.end(),
			[](const std::pair<HouseInfo, float>& pair)
			{
				return pair.second >= 300.f;
			});

		if (it != pFoundHouses.end())
		{
			// Element with second value > 300 found
			// 'it' points to the found element
			targetHouse = it->first;
			blackboard.ChangeData("TargetHouse", targetHouse);
			target = targetHouse.Center;
			blackboard.ChangeData("Target", target);
			return true;
		}
	}
	else if(m_WorldStates.movingToHouse)
	{
		return true;
	}

	return false;
}

bool WorldState::AgentInHouse(Elite::Blackboard& blackboard)
{
	AgentInfo pAgent{};
	HouseInfo targetHouse{};

	if (!blackboard.GetData("Agent", pAgent))
	{
		return false;
	}

	if (!blackboard.GetData("TargetHouse", targetHouse))
	{
		return false;
	}

	const Elite::Vector2 agentPos = pAgent.Position;

	//House corners
	const Elite::Vector2 bottomLeft = Elite::Vector2{ targetHouse.Center.x - targetHouse.Size.x / 2,
													  targetHouse.Center.y - targetHouse.Size.y / 2 };

	const Elite::Vector2 topRight = Elite::Vector2{ targetHouse.Center.x + targetHouse.Size.x / 2,
													targetHouse.Center.y + targetHouse.Size.y / 2 };

	if (agentPos.x > bottomLeft.x && agentPos.x < topRight.x)
	{
		if (agentPos.y > bottomLeft.y && agentPos.y < topRight.y)
		{
			return true;
		}
	}

	if(m_WorldStates.leavingHouse)
	{
		blackboard.ChangeData("TargetHouse", HouseInfo{});
		m_WorldStates.leavingHouse = false;
	}

	return false;
}