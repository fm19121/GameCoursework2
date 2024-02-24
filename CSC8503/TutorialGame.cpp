#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"

#include "PushdownState.h"
#include "PushdownMachine.h"
#include "../CSC8503CoreClasses/CollisionDetection.h"

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame() : controller(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()) {
	world		= new GameWorld();
#ifdef USEVULKAN
	renderer	= new GameTechVulkanRenderer(*world);
	renderer->Init();
	renderer->InitStructures();
#else 
	renderer = new GameTechRenderer(*world);
#endif

	physics		= new PhysicsSystem(*world);

	//forceMagnitude	= 10.0f;
	useGravity		= true;
	inSelectionMode = false;
	isFirstPerson = false;
	canFly = false;

	world->GetMainCamera().SetController(controller);

	controller.MapAxis(0, "Sidestep");
	controller.MapAxis(1, "UpDown");
	controller.MapAxis(2, "Forward");

	controller.MapAxis(3, "XLook");
	controller.MapAxis(4, "YLook");

	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	cubeMesh	= renderer->LoadMesh("cube.msh");
	sphereMesh	= renderer->LoadMesh("sphere.msh");
	charMesh	= renderer->LoadMesh("goat.msh");
	enemyMesh	= renderer->LoadMesh("Keeper.msh");
	bonusMesh	= renderer->LoadMesh("apple.msh");
	gooseMesh = renderer->LoadMesh("goose.msh");

	coinMesh = renderer->LoadMesh("coin.msh");

	basicTex	= renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");

	InitCamera();
	InitWorld();
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete charMesh;
	delete enemyMesh;
	delete bonusMesh;
	delete coinMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

void TutorialGame::UpdateGame(float dt, bool isUpdate) {
	if (isUpdate) {
		CoinCollisionDetection(coin1);
		CoinCollisionDetection(coin2);
		CoinCollisionDetection(coin3);
		count += dt;
		coin1->GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(count * 30, Vector3(0, 1, 0))));
		coin2->GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(count * -30, Vector3(0, 1, 0))));
		coin3->GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(count * 40, Vector3(0, 1, 0))));
		if (!inSelectionMode) {
			world->GetMainCamera().UpdateCamera(dt);
		}
		//if (lockedObject != nullptr) {
		//	Vector3 objPos = lockedObject->GetTransform().GetPosition();
		//	Vector3 camPos = objPos + lockedOffset;

		//	Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

		//	Matrix4 modelMat = temp.Inverse();

		//	Quaternion q(modelMat);
		//	Vector3 angles = q.ToEuler(); //nearly there now!

		//	world->GetMainCamera().SetPosition(camPos);
		//	world->GetMainCamera().SetPitch(angles.x + 20);
		//	world->GetMainCamera().SetYaw(angles.y);
		//}
		/*if (Window::GetKeyboard()->KeyPressed(KeyCodes::L)) {
			lockedObject = player;
		}*/
		if (isFirstPerson) {
			Vector3 objPos = player->GetTransform().GetPosition();
			Vector3 camPos = objPos + lockedOffset;

			Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

			Matrix4 modelMat = temp.Inverse();

			Quaternion q(modelMat);
			Vector3 angles = q.ToEuler(); //nearly there now!

			world->GetMainCamera().SetPosition(camPos);
			world->GetMainCamera().SetPitch(angles.x + 20);
			world->GetMainCamera().SetYaw(angles.y);
			ControlPlayer(player);
		}
		else {
			ThirdPersonControl(player);
		}
		UpdateKeys();
		physics->UseGravity(useGravity);
		isSpoted = SpotPlayer();
		if (AiEnemy) {
			AiEnemy->Update(dt, player->GetTransform().GetPosition(), AiEnemy->GetTransform().GetPosition(), isSpoted);
		}

		PrintText();
		/*if (useGravity) {
			Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED);
		}
		else {
			Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);
		}*/
		
		//RayCollision closestCollision;
		///*if (Window::GetKeyboard()->KeyPressed(KeyCodes::K) && selectionObject) {
		//	Vector3 rayPos;
		//	Vector3 rayDir;*/

		//	/*Vector3 rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);
		//	Vector3 rayPos = selectionObject->GetTransform().GetPosition();*/

		/*Vector3 rayDir = AiEnemy->GetTransform().GetOrientation() * Vector3(0, 0, -1);
		Vector3 rayPos = AiEnemy->GetTransform().GetPosition();

			Ray r = Ray(rayPos, rayDir);

			if (world->Raycast(r, closestCollision, true, AiEnemy)) {
				if (objClosest) {
					objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				}
				objClosest = (GameObject*)closestCollision.node;

				objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
			}
		}*/

		//Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));

		SelectObject();
		//MoveSelectedObject();

		world->UpdateWorld(dt);
		renderer->Update(dt);
		physics->Update(dt);
	}

	renderer->Render();
	Debug::UpdateRenderables(dt);
	
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::L)) {
		isFirstPerson = !isFirstPerson;
		if (isFirstPerson == false) {
			InitCamera();
		}
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F1)) {
		Init();
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	//if (Window::GetKeyboard()->KeyPressed(KeyCodes::G)) {
	//	useGravity = !useGravity; //Toggle gravity!
	//	physics->UseGravity(useGravity);
	//}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F8)) {
		world->ShuffleObjects(false);
	}

	/*if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}*/
}

