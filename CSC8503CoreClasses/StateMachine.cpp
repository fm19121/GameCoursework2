#include "StateMachine.h"
#include "State.h"
#include "StateTransition.h"

using namespace NCL::CSC8503;

StateMachine::StateMachine()	{
	activeState = nullptr;
}

StateMachine::~StateMachine()	{
	for (auto& i : allStates) {
		delete i;
	}
	for (auto& i : allTransitions) {
		delete i.second;
	}
}

void StateMachine::AddState(State* s) {
	allStates.emplace_back(s);
	if (activeState == nullptr) {
		activeState = s;
	}
}

void StateMachine::AddTransition(StateTransition* t) {
	allTransitions.insert(std::make_pair(t->GetSourceState(), t));
}

void StateMachine::Update(float dt, Vector3 playerPos, Vector3 enemyPos, bool spotPlayer) {
	if (activeState->GetId() == 2) {
		activeState->Update(dt, enemyPos, playerPos);
	}
	if (activeState) {
		activeState->Update(dt);
	
		//Get the transition set starting from this state node;
		std::pair<TransitionIterator, TransitionIterator> range = allTransitions.equal_range(activeState);

		for (auto& i = range.first; i != range.second; ++i) {
			if (i->second->CanTransition(playerPos, enemyPos, spotPlayer)) {
				State* newState = i->second->GetDestinationState();
				activeState = newState;
			}
		}
	}
}