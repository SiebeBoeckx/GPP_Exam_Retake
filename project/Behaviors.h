/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
//#include "framework/EliteMath/EMath.h"
#include "stdafx.h"
#include "Exam_HelperStructs.h"
#include "IExamInterface.h"
#include "EBehaviorTree.h"
#include "SteeringBehaviours.h"
#include "WorldState.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------

namespace BT_Actions
{
	Elite::BehaviorState HouseFunctionality(Elite::Blackboard* pBlackboard, WorldState* pWorldState)
	{
		//std::cout << "Going to a house\n";
#pragma region GetVariables

		AgentInfo pAgent{};
		std::vector<std::pair<HouseInfo, float>> pFoundHouses{};
		HouseInfo pTargetHouse{};
		Elite::Vector2 target{};
		Elite::Vector2 houseEntrance{};
		float dt{};
		SteeringPlugin_Output pSteeringBehaviour{};
		Seek seek{};
		IExamInterface* pInterface{};

		float currentTimeInHouse{};

		if (!pBlackboard->GetData("Agent", pAgent))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("FoundHouses", pFoundHouses))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("TargetHouse", pTargetHouse))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Target", target))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("HouseEntrance", houseEntrance))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("dt", dt))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("SteeringBehaviour", pSteeringBehaviour))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Seek", seek))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		//if (!pBlackboard->GetData("EntranceSet", entranceSet))
		//{
		//	return Elite::BehaviorState::Failure;
		//}

		//if (!pBlackboard->GetData("LeavingHouse", leavingHouse))
		//{
		//	return Elite::BehaviorState::Failure;
		//}

		//if (!pBlackboard->GetData("MovingToHouse", movingToHouse))
		//{
		//	return Elite::BehaviorState::Failure;
		//}

		//if (!pBlackboard->GetData("AgentInHouse", agentInHouse))
		//{
		//	return Elite::BehaviorState::Failure;
		//}

		if (!pBlackboard->GetData("CurrentTimeInHouse", currentTimeInHouse))
		{
			return Elite::BehaviorState::Failure;
		}

