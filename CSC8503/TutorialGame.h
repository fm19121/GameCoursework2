#include "../NCLCoreClasses/KeyboardMouseController.h"

#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"

#include "StateGameObject.h"

namespace NCL {
	namespace CSC8503 {
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt, bool isUpdate);

			bool LoseCondition();

			bool WinCondition();

			void Init();

			int GetScore() { return score; }

		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

		//	void InitWorld();

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			void InitGameExamples();
			void InitWorld();

			//void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			//void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void InitMazeWorld();

			void ControlPlayer(GameObject* player);
			void ThirdPersonControl(GameObject* player);

			void InitDefaultFloor();

			bool SelectObject();
			//void MoveSelectedObject();
			//void DebugObjectMovement();
			//void LockedObjectMovement();

			void BridgeConstraintTest();

			void CoinCollisionDetection(GameObject* coin);

			bool SpotPlayer();

			GameObject* AddFloorToWorld(const Vector3& position, Vector3 dimensions);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);

			GameObject* AddGooseToWorld(const Vector3& position, float inverseMass = 10.0f);

			GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);

			StateGameObject* AddStateEnemyToWorld(const Vector3& position);

			GameObject* AddCoinToWorld(const Vector3& position);
			GameObject* AddWallToWorld(const Vector3& position, Vector3 dimensions);
			GameObject* AddDoorToWorld(const Vector3& position, Vector3 dimensions);

			void PrintText();

#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem*		physics;
			GameWorld*			world;

			KeyboardMouseController controller;

			bool useGravity;
			bool inSelectionMode;

			bool isFirstPerson;

			bool isSpoted;

			bool canFly;

			//float		forceMagnitude;

			float count;

			int score = 0;

			GameObject* selectionObject = nullptr;

			GameObject* player = nullptr;
			//GameObject* enemy = nullptr;
			GameObject* destination = nullptr;
			GameObject* goose = nullptr;

			GameObject* coin1 = nullptr;
			GameObject* coin2 = nullptr;
			GameObject* coin3 = nullptr;

			GameObject* destinationDoor = nullptr;
			GameObject* topDoor = nullptr;
			GameObject* leftDoor = nullptr;

			StateGameObject* AiEnemy = nullptr;

			Mesh*	gooseMesh = nullptr;
			Mesh*	cubeMesh	= nullptr;
			Mesh*	sphereMesh	= nullptr;

			Texture*	basicTex	= nullptr;
			Shader*		basicShader = nullptr;

			//Coursework Meshes
			Mesh*	charMesh	= nullptr;
			Mesh*	enemyMesh	= nullptr;
			Mesh*	bonusMesh	= nullptr;
			Mesh*   coinMesh    = nullptr;

			//Coursework Additional functionality	
			//GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			/*void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}*/

			GameObject* objClosest = nullptr;
		};
	}
}