void TutorialGame::PrintText() {
	std::string a = "Your Score: " + std::to_string(score);
	Debug::Print(a, Vector2(3, 10), Debug::BLUE);

	Debug::Print("Press F1 to Restart Game", Vector2(3, 90), Debug::CYAN);
	Debug::Print("Press ESC to call Menu", Vector2(3, 80), Debug::CYAN);
	Debug::Print("Press L to toggle view", Vector2(55, 10), Debug::YELLOW);

	if (score < 2 || !canFly) {
		Debug::Print("Get 2 coins and hit goose to get fly ability", Vector2(2, 95), Debug::MAGENTA);
	}
	
}
//void TutorialGame::LockedObjectMovement() {
//	Matrix4 view		= world->GetMainCamera().BuildViewMatrix();
//	Matrix4 camWorld	= view.Inverse();
//
//	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!
//
//	//forward is more tricky -  camera forward is 'into' the screen...
//	//so we can take a guess, and use the cross of straight up, and
//	//the right axis, to hopefully get a vector that's good enough!
//
//	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
//	fwdAxis.y = 0.0f;
//	fwdAxis.Normalise();
//
//
//	if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
//		selectionObject->GetPhysicsObject()->AddForce(fwdAxis);
//	}
//
//	if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
//		selectionObject->GetPhysicsObject()->AddForce(-fwdAxis);
//	}
//
//	if (Window::GetKeyboard()->KeyDown(KeyCodes::NEXT)) {
//		selectionObject->GetPhysicsObject()->AddForce(Vector3(0,-10,0));
//	}
//}

//void TutorialGame::DebugObjectMovement() {
////If we've selected an object, we can manipulate it with some key presses
//	if (inSelectionMode && selectionObject) {
//		//Twist the selected object!
//		if (Window::GetKeyboard()->KeyDown(KeyCodes::LEFT)) {
//			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
//		}
//
//		if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
//			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
//		}
//
//		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM7)) {
//			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
//		}
//
//		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM8)) {
//			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
//		}
//
//		if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
//			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
//		}
//
//		if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
//			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
//		}
//
//		if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
//			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
//		}
//
//		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM5)) {
//			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
//		}
//	}
//}

void TutorialGame::ControlPlayer(GameObject* player) {
	Vector3 forward = player->GetTransform().GetOrientation() * Vector3(0, 0, -1);
	if (Window::GetKeyboard()->KeyDown(KeyCodes::LEFT)) {
		player->GetPhysicsObject()->AddTorque(Vector3(0, 7, 0));
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
		player->GetPhysicsObject()->AddTorque(Vector3(0, -7, 0));
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
		player->GetPhysicsObject()->AddForce(forward * 20);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
		player->GetPhysicsObject()->AddForce(forward * -20);
	}
	if (score >= 2 && canFly) {
		Debug::Print("You can Press [ + ] to fly now !", Vector2(2, 95), Debug::MAGENTA);
		if (Window::GetKeyboard()->KeyDown(KeyCodes::ADD)) {
			player->GetPhysicsObject()->AddForce(Vector3(0, 200, 0));
		}
	}
	lockedOffset = player->GetTransform().GetOrientation() * Vector3(0, 14, 20);
}