#pragma endregion
		//House functionality

		const float maxTimeIdleInHouse{ 3.f };
		Elite::Vector2 nextTargetPos = pInterface->NavMesh_GetClosestPathPoint(target);
		pInterface->Draw_Circle(nextTargetPos, 1, { 0,0,1 }, pInterface->NextDepthSlice());
		//pInterface->Draw_Circle(houseEntrance, 1, { 0,1,0 }, pInterface->NextDepthSlice());

		if (pWorldState->ChangeWorldState().movingToHouse)
		{
			//std::cout << std::to_string(target.x) << ", " << std::to_string(target.y) << '\n';
			//std::cout << std::to_string(pTargetHouse.Center.x) << ", " << std::to_string(pTargetHouse.Center.y) << '\n';
			target = pTargetHouse.Center;
			pBlackboard->ChangeData("Target", target);

			//if (pWorldState->GetWorldState().agentInHouse && !pWorldState->GetWorldState().entranceSet && !pWorldState->GetWorldState().leavingHouse) //set entrance when entering house, also set the timer back to 0
			//{
			//	pBlackboard->ChangeData("HouseEntrance", pAgent.Position);
			//	pWorldState->ChangeWorldState().entranceSet = true;
			//	pWorldState->ChangeWorldState().movingToHouse = false;
			//	pBlackboard->ChangeData("CurrentTimeInHouse", 0.f);
			//}

			//Removed above because i use a safer way to do it further down

			if (!pWorldState->GetWorldState().agentInHouse) //go to the house
			{
				seek.SetTarget(nextTargetPos);
				pSteeringBehaviour = seek.CalculateSteering(dt, &pAgent);
				pBlackboard->ChangeData("SteeringBehaviour", pSteeringBehaviour);
				//pBlackboard->ChangeData("LeavingHouse", false);
				pWorldState->ChangeWorldState().leavingHouse = false;
				pBlackboard->ChangeData("CurrentTimeInHouse", 0.f);
				return Elite::BehaviorState::Success;
			}

		}

		if (pWorldState->GetWorldState().agentInHouse)
		{
			if(!pWorldState->GetWorldState().entranceSet && !pWorldState->GetWorldState().leavingHouse) //Entered house without setting entrance, can happen when going to collect item
			{
				const Elite::Vector2 tempTarget = pTargetHouse.Center + (pTargetHouse.Size * 2); //Set target outside house
				const Elite::Vector2 tempNextTargetPos = pInterface->NavMesh_GetClosestPathPoint(tempTarget); //Get closest point till there, aka the door
				pBlackboard->ChangeData("HouseEntrance", tempNextTargetPos);
				pWorldState->ChangeWorldState().entranceSet = true;
				pWorldState->ChangeWorldState().movingToHouse = false;
				pBlackboard->ChangeData("CurrentTimeInHouse", 0.f);
			}
			if (currentTimeInHouse > maxTimeIdleInHouse) //Seek to exit house after a while
			{
				//std::cout << "Trying to exit\n";
				const Elite::Vector2 tempTarget = pTargetHouse.Center + (pTargetHouse.Size * 2); //Set target outside house
				const Elite::Vector2 tempNextTargetPos = pInterface->NavMesh_GetClosestPathPoint(tempTarget); //Get closest point till there, aka the door
				seek.SetTarget(tempNextTargetPos);
				pSteeringBehaviour = seek.CalculateSteering(dt, &pAgent);
				pBlackboard->ChangeData("Target", tempNextTargetPos);
				pBlackboard->ChangeData("SteeringBehaviour", pSteeringBehaviour);
				pWorldState->ChangeWorldState().entranceSet = false;
				
				pWorldState->ChangeWorldState().leavingHouse = true;

				// Find the HouseInfo* that matches the raw value
				auto it = std::find_if(pFoundHouses.begin(), pFoundHouses.end(),
					[&pTargetHouse](const std::pair<HouseInfo, float>& pair) 
					{
						return pair.first.Center.x == pTargetHouse.Center.x;
					});

				// Check if the HouseInfo* was found before updating
				if (it != pFoundHouses.end())
				{
					// Update the float value of the pair
					it->second = 0.0f;
					pBlackboard->ChangeData("FoundHouses", pFoundHouses);
				}

				return Elite::BehaviorState::Success;
			}

			if (!pWorldState->GetWorldState().leavingHouse) //Look around the house
			{
				//std::cout << "Trying to search house\n";
				currentTimeInHouse += dt;
				target = pTargetHouse.Center;
				seek.SetTarget(target);
				pSteeringBehaviour = seek.CalculateSteering(dt, &pAgent);
				pBlackboard->ChangeData("Target", target);
				pBlackboard->ChangeData("SteeringBehaviour", pSteeringBehaviour);
				pBlackboard->ChangeData("CurrentTimeInHouse", currentTimeInHouse);
				return Elite::BehaviorState::Success;
			}
			else //Started leaving, but got interupted (enemy/item)
			{
				target = houseEntrance;
				seek.SetTarget(target);
				pSteeringBehaviour = seek.CalculateSteering(dt, &pAgent);
				pBlackboard->ChangeData("Target", target);
				pBlackboard->ChangeData("SteeringBehaviour", pSteeringBehaviour);
			}
		}

		//Reaches here once when entering new house, leaving house does not get set false when leaving previous one
		pWorldState->ChangeWorldState().leavingHouse = false;
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState FleeEnemy(Elite::Blackboard* pBlackboard)
	{
#pragma region GetVariables

		AgentInfo pAgent{};
		SteeringPlugin_Output pSteering{};
		Flee pFlee{};
		float dt{};
		std::vector<EntityInfo*> pEnemyVec{};
		Elite::Vector2 target{};
		IExamInterface* pInterface{};

		if (!pBlackboard->GetData("Agent", pAgent))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("SteeringBehaviour", pSteering))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Flee", pFlee))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("dt", dt))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("EnemiesInFOV", pEnemyVec))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Target", target))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

