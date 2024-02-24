#pragma once

namespace NCL {
	namespace CSC8503 {
		class TutorialGame;
		class PushdownState
		{
		public:
			enum PushdownResult {
				Push, Pop, NoChange, Reset
			};
			PushdownState()  {
			}
			virtual ~PushdownState() {}

			virtual PushdownResult OnUpdate(float dt, PushdownState** pushFunc, TutorialGame* g) = 0;
			virtual void OnAwake(float dt, TutorialGame*) {}
			virtual void OnSleep() {}
			
		protected:
		};
	}
}