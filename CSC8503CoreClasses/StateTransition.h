#pragma once
#include "GameObject.h"
#include "Vector3.h"
#include "../CSC8503/StateGameObject.h"

namespace NCL {
	namespace CSC8503 {
		class State;
		typedef std::function<bool()> StateTransitionFunction;
		typedef std::function<bool(Vector3, Vector3, bool)> StateTransitionArgFunction;
		typedef std::function<bool(bool)> StateTransistionBoolFunc;
		class StateTransition	{
		public:
			StateTransition(State* source, State* dest, StateTransitionFunction f) {
				sourceState		 = source;
				destinationState = dest;
				function		 = f;
			}

			StateTransition(State* source, State* dest, Vector3 destPos, Vector3 startPos, bool spotPlayer, StateTransitionArgFunction f) {
				sourceState = source;
				destinationState = dest;
				playerPos = destPos;
				enemyPos = startPos;
				isSpot = spotPlayer;
				toChasingFunction = f;
			}

			StateTransition(State* source, State* dest, bool found, StateTransistionBoolFunc f) {
				sourceState = source;
				destinationState = dest;
				foundPath = found;
				backToPatrol = f;
			}

			bool CanTransition(Vector3 playerPos, Vector3 enemyPos, bool spotPlayer) const {
				if (function) {
					return function();
				}
				else if(toChasingFunction){
					return toChasingFunction(playerPos, enemyPos, spotPlayer);
				}
				else {
					return backToPatrol(foundPath);
				}
			}

			State* GetDestinationState()  const {
				return destinationState;
			}

			State* GetSourceState() const {
				return sourceState;
			}

		protected:
			State * sourceState;
			State * destinationState;
			StateTransitionFunction function;
			StateTransitionArgFunction toChasingFunction;
			StateTransistionBoolFunc backToPatrol;
			Vector3 playerPos;
			Vector3 enemyPos;
			bool isSpot;
			bool foundPath;
		};
	}
}

