#include "Window.h"

#include "Debug.h"

#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"

#include "GameServer.h"
#include "GameClient.h"

#include "NavigationGrid.h"
#include "NavigationMesh.h"

#include "TutorialGame.h"
#include "NetworkedGame.h"

#include "PushdownMachine.h"

#include "PushdownState.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"

using namespace NCL;
using namespace CSC8503;

#include <chrono>
#include <thread>
#include <sstream>

int isExit = false;
class WinState : public PushdownState {
	PushdownResult OnUpdate(float dt, PushdownState** newState, TutorialGame* g) override {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
			g->Init();
			return PushdownResult::Reset;
		}
		return PushdownResult::NoChange;
	}
	void OnAwake(float dt, TutorialGame* g) override {
		g->UpdateGame(dt, false);
		int score = g->GetScore();
		Debug::Print("!!!You Win!!!", Vector2(38, 50), Debug::RED);
		Debug::Print(("Your Score:" + std::to_string(score)), Vector2(40, 70), Debug::BLUE);
		Debug::Print("Press ESC to call menu", Vector2(30, 95), Debug::MAGENTA);
	}
};

class LoseState : public PushdownState {
	PushdownResult OnUpdate(float dt, PushdownState** newState, TutorialGame* g) override {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
			g->Init();
			return PushdownResult::Reset;
		}
		return PushdownResult::NoChange;
	}
	void OnAwake(float dt, TutorialGame* g) override {
		g->UpdateGame(dt, false);
		int score = g->GetScore();
		Debug::Print("Noooo!! You Lose", Vector2(35, 50), Debug::RED);
		Debug::Print(("Your Score:" + std::to_string(score)), Vector2(40, 70), Debug::BLUE);
		Debug::Print("Press ESC to call menu", Vector2(30, 95), Debug::MAGENTA);
	}
};

class GameStartState : public PushdownState {
	PushdownResult OnUpdate(float dt, PushdownState** newState, TutorialGame* g) override {
		bool isWin = g->WinCondition();
		bool isFalse = g->LoseCondition();
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
			return PushdownResult::Pop;
		}
		if (isWin) {
			*newState = new WinState();
			return PushdownResult::Push;
		}
		else if(isFalse){
			*newState = new LoseState();
			return PushdownResult::Push;
		}
		return PushdownResult::NoChange;
	}
	void OnAwake(float dt, TutorialGame* g) override {
		g->UpdateGame(dt, true);
		//Debug::DeletePrint();
	}
};

class ExitState : public PushdownState {
	PushdownResult OnUpdate(float dt, PushdownState** newState, TutorialGame* g) override {
		return PushdownResult::NoChange;
	}
	void OnAwake(float dt, TutorialGame* g) override {
		isExit = true;
	}
};

class ExitSelectedState : public PushdownState {
	PushdownResult OnUpdate(float dt, PushdownState** newState, TutorialGame* g) override {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::UP) || Window::GetKeyboard()->KeyPressed(KeyCodes::DOWN)) {
			return PushdownResult::Pop;
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::RETURN)) {
			*newState = new ExitState();
			return PushdownResult::Push;
		}
		return PushdownResult::NoChange;
	}
	void OnAwake(float dt, TutorialGame* g) override {
		g->UpdateGame(dt, false);
		Debug::Print("Press UP or DOWN to switch selection", Vector2(20, 20), Debug::BLUE);
		Debug::Print("Press Enter to confirm selection", Vector2(15, 30), Debug::BLUE);
		Debug::Print("Start Game", Vector2(40, 40), Debug::WHITE);
		Debug::Print("> Exit Game <", Vector2(30, 60), Debug::RED);
	}
};

class MenuState : public PushdownState {
	PushdownResult OnUpdate(float dt, PushdownState** newState, TutorialGame* g) override {
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::UP) || Window::GetKeyboard()->KeyPressed(KeyCodes::DOWN)) {
			*newState = new ExitSelectedState();
			return PushdownResult::Push;
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::RETURN)) {
			*newState = new GameStartState();
			return PushdownResult::Push;
		}
		return PushdownResult::NoChange;
	}
	void OnAwake(float dt, TutorialGame* g) override {
		g->UpdateGame(dt, false);
		Debug::Print("Press UP or DOWN to switch selection", Vector2(20, 20), Debug::BLUE);
		Debug::Print("Press Enter to confirm selection", Vector2(15, 30), Debug::BLUE);
		Debug::Print("> Start Game <", Vector2(30, 40), Debug::RED);
		Debug::Print("Exit Game", Vector2(40, 60), Debug::WHITE);
	}
};

//void TestMenu(Window* w, float dt) {
//	PushdownMachine menu(new MenuState());
//	//PushdownMachine menu(new Staw());
//	while (w->UpdateWindow()) {
//		if (!menu.Update(dt)) {
//			return;
//		}
//	}
//
//	//menu.Update(dt);
//}


/*

The main function should look pretty familar to you!
We make a window, and then go into a while loop that repeatedly
runs our 'game' until we press escape. Instead of making a 'renderer'
and updating it, we instead make a whole game, and repeatedly update that,
instead. 

This time, we've added some extra functionality to the window class - we can
hide or show the 

*/
int main() {
	Window*w = Window::CreateGameWindow("CSC8503 Game technology!", 1280, 720);

	if (!w->HasInitialised()) {
		return -1;
	}	
	//TestWindo(w);
	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);

	PushdownMachine menu(new MenuState());
	TutorialGame* g = new TutorialGame();
	w->GetTimer().GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
	while (w->UpdateWindow() && !isExit) {
		float dt = w->GetTimer().GetTimeDeltaSeconds();
		if (dt > 0.1f) {
			std::cout << "Skipping large time delta" << std::endl;
			continue; //must have hit a breakpoint or something to have a 1 second frame time!
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::NEXT)) {
			w->ShowConsole(false);
		}

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::T)) {
			w->SetWindowPosition(0, 0);
		}
		w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));
		//g->UpdateGame(dt, true);
		if(!menu.Update(dt, g)) {
			return 0;
		}
	}
	Window::DestroyGameWindow();
}