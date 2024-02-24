#pragma once
#include "GameObject.h"

namespace NCL {
    namespace CSC8503 {
        class StateMachine;
        class GameWorld;
        class StateGameObject : public GameObject  {
        public:
            //StateGameObject();
            StateGameObject(Vector3 playerPos, Vector3 enemyPos, bool spotPlayer);
            ~StateGameObject();

            virtual void Update(float dt, Vector3 startPos, Vector3 destPos, bool spotPlayer);

        protected:
            void MoveUp(float dt);
            void MoveDown(float dt);

            void Chasing(float dt, Vector3 startPos, Vector3 destPos);

            void Rotating(float dt, Vector3 startPos, Vector3 destPos);

            StateMachine* stateMachine;
            float counter;

            GameWorld* world;

            bool foundPath;
        };
    }
}
