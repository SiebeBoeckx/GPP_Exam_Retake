/*=============================================================================*/
// Copyright 2021-2022 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// EBehaviorTree.h: Implementation of a BehaviorTree and the components of a Behavior Tree
/*=============================================================================*/
#ifndef ELITE_BEHAVIOR_TREE
#define ELITE_BEHAVIOR_TREE

//--- Includes ---
#include "EBlackboard.h"
#include "EDecisionMaking.h"
#include "WorldState.h"

class WorldState;

namespace Elite
{
	//-----------------------------------------------------------------
	// BEHAVIOR TREE HELPERS
	//-----------------------------------------------------------------
	enum class BehaviorState
	{
		Failure,
		Success,
		Running
	};

	//-----------------------------------------------------------------
	// BEHAVIOR INTERFACES (BASE)
	//-----------------------------------------------------------------
	class IBehavior
	{
	public:
		IBehavior() = default;
		virtual ~IBehavior() = default;
		virtual BehaviorState Execute(Blackboard* pBlackBoard, WorldState* pWorldState) = 0;

	protected:
		BehaviorState m_CurrentState = BehaviorState::Failure;
	};

	//-----------------------------------------------------------------
	// BEHAVIOR TREE COMPOSITES (IBehavior)
	//-----------------------------------------------------------------
#pragma region COMPOSITES
	//--- COMPOSITE BASE ---
	class BehaviorComposite : public IBehavior
	{
	public:
		explicit BehaviorComposite(std::vector<IBehavior*> childBehaviors)
		{ m_ChildBehaviors = childBehaviors;	}
		virtual ~BehaviorComposite()
		{
			for (auto pb : m_ChildBehaviors)
				SAFE_DELETE(pb);
			m_ChildBehaviors.clear();
		}

		virtual BehaviorState Execute(Blackboard* pBlackBoard, WorldState* pWorldState) override = 0;

	protected:
		std::vector<IBehavior*> m_ChildBehaviors = {};
	};

	//--- SELECTOR ---
	class BehaviorSelector : public BehaviorComposite
	{
	public:
		explicit BehaviorSelector(std::vector<IBehavior*> childBehaviors) :
			BehaviorComposite(childBehaviors) {}
		virtual ~BehaviorSelector() = default;

		virtual BehaviorState Execute(Blackboard* pBlackBoard, WorldState* pWorldState) override;
	};

	//--- SEQUENCE ---
	class BehaviorSequence : public BehaviorComposite
	{
	public:
		explicit BehaviorSequence(std::vector<IBehavior*> childBehaviors) :
			BehaviorComposite(childBehaviors) {}
		virtual ~BehaviorSequence() = default;

		virtual BehaviorState Execute(Blackboard* pBlackBoard, WorldState* pWorldState) override;
	};

	//--- PARTIAL SEQUENCE ---
	class BehaviorPartialSequence : public BehaviorSequence
	{
	public:
		explicit BehaviorPartialSequence(std::vector<IBehavior*> childBehaviors)
			: BehaviorSequence(childBehaviors) {}
		virtual ~BehaviorPartialSequence() = default;

		virtual BehaviorState Execute(Blackboard* pBlackBoard, WorldState* pWorldState) override;

	private:
		unsigned int m_CurrentBehaviorIndex = 0;
	};
#pragma endregion

	//-----------------------------------------------------------------
	// BEHAVIOR TREE CONDITIONAL (IBehavior)
	//-----------------------------------------------------------------
	class BehaviorConditional : public IBehavior
	{
	public:
		explicit BehaviorConditional(std::function<bool(Blackboard*)> fp) : m_fpConditional(fp) {}
		virtual BehaviorState Execute(Blackboard* pBlackBoard, WorldState* pWorldState) override;

	private:
		std::function<bool(Blackboard*)> m_fpConditional = nullptr;
	};

	class BehaviorConditionalBool : public IBehavior
	{
	public:
		explicit BehaviorConditionalBool(bool* pState) : m_pState(pState) {}
		virtual BehaviorState Execute(Blackboard* pBlackBoard, WorldState* pWorldState) override;

	private:
		bool* m_pState{};
	};

	//-----------------------------------------------------------------
	// BEHAVIOR TREE ACTION (IBehavior)
	//-----------------------------------------------------------------
	class BehaviorAction : public IBehavior
	{
	public:
		explicit BehaviorAction(std::function<BehaviorState(Blackboard*)> fp) : m_fpAction(fp) {}
		virtual BehaviorState Execute(Blackboard* pBlackBoard, WorldState* pWorldState) override;

	private:
		std::function<BehaviorState(Blackboard*)> m_fpAction = nullptr;
	};

	class BehaviorActionBool : public IBehavior
	{
	public:
		explicit BehaviorActionBool(std::function<BehaviorState(Blackboard*, WorldState*)> fp) : m_fpAction(fp) {}
		virtual BehaviorState Execute(Blackboard* pBlackBoard, WorldState* pWorldState) override;

	private:
		std::function<BehaviorState(Blackboard*, WorldState*)> m_fpAction = nullptr;
	};

	//-----------------------------------------------------------------
	// BEHAVIOR TREE (BASE)
	//-----------------------------------------------------------------
	class BehaviorTree final : public Elite::IDecisionMaking
	{
	public:
		explicit BehaviorTree(Blackboard* pBlackBoard, IBehavior* pRootBehavior, WorldState* pWorldState)
			: m_pBlackBoard(pBlackBoard), m_pRootBehavior(pRootBehavior), m_pWorldState(pWorldState) {};
		~BehaviorTree()
		{
			SAFE_DELETE(m_pRootBehavior);
			SAFE_DELETE(m_pBlackBoard); //Takes ownership of passed blackboard!
		};

		virtual void Update(float deltaTime) override
		{
			if (m_pRootBehavior == nullptr)
			{
				m_CurrentState = BehaviorState::Failure;
				return;
			}
				
			m_CurrentState = m_pRootBehavior->Execute(m_pBlackBoard, m_pWorldState);
		}
		Blackboard* GetBlackboard() const
		{ return m_pBlackBoard;	}

	private:
		BehaviorState m_CurrentState = BehaviorState::Failure;
		Blackboard* m_pBlackBoard = nullptr;
		IBehavior* m_pRootBehavior = nullptr;
		WorldState* m_pWorldState{};
	};
}
#endif