void TutorialGame::ThirdPersonControl(GameObject* player) {
	if (Window::GetKeyboard()->KeyDown(KeyCodes::LEFT)) {
		player->GetPhysicsObject()->AddForce(Vector3(-20, 0, 0));
		player->GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(90, Vector3(0, 1, 0))));
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
		player->GetPhysicsObject()->AddForce(Vector3(20, 0, 0));
		player->GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(-90, Vector3(0, 1, 0))));
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
		player->GetPhysicsObject()->AddForce(Vector3(0, 0, -20));
		player->GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(0, Vector3(0, 1, 0))));
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
		player->GetPhysicsObject()->AddForce(Vector3(0, 0, 20));
		player->GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(180, Vector3(0, 1, 0))));
	}
	if(score >= 2 && canFly){
		Debug::Print("You can Press [ + ] to fly now !", Vector2(2, 95), Debug::MAGENTA);
		if (Window::GetKeyboard()->KeyDown(KeyCodes::ADD)) {
			player->GetPhysicsObject()->AddForce(Vector3(0, 200, 0));
		}
	}
}

void TutorialGame::InitCamera() {
	world->GetMainCamera().SetNearPlane(0.1f);
	world->GetMainCamera().SetFarPlane(500.0f);
	world->GetMainCamera().SetPitch(-67.0f);
	world->GetMainCamera().SetYaw(0.0f);
	world->GetMainCamera().SetPosition(Vector3(100, 200, 200));
	//lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	//InitMixedGridWorld(15, 15, 3.5f, 3.5f);

	goose = AddGooseToWorld(Vector3(180, -15, 160), 0.5);
	InitGameExamples();
	InitDefaultFloor();
	BridgeConstraintTest();
	InitMazeWorld();
	AiEnemy = AddStateEnemyToWorld(Vector3(60, -16, 100));

}

void TutorialGame::Init() {
	score = 0;
	InitCamera();
	InitWorld();
	selectionObject = nullptr;
}
/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position, Vector3 dimensions) {
	GameObject* floor = new GameObject();

	Vector3 floorSize = dimensions;
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}
/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddGooseToWorld(const Vector3& position, float inverseMass) {
	GameObject* goose = new GameObject();
	Vector3 gooseSize = Vector3(2, 2, 2);

	SphereVolume* volume = new SphereVolume(5);

	goose->SetBoundingVolume((CollisionVolume*)volume);

	goose->GetTransform()
		.SetPosition(position)
		.SetScale(gooseSize * 2);

	goose->SetRenderObject(new RenderObject(&goose->GetTransform(), gooseMesh, basicTex, basicShader));
	goose->SetPhysicsObject(new PhysicsObject(&goose->GetTransform(), goose->GetBoundingVolume()));

	goose->GetPhysicsObject()->SetInverseMass(inverseMass);
	goose->GetPhysicsObject()->InitCubeInertia();
	goose->GetRenderObject()->SetColour(Vector4(0.3, 0.8, 0.4, 1));

	world->AddGameObject(goose);

	return goose;
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float inverseMass	= 0.5f;

	GameObject* character = new GameObject();
	Vector3 playerSize = Vector3(2, 2, 2);

	SphereVolume* volume = new SphereVolume(2);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), charMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();
	character->GetRenderObject()->SetColour(Vector4(0.2, 0.4, 0.68, 1));

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize		= 3.0f;
	float inverseMass	= 0.5f;

	GameObject* character = new GameObject();

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

StateGameObject* TutorialGame::AddStateEnemyToWorld(const Vector3& position) {
	float meshSize = 3.0f;
	float inverseMass = 0.5f;
	Vector3 playerPos = player->GetTransform().GetPosition();
	
	StateGameObject* character = new StateGameObject(playerPos, position, isSpoted);

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.32f, 0.3f) * 6);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();
	character->GetRenderObject()->SetColour(Vector4(0.7, 0.3, 0.2, 1));

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(20, 20, 20))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

GameObject* TutorialGame::AddCoinToWorld(const Vector3& position) {
	GameObject* coin = new GameObject();

	Vector3 coinSize = Vector3(0.4, 0.4, 0.2);
	AABBVolume* volume = new AABBVolume((coinSize * 5));
	coin->SetBoundingVolume((CollisionVolume*)volume);
	coin->GetTransform()
		.SetScale(coinSize * 2)
		.SetPosition(position);

	coin->SetRenderObject(new RenderObject(&coin->GetTransform(), coinMesh, nullptr, basicShader));
	coin->SetPhysicsObject(new PhysicsObject(&coin->GetTransform(), coin->GetBoundingVolume()));

	coin->GetPhysicsObject()->SetInverseMass(0.0f);
	coin->GetPhysicsObject()->InitSphereInertia();
	coin->GetRenderObject()->SetColour(Vector4(0.5, 0.4, 0.0, 1));

	world->AddGameObject(coin);

	return coin;
}