#pragma endregion

		bool hasGun{};

		ItemInfo gunInfo{};
		if (pInterface->Inventory_GetItem(0, gunInfo))
		{
			hasGun = true;
		}
		if (pInterface->Inventory_GetItem(1, gunInfo))
		{
			hasGun = true;
		}

		if (hasGun)
		{
			target = pEnemyVec[0]->Location;
			pFlee.SetTarget(target);
			pFlee.SetFleeRadius(pAgent.FOV_Range * 1.5f);
			pSteering = pFlee.CalculateSteering(dt, &pAgent);
			pBlackboard->ChangeData("Target", target);
			pBlackboard->ChangeData("SteeringBehaviour", pSteering);
			pBlackboard->ChangeData("Flee", pFlee);
			return Elite::BehaviorState::Running;
		}
		return Elite::BehaviorState::Failure;
	}

	Elite::BehaviorState UseGun(Elite::Blackboard* pBlackboard)
	{
#pragma region GetVariables
		IExamInterface* pInterface{};
		AgentInfo agent{};
		std::vector<EntityInfo*> pEnemyVec{};

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Agent", agent))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("EnemiesInFOV", pEnemyVec))
		{
			return Elite::BehaviorState::Failure;
		}

#pragma endregion

		bool hasPistol{};
		bool hasShotgun{};

		ItemInfo gunInfo{};
		if (pInterface->Inventory_GetItem(0, gunInfo))
		{
			if (pInterface->Weapon_GetAmmo(gunInfo) > 0)
			{
				hasPistol = true;
			}
			else
			{
				pInterface->Inventory_RemoveItem(0);
			}
		}
		if (pInterface->Inventory_GetItem(1, gunInfo))
		{
			if (pInterface->Weapon_GetAmmo(gunInfo) > 0)
			{
				hasShotgun = true;
			}
			else
			{
				pInterface->Inventory_RemoveItem(1);
			}
		}

		if(!hasPistol && !hasShotgun)
		{
			return Elite::BehaviorState::Failure;
		}

		if (pEnemyVec[0]->Location.DistanceSquared(agent.Position) <= (agent.FOV_Range / 2) && hasShotgun)//using grab range since i kinda know the size (is rendered)
		{
			pInterface->Inventory_UseItem(1);
			return Elite::BehaviorState::Success;
		}

		if(hasPistol)
		{
			pInterface->Inventory_UseItem(0);
			return Elite::BehaviorState::Success;
		}

		if(hasShotgun)
		{
			pInterface->Inventory_UseItem(1);
			return Elite::BehaviorState::Success;
		}

		return Elite::BehaviorState::Failure;
	}

	Elite::BehaviorState PickupFunctionality(Elite::Blackboard* pBlackboard, WorldState* pWorldState)
	{
		//std::cout << "Picking something up\n";
#pragma region GetVariables

		AgentInfo pAgent{};
		SteeringPlugin_Output pSteering{};
		Arrive pArrive{};
		float dt{};
		std::vector<EntityInfo*> pItemVec{};
		IExamInterface* pInterface{};
		Elite::Vector2 target{};

		if (!pBlackboard->GetData("Agent", pAgent))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("SteeringBehaviour", pSteering))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Arrive", pArrive))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("dt", dt))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("ItemsInFOV", pItemVec))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Target", target))
		{
			return Elite::BehaviorState::Failure;
		}

