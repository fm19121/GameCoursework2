#include "PushdownMachine.h"
#include "PushdownState.h"

using namespace NCL::CSC8503;

PushdownMachine::PushdownMachine(PushdownState* initialState)
{
	this->initialState = initialState;
}

PushdownMachine::~PushdownMachine()
{
}

bool PushdownMachine::Update(float dt, TutorialGame* g) {
	if (activeState) {
		PushdownState* newState = nullptr;
		PushdownState::PushdownResult result = activeState->OnUpdate(dt, &newState, g);

		switch (result) {
			case PushdownState::Pop: {
				activeState->OnSleep();
				delete activeState;
				stateStack.pop();
				if (stateStack.empty()) {
					return false;
				}
				else {
					activeState = stateStack.top();
					activeState->OnAwake(dt, g);
				}					
			}break;
			case PushdownState::Push: {
				activeState->OnSleep();		

				stateStack.push(newState);
				activeState = newState;
				activeState->OnAwake(dt, g);
			}break;
			case PushdownState::NoChange:{
				activeState->OnAwake(dt, g);
			}break;
			case PushdownState::Reset: {
				activeState->OnSleep();
				delete activeState;
				for (int i = 0; i < stateStack.size(); i++) {
					stateStack.pop();
				}
				activeState = stateStack.top();
				activeState->OnAwake(dt, g);
			}break;
		}
	}
	else {
		stateStack.push(initialState);
		activeState = initialState;
		activeState->OnAwake(dt, g);
	}
	return true;
}