GameObject* TutorialGame::AddWallToWorld(const Vector3& position, Vector3 dimensions) {
	GameObject* wall = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	wall->SetBoundingVolume((CollisionVolume*)volume);

	wall->GetTransform()
		.SetScale(dimensions * 2)
		.SetPosition(position);

	wall->SetRenderObject(new RenderObject(&wall->GetTransform(), cubeMesh, basicTex, basicShader));
	wall->SetPhysicsObject(new PhysicsObject(&wall->GetTransform(), wall->GetBoundingVolume()));

	wall->GetPhysicsObject()->SetInverseMass(0);
	wall->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(wall);

	return wall;
}

GameObject* TutorialGame::AddDoorToWorld(const Vector3& position, Vector3 dimensions) {
	GameObject* door = new GameObject();

	OBBVolume* volume = new OBBVolume(dimensions);
	door->SetBoundingVolume((CollisionVolume*)volume);
	door->GetTransform()
		.SetScale(dimensions * 2)
		.SetPosition(position);

	door->SetRenderObject(new RenderObject(&door->GetTransform(), cubeMesh, basicTex, basicShader));
	door->SetPhysicsObject(new PhysicsObject(&door->GetTransform(), door->GetBoundingVolume()));

	door->GetPhysicsObject()->SetInverseMass(0);
	door->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(door);

	return door;
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(100, -20, 100), Vector3(100, 2, 100));
	destination = AddFloorToWorld(Vector3(340, -22, 100), Vector3(40, 2, 40));
	destination->GetRenderObject()->SetColour(Vector4(0.7, 0.0, 0.2, 1));
}

void TutorialGame::InitGameExamples() {
	player = AddPlayerToWorld(Vector3(180, -16, 100));
	AddCubeToWorld(Vector3(150, -16, 28), Vector3(3, 3, 3), 0.6);
	AddSphereToWorld(Vector3(120, -15, 30), 2, 0.5);
	AddSphereToWorld(Vector3(120, -15, 160), 3, 0.5);
	//enemy = AddEnemyToWorld(Vector3(90, 5, 60));
	coin1 = AddCoinToWorld(Vector3(160, -13, 20));
	coin2 = AddCoinToWorld(Vector3(20, -13, 150));
	coin3 = AddCoinToWorld(Vector3(20, -13, 100));
}

//void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
//	for (int x = 0; x < numCols; ++x) {
//		for (int z = 0; z < numRows; ++z) {
//			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
//			AddSphereToWorld(position, radius, 1.0f);
//		}
//	}
//	//AddFloorToWorld(Vector3(0, -2, 0), );
//}

//void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
//	float sphereRadius = 1.0f;
//	Vector3 cubeDims = Vector3(1, 1, 1);
//
//	for (int x = 0; x < numCols; ++x) {
//		for (int z = 0; z < numRows; ++z) {
//			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
//
//			if (rand() % 2) {
//				AddCubeToWorld(position, cubeDims);
//			}
//			else {
//				AddSphereToWorld(position, sphereRadius);
//			}
//		}
//	}
//}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

void TutorialGame::InitMazeWorld() {
	//left
	AddWallToWorld(Vector3(0, -10, 126), Vector3(10, 10, 76));
	AddWallToWorld(Vector3(0, -10, 20), Vector3(10, 10, 16));
	//top
	AddWallToWorld(Vector3(50, -10, 0), Vector3(50, 10, 10));
	AddWallToWorld(Vector3(160, -10, 0), Vector3(40, 10, 10));
	// butoom
	AddWallToWorld(Vector3(100, -10, 200), Vector3(100, 10, 10));
	//right
	AddWallToWorld(Vector3(200, -10, 160), Vector3(10, 10, 40));
	AddWallToWorld(Vector3(200, -10, 40), Vector3(10, 10, 40));

	AddWallToWorld(Vector3(40, -10, 100), Vector3(10, 7, 70));
	AddWallToWorld(Vector3(90, -10, 180), Vector3(10, 7, 20));
	AddWallToWorld(Vector3(140, -10, 100), Vector3(10, 7, 50));
	AddWallToWorld(Vector3(80, -10, 20), Vector3(10, 7, 20));

	AddWallToWorld(Vector3(40, -10, 130), Vector3(40, 7, 10));
	AddWallToWorld(Vector3(150, -10, 70), Vector3(50, 7, 10));
	AddWallToWorld(Vector3(80, -10, 100), Vector3(10, 7, 5));

	AddWallToWorld(Vector3(180, -10, 130), Vector3(20, 7, 10));
	//door
	leftDoor = AddDoorToWorld(Vector3(0, -10, 43), Vector3(2, 20, 10));
	leftDoor->GetRenderObject()->SetColour(Vector4(0.6, 0.3, 0.7, 1.0));
	topDoor = AddDoorToWorld(Vector3(110, -10, 0), Vector3(10, 20, 2));
	topDoor->GetRenderObject()->SetColour(Vector4(0.6, 0.3, 0.7, 1.0));
	destinationDoor = AddWallToWorld(Vector3(200, -10, 100), Vector3(2, 10, 20));
	destinationDoor->GetRenderObject()->SetColour(Vector4(0.3, 0.7, 0.4, 1.0));
}