#pragma endregion

		auto item = pItemVec[0];
		target = item->Location;
		pArrive.SetTarget(target);
		pSteering = pArrive.CalculateSteering(dt, &pAgent);
		pBlackboard->ChangeData("Target", target);
		pBlackboard->ChangeData("SteeringBehaviour", pSteering);


		if (pWorldState->GetWorldState().inPickupRange)
		{
			ItemInfo info{};
			ItemInfo tempInfo{};
			pInterface->Item_GetInfo(*item, info);

			switch (info.Type)
			{
			case eItemType::GARBAGE:
				pInterface->Item_Destroy(*item);
				return Elite::BehaviorState::Success;
			case eItemType::PISTOL:
			{
				if (pInterface->Inventory_GetItem(0, tempInfo))
				{
					if (pInterface->Weapon_GetAmmo(tempInfo) < pInterface->Weapon_GetAmmo(info))
					{
						pInterface->Inventory_RemoveItem(0);
						pInterface->Item_Grab(*item, info);
						pInterface->Inventory_AddItem(0, info);
						return Elite::BehaviorState::Success;
					}
					else //destroy item if worse to clear item spawn point
					{
						pInterface->Item_Destroy(*item);
						return Elite::BehaviorState::Success;
					}
				}
				else
				{
					pInterface->Item_Grab(*item, info);
					pInterface->Inventory_AddItem(0, info);
					return Elite::BehaviorState::Success;
				}
			}
			case eItemType::SHOTGUN:
			{
				if (pInterface->Inventory_GetItem(1, tempInfo))
				{
					if (pInterface->Weapon_GetAmmo(tempInfo) < pInterface->Weapon_GetAmmo(info))
					{
						pInterface->Inventory_RemoveItem(1);
						pInterface->Item_Grab(*item, info);
						pInterface->Inventory_AddItem(1, info);
						return Elite::BehaviorState::Success;
					}
					else //destroy item if worse to clear item spawn point
					{
						pInterface->Item_Destroy(*item);
						return Elite::BehaviorState::Success;
					}
				}
				else
				{
					pInterface->Item_Grab(*item, info);
					pInterface->Inventory_AddItem(1, info);
					return Elite::BehaviorState::Success;
				}
			}

			case eItemType::MEDKIT:
			{
				if (pInterface->Inventory_GetItem(2, tempInfo))
				{
					if (pInterface->Medkit_GetHealth(tempInfo) < pInterface->Medkit_GetHealth(info))
					{
						pInterface->Inventory_RemoveItem(2);
						pInterface->Item_Grab(*item, info);
						pInterface->Inventory_AddItem(2, info);
						return Elite::BehaviorState::Success;
					}
					else //destroy item if worse to clear item spawn point
					{
						pInterface->Item_Destroy(*item);
						return Elite::BehaviorState::Success;
					}
				}
				else
				{
					pInterface->Item_Grab(*item, info);
					pInterface->Inventory_AddItem(2, info);
					return Elite::BehaviorState::Success;
				}
			}
			case eItemType::FOOD:
			{
				if (pInterface->Inventory_GetItem(3, tempInfo))
				{
					if (pInterface->Food_GetEnergy(tempInfo) < pInterface->Food_GetEnergy(info))
					{
						pInterface->Inventory_RemoveItem(3);
						pInterface->Item_Grab(*item, info);
						pInterface->Inventory_AddItem(3, info);
						return Elite::BehaviorState::Success;
					}
					//Don't destroy item here because there's a second slot to check
				}
				else
				{
					pInterface->Item_Grab(*item, info);
					pInterface->Inventory_AddItem(3, info);
					return Elite::BehaviorState::Success;
				}

				if (pInterface->Inventory_GetItem(4, tempInfo))
				{
					if (pInterface->Food_GetEnergy(tempInfo) < pInterface->Food_GetEnergy(info))
					{
						pInterface->Inventory_RemoveItem(4);
						pInterface->Item_Grab(*item, info);
						pInterface->Inventory_AddItem(4, info);
						return Elite::BehaviorState::Success;
					}
					else //destroy item if worse to clear item spawn point
					{
						pInterface->Item_Destroy(*item);
						return Elite::BehaviorState::Success;
					}
				}
				else
				{
					pInterface->Item_Grab(*item, info);
					pInterface->Inventory_AddItem(4, info);
					return Elite::BehaviorState::Success;
				}
			}
			default:
				return Elite::BehaviorState::Failure;
			}
		}
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ExploreWorld(Elite::Blackboard* pBlackboard)
	{
#pragma region GetVariables
		AgentInfo agent{};
		SteeringPlugin_Output pSteeringBehaviour{};
		float dt{};
		Seek seek{};
		ExploreDirections direction{};
		IExamInterface* pInterface{};
		Elite::Vector2 target{};

		if (!pBlackboard->GetData("Agent", agent))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("SteeringBehaviour", pSteeringBehaviour))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("dt", dt))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Seek", seek))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("ExploreDirection", direction))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Target", target))
		{
			return Elite::BehaviorState::Failure;
		}

