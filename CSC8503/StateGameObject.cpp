#include "StateGameObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"
#include "TutorialGame.h"
#include "NavigationGrid.h"
#include "NavigationMesh.h"
#include "StateGameObject.h"

using namespace NCL;
using namespace CSC8503;

StateGameObject::StateGameObject(Vector3 playerPos, Vector3 enemyPos, bool spotPlayer) {
	counter = 0.0f;
	stateMachine = new StateMachine();
	foundPath = false;

	State* stateLeft = new State([&](float dt) -> void
	{
			this->MoveUp(dt);
	});

	State* stateRight = new State([&](float dt) -> void
	{
		this->MoveDown(dt);
	});

	State* chasingState = new State([&](float dt, Vector3 playerPos, Vector3 enemyPos)-> void
	{
		this->Chasing(dt, playerPos, enemyPos);
	});

	State* lookAround = new State([&](float dt, Vector3 playerPos, Vector3 enemyPos) -> void
	{
		this->Rotating(dt, playerPos, enemyPos);
	});

	stateLeft->SetId(0);
	stateRight->SetId(1);
	chasingState->SetId(2);
	lookAround->SetId(2);
	stateMachine->AddState(stateLeft);
	stateMachine->AddState(stateRight);
	stateMachine->AddState(chasingState);
	stateMachine->AddState(lookAround);

	stateMachine->AddTransition(new StateTransition(stateLeft, stateRight, [&]()->bool
	{
		return this->counter > 5.0f;
	}));

	stateMachine->AddTransition(new StateTransition(stateRight, stateLeft, [&]()->bool
	{
		return this->counter < -2.0f;
	}));

	stateMachine->AddTransition(new StateTransition(stateLeft, chasingState, playerPos, enemyPos, spotPlayer, [&](Vector3 playerPos, Vector3 enemyPos,  bool spotPlayer)->bool
	{
		bool inRange;
		inRange = (playerPos.x <= enemyPos.x + 30) && (playerPos.x >= enemyPos.x - 30) && (playerPos.z <= enemyPos.z + 30) && (playerPos.z >= enemyPos.z - 30);
		return inRange || spotPlayer;
	}));

	stateMachine->AddTransition(new StateTransition(stateRight, chasingState, playerPos, enemyPos, spotPlayer, [&](Vector3 playerPos, Vector3 enemyPos, bool spotPlayer)->bool
	{
		bool inRange;
		inRange = (playerPos.x <= enemyPos.x + 30) && (playerPos.x >= enemyPos.x - 30) && (playerPos.z <= enemyPos.z + 30) && (playerPos.z >= enemyPos.z - 30);

		return inRange || spotPlayer;
	}));

	stateMachine->AddTransition(new StateTransition(chasingState, chasingState, foundPath, [&](bool found) -> bool 
	{
		if (foundPath) {
			return true;
		}
		else {
			return false;
		}
	}));

	stateMachine->AddTransition(new StateTransition(chasingState, lookAround, foundPath, [&](bool found) -> bool
	{
		if (!foundPath) {
			return true;
		}
		else {
			return false;
		}
	}));

	stateMachine->AddTransition(new StateTransition(lookAround, lookAround, foundPath, [&](bool found) -> bool
	{
		if (!foundPath) {
			return true;
		}
		else {
			return false;
		}
	}));

	stateMachine->AddTransition(new StateTransition(lookAround, chasingState, playerPos, enemyPos, spotPlayer, [&](Vector3 playerPos, Vector3 enemyPos, bool spotPlayer)->bool
		{
			bool inRange;
			inRange = (playerPos.x <= enemyPos.x + 30) && (playerPos.x >= enemyPos.x - 30) && (playerPos.z <= enemyPos.z + 30) && (playerPos.z >= enemyPos.z - 30);

			return inRange || spotPlayer;
		}));
}

StateGameObject::~StateGameObject() {
	delete stateMachine;
}

void StateGameObject::Update(float dt, Vector3 startPos, Vector3 destPos, bool spotPlayer) {
	stateMachine->Update(dt, startPos, destPos, spotPlayer);
}

void StateGameObject::MoveUp(float dt) {
	GetPhysicsObject()->AddForce({ 0, 0, -16 });
	GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(0, Vector3(0, 1, 0))));
	counter += dt;
}

void StateGameObject::MoveDown(float dt) {
	GetPhysicsObject()->AddForce({ 0, 0, 16 });
	GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(180, Vector3(0, 1, 0))));
	counter -= dt;
}

void StateGameObject::Chasing(float dt, Vector3 startPos, Vector3 destPos) {
	NavigationGrid grid("map.txt");

	//counter = 1 + counter;
	NavigationPath outPath;

	foundPath = grid.FindPath(startPos, destPos, outPath);

	Vector3 pos;

	Vector3 node;
	for (int j = 0; outPath.PopWaypoint(pos) && j < 2; j++) {
	}

	outPath.Clear();

	//if (counter >= 40) {
		Vector3 c = Vector3((pos - startPos).x * 5, (pos - startPos).y, (pos - startPos).z * 5);
		GetPhysicsObject()->AddForce(c);
		//counter = 0;
		Debug::DrawLine(startPos, pos, Vector4(0, 1, 0, 1));
		//std::cout << pos << std::endl;
	//}
}

void StateGameObject::Rotating(float dt, Vector3 startPos, Vector3 destPos) {

	NavigationGrid grid("map.txt");
	NavigationPath outPath;
	foundPath = grid.FindPath(startPos, destPos, outPath);

	GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(20 + counter, Vector3(0, 1, 0))));
	counter++;
}