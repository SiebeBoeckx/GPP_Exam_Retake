#pragma once
#include "EBehaviorTree.h"
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"

class Arrive;
class Seek;
class Wander;
class Flee;
class IBaseInterface;
class IExamInterface;
class WorldState;

class Plugin :public IExamPlugin
{
public:
	Plugin() {};
	virtual ~Plugin() {};

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	void InitGameDebugParams(GameDebugParams& params) override;
	void Update(float dt) override;

	SteeringPlugin_Output UpdateSteering(float dt) override;
	void Render(float dt) const override;

private:
	//Interface, used to request data from/perform actions with the AI Framework
	IExamInterface* m_pInterface = nullptr;
	std::vector<HouseInfo> GetHousesInFOV() const;
	std::vector<EntityInfo> GetEntitiesInFOV() const;

	Elite::Vector2 m_Target = {};
	bool m_CanRun = false; //Demo purpose
	bool m_GrabItem = false; //Demo purpose
	bool m_UseItem = false; //Demo purpose
	bool m_RemoveItem = false; //Demo purpose
	float m_AngSpeed = 0.f; //Demo purpose

	UINT m_InventorySlot = 0;

	Elite::Blackboard* Plugin::CreateBlackboard(AgentInfo& a);
	Elite::Blackboard* m_pBlackboard{};

	Elite::BehaviorTree* m_pBehaviourTree{};

	//GOAP
	WorldState* m_pWorldState{};
	WorldStates m_States{};
};

//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	__declspec (dllexport) IPluginBase* Register()
	{
		return new Plugin();
	}
}