#pragma endregion

		const Elite::Vector2 topLeft {-150.f, 200.f};
		const Elite::Vector2 topRight {150.f, 200.f};
		const Elite::Vector2 bottomLeft {-150.f, -200.f};
		const Elite::Vector2 bottomRight {150.f, -200.f};

		//pInterface->Draw_SolidCircle(topLeft, 10.f, { 0,0 }, { 1,1,1 });
		//pInterface->Draw_SolidCircle(topRight, 10.f, { 0,0 }, { 1,1,1 });
		//pInterface->Draw_SolidCircle(bottomLeft, 10.f, { 0,0 }, { 1,1,1 });
		//pInterface->Draw_SolidCircle(bottomRight, 10.f, { 0,0 }, { 1,1,1 });
		const Elite::Vector2 nextTargetPos = pInterface->NavMesh_GetClosestPathPoint(target);

		switch(direction) //Go to corners to explore
		{
		case ExploreDirections::TopLeft:
			target = topLeft;
			seek.SetTarget(nextTargetPos);
			if (topLeft.DistanceSquared(agent.Position) <= 5.f)//Arrived at location
			{
				direction = ExploreDirections::TopRight; //Go to next corner
				pBlackboard->ChangeData("ExploreDirection", direction);
			}
			break;
		case ExploreDirections::TopRight:
			target = topRight;
			seek.SetTarget(nextTargetPos);
			if (topRight.DistanceSquared(agent.Position) <= 5.f)//Arrived at location
			{
				direction = ExploreDirections::BottomLeft; //Go to next corner
				pBlackboard->ChangeData("ExploreDirection", direction);
			}
			break;
		case ExploreDirections::BottomLeft:
			target = bottomLeft;
			seek.SetTarget(nextTargetPos);
			if (bottomLeft.DistanceSquared(agent.Position) <= 5.f)//Arrived at location
			{
				direction = ExploreDirections::BottomRight; //Go to next corner
				pBlackboard->ChangeData("ExploreDirection", direction);
			}
			break;
		case ExploreDirections::BottomRight:
			target = bottomRight;
			seek.SetTarget(nextTargetPos);
			if (bottomRight.DistanceSquared(agent.Position) <= 5.f)//Arrived at location
			{
				direction = ExploreDirections::TopLeft; //Go to next corner, made full route
				pBlackboard->ChangeData("ExploreDirection", direction);
			}
			break;
		default:
			return Elite::BehaviorState::Failure;
		}

		pSteeringBehaviour = seek.CalculateSteering(dt, &agent);
		pBlackboard->ChangeData("SteeringBehaviour", pSteeringBehaviour);
		pBlackboard->ChangeData("Target", target);
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState UseFood(Elite::Blackboard* pBlackboard)
	{
#pragma region GetVariables

		IExamInterface* pInterface{};

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

#pragma endregion

		ItemInfo foodInfo{};
		if (pInterface->Inventory_GetItem(3, foodInfo))
		{
			pInterface->Inventory_UseItem(3);
			pInterface->Inventory_RemoveItem(3);
			return Elite::BehaviorState::Success;
		}
		else if (pInterface->Inventory_GetItem(4, foodInfo))
		{
			pInterface->Inventory_UseItem(4);
			pInterface->Inventory_RemoveItem(4);
			return Elite::BehaviorState::Success;
		}
		return Elite::BehaviorState::Failure;
	}

	Elite::BehaviorState UseMedkit(Elite::Blackboard* pBlackboard)
	{
#pragma region GetVariables

		IExamInterface* pInterface{};

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

#pragma endregion

		ItemInfo medkitInfo{};
		if (pInterface->Inventory_GetItem(2, medkitInfo))
		{
			pInterface->Inventory_UseItem(2);
			pInterface->Inventory_RemoveItem(2);
			return Elite::BehaviorState::Success;
		}
		return Elite::BehaviorState::Failure;
	}

	Elite::BehaviorState FleePurge(Elite::Blackboard* pBlackboard)
	{
#pragma region GetVariables

		AgentInfo pAgent{};
		SteeringPlugin_Output pSteering{};
		Flee pFlee{};
		float dt{};
		std::vector<EntityInfo*> pPurgeVec{};
		Elite::Vector2 target{};
		IExamInterface* pInterface{};

		if (!pBlackboard->GetData("Agent", pAgent))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("SteeringBehaviour", pSteering))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Flee", pFlee))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("dt", dt))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("PurgesInFOV", pPurgeVec))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Target", target))
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Interface", pInterface) || pInterface == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

