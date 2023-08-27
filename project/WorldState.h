#pragma once

namespace Elite
{
	class Blackboard;
}

struct WorldStates
{
	bool seeEnemy{};
	bool seeItem{};
	bool seePurge{};
	bool lookingAtEnemy{};
	bool lookingAtItem{};
	bool inPickupRange{};
	bool lowHP{};
	bool lowEnergy{};

	//House bools, don't reset every update
	bool entranceSet{};
	bool leavingHouse{};
	bool movingToHouse{};
	bool agentInHouse{};
	bool newHouse{};
};

class WorldState final
{
public:
	WorldState();
	~WorldState();

    WorldState(const WorldState& other) = delete;
    WorldState(WorldState&& other) = delete;
    WorldState& operator=(const WorldState& other) = delete;
    WorldState& operator=(WorldState&& other) = delete;

	void UpdateWorldState(Elite::Blackboard& blackboard);
	const WorldStates& GetWorldState() const { return m_WorldStates; }
	WorldStates& ChangeWorldState() { return m_WorldStates; }

private:
	WorldStates m_WorldStates{};

	bool NewHouseInFOV(Elite::Blackboard& blackboard);
	bool ShouldMoveToHouse(Elite::Blackboard& pBlackboard);
	bool AgentInHouse(Elite::Blackboard& blackboard);
};

