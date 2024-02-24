#pragma once
#include "GameObject.h"

namespace NCL {
	namespace CSC8503 {
		typedef std::function<void(float)> StateUpdateFunction;
		typedef std::function<void(float, Vector3, Vector3)> StateChasingFunction;

		class  State		{
		public:
			State() {}
			State(StateUpdateFunction someFunc) {
				func		= someFunc;
			}
			State(StateChasingFunction someFunc) {
				chase = someFunc;
			}
			void Update(float dt)  {
				if (func != nullptr) {
					func(dt);
				}
			}

			void Update(float dt, Vector3 startPos, Vector3 destPos) {
				if (chase != nullptr) {
					chase(dt, startPos, destPos);
				}
			}

			void SetId(int a) {
				id = a;
			}
			int GetId() {
				return id;
			}

		protected:
			StateUpdateFunction func;
			StateChasingFunction chase;
			int id = -1;
		};
	}
}