#pragma endregion
		EntityInfo purgeEntity = *pPurgeVec[0];
		PurgeZoneInfo purge{};
		pInterface->PurgeZone_GetInfo(purgeEntity, purge);

		if (purge.Center.DistanceSquared(pAgent.Position) < purge.Radius * purge.Radius + pAgent.FOV_Range)
		{
			target = purge.Center;
			pFlee.SetTarget(target);
			pFlee.SetFleeRadius(purge.Radius + pAgent.FOV_Range * 0.8f); //keep looking to see when it's gone
			pSteering = pFlee.CalculateSteering(dt, &pAgent);
			pBlackboard->ChangeData("Target", target);
			pBlackboard->ChangeData("SteeringBehaviour", pSteering);
			pBlackboard->ChangeData("Flee", pFlee);
			return Elite::BehaviorState::Running;
		}
		return Elite::BehaviorState::Running;
	}
}

//NOT USING THESE CONDITIONS ANYMORE, REPLACED BY GOAP
namespace BT_Conditions
{
	bool NewHouseInFOV(Elite::Blackboard* pBlackboard)
	{
		std::vector<HouseInfo*> pHousesInFOV{};
		std::vector<std::pair<HouseInfo*, float>> pFoundHouses{};

		if (!pBlackboard->GetData("HousesInFOV", pHousesInFOV) || &pHousesInFOV == nullptr)
		{
			return false;
		}

		if (pHousesInFOV.size() == 0)
		{
			return false;
		}

		if (!pBlackboard->GetData("FoundHouses", pFoundHouses) || &pFoundHouses == nullptr)
		{
			return false;
		}

		std::vector<HouseInfo*> foundHouseInfos{};

		for (const auto house : pFoundHouses) //Extract house infos of all houses
		{
			foundHouseInfos.push_back(house.first);
		}

		for (const auto houseToCheck : foundHouseInfos)
		{
			if (pHousesInFOV[0]->Center == houseToCheck->Center) //If house isn't already found
			{
				return false;
			}
		}
		
		pFoundHouses.push_back({ pHousesInFOV[0], 300.f });
		pBlackboard->ChangeData("FoundHouses", pFoundHouses);
		return true;
	}