/*
Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		Debug::Print("Press Q to change to camera mode!", Vector2(2, 85), Debug::CYAN);

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Left)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;

				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
		/*if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}*/
	}
	else {
		Debug::Print("Press Q to change to select mode!", Vector2(2, 85), Debug::CYAN);
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

//void TutorialGame::MoveSelectedObject() {
	//Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
	//forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	//if (!selectionObject) {
	//	return;//we haven't selected anything!
	//}
	////Push the selected object!
	//if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::Right)) {
	//	Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

	//	RayCollision closestCollision;
	//	if (world->Raycast(ray, closestCollision, true)) {
	//		if (closestCollision.node == selectionObject) {
	//			selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
	//		}
	//	}
	//}
//}

void TutorialGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(8, 5, 20);

	float invCubeMass = 2;
	int numLinks = 4;
	float maxDistance = 20;
	float cubeDistance = 15;

	Vector3 startPos = Vector3(200, -30, 100);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; i++) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}
	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);
}

bool TutorialGame::SpotPlayer() {
	RayCollision closestCollision;
	Vector3 rayDir = AiEnemy->GetTransform().GetOrientation() * Vector3(0, 0, -1);
	Vector3 rayPos = AiEnemy->GetTransform().GetPosition();

	Ray r = Ray(rayPos, rayDir);

	if (world->Raycast(r, closestCollision, true, AiEnemy)) {
		/*if (objClosest) {
			objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
		}*/
		objClosest = (GameObject*)closestCollision.node;

		//objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
	}
	return objClosest == player;
}

void TutorialGame::CoinCollisionDetection(GameObject* coin){

	CollisionDetection* colldeDetection;
	CollisionDetection::CollisionInfo collisionInfo;
	bool isCollide;

	isCollide = colldeDetection->ObjectIntersection(player, coin, collisionInfo);
	if (isCollide == true) {
		coin->GetTransform().SetPosition(Vector3(0, -100, 0));
		score++;
		if (score == 2) {
			destinationDoor->GetTransform().SetPosition(Vector3(200, -60, 100));
			topDoor->GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(90, Vector3(1, 0, 0))));
			topDoor->GetTransform().SetPosition(Vector3(110, -20, -20));
			leftDoor->GetTransform().SetOrientation(Quaternion(Matrix4::Rotation(90, Vector3(0, 0, 1))));
			leftDoor->GetTransform().SetPosition(Vector3(-20, -20, 43));
			AddFloorToWorld(Vector3(30, -20, -20), Vector3(70, 2, 20));
			AddFloorToWorld(Vector3(-20, -20, 18), Vector3(20, 2, 18));
		}
	}

	if (colldeDetection->ObjectIntersection(player, goose, collisionInfo) && !canFly) {
		canFly = true;
	}
		
}

bool TutorialGame::WinCondition() {
	CollisionDetection* colldeDetection;
	CollisionDetection::CollisionInfo collisionInfo;
	bool isWin;
	isWin = colldeDetection->ObjectIntersection(player, destination, collisionInfo);
	if (isWin) {
		return true;
	}
	else {
		return false;
	}
}

bool TutorialGame::LoseCondition() {
	CollisionDetection* colldeDetection;
	CollisionDetection::CollisionInfo collisionInfo;
	bool isCollide;
	isCollide = colldeDetection->ObjectIntersection(player, AiEnemy, collisionInfo);
	float playerHeight = player->GetTransform().GetPosition().y;
	if (isCollide || playerHeight <= -80) {
		return true;
	}
	else {
		return false;
	}
}