	bool ShouldMoveToHouse(Elite::Blackboard* pBlackboard)
	{
		std::vector<std::pair<HouseInfo*, float>> pFoundHouses{};
		HouseInfo pTargetHouse{};
		//Elite::Vector2 target{};

		//bool movingToHouse{};
		//bool agentInHouse{};

		if (!pBlackboard->GetData("FoundHouses", pFoundHouses) || &pFoundHouses == nullptr)
		{
			return false;
		}

		if (!pBlackboard->GetData("TargetHouse", pTargetHouse))
		{
			return false;
		}

		//if (!pBlackboard->GetData("Target", target))
		//{
		//	return false;
		//}

		//if (!pBlackboard->GetData("MovingToHouse", movingToHouse))
		//{
		//	return false;
		//}

		//if (!pBlackboard->GetData("MovingToHouse", agentInHouse))
		//{
		//	return false;
		//}

		//std::cout << pFoundHouses.size() << '\n';

		//not being used

		return false;
	}

	bool AgentInHouse(Elite::Blackboard* pBlackboard)
	{
		AgentInfo pAgent{};
		HouseInfo pTargetHouse{};
		bool agentInHouse{};

		if (!pBlackboard->GetData("Agent", pAgent) || &pAgent == nullptr)
		{
			return false;
		}

		if (!pBlackboard->GetData("TargetHouse", pTargetHouse))
		{
			return false;
		}

		if (!pBlackboard->GetData("AgentInHouse", agentInHouse))
		{
			return false;
		}

		const Elite::Vector2 agentPos = pAgent.Position;

		//House corners
		const Elite::Vector2 bottomLeft = Elite::Vector2{ pTargetHouse.Center.x - pTargetHouse.Size.x / 2,
														  pTargetHouse.Center.y - pTargetHouse.Size.y / 2 };

		const Elite::Vector2 topRight = Elite::Vector2{ pTargetHouse.Center.x + pTargetHouse.Size.x / 2,
														pTargetHouse.Center.y + pTargetHouse.Size.y / 2 };

		if (agentPos.x > bottomLeft.x && agentPos.x < topRight.x)
		{
			if (agentPos.y > bottomLeft.y && agentPos.y < topRight.y)
			{
				pBlackboard->ChangeData("AgentInHouse", true);
				return true;
			}
		}

		return false;
	}

	bool IsEnemyInFOV(Elite::Blackboard* pBlackboard)
	{
		std::vector<EntityInfo*> pEnemiesInFOV{};

		if (!pBlackboard->GetData("EnemiesInFOV", pEnemiesInFOV) || &pEnemiesInFOV == nullptr)
		{
			return false;
		}

		if (!pEnemiesInFOV.empty())
		{
			pBlackboard->ChangeData("TargetEnemy", *pEnemiesInFOV[0]);
			return true;
		}
		return false;
	}

	bool IsLookingAtEnemy(Elite::Blackboard* pBlackboard)
	{
		AgentInfo pAgent{};
		EntityInfo pEnemy{};

		if (!pBlackboard->GetData("Agent", pAgent) || &pAgent == nullptr)
		{
			return false;
		}

		if (!pBlackboard->GetData("TargetEnemy", pEnemy) || &pEnemy == nullptr)
		{
			return false;
		}

		const Elite::Vector2 desiredVector = Elite::Vector2(pEnemy.Location - pAgent.Position);
		const Elite::Vector2 lookVector{ std::cosf(pAgent.Orientation),std::sinf(pAgent.Orientation) };

		bool returnBool{ false };

		if (fabsf(Elite::AngleBetween(desiredVector, lookVector)) < 0.1f)
		{
			returnBool = true;
		}

		return returnBool;
	}

	bool IsItemInFOV(Elite::Blackboard* pBlackboard)
	{
		std::vector<EntityInfo*> pItemsInFOV{};

		if (!pBlackboard->GetData("ItemsInFOV", pItemsInFOV) || &pItemsInFOV == nullptr)
		{
			return false;
		}

		if (!pItemsInFOV.empty())
		{
			return true;
		}
		return false;
	}
}

#endif