#include "DefaultSceneLayer.h"

// GLM math library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/gtx/common.hpp> // for fmod (floating modulus)

#include <filesystem>
#include "Application/Timing.h"

// Graphics
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/VertexArrayObject.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Textures/TextureCube.h"
#include "Graphics/VertexTypes.h"
#include "Graphics/Font.h"
#include "Graphics/GuiBatcher.h"
#include "Graphics/Framebuffer.h"

// Utilities
#include "Utils/MeshBuilder.h"
#include "Utils/MeshFactory.h"
#include "Utils/ObjLoader.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/FileHelpers.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/StringUtils.h"
#include "Utils/GlmDefines.h"
#include "ToneFire.h"

// Gameplay
#include "Gameplay/Material.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"

// Components
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Components/Camera.h"
#include "Gameplay/Components/RotatingBehaviour.h"
#include "Gameplay/Components/JumpBehaviour.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Gameplay/Components/MaterialSwapBehaviour.h"
#include "Gameplay/Components/TriggerVolumeEnterBehaviour.h"
#include "Gameplay/Components/SimpleCameraControl.h"
#include "Gameplay/Components/Light.h"

//Rubbish Rush Components
#include "Gameplay/Components/DeleteObjectBehaviour.h"
#include "Gameplay/Components/CollectTrashBehaviour.h"
#include "Gameplay/Components/SubmittingTrashBehaviour.h"
#include "Gameplay/Components/PlayerMovementBehavior.h"
#include "Gameplay/Components/ConveyorBeltBehaviour.h"
#include "Gameplay/Components/SpillBehaviour.h"
#include "Gameplay/Components/SteeringBehaviour.h"
#include "Gameplay/Components/FollowBehaviour.h"
#include "Gameplay/Components/MorphAnimator.h"
#include "Gameplay/Components/MorphMeshRenderer.h"
#include "Gameplay/Components/GroundBehaviour.h"
#include "Gameplay/Components/InventoryUI.h"
#include "Gameplay/Components/AudioEngine.h"
#include "Gameplay/Components/ShadowCamera.h"

// Physics
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Physics/Colliders/BoxCollider.h"
#include "Gameplay/Physics/Colliders/PlaneCollider.h"
#include "Gameplay/Physics/Colliders/SphereCollider.h"
#include "Gameplay/Physics/Colliders/ConvexMeshCollider.h"
#include "Gameplay/Physics/Colliders/CylinderCollider.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Graphics/DebugDraw.h"

// GUI
#include "Gameplay/Components/GUI/RectTransform.h"
#include "Gameplay/Components/GUI/GuiPanel.h"
#include "Gameplay/Components/GUI/GuiText.h"
#include "Gameplay/InputEngine.h"

#include "Application/Application.h"
#include "Gameplay/Components/ParticleSystem.h"
#include "Gameplay/InputEngine.h"
#include "Application/Layers/RenderLayer.h"

DefaultSceneLayer::DefaultSceneLayer() :
	ApplicationLayer()
{
	Name = "Default Scene";
	Overrides = AppLayerFunctions::OnAppLoad | AppLayerFunctions::OnUpdate;
	
}

DefaultSceneLayer::~DefaultSceneLayer() = default;

void DefaultSceneLayer::OnAppLoad(const nlohmann::json& config) {
	_CreateScene();
	

}
std::string TimeCountdown(float DisplayTime) { //Timer Function
	float minutes = floorf(DisplayTime / 60);
	float seconds = floorf(fmodf(DisplayTime, 60));
	//Use string and stringstream so it can work with the GUI
	std::string minString;

	std::stringstream testm; //Minutes
	testm << minutes << ":";
	std::stringstream tests;
	tests << seconds;
	std::stringstream tests10;
	tests10 << "0" << seconds; // Seconds

	//std::cout << testm.str(); //print minutes

	//if secolnds is lower than 10, uses tests10 so 9 - 0 has a 0 to their left else it will print out 59-10 normally
	if (seconds < 10) {
		minString = tests10.str();
	}
	else {
		minString = tests.str();
	}

	return (testm.str() + minString); //Adds the minute and seconds strings together

	//std::cout << timeValue;

}
// Grab current time as the previous frame
double lastFrame = glfwGetTime();
void DefaultSceneLayer::OnUpdate()
{
	Application& app = Application::Get();
	_currentScene = app.CurrentScene();
	// Figure out the current time, and the time since the last frame
	double thisFrame = glfwGetTime();
	float dt = static_cast<float>(thisFrame - lastFrame);
	
	//mallStudio.Update();

	//fetch resources
	if (!activated)
	{		
		
		
		trashyM = _currentScene->FindObjectByName("Trashy");
		binM = _currentScene->FindObjectByName("Bin");
		RectangleE = _currentScene->FindObjectByName("Rec");
		TrashyE = _currentScene->FindObjectByName("TrashyE");
		//limit rotation
		trashyM->Get<Gameplay::Physics::RigidBody>()->SetAngularFactor(glm::vec3(0.0f, 0.0f, 0.0f));
		trashyM->Get<Gameplay::Physics::RigidBody>()->SetLinearDamping(0.9f);

		//UI
		startMenu = _currentScene->FindObjectByName("Start");
		pauseMenu = _currentScene->FindObjectByName("Pause");
		failMenu = _currentScene->FindObjectByName("Fail");
		winMenu = _currentScene->FindObjectByName("Win");
		UIText = _currentScene->FindObjectByName("Time Text");
		trashRemainder = _currentScene->FindObjectByName("Trash Remaining");
		objective = _currentScene->FindObjectByName("Objective UI Canvas");
		returnUI = _currentScene->FindObjectByName("Return Feedback");
		submitUI = _currentScene->FindObjectByName("Submit Feedback");

		conveyor_belt = _currentScene->FindObjectByName("Conveyor")->Get<RenderComponent>()->GetMaterial();
		//randomize
		//RandomizePositions();

		//only run this once
		activated = true;
	}
	
	if (!start)
	{

		if (InputEngine::GetKeyState(GLFW_KEY_ENTER) == ButtonState::Pressed)
		{
			spressed = true;
			_currentScene->IsPlaying = true;
			
		}
		if (glfwGetKey(app.GetWindow(), GLFW_KEY_ESCAPE)) //exit
		{
			exit(0);

		}
		//_currentScene->IsPlaying = false;
		playMenu = false;
		//FREEZE TRASHY
		trashyM->Get<Gameplay::Physics::RigidBody>()->IsEnabled = false;
		submitUI->Get<GuiText>()->IsEnabled = false;
		objective->Get<GuiPanel>()->IsEnabled = false;

		if (spressed)
		{
			//allows the game to start when enter is pressed, trashy becomes unfrozen so the player can control them
			start = true;
			startMenu->Get<GuiPanel>()->IsEnabled = false;
			trashyM->Get<Gameplay::Physics::RigidBody>()->IsEnabled = true;
			//startMenu->GetScene()->DeleteGameObject
			playMenu = true;
			spressed = false;
			AudioEngine::playEventS("event:/Music Fast");

			_currentScene->FindObjectByName("Inventory UI")->Get<GuiPanel>()->IsEnabled = true;
			//CREATE THE TRASH AHHHH
			_CreateTrash();

		}

	}

	// Core update loop
	if (_currentScene != nullptr) {
		//update conveyor belt
		tracker += 0.01;
		conveyor_belt->Set("Time", tracker);
		//custom function
		//CheckTrash();	

		if (_currentScene->playrecyclesound) {
			AudioEngine::playEventS("event:/Plastic trash crush");
			_currentScene->playrecyclesound = false;
		}

		if (_currentScene->playtrashsound) {
			//	test.PlayEvent("event:/Can Crush");
			AudioEngine::playEventS("event:/Can Crush");
			_currentScene->playtrashsound = false;
		}

		if (_currentScene->playmulti) {
			//test.SetEventParameter("event:/Trash multi", "parameter:/Pitch", glm::linearRand(0.f, 1.f));
			AudioEngine::playEventS("event:/Trash multi");
			_currentScene->playmulti = false;
		}

		if (Timing::Current().TimeSinceAppLoad() - currentTime >= 0.3f && _currentScene->walk) {
			AudioEngine::playEventS("event:/Footsteps");
			currentTime = Timing::Current().TimeSinceAppLoad();
		}

		if (Timing::Current().TimeSinceAppLoad() - currentTime >= 0.5f && _currentScene->walk == false || trashyM->Get<JumpBehaviour>()->in_air) {
			AudioEngine::stopEventS("event:/Footsteps");
		}


		//MENU ANIMATED UPDATED
		if (_currentScene->IsPlaying && !timerDone && playMenu && start)
		{
			currentTime = Timing::Current().TimeSinceAppLoad();
			//When the game starts a menu plays, this just ensure the menu plays in that amount of time
			if (timeLoop > 0) {
				timerDone = false;
				timeLoop -= dt;
			}
			else {
				timerDone = true;
			}

			if (RectangleE->GetPosition().z > 3.3f)
			{
				RectangleE->UpdateLerp(pointsPos, dt);
			}
			else
			{
				if (TrashyE->GetPosition().x >= -1.22f)
				{
					TrashyE->UpdateCAT(pointsPos2, dt);
				}
				else
				{
					TrashyE->UpdateScale(pointsS, pointsR, dt);
				}
			}
			//FREEZE TRASHY
			if (trashyM->GetPosition().z < 1.0f)
			{
				trashyM->Get<Gameplay::Physics::RigidBody>()->SetLinearVelocity(glm::vec3(0.0f));
			}
			//trashyM->Get<RigidBody>()->IsEnabled = false;
			//trashyM->Get<RigidBody>()->SetMass(0.0f);
		}
		else if (_currentScene->IsPlaying && timerDone && playMenu && start)
		{
			if (TrashyE->GetPosition().x < 1.5f)
			{
				TrashyE->UpdateCAT(pointsPos2, dt);
				TrashyE->UpdateScale(pointsS2, pointsR2, dt);
			}
			else
			{
				if (RectangleE->GetPosition().z < 7.0f)
				{
					RectangleE->UpdateLerp(pointsPos, dt);
				}
			}
			//FREEZE TRASHY
			if (trashyM->GetPosition().z < 1.0f)
			{
				trashyM->Get<Gameplay::Physics::RigidBody>()->SetLinearVelocity(glm::vec3(0.0f));
			}
			if (TrashyE->GetPosition().x >= 0.4f && RectangleE->GetPosition().z >= 6.9f)
			{
				TrashyE->SetPostionZ(-10.0f);
				RectangleE->SetPostionZ(-10.0f);
				//TrashyE->SetDirty(true);
				//RectangleE->SetDirty(true);
				//TrashyE->GetScene()->DeleteGameObject(TrashyE->GetScene()->FindObjectByGUID(TrashyE->GUID));
				//RectangleE->GetScene()->DeleteGameObject(TrashyE->GetScene()->FindObjectByGUID(RectangleE->GUID));
				TrashyE->Get<RenderComponent>()->IsEnabled = false;
				RectangleE->Get<RenderComponent>()->IsEnabled = false;
				//_currentScene->RemoveGameObject(TrashyE); //DONT DELETE WE NEED TO REUSE
				//_currentScene->RemoveGameObject(RectangleE); //DONT DELETE WE NEED TO REUSE
				std::cout << "should be deleted\n";
				playMenu = false;
				//trashyM->Get<RigidBody>()->IsEnabled = true;
				//trashyM->SetDirty(true);

				//enable timer stuff
				UIText->Get<GuiText>()->IsEnabled = true;
				trashRemainder->Get<GuiText>()->IsEnabled = true;
				objective->Get<GuiPanel>()->IsEnabled = true;
			}
		}
		
		else if (_currentScene->IsPlaying && !playMenu && !timeleveltDone && start)
		{
			
			
			//put gui here
			//if all trash is collected, the timer stops, and victory is set to true to show of the victory screen
			if (_currentScene->score == max_trash)
			{
				//trashRemainder->Get<GuiText>()->SetText(3 - _currentScene->score);
				timeleveltDone = true;
				Victory = true;
			}
			else
			{
				//just make sure the condition of the timer of the menu is completed and to ensure no complications happen
				if (_currentScene->held == 0) {
					timerDone = true;
				}
				else {

				}
			}
			//If the player does not pause the game, the timer will keep reducing till 0
			if (!isPaused)
			{
				//Will reduce as long as it is greater than 0 and is not completed, also shows off how much trash is remaining
				if (timelevelt > 0 && !timeleveltDone) {
					timelevelt -= dt;

					UIText->Get<GuiText>()->SetText(TimeCountdown(timelevelt));

					trashRemainder->Get<GuiText>()->SetText(std::to_string(max_trash - _currentScene->score - _currentScene->held) + " Trash Remaining!");

					//GUI disappears when all trash is collected
					if ((max_trash - _currentScene->score - _currentScene->held == 0) || _currentScene->held >= inventory)
					{
						returnUI->Get<GuiText>()->IsEnabled = true;
					}
				}
				//checks if timer reaches 0 if it does the lose screen will get activated
				else if (timelevelt <= 0)
				{
					timeleveltDone = true;
					lose = true;

				}
			}
			else
			{

			}

		}
		else if (timeleveltDone && _currentScene->IsPlaying && start)
		{
			if (lose)
			{


				//end menu
				trashyM->Get<Gameplay::Physics::RigidBody>()->IsEnabled = false;
				failMenu->Get<GuiPanel>()->IsEnabled = true;
				UIText->Get<GuiText>()->IsEnabled = false;
				trashRemainder->Get<GuiText>()->IsEnabled = false;
				objective->Get<GuiPanel>()->IsEnabled = false;
				returnUI->Get<GuiText>()->IsEnabled = false;
				//need this so program doesnt detect multiple presses
				press_once = false;
				//pause the timer*****

				if (InputEngine::GetKeyState(GLFW_KEY_ENTER) == ButtonState::Pressed && !press_once) //return to game
				{
					//need this so program doesnt detect multiple presses
					press_once = true;
					//trashyM->Get<RigidBody>()->IsEnabled = true; 
					failMenu->Get<GuiPanel>()->IsEnabled = false; //dont show lose menu
					startMenu->Get<GuiPanel>()->IsEnabled = true;

					//reset variables
					lose = false;
					start = false;
					playMenu = false;
					timeLoop = 7.0f;
					timelevelt = 300.f;
					timerDone = false;
					timeleveltDone = false;
					trashyM->SetPostion(glm::vec3(-1.5f, 0.0f, 1.0f)); //reset position to start
					_currentScene->score = 0;
					_currentScene->trash = 0;
					_currentScene->held = 0;
					_currentScene->held_normal = 0;
					_currentScene->held_recycle = 0;
					//create trash objects again
					//delete any remaining trash objects
					for (int i = 0; i < all_trash.size(); i++)
					{
						if (all_trash[i] != nullptr)
						{
							_currentScene->RemoveGameObject(all_trash[i]);
						}
					}
					all_trash.clear();
					//_CreateTrash();
					//randomize again
					//RandomizePositions();
					//put menus back in
					TrashyE->Get<RenderComponent>()->IsEnabled = true;
					RectangleE->Get<RenderComponent>()->IsEnabled = true;
					TrashyE->SetPostion(glm::vec3(0.5f, 1.49f, 3.3f));
					RectangleE->SetPostion(glm::vec3(-1.36f, 1.22f, 7.0f));
					AudioEngine::stopEventS("event:/Music Fast");

				}
				if (glfwGetKey(app.GetWindow(), GLFW_KEY_ESCAPE)) //exit
				{
					exit(0);

				}
			}
			if (Victory)
			{
				//winMenu->SetPostion(trashyM->GetPosition() + glm::vec3(0.07f, 0.14f, 1.81f)); //offset from player
				trashyM->Get<Gameplay::Physics::RigidBody>()->IsEnabled = false;
				winMenu->Get<GuiPanel>()->IsEnabled = true;
				UIText->Get<GuiText>()->IsEnabled = false;
				trashRemainder->Get<GuiText>()->IsEnabled = false;
				objective->Get<GuiPanel>()->IsEnabled = false;
				returnUI->Get<GuiText>()->IsEnabled = false;
				//need this so program doesnt detect multiple presses
				press_once = false;
				//pause the timer*****

				if (InputEngine::GetKeyState(GLFW_KEY_ENTER) == ButtonState::Pressed && !press_once) //return to game
				{
					//need this so program doesnt detect multiple presses
					press_once = true;
					//trashyM->Get<RigidBody>()->IsEnabled = true; 
					winMenu->Get<GuiPanel>()->IsEnabled = false; //dont show win menu
					startMenu->Get<GuiPanel>()->IsEnabled = true;
					//reset variables
					Victory = false;
					start = false;
					playMenu = false;
					timeLoop = 7.0f;
					timelevelt = 300.f;
					timerDone = false;
					timeleveltDone = false;
					trashyM->SetPostion(glm::vec3(-1.5f, 0.0f, 1.0f)); //reset position to start
					_currentScene->score = 0;
					_currentScene->trash = 0;
					_currentScene->held = 0;
					_currentScene->held_normal = 0;
					_currentScene->held_recycle = 0;
					//create trash objects again
					all_trash.clear();
					//_CreateTrash();
					//randomize again
					//RandomizePositions();
					//put menus back in
					TrashyE->Get<RenderComponent>()->IsEnabled = true;
					RectangleE->Get<RenderComponent>()->IsEnabled = true;
					TrashyE->SetPostion(glm::vec3(0.5f, 1.49f, 3.3f));
					RectangleE->SetPostion(glm::vec3(-1.36f, 1.22f, 7.0f));
					AudioEngine::stopEventS("event:/Music Fast");

				}
				if (glfwGetKey(app.GetWindow(), GLFW_KEY_ESCAPE)) //exit
				{
					exit(0);

				}



			}
		}
		//PAUSE MENU
		if (start && !playMenu)
		{
			if (glfwGetKey(app.GetWindow(), GLFW_KEY_ESCAPE) && !isPaused)
			{
				isPaused = true;
				//pauseMenu->SetPostion(trashyM->GetPosition() + glm::vec3(0.07f, 0.14f, 1.81f)); //offset from player
				UIText->Get<GuiText>()->IsEnabled = false;
				trashRemainder->Get<GuiText>()->IsEnabled = false;
				objective->Get<GuiPanel>()->IsEnabled = false;
				trashyM->Get<Gameplay::Physics::RigidBody>()->IsEnabled = false;
				pauseMenu->Get<GuiPanel>()->IsEnabled = true;

			}
			if (isPaused)
			{

				//pause the timer*****
				if (glfwGetKey(app.GetWindow(), GLFW_KEY_ENTER)) //return to game
				{
					trashyM->Get<Gameplay::Physics::RigidBody>()->IsEnabled = true;
					pauseMenu->Get<GuiPanel>()->IsEnabled = false;
					isPaused = false;
					UIText->Get<GuiText>()->IsEnabled = true;
					trashRemainder->Get<GuiText>()->IsEnabled = true;
					objective->Get<GuiPanel>()->IsEnabled = true;
				}
			}
		}


		//PROBABLY FIND A BETTER WAY TO DO THIS
	// Grab shorthands to the camera and shader from the _currentScene
		Gameplay::Camera::Sptr camera = _currentScene->MainCamera;

		//EDIT THIS TO ALLOW CAMERA CONTROL
		//if (!camera->GetComponent<SimpleCameraControl>()->moving)
		{
			camera->GetGameObject()->SetPostion(trashyM->GetPosition() + glm::vec3(0.0f, 2.50f, 6.f));
			camera->GetGameObject()->LookAt(trashyM->GetPosition() + glm::vec3(0.0f, -3.9f, -2.0f));
		}
	}
	// Store timing for next loop
	lastFrame = thisFrame;

}



void DefaultSceneLayer::_CreateScene()
{
	// SFX MUSIC AND AUDIO//

	/*mallStudio.LoadBank("Master.bank");
	mallStudio.LoadBank("Master.strings.bank");
	mallStudio.LoadBank("Sound.bank");
	mallStudio.LoadBank("Music_Background.bank");
	audio.LoadEvent("event:/Music Fast");
	audio.LoadEvent("event:/Footsteps");
	audio.LoadEvent("event:/Can Crush");
	audio.LoadEvent("event:/Plastic trash crush");
	audio.LoadEvent("event:/Trash multi");
	audio.SetEventPosition("event:/Can Crush", FMOD_VECTOR{ 0.0f,0.0f,7.f });*/



	pointsPos.push_back(glm::vec3(-1.36f, 1.22f, 7.1f));
	pointsPos.push_back(glm::vec3(-1.36f, 1.22f, 3.27f));

	pointsPos2.push_back(glm::vec3(1.5f, 1.49f, 3.29f));
	pointsPos2.push_back(glm::vec3(0.5f, 1.49f, 3.29f));
	pointsPos2.push_back(glm::vec3(-0.5f, 1.49f, 3.29f));
	pointsPos2.push_back(glm::vec3(-1.0f, 1.49f, 3.29f));
	pointsPos2.push_back(glm::vec3(-1.18f, 1.49f, 3.29f));

	pointsS.push_back(glm::vec3(1.0f, 0.8f, 0.8f));
	pointsS.push_back(glm::vec3(1.0f, 1.8f, 1.4f));

	pointsR.push_back(glm::vec3(62.0f, 0.0f, 90.0f));
	pointsR.push_back(glm::vec3(100.0f, 0.0f, 90.0f));

	pointsS2.push_back(glm::vec3(1.0f, 1.46f, 1.0f));
	pointsS2.push_back(glm::vec3(1.0f, 1.0f, 0.4f));

	pointsR2.push_back(glm::vec3(90.0f, 0.0f, 90.0f));
	pointsR2.push_back(glm::vec3(90.0f, 0.0f, 90.0f));
	//using namespace Gameplay;
	//using namespace Gameplay::Physics;

	Application& app = Application::Get();
	//all conditions to change between in-game conditions and menus
	bool loadScene = false;


	//Initialization of Animations
	std::vector <Gameplay::MeshResource::Sptr> walking;
	std::vector <Gameplay::MeshResource::Sptr> idle;
	std::vector <Gameplay::MeshResource::Sptr> jumping;
	std::vector <Gameplay::MeshResource::Sptr> interaction;


	// For now we can use a toggle to generate our scene vs load from file
	if (loadScene && std::filesystem::exists("scene.json")) {
		app.LoadScene("scene.json");
	}
	else {
		
		// This shader handles our basic materials without reflections (cause they expensive)
		ShaderProgram::Sptr rackShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forwardRACK.glsl" }
		});

		// ANIMATION SHADER??
		ShaderProgram::Sptr animShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/morph.vert" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});

		
		// This shader handles our foliage vertex shader example
		ShaderProgram::Sptr foliageShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/foliage.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});

		//for moving the conveyor belt?
		ShaderProgram::Sptr conveyorShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic_moving.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forwardCONVEYOR.glsl" }
		});

		///////////////////// NEW SHADERS ////////////////////////////////////////////

			// This shader handles our displacement mapping example
		ShaderProgram::Sptr displacementShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/displacement_mapping.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});
		displacementShader->SetDebugName("Displacement Mapping");

		// This shader handles our multitexturing example
		ShaderProgram::Sptr multiTextureShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/vert_multitextured.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_multitextured.glsl" }
		});
		// This shader handles our cel shading example
		ShaderProgram::Sptr celShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/displacement_mapping.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/cel_shader.glsl" }
		});
		celShader->SetDebugName("Cel Shader");

		// Basic gbuffer generation with no vertex manipulation
		ShaderProgram::Sptr deferredForward = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});
		deferredForward->SetDebugName("Deferred - GBuffer Generation");

#pragma region Basic Texture Creation
		Texture2DDescription singlePixelDescriptor;
		singlePixelDescriptor.Width = singlePixelDescriptor.Height = 1;
		singlePixelDescriptor.Format = InternalFormat::RGB8;

		float normalMapDefaultData[3] = { 0.5f, 0.5f, 1.0f };
		Texture2D::Sptr normalMapDefault = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		normalMapDefault->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, normalMapDefaultData);

		float solidBlack[3] = { 0.5f, 0.5f, 0.5f };
		Texture2D::Sptr solidBlackTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		solidBlackTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidBlack);

		float solidGrey[3] = { 0.0f, 0.0f, 0.0f };
		Texture2D::Sptr solidGreyTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		solidGreyTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidGrey);

		float solidWhite[3] = { 1.0f, 1.0f, 1.0f };
		Texture2D::Sptr solidWhiteTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		solidWhiteTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidWhite);

#pragma endregion 

		//LOAD OBJECTS

		//CUP
		Gameplay::MeshResource::Sptr trashMesh2 = ResourceManager::CreateAsset<Gameplay::MeshResource>("cup.obj");
		Texture2D::Sptr trashTex = ResourceManager::CreateAsset<Texture2D>("textures/cup.jpg");
		Gameplay::Material::Sptr trashMaterial2 = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			trashMaterial2->Name = "Trash";
			trashMaterial2->Set("u_Material.AlbedoMap", trashTex);
			trashMaterial2->Set("u_Material.Shininess", 0.3f);
			trashMaterial2->Set("u_Material.NormalMap", normalMapDefault);

		}
		trashMesh = trashMesh2;
		trashMaterial = trashMaterial2;
		// TRASH BAG
		Gameplay::MeshResource::Sptr bagtrashMesh2 = ResourceManager::CreateAsset<Gameplay::MeshResource>("Trashbag.obj");
		Texture2D::Sptr bagtrashTex = ResourceManager::CreateAsset<Texture2D>("textures/TrashBagTex.jpg");

		Gameplay::Material::Sptr bagtrashMaterial2 = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			bagtrashMaterial2->Name = "Bag Trash";
			bagtrashMaterial2->Set("u_Material.AlbedoMap", bagtrashTex);
			bagtrashMaterial2->Set("u_Material.Shininess", 0.3f);
			bagtrashMaterial2->Set("u_Material.NormalMap", normalMapDefault);
		}
		bagtrashMesh = bagtrashMesh2;
		bagtrashMaterial = bagtrashMaterial2;
		//SPILL
		Gameplay::MeshResource::Sptr spillMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("spill.obj");
		Texture2D::Sptr spillTex = ResourceManager::CreateAsset<Texture2D>("textures/goo.png");
		// Create our material
		Gameplay::Material::Sptr spillMaterial = ResourceManager::CreateAsset<Gameplay::Material>(rackShader);
		{
			spillMaterial->Name = "Spill";
			spillMaterial->Set("u_Material.AlbedoMap", spillTex);
			spillMaterial->Set("u_Material.Shininess", 1.0f);
			spillMaterial->Set("u_Material.NormalMap", normalMapDefault);

		}
		//RECYCLE BIN
		Gameplay::MeshResource::Sptr bin2Mesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("recycle bin.obj");
		Texture2D::Sptr bin2Tex = ResourceManager::CreateAsset<Texture2D>("textures/recycle.jpg");
		// Create our material
		Gameplay::Material::Sptr bin2Material = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			bin2Material->Name = "Bin";
			bin2Material->Set("u_Material.AlbedoMap", bin2Tex);
			bin2Material->Set("u_Material.Shininess", 0.5f);
			bin2Material->Set("u_Material.NormalMap", normalMapDefault);

		}
		// CONVEYOR
		Gameplay::MeshResource::Sptr conveyorMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("conveyor.obj");
		Texture2D::Sptr conveyorTex = ResourceManager::CreateAsset<Texture2D>("textures/conveyor.jpg");
		//repeat conveyor belt texture
		conveyorTex->SetWrap(WrapMode::Repeat);
		Gameplay::Material::Sptr conveyorMaterial = ResourceManager::CreateAsset<Gameplay::Material>(conveyorShader);
		{
			conveyorMaterial->Name = "Conveyor";
			conveyorMaterial->Set("u_Material.AlbedoMap", conveyorTex);
			conveyorMaterial->Set("u_Material.Shininess", 0.2f);
			conveyorMaterial->Set("u_Material.NormalMap", normalMapDefault);
			conveyorMaterial->Set("Time", 0.0f);

		}

		Gameplay::MeshResource::Sptr shelfMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("shelf.obj");
		Texture2D::Sptr shelfTex = ResourceManager::CreateAsset <Texture2D>("textures/shelf.jpg");
		//Create Material
		Gameplay::Material::Sptr shelfMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			shelfMaterial->Name = "Shelf";
			shelfMaterial->Set("u_Material.AlbedoMap", shelfTex);
			shelfMaterial->Set("u_Material.Shininess", 0.2f);
			shelfMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Gameplay::MeshResource::Sptr tvboxMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("tvbox2.obj");
		Texture2D::Sptr tvboxTex = ResourceManager::CreateAsset<Texture2D>("textures/tvbox.png");
		//Create Material
		Gameplay::Material::Sptr tvboxMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			tvboxMaterial->Name = "TvBox";
			tvboxMaterial->Set("u_Material.AlbedoMap", tvboxTex);
			tvboxMaterial->Set("u_Material.Shininess", 0.0f);
			tvboxMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Gameplay::MeshResource::Sptr cashMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("cashcounter.obj");
		Texture2D::Sptr cashTex = ResourceManager::CreateAsset<Texture2D>("textures/cash.png");
		//create Material
		Gameplay::Material::Sptr cashMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			cashMaterial->Name = "Cash";
			cashMaterial->Set("u_Material.AlbedoMap", cashTex);
			cashMaterial->Set("u_Material.Shininess", 0.75f);
			cashMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Gameplay::MeshResource::Sptr benchMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("bench.obj");
		Texture2D::Sptr benchTex = ResourceManager::CreateAsset<Texture2D>("textures/bench.jpg");
		Gameplay::Material::Sptr benchMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			benchMaterial->Name = "Bench";
			benchMaterial->Set("u_Material.AlbedoMap", benchTex);
			benchMaterial->Set("u_Material.Shininess", 0.0f);
			benchMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Gameplay::MeshResource::Sptr computerMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("Computer.obj");
		Texture2D::Sptr computerTex = ResourceManager::CreateAsset<Texture2D>("textures/desktoptex.jpg");
		Gameplay::Material::Sptr computerMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			computerMaterial->Name = "Computer";
			computerMaterial->Set("u_Material.AlbedoMap", computerTex);
			computerMaterial->Set("u_Material.Shininess", 0.3f);
			computerMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Gameplay::MeshResource::Sptr boothMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("booth.obj");
		Texture2D::Sptr boothTex = ResourceManager::CreateAsset<Texture2D>("textures/BOOTH.jpg");
		Gameplay::Material::Sptr boothMat = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			boothMat->Name = "Booth";
			boothMat->Set("u_Material.AlbedoMap", boothTex);
			boothMat->Set("u_Material.Shininess", 0.3f);
			boothMat->Set("u_Material.NormalMap", normalMapDefault);
		}
		
		Gameplay::MeshResource::Sptr sqrtableMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("table.obj");
		Texture2D::Sptr sqrtableTex = ResourceManager::CreateAsset<Texture2D>("textures/lib table.jpg");
		Gameplay::Material::Sptr sqrtableMat = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			sqrtableMat->Name = "Square Table";
			sqrtableMat->Set("u_Material.AlbedoMap", sqrtableTex);
			sqrtableMat->Set("u_Material.Shininess", 0.3f);
			sqrtableMat->Set("u_Material.NormalMap", normalMapDefault);
		}

		Gameplay::MeshResource::Sptr fridgeMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("fridge.obj");
		Texture2D::Sptr fridgeTex = ResourceManager::CreateAsset<Texture2D>("textures/fridge.jpg");
		Gameplay::Material::Sptr fridgeMat = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			fridgeMat->Name = "Fridge";
			fridgeMat->Set("u_Material.AlbedoMap", fridgeTex);
			fridgeMat->Set("u_Material.Shininess", 0.3f);
			fridgeMat->Set("u_Material.NormalMap", normalMapDefault);
		}

		Gameplay::MeshResource::Sptr stoveMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("stove.obj");
		Texture2D::Sptr stoveTex = ResourceManager::CreateAsset<Texture2D>("textures/stove.jpg");
		Gameplay::Material::Sptr stoveMat = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			stoveMat->Name = "Stove";
			stoveMat->Set("u_Material.AlbedoMap", stoveTex);
			stoveMat->Set("u_Material.Shininess", 0.3f);
			stoveMat->Set("u_Material.NormalMap", normalMapDefault);
		}
		
		Gameplay::MeshResource::Sptr plantMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("plant.obj");
		Texture2D::Sptr plantTex = ResourceManager::CreateAsset<Texture2D>("textures/planttex.png");
		Gameplay::Material::Sptr plantMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			plantMaterial->Name = "Plant";
			plantMaterial->Set("u_Material.AlbedoMap", plantTex);
			plantMaterial->Set("u_Material.Shininess", 0.0f);
			plantMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Gameplay::MeshResource::Sptr dinerchairMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("diner chair.obj");
		Texture2D::Sptr dinerchairTex = ResourceManager::CreateAsset<Texture2D>("textures/dine chair.jpg");
		Gameplay::Material::Sptr dinerchairMat = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			dinerchairMat->Name = "Diner Chair";
			dinerchairMat->Set("u_Material.AlbedoMap", dinerchairTex);
			dinerchairMat->Set("u_Material.Shininess", 0.3f);
			dinerchairMat->Set("u_Material.NormalMap", normalMapDefault);
		}
		
		Gameplay::MeshResource::Sptr dinertableMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("diner table.obj");
		Texture2D::Sptr dinertableTex = ResourceManager::CreateAsset<Texture2D>("textures/lib table.jpg");
		Gameplay::Material::Sptr dinertableMat = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			dinertableMat->Name = "Diner Table";
			dinertableMat->Set("u_Material.AlbedoMap", dinertableTex);
			dinertableMat->Set("u_Material.Shininess", 0.3f);
			dinertableMat->Set("u_Material.NormalMap", normalMapDefault);
		}

		Gameplay::MeshResource::Sptr showerMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("shower.obj");
		Texture2D::Sptr showerTex = ResourceManager::CreateAsset<Texture2D>("textures/shower.jpg");
		Gameplay::Material::Sptr showerMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			showerMaterial->Name = "Shower";
			showerMaterial->Set("u_Material.AlbedoMap", showerTex);
			showerMaterial->Set("u_Material.Shininess", 0.5f);
			showerMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Gameplay::MeshResource::Sptr libshelfMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("library shelf.obj");
		Texture2D::Sptr libshelfTex = ResourceManager::CreateAsset<Texture2D>("textures/books2.jpg");
		Texture2D::Sptr libshelfTex2 = ResourceManager::CreateAsset<Texture2D>("textures/books3.png");
		Gameplay::Material::Sptr libshelfMat = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			libshelfMat->Name = "Library Shelf";
			libshelfMat->Set("u_Material.AlbedoMap", libshelfTex);
			libshelfMat->Set("u_Material.Shininess", 0.3f);
			libshelfMat->Set("u_Material.NormalMap", normalMapDefault);
		}

		Gameplay::Material::Sptr libshelfMat2 = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			libshelfMat2->Name = "Library Shelf";
			libshelfMat2->Set("u_Material.AlbedoMap", libshelfTex2);
			libshelfMat2->Set("u_Material.Shininess", 0.3f);
			libshelfMat2->Set("u_Material.NormalMap", normalMapDefault);
		}

		Gameplay::MeshResource::Sptr lchairMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("LoungeChair.obj");
		Texture2D::Sptr lchairTex = ResourceManager::CreateAsset<Texture2D>("textures/Wall.png");
		Gameplay::Material::Sptr lchairMat = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			lchairMat->Name = "Lounge Chair";
			lchairMat->Set("u_Material.AlbedoMap", lchairTex);
			lchairMat->Set("u_Material.Shininess", 0.3f);
			lchairMat->Set("u_Material.NormalMap", normalMapDefault);
		}

		Gameplay::MeshResource::Sptr toiletMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("toilet.obj");
		Texture2D::Sptr toiletTex = ResourceManager::CreateAsset<Texture2D>("textures/toilet.jpg");
		Gameplay::Material::Sptr toiletMat = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			toiletMat->Name = "Toilet";
			toiletMat->Set("u_Material.AlbedoMap", toiletTex);
			toiletMat->Set("u_Material.Shininess", 0.3f);
			toiletMat->Set("u_Material.NormalMap", normalMapDefault);
		}

		Gameplay::MeshResource::Sptr sinkMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("Sink.obj");
		Texture2D::Sptr sinkTex = ResourceManager::CreateAsset<Texture2D>("textures/sinktex.jpg");
		Gameplay::Material::Sptr sinkMat = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			sinkMat->Name = "Sink";
			sinkMat->Set("u_Material.AlbedoMap", sinkTex);
			sinkMat->Set("u_Material.Shininess", 0.3f);
			sinkMat->Set("u_Material.NormalMap", normalMapDefault);
		}

		Gameplay::MeshResource::Sptr tubMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("tub.obj");
		Texture2D::Sptr tubTex = ResourceManager::CreateAsset<Texture2D>("textures/tub.jpg");
		Gameplay::Material::Sptr tubMat = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			tubMat->Name = "Tub";
			tubMat->Set("u_Material.AlbedoMap", tubTex);
			tubMat->Set("u_Material.Shininess", 0.3f);
			tubMat->Set("u_Material.NormalMap", normalMapDefault);
		}

		Gameplay::MeshResource::Sptr statueMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("wolfstatue.obj");
		Texture2D::Sptr statueTex = ResourceManager::CreateAsset <Texture2D>("textures/statue.jpg");
		Gameplay::Material::Sptr statueMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			statueMaterial->Name = "Statue";
			statueMaterial->Set("u_Material.AlbedoMap", statueTex);
			statueMaterial->Set("u_Material.Shininess", 1.0f);
			statueMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Gameplay::MeshResource::Sptr tallfountainMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("FountainTall.obj");
		Texture2D::Sptr tallfountainTex = ResourceManager::CreateAsset<Texture2D>("textures/fountain.jpg");
		Gameplay::Material::Sptr tallfountainMat = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			tallfountainMat->Name = "Statue";
			tallfountainMat->Set("u_Material.AlbedoMap", tallfountainTex);
			tallfountainMat->Set("u_Material.Shininess", 1.0f);
			tallfountainMat->Set("u_Material.NormalMap", normalMapDefault);
		}

		Gameplay::MeshResource::Sptr longfountainMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("long fountain.obj");
		Texture2D::Sptr longfountainTex = ResourceManager::CreateAsset<Texture2D>("textures/long fountain.jpg");
		Gameplay::Material::Sptr longfountainMat = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			longfountainMat->Name = "Statue";
			longfountainMat->Set("u_Material.AlbedoMap", longfountainTex);
			longfountainMat->Set("u_Material.Shininess", 1.0f);
			longfountainMat->Set("u_Material.NormalMap", normalMapDefault);
		}

		Gameplay::MeshResource::Sptr posterMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("Poster.obj");
		Texture2D::Sptr posterTex = ResourceManager::CreateAsset<Texture2D>("textures/poster.jpg");
		Texture2D::Sptr posterTex2 = ResourceManager::CreateAsset<Texture2D>("textures/poster2.jpg");
		Texture2D::Sptr posterTex3 = ResourceManager::CreateAsset<Texture2D>("textures/poster3.jpg");
		Gameplay::Material::Sptr posterMat = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			posterMat->Name = "Poster 1";
			posterMat->Set("u_Material.AlbedoMap", posterTex);
			posterMat->Set("u_Material.Shininess", 1.0f);
			posterMat->Set("u_Material.NormalMap", normalMapDefault);
		}
		Gameplay::Material::Sptr posterMat2 = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			posterMat2->Name = "Poster 2";
			posterMat2->Set("u_Material.AlbedoMap", posterTex2);
			posterMat2->Set("u_Material.Shininess", 1.0f);
			posterMat2->Set("u_Material.NormalMap", normalMapDefault);
		}
		Gameplay::Material::Sptr posterMat3 = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			posterMat3->Name = "Statue";
			posterMat3->Set("u_Material.AlbedoMap", posterTex3);
			posterMat3->Set("u_Material.Shininess", 1.0f);
			posterMat3->Set("u_Material.NormalMap", normalMapDefault);
		}
		

		// Here we'll load in the cubemap, as well as a special shader to handle drawing the skybox
		TextureCube::Sptr testCubemap = ResourceManager::CreateAsset<TextureCube>("cubemaps/ocean/ocean.jpg");
		ShaderProgram::Sptr      skyboxShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/skybox_vert.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/skybox_frag.glsl" }
		});

		// Create an empty scene
		Gameplay::Scene::Sptr scene = std::make_shared<Gameplay::Scene>();

		// Loading in a color lookup table
		Texture3D::Sptr lut = ResourceManager::CreateAsset<Texture3D>("luts/sepia.CUBE");  //MY CUSTOM
		// Configure the color correction LUT
		scene->SetColorLUT(lut);

		// Setting up our enviroment map
		//scene->SetSkyboxTexture(testCubemap);
		//scene->SetSkyboxShader(skyboxShader);
		// Since the skybox I used was for Y-up, we need to rotate it 90 deg around the X-axis to convert it to z-up
		//scene->SetSkyboxRotation(glm::rotate(MAT4_IDENTITY, glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)));

		// Create some lights for our scene
		Gameplay::GameObject::Sptr lightParent = scene->CreateGameObject("Lights");
		//light collection
		//set this up for every light
		 {
			{
				Gameplay::GameObject::Sptr light = scene->CreateGameObject("Light 1");
				light->SetPostion(glm::vec3(2.04f, 7.9f, 3.0f));
				lightParent->AddChild(light);

				Light::Sptr lightComponent = light->Add<Light>();
				lightComponent->SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
				lightComponent->SetRadius(20.0f);
				lightComponent->SetIntensity(6.0f);
			}
			/*{
				Gameplay::GameObject::Sptr light = scene->CreateGameObject("Light 2");
				light->SetPostion(glm::vec3(13.4f, 12.35f, 3.0f));
				lightParent->AddChild(light);

				Light::Sptr lightComponent = light->Add<Light>();
				lightComponent->SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
				lightComponent->SetRadius(20.0f);
				lightComponent->SetIntensity(6.0f);
			}
			{
				Gameplay::GameObject::Sptr light = scene->CreateGameObject("Light 3");
				light->SetPostion(glm::vec3(21.78f, 0.380f, 3.0f));
				lightParent->AddChild(light);

				Light::Sptr lightComponent = light->Add<Light>();
				lightComponent->SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
				lightComponent->SetRadius(20.0f);
				lightComponent->SetIntensity(6.0f);
			}
			{
				Gameplay::GameObject::Sptr light = scene->CreateGameObject("Light 4");
				light->SetPostion(glm::vec3(11.7f, 3.06f, 3.0f));
				lightParent->AddChild(light);

				Light::Sptr lightComponent = light->Add<Light>();
				lightComponent->SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
				lightComponent->SetRadius(20.0f);
				lightComponent->SetIntensity(6.0f);
			}
			{
				Gameplay::GameObject::Sptr light = scene->CreateGameObject("Light 5");
				light->SetPostion(glm::vec3(-2.19f, 22.51f, 3.0f));
				lightParent->AddChild(light);

				Light::Sptr lightComponent = light->Add<Light>();
				lightComponent->SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
				lightComponent->SetRadius(20.0f);
				lightComponent->SetIntensity(10.0f);
			}
			{
				Gameplay::GameObject::Sptr light = scene->CreateGameObject("Light 6");
				light->SetPostion(glm::vec3(-14.41f, 0.56f, 3.0f));
				lightParent->AddChild(light);

				Light::Sptr lightComponent = light->Add<Light>();
				lightComponent->SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
				lightComponent->SetRadius(20.0f);
				lightComponent->SetIntensity(1.0f);
			}
			{
				Gameplay::GameObject::Sptr light = scene->CreateGameObject("Light 7");
				light->SetPostion(glm::vec3(-8.17f, 0.87f, 3.0f));
				lightParent->AddChild(light);

				Light::Sptr lightComponent = light->Add<Light>();
				lightComponent->SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
				lightComponent->SetRadius(20.0f);
				lightComponent->SetIntensity(1.0f);
			}
			{
				Gameplay::GameObject::Sptr light = scene->CreateGameObject("Light 8");
				light->SetPostion(glm::vec3(-2.060f, -0.94f, 4.60f));
				lightParent->AddChild(light);

				Light::Sptr lightComponent = light->Add<Light>();
				lightComponent->SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
				lightComponent->SetRadius(-0.600f);
				lightComponent->SetIntensity(1.0f);
			}*/
		}

		// We'll create a mesh that is a simple plane that we can resize later
		Gameplay::MeshResource::Sptr planeMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>();
		planeMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(1.0f)));
		planeMesh->GenerateMesh();

	

		// Set up the scene's camera
		Gameplay::GameObject::Sptr camera = scene->MainCamera->GetGameObject()->SelfRef();
		{
			camera->SetPostion(glm::vec3(-1.42f, 4.69f, 5.73f));
			//camera->SetPostion(glm::vec3(-1.42f, 18.67f, 17.420));
			camera->LookAt(glm::vec3(0.0f));
			camera->SetRotation(glm::vec3(59.0f, 0.0f, 177.0f));
			camera->SetScale(glm::vec3(1.0f, 1.0f, 3.1f));
			scene->MainCamera->SetFovDegrees(105.f);

			camera->Add<SimpleCameraControl>();

		}

		Gameplay::GameObject::Sptr shadowCaster = scene->CreateGameObject("Shadow Light");
		{
			// Set position in the scene
			shadowCaster->SetPostion(glm::vec3(3.0f, 3.0f, 5.0f));
			shadowCaster->LookAt(glm::vec3(0.0f));

			// Create and attach a renderer for the monkey
			ShadowCamera::Sptr shadowCam = shadowCaster->Add<ShadowCamera>();
			//shadowCam->SetProjection(glm::perspective(glm::radians(120.0f), -1.42f, 4.69f, 5.73f));
			shadowCam->SetProjection(glm::ortho(15.0f, 30.0f, 30.0f, 15.0f, 0.225f, 22555.f));
		}
		

		// Set up all our sample objects
		//setup trashy
		Gameplay::MeshResource::Sptr trashyMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashy.obj");
		Texture2D::Sptr trashyTex = ResourceManager::CreateAsset<Texture2D>("textures/trashyTEX.png");
		// Create our material
		Gameplay::Material::Sptr trashyMaterial = ResourceManager::CreateAsset<Gameplay::Material>(animShader);
		{
			trashyMaterial->Name = "Trashy";
			trashyMaterial->Set("u_Material.AlbedoMap", trashyTex);
			trashyMaterial->Set("u_Material.Shininess", 1.0f);
			trashyMaterial->Set("u_Material.NormalMap", normalMapDefault);


		}
		
		Gameplay::GameObject::Sptr trashyM = scene->CreateGameObject("Trashy"); //SEARCHBAR TAGS: PLAYERENTITY, PLAYER, TRASHYENTITY, TRASHYOBJECT
		{
			trashyM->SetPostion(glm::vec3(-1.5f, 0.0f, 1.0f));
			trashyM->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			trashyM->SetScale(glm::vec3(0.4f, 0.4f, 0.4f));
			// Add a render component
			RenderComponent::Sptr renderer = trashyM->Add<RenderComponent>();
			renderer->SetMesh(trashyMesh);
			renderer->SetMaterial(trashyMaterial);
			// Add a dynamic rigid body to this monkey
			Gameplay::Physics::RigidBody::Sptr physics = trashyM->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Dynamic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();

			box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			box->SetScale(glm::vec3(0.33f, 0.580f, 0.22f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			physics->AddCollider(box);
			//physics->AddCollider(BoxCollider::Create());
			//physics->SetMass(0.0f);
			//add trigger for collisions and behaviours
			Gameplay::Physics::TriggerVolume::Sptr volume = trashyM->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();

			box2->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			box2->SetScale(glm::vec3(0.33f, 0.58f, 0.22f));
			//box2->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			volume->AddCollider(box2);
			JumpBehaviour::Sptr behaviour = trashyM->Add<JumpBehaviour>();
			//CollectTrashBehaviour::Sptr behaviour2 = trashyM->Add<CollectTrashBehaviour>();

			PlayerMovementBehavior::Sptr movement = trashyM->Add<PlayerMovementBehavior>();
			trashyM->Add<InventoryUI>();

			//ANIMATION STUFF////
			MorphMeshRenderer::Sptr morph1 = trashyM->Add<MorphMeshRenderer>();
			morph1->SetMorphMeshRenderer(trashyMesh, trashyMaterial);
			MorphAnimator::Sptr morph2 = trashyM->Add<MorphAnimator>();

			//walking frames
			//std::vector <MeshResource::Sptr> frames;
			//MeshResource::Sptr trashyMesh1 = ResourceManager::CreateAsset<MeshResource>("trashyWalk/trashywalk_000001.obj");
			Gameplay::MeshResource::Sptr trashyMesh2 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyWalk/trashywalk_000002.obj");
			Gameplay::MeshResource::Sptr trashyMesh7 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyWalk/trashywalk_000010.obj");
			Gameplay::MeshResource::Sptr trashyMesh3 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyWalk/trashywalk_000016.obj");
			Gameplay::MeshResource::Sptr trashyMesh4 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyWalk/trashywalk_000020.obj");
			Gameplay::MeshResource::Sptr trashyMesh5 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyWalk/trashywalk_000025.obj");
			Gameplay::MeshResource::Sptr trashyMesh6 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyWalk/trashywalk_000030.obj");
			//idle frames
			//std::vector <MeshResource::Sptr> frames2;
			Gameplay::MeshResource::Sptr trashyMesh8 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyIdle/trashyidle_000001.obj");
			Gameplay::MeshResource::Sptr trashyMesh11 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyIdle/trashyidle_000011.obj");
			Gameplay::MeshResource::Sptr trashyMesh10 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyIdle/trashyidle_000020.obj");
			Gameplay::MeshResource::Sptr trashyMesh12 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyIdle/trashyidle_000030.obj");
			Gameplay::MeshResource::Sptr trashyMesh9 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyIdle/trashyidle_000040.obj");
			//jump frames
			Gameplay::MeshResource::Sptr trashyJump1 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyJump/trashy jump_000001.obj");
			Gameplay::MeshResource::Sptr trashyJump13 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyJump/trashy jump_000003.obj");
			Gameplay::MeshResource::Sptr trashyJump2 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyJump/trashy jump_000005.obj");
			Gameplay::MeshResource::Sptr trashyJump3 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyJump/trashy jump_000008.obj");
			Gameplay::MeshResource::Sptr trashyJump12 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyJump/trashy jump_000010.obj");
			//fall, land
			Gameplay::MeshResource::Sptr trashyJump4 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyJump/trashy jump_000013.obj");
			Gameplay::MeshResource::Sptr trashyJump5 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyJump/trashy jump_000014.obj");
			Gameplay::MeshResource::Sptr trashyJump6 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyJump/trashy jump_000015.obj");
			Gameplay::MeshResource::Sptr trashyJump7 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyJump/trashy jump_000016.obj");
			Gameplay::MeshResource::Sptr trashyJump8 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyJump/trashy jump_000017.obj");
			Gameplay::MeshResource::Sptr trashyJump9 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyJump/trashy jump_000018.obj");
			Gameplay::MeshResource::Sptr trashyJump10 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyJump/trashy jump_000019.obj");
			Gameplay::MeshResource::Sptr trashyJump11 = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashyJump/trashy jump_000022.obj");


			idle.push_back(trashyMesh8);
			idle.push_back(trashyMesh11);
			idle.push_back(trashyMesh10);
			idle.push_back(trashyMesh12);
			idle.push_back(trashyMesh9);

			//need to also set frames for other animations
			//simply switch the set frames
			//frames.push_back(trashyMesh1);
			walking.push_back(trashyMesh2);
			walking.push_back(trashyMesh7);
			walking.push_back(trashyMesh3);
			walking.push_back(trashyMesh4);
			walking.push_back(trashyMesh5);
			walking.push_back(trashyMesh6);
			//walking = frames; //may need to more manually copy frames over
			morph2->SetInitial();
			morph2->SetFrameTime(0.2f);
			morph2->SetFrames(idle);

			//for changing animations
			morph2->SetIdle(idle);
			morph2->SetWalking(walking);
			//jump frames
			jumping.push_back(trashyJump1);
			jumping.push_back(trashyJump13);
			jumping.push_back(trashyJump2);
			jumping.push_back(trashyJump3);
			jumping.push_back(trashyJump12);
			jumping.push_back(trashyJump4);
			jumping.push_back(trashyJump5);
			jumping.push_back(trashyJump6);
			jumping.push_back(trashyJump7);
			jumping.push_back(trashyJump8);
			jumping.push_back(trashyJump9);
			jumping.push_back(trashyJump10);
			jumping.push_back(trashyJump11);

			morph2->SetJumping(jumping);
		}




		Texture2D::Sptr planeTex = ResourceManager::CreateAsset<Texture2D>("textures/floor.jpg");

		//MeshResource::Sptr layoutMesh = ResourceManager::CreateAsset<MeshResource>("layout2.obj");
		Gameplay::Material::Sptr planeMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward); {
			planeMaterial->Name = "Plane";
			planeMaterial->Set("u_Material.AlbedoMap", planeTex);
			planeMaterial->Set("u_Material.Shininess", 1.0f);
			planeMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		// Set up all our sample objects
		Gameplay::GameObject::Sptr plane = scene->CreateGameObject("Plane");
		{
			plane->SetPostion(glm::vec3(0.0f, 0.0f, -0.08));
			plane->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			// Make a big tiled mesh
			Gameplay::MeshResource::Sptr tiledMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>();
			tiledMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(100.0f), glm::vec2(20.0f)));
			tiledMesh->GenerateMesh();


			// Create and attach a RenderComponent to the object to draw our mesh
			/*RenderComponent::Sptr renderer = plane->Add<RenderComponent>();
			renderer->SetMesh(layoutMesh);
			renderer->SetMaterial(planeMaterial);*/

			// Attach a plane collider that extends infinitely along the X/Y axis
			Gameplay::Physics::RigidBody::Sptr physics = plane->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			//box->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
			box->SetScale(glm::vec3(50.0f, -0.12f, 50.0f));
			physics->AddCollider(box);
			Gameplay::Physics::TriggerVolume::Sptr volume = plane->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			//box2->SetPosition(glm::vec3(0.00f, 0.05f, 0.0f));
			box2->SetScale(glm::vec3(50.0f, 1.0f, 50.0f));
			volume->AddCollider(box2);
			//give to our floor tiles to tag them
			GroundBehaviour::Sptr behaviour = plane->Add<GroundBehaviour>();

		}

		//layout
		Gameplay::MeshResource::Sptr layoutMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("malllayoutwall.obj");
		Texture2D::Sptr layoutTex = ResourceManager::CreateAsset<Texture2D>("textures/mall.png");
		Gameplay::Material::Sptr layoutMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			layoutMaterial->Name = "Layout";
			layoutMaterial->Set("u_Material.AlbedoMap", layoutTex);
			layoutMaterial->Set("u_Material.Shininess", 0.0f);
			layoutMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}

		Gameplay::GameObject::Sptr layout = scene->CreateGameObject("Layout");
		{
			layout->SetPostion(glm::vec3(0.0f, 0.0f, 0.0f));
			layout->SetRotation(glm::vec3(90.0f, 0.0f, -180.0f));
			layout->SetScale(glm::vec3(0.4f, 0.4f, 0.4f));

			RenderComponent::Sptr renderer = layout->Add<RenderComponent>();
			renderer->SetMesh(layoutMesh);
			renderer->SetMaterial(layoutMaterial);
			//GroundBehaviour::Sptr behaviour = layout->Add<GroundBehaviour>();
		}
		//exterior walls
		
		Gameplay::GameObject::Sptr ExteriorWalls = scene->CreateGameObject("Exterior Walls");

		{
			//Walls
			Gameplay::GameObject::Sptr layoutwall1 = scene->CreateGameObject("Layout Wall Bottom");
			{
				layoutwall1->SetPostion(glm::vec3(-0.58f, 11.46f, 0.0f));
				layoutwall1->SetScale(glm::vec3(20.5f, 0.34f, 1.5f));
				Gameplay::Physics::RigidBody::Sptr wall1Phys = layoutwall1->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr wall1 = Gameplay::Physics::BoxCollider::Create();
				//wall1->SetPosition(glm::vec3(11.85f, 3.23f, 1.05f));
				wall1->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
				wall1->SetScale(glm::vec3(20.5f, 0.34f, 1.5f));
				wall1->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
				wall1Phys->AddCollider(wall1);
			}

			Gameplay::GameObject::Sptr layoutwall2 = scene->CreateGameObject("Layout Wall Top");
			{
				layoutwall2->SetPostion(glm::vec3(0.0f, -10.61f, 0.0f));
				layoutwall2->SetScale(glm::vec3(21.78f, 0.27f, 1.5f));
				Gameplay::Physics::RigidBody::Sptr wall2Phys = layoutwall2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr wall2 = Gameplay::Physics::BoxCollider::Create();
				//wall1->SetPosition(glm::vec3(11.85f, 3.23f, 1.05f));
				wall2->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
				wall2->SetScale(glm::vec3(21.78f, 0.27f, 1.5f));
				wall2->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
				wall2Phys->AddCollider(wall2);
			}

			//Right Bottom
			Gameplay::GameObject::Sptr layoutwall3 = scene->CreateGameObject("Layout Wall Right");
			{
				layoutwall3->SetPostion(glm::vec3(-17.08f, 0.0f, 0.0f));
				layoutwall3->SetScale(glm::vec3(0.3f, 15.1f, 1.5f));
				Gameplay::Physics::RigidBody::Sptr wall3Phys = layoutwall3->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr wall3 = Gameplay::Physics::BoxCollider::Create();
				wall3->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
				wall3->SetScale(glm::vec3(0.3f, 15.1f, 1.5f));
				wall3->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
				wall3Phys->AddCollider(wall3);
			}

			//Left Wall Corner Top
			Gameplay::GameObject::Sptr layoutwall5 = scene->CreateGameObject("Layout Wall Left Corner Right");
			{
				layoutwall5->SetPostion(glm::vec3(14.1f, 9.85f, 0.0f));
				layoutwall5->SetScale(glm::vec3(0.19f, 2.26f, 1.5f));
				Gameplay::Physics::RigidBody::Sptr wall5Phys = layoutwall5->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr wall5 = Gameplay::Physics::BoxCollider::Create();
				wall5->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
				wall5->SetScale(glm::vec3(0.19f, 2.26f, 1.5f));
				wall5->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
				wall5Phys->AddCollider(wall5);
			}
			Gameplay::GameObject::Sptr layoutwall5b = scene->CreateGameObject("Layout Wall Left Corner Top");
			{
				layoutwall5b->SetPostion(glm::vec3(16.02f, 7.73f, 0.0f));
				layoutwall5b->SetScale(glm::vec3(2.08f, 0.18f, 1.5f));
				Gameplay::Physics::RigidBody::Sptr wall5Physb = layoutwall5b->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr wall5b = Gameplay::Physics::BoxCollider::Create();
				wall5b->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
				wall5b->SetScale(glm::vec3(2.08f, 0.18f, 1.5f));
				wall5b->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
				wall5Physb->AddCollider(wall5b);
			}

			//Left Wall
			Gameplay::GameObject::Sptr layoutwall6 = scene->CreateGameObject("Layout Wall Left");
			{
				layoutwall6->SetPostion(glm::vec3(17.23f, -0.89f, 0.0f));
				layoutwall6->SetScale(glm::vec3(0.32f, 13.69f, 1.5f));
				Gameplay::Physics::RigidBody::Sptr wall6Phys = layoutwall6->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr wall6 = Gameplay::Physics::BoxCollider::Create();
				wall6->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
				wall6->SetScale(glm::vec3(0.32f, 13.69f, 1.5f));
				wall6->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
				wall6Phys->AddCollider(wall6);
			}
			ExteriorWalls->AddChild(layoutwall1);
			ExteriorWalls->AddChild(layoutwall2);
			ExteriorWalls->AddChild(layoutwall3);
			//ExteriorWalls->AddChild(layoutwall4);
			ExteriorWalls->AddChild(layoutwall5);
			ExteriorWalls->AddChild(layoutwall5b);
			ExteriorWalls->AddChild(layoutwall6);
		}
		
		/*
		layoutwall9->SetPostion(glm::vec3(0.0f, 0.0f, 0.0f));
		layoutwall9->SetScale(glm::vec3(1.0f, 1.0f, 1.0f));
		Gameplay::Physics::RigidBody::Sptr wallPhys = layoutwall9->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
		Gameplay::Physics::BoxCollider::Sptr wall = Gameplay::Physics::BoxCollider::Create();
		wall->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		wall->SetScale(glm::vec3(1.0f, 1.0f, 1.0f));
		wall->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
		wallPhys->AddCollider(wall);
		*/

		//interior walls
		{
			Gameplay::GameObject::Sptr InteriorWallsTopRight = scene->CreateGameObject("Interior Walls Top Right");

			{
				Gameplay::GameObject::Sptr layoutwall7 = scene->CreateGameObject("Left");
				{
					layoutwall7->SetPostion(glm::vec3(-4.46f, -6.92f, 0.0f));
					layoutwall7->SetScale(glm::vec3(0.34f, 3.46f, 1.5f));
					Gameplay::Physics::RigidBody::Sptr wallPhys = layoutwall7->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
					Gameplay::Physics::BoxCollider::Sptr wall = Gameplay::Physics::BoxCollider::Create();
					wall->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
					wall->SetScale(glm::vec3(0.34f, 3.46f, 1.5f));
					wall->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
					wallPhys->AddCollider(wall);
				}

				Gameplay::GameObject::Sptr layoutwall8 = scene->CreateGameObject("Bottom");
				{
					layoutwall8->SetPostion(glm::vec3(-8.71f, -3.77f, 0.0f));
					layoutwall8->SetScale(glm::vec3(3.86f, 0.31f, 1.5f));
					Gameplay::Physics::RigidBody::Sptr wallPhys = layoutwall8->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
					Gameplay::Physics::BoxCollider::Sptr wall = Gameplay::Physics::BoxCollider::Create();
					wall->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
					wall->SetScale(glm::vec3(4.38f, 0.31f, 1.5f));
					wall->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
					wallPhys->AddCollider(wall);

					//GroundBehaviour::Sptr behaviour = layoutwall8->Add<GroundBehaviour>();
				}
				InteriorWallsTopRight->AddChild(layoutwall7);
				InteriorWallsTopRight->AddChild(layoutwall8);
			}

			Gameplay::GameObject::Sptr InteriorWallsBotLeft = scene->CreateGameObject("Interior Walls Bot Left");
			{
				Gameplay::GameObject::Sptr layoutwall9 = scene->CreateGameObject("Top");
				{
					layoutwall9->SetPostion(glm::vec3(12.65f, 2.81f, 0.0f));
					layoutwall9->SetScale(glm::vec3(4.64f, 0.38f, 1.5f));
					Gameplay::Physics::RigidBody::Sptr wallPhys = layoutwall9->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
					Gameplay::Physics::BoxCollider::Sptr wall = Gameplay::Physics::BoxCollider::Create();
					wall->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
					wall->SetScale(glm::vec3(4.64f, 0.38f, 1.5f));
					wall->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
					wallPhys->AddCollider(wall);
				}

				Gameplay::GameObject::Sptr layoutwall10 = scene->CreateGameObject("Right Top");
				{
					layoutwall10->SetPostion(glm::vec3(4.4f, 4.79f, 0.0f));
					layoutwall10->SetScale(glm::vec3(0.4f, 2.32f, 1.5f));
					Gameplay::Physics::RigidBody::Sptr wallPhys = layoutwall10->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
					Gameplay::Physics::BoxCollider::Sptr wall = Gameplay::Physics::BoxCollider::Create();
					wall->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
					wall->SetScale(glm::vec3(0.4f, 2.32f, 1.5f));
					wall->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
					wallPhys->AddCollider(wall);
				}

				Gameplay::GameObject::Sptr layoutwall11 = scene->CreateGameObject("Right Mid");
				{
					layoutwall11->SetPostion(glm::vec3(1.25f, 6.67f, 0.0f));
					layoutwall11->SetScale(glm::vec3(3.52f, 0.34f, 1.5f));
					Gameplay::Physics::RigidBody::Sptr wallPhys = layoutwall11->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
					Gameplay::Physics::BoxCollider::Sptr wall = Gameplay::Physics::BoxCollider::Create();
					wall->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
					wall->SetScale(glm::vec3(3.52f, 0.34f, 1.5f));
					wall->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
					wallPhys->AddCollider(wall);
				}

				Gameplay::GameObject::Sptr layoutwall25 = scene->CreateGameObject("Right");
				{
					layoutwall25->SetPostion(glm::vec3(-1.88f, 8.87f, 0.0f));
					layoutwall25->SetScale(glm::vec3(0.4f, 2.56f, 1.5f));
					Gameplay::Physics::RigidBody::Sptr wallPhys = layoutwall25->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
					Gameplay::Physics::BoxCollider::Sptr wall = Gameplay::Physics::BoxCollider::Create();
					wall->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
					wall->SetScale(glm::vec3(0.4f, 2.56f, 1.5f));
					wall->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
					wallPhys->AddCollider(wall);
				}

				InteriorWallsBotLeft->AddChild(layoutwall9);
				InteriorWallsBotLeft->AddChild(layoutwall10);
				InteriorWallsBotLeft->AddChild(layoutwall11);
				//InteriorWallsBotLeft->AddChild(layoutwall12);
				InteriorWallsBotLeft->AddChild(layoutwall25);
			}

			Gameplay::GameObject::Sptr InteriorWallsBotRight = scene->CreateGameObject("Interior Walls Bot Right");
			{
				Gameplay::GameObject::Sptr layoutwall13 = scene->CreateGameObject("Top");
				{
					layoutwall13->SetPostion(glm::vec3(-12.67f, -0.68f, 0.0f));
					layoutwall13->SetScale(glm::vec3(4.31f, 0.3f, 1.5f));
					Gameplay::Physics::RigidBody::Sptr wallPhys = layoutwall13->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
					Gameplay::Physics::BoxCollider::Sptr wall = Gameplay::Physics::BoxCollider::Create();
					wall->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
					wall->SetScale(glm::vec3(4.31f, 0.3f, 1.5f));
					wall->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
					wallPhys->AddCollider(wall);
				}

				Gameplay::GameObject::Sptr layoutwall14 = scene->CreateGameObject("Left Top");
				{
					layoutwall14->SetPostion(glm::vec3(-8.76f, 1.12f, 0.0f));
					layoutwall14->SetScale(glm::vec3(0.37f, 2.05f, 1.5f));
					Gameplay::Physics::RigidBody::Sptr wallPhys = layoutwall14->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
					Gameplay::Physics::BoxCollider::Sptr wall = Gameplay::Physics::BoxCollider::Create();
					wall->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
					wall->SetScale(glm::vec3(0.37f, 2.05f, 1.5f));
					wall->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
					wallPhys->AddCollider(wall);
				}

				Gameplay::GameObject::Sptr layoutwall15 = scene->CreateGameObject("Left Mid");
				{
					layoutwall15->SetPostion(glm::vec3(-7.45f, 2.79f, 0.0f));
					layoutwall15->SetScale(glm::vec3(1.67f, 0.39f, 1.5f));
					Gameplay::Physics::RigidBody::Sptr wallPhys = layoutwall15->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
					Gameplay::Physics::BoxCollider::Sptr wall = Gameplay::Physics::BoxCollider::Create();
					wall->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
					wall->SetScale(glm::vec3(1.67f, 0.39f, 1.5f));
					wall->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
					wallPhys->AddCollider(wall);
				}

				Gameplay::GameObject::Sptr layoutwall16 = scene->CreateGameObject("Left Bot");
				{
					layoutwall16->SetPostion(glm::vec3(-5.98f, 4.81f, 0.0f));
					layoutwall16->SetScale(glm::vec3(0.22f, 2.38f, 1.5f));
					Gameplay::Physics::RigidBody::Sptr wallPhys = layoutwall16->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
					Gameplay::Physics::BoxCollider::Sptr wall = Gameplay::Physics::BoxCollider::Create();
					wall->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
					wall->SetScale(glm::vec3(0.22f, 2.38f, 1.5f));
					wall->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
					wallPhys->AddCollider(wall);
				}

				Gameplay::GameObject::Sptr layoutwall17 = scene->CreateGameObject("Bottom");
				{
					layoutwall17->SetPostion(glm::vec3(-9.7f, 6.77f, 0.0f));
					layoutwall17->SetScale(glm::vec3(3.89f, 0.41f, 1.5f));
					Gameplay::Physics::RigidBody::Sptr wallPhys = layoutwall17->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
					Gameplay::Physics::BoxCollider::Sptr wall = Gameplay::Physics::BoxCollider::Create();
					wall->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
					wall->SetScale(glm::vec3(3.89f, 0.41f, 1.5f));
					wall->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
					wallPhys->AddCollider(wall);

					//GroundBehaviour::Sptr behaviour = layoutwall17->Add<GroundBehaviour>();
				}

				Gameplay::GameObject::Sptr layoutwall18 = scene->CreateGameObject("Right");
				{
					layoutwall18->SetPostion(glm::vec3(-13.33f, 5.21f, 0.0f));
					layoutwall18->SetScale(glm::vec3(0.26f, 1.98f, 1.5f));
					Gameplay::Physics::RigidBody::Sptr wallPhys = layoutwall18->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
					Gameplay::Physics::BoxCollider::Sptr wall = Gameplay::Physics::BoxCollider::Create();
					wall->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
					wall->SetScale(glm::vec3(0.26f, 1.98f, 1.5f));
					wall->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
					wallPhys->AddCollider(wall);
				}
				InteriorWallsBotRight->AddChild(layoutwall13);
				InteriorWallsBotRight->AddChild(layoutwall14);
				InteriorWallsBotRight->AddChild(layoutwall15);
				InteriorWallsBotRight->AddChild(layoutwall16);
				InteriorWallsBotRight->AddChild(layoutwall17);
				InteriorWallsBotRight->AddChild(layoutwall18);
			}

			Gameplay::GameObject::Sptr InteriorWallsTopLeft = scene->CreateGameObject("Interior Walls Top Left");
			{
				Gameplay::GameObject::Sptr layoutwall19 = scene->CreateGameObject("Left");
				{
					layoutwall19->SetPostion(glm::vec3(13.65f, -3.08f, 0.0f));
					layoutwall19->SetScale(glm::vec3(0.36f, 0.87f, 1.5f));
					Gameplay::Physics::RigidBody::Sptr wallPhys = layoutwall19->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
					Gameplay::Physics::BoxCollider::Sptr wall = Gameplay::Physics::BoxCollider::Create();
					wall->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
					wall->SetScale(glm::vec3(0.36f, 0.87f, 1.5f));
					wall->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
					wallPhys->AddCollider(wall);
				}

				Gameplay::GameObject::Sptr layoutwall20 = scene->CreateGameObject("Bottom");
				{
					layoutwall20->SetPostion(glm::vec3(10.29f, -2.47f, 0.0f));
					layoutwall20->SetScale(glm::vec3(3.73f, 0.28f, 1.5f));
					Gameplay::Physics::RigidBody::Sptr wallPhys = layoutwall20->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
					Gameplay::Physics::BoxCollider::Sptr wall = Gameplay::Physics::BoxCollider::Create();
					wall->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
					wall->SetScale(glm::vec3(3.73f, 0.28f, 1.5f));
					wall->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
					wallPhys->AddCollider(wall);

					//GroundBehaviour::Sptr behaviour = layoutwall20->Add<GroundBehaviour>();
				}

				Gameplay::GameObject::Sptr layoutwall21 = scene->CreateGameObject("Right Bot");
				{
					layoutwall21->SetPostion(glm::vec3(6.77f, -4.22f, 0.0f));
					layoutwall21->SetScale(glm::vec3(0.24f, 2.05f, 1.5f));
					Gameplay::Physics::RigidBody::Sptr wallPhys = layoutwall21->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
					Gameplay::Physics::BoxCollider::Sptr wall = Gameplay::Physics::BoxCollider::Create();
					wall->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
					wall->SetScale(glm::vec3(0.24f, 2.05f, 1.5f));
					wall->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
					wallPhys->AddCollider(wall);
				}

				Gameplay::GameObject::Sptr layoutwall22 = scene->CreateGameObject("Right Mid");
				{
					layoutwall22->SetPostion(glm::vec3(6.21f, -6.65f, 0.0f));
					layoutwall22->SetRotation(glm::vec3(0.0f, 0.0f, -45.0f));
					layoutwall22->SetScale(glm::vec3(0.3f, 0.81f, 1.5f));
					Gameplay::Physics::RigidBody::Sptr wallPhys = layoutwall22->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
					Gameplay::Physics::BoxCollider::Sptr wall = Gameplay::Physics::BoxCollider::Create();
					wall->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
					wall->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
					wall->SetScale(glm::vec3(0.3f, 0.81f, 1.5f));
					wall->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
					wallPhys->AddCollider(wall);
				}

				Gameplay::GameObject::Sptr layoutwall23 = scene->CreateGameObject("Right Top");
				{
					layoutwall23->SetPostion(glm::vec3(4.81f, -7.23f, 0.0f));
					layoutwall23->SetScale(glm::vec3(1.06f, 0.27f, 1.5f));
					Gameplay::Physics::RigidBody::Sptr wallPhys = layoutwall23->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
					Gameplay::Physics::BoxCollider::Sptr wall = Gameplay::Physics::BoxCollider::Create();
					wall->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
					wall->SetScale(glm::vec3(1.06f, 0.27f, 1.5f));
					wall->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
					wallPhys->AddCollider(wall);
				}

				Gameplay::GameObject::Sptr layoutwall24 = scene->CreateGameObject("Right");
				{
					layoutwall24->SetPostion(glm::vec3(3.98f, -8.86f, 0.0f));
					layoutwall24->SetScale(glm::vec3(0.25f, 1.88f, 1.5f));
					Gameplay::Physics::RigidBody::Sptr wallPhys = layoutwall24->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
					Gameplay::Physics::BoxCollider::Sptr wall = Gameplay::Physics::BoxCollider::Create();
					wall->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
					wall->SetScale(glm::vec3(0.25f, 1.88f, 1.5f));
					wall->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
					wallPhys->AddCollider(wall);
				}
				
				InteriorWallsTopLeft->AddChild(layoutwall19);
				InteriorWallsTopLeft->AddChild(layoutwall20);
				InteriorWallsTopLeft->AddChild(layoutwall21);
				InteriorWallsTopLeft->AddChild(layoutwall22);
				InteriorWallsTopLeft->AddChild(layoutwall23);
				InteriorWallsTopLeft->AddChild(layoutwall24);
			}
		}

		
		//Top Right Store
		Gameplay::GameObject::Sptr TopRight = scene->CreateGameObject("Top Right Store");

			Gameplay::GameObject::Sptr shelf = scene->CreateGameObject("Shelf 1");
			{
				shelf->SetPostion(glm::vec3(-12.71f, -5.31f, 0.0f));
				shelf->SetRotation(glm::vec3(90.0, 0.0f, 0.0f));
				shelf->SetScale(glm::vec3(0.4f, 0.4f, 0.4f));

				RenderComponent::Sptr renderer = shelf->Add<RenderComponent>();
				renderer->SetMesh(shelfMesh);
				renderer->SetMaterial(shelfMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = shelf->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.42f, 1.34f, 1.21f));
				box->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr shelf2 = scene->CreateGameObject("Shelf 2");
			{
				shelf2->SetPostion(glm::vec3(-15.59f, -8.21f, 0.0f));
				shelf2->SetRotation(glm::vec3(90.0, 0.0f, -90.0f));
				shelf2->SetScale(glm::vec3(0.4f, 0.4f, 0.4f));

				RenderComponent::Sptr renderer = shelf2->Add<RenderComponent>();
				renderer->SetMesh(shelfMesh);
				renderer->SetMaterial(shelfMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = shelf2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.42f, 1.34f, 1.21f));
				box->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr shelf3 = scene->CreateGameObject("Shelf 3");
			{
				shelf3->SetPostion(glm::vec3(-12.7f, -9.12f, 0.0f));
				shelf3->SetRotation(glm::vec3(90.0, 0.0f, 0.0f));
				shelf3->SetScale(glm::vec3(0.4f, 0.4f, 0.4f));

				RenderComponent::Sptr renderer = shelf3->Add<RenderComponent>();
				renderer->SetMesh(shelfMesh);
				renderer->SetMaterial(shelfMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = shelf3->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.42f, 1.34f, 1.21f));
				box->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr shelf4 = scene->CreateGameObject("Shelf 4");
			{
				shelf4->SetPostion(glm::vec3(-6.09f, -7.86f, 0.0f));
				shelf4->SetRotation(glm::vec3(90.0, 0.0f, -90.0f));
				shelf4->SetScale(glm::vec3(0.4f, 0.4f, 0.4f));

				RenderComponent::Sptr renderer = shelf4->Add<RenderComponent>();
				renderer->SetMesh(shelfMesh);
				renderer->SetMaterial(shelfMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = shelf4->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.42f, 1.34f, 1.21f));
				box->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr tvbox1 = scene->CreateGameObject("tv 1");
			{
				tvbox1->SetPostion(glm::vec3(-15.02f, -4.29f, 0.0f));
				tvbox1->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				tvbox1->SetScale(glm::vec3(0.7f, 0.8f, 0.7f));

				RenderComponent::Sptr renderer = tvbox1->Add<RenderComponent>();
				renderer->SetMesh(tvboxMesh);
				renderer->SetMaterial(tvboxMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = tvbox1->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.1f, 0.98f, 0.82f));
				box->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr tvbox2 = scene->CreateGameObject("tv 2");
			{
				tvbox2->SetPostion(glm::vec3(-11.53f, -8.07f, 0.0f));
				tvbox2->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				tvbox2->SetScale(glm::vec3(0.7f, 0.8f, 0.7f));

				RenderComponent::Sptr renderer = tvbox2->Add<RenderComponent>();
				renderer->SetMesh(tvboxMesh);
				renderer->SetMaterial(tvboxMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = tvbox2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.1f, 0.98f, 0.82f));
				box->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr tvbox3 = scene->CreateGameObject("tv 3");
			{
				tvbox3->SetPostion(glm::vec3(-10.07f, -8.07f, 0.0f));
				tvbox3->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
				tvbox3->SetScale(glm::vec3(0.7f, 0.8f, 0.7f));

				RenderComponent::Sptr renderer = tvbox3->Add<RenderComponent>();
				renderer->SetMesh(tvboxMesh);
				renderer->SetMaterial(tvboxMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = tvbox3->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.1f, 0.98f, 0.82f));
				box->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr tvbox4 = scene->CreateGameObject("tv4");
			{
				tvbox4->SetPostion(glm::vec3(-5.78f, -9.31f, 0.0f));
				tvbox4->SetRotation(glm::vec3(90.0f, 0.0f, -20.0f));
				tvbox4->SetScale(glm::vec3(0.8f, 0.8f, 0.8f));

				RenderComponent::Sptr renderer = tvbox4->Add<RenderComponent>();
				renderer->SetMesh(tvboxMesh);
				renderer->SetMaterial(tvboxMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = tvbox4->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.1f, 0.98f, 0.82f));
				box->SetExtents(glm::vec3(1.0f, 1.0f, 1.0f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr cashcounter = scene->CreateGameObject("CashCounter");
			{
				cashcounter->SetPostion(glm::vec3(-6.58f, -5.22f, 0.0f));
				cashcounter->SetRotation(glm::vec3(90.0f, 0.0f, 180.0f));
				cashcounter->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				RenderComponent::Sptr renderer = cashcounter->Add<RenderComponent>();
				renderer->SetMesh(cashMesh);
				renderer->SetMaterial(cashMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = cashcounter->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.520f, 0.740f, 1.210f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr computer1 = scene->CreateGameObject("Desktop1");
			{
				computer1->SetPostion(glm::vec3(-13.99f, -8.15f, 0.0f));
				computer1->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				computer1->SetScale(glm::vec3(0.3f, 0.3f, 0.3f));

				RenderComponent::Sptr renderer = computer1->Add<RenderComponent>();
				renderer->SetMesh(computerMesh);
				renderer->SetMaterial(computerMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = computer1->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.3f, 0.61f, 0.14f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr computer2 = scene->CreateGameObject("Desktop2");
			{
				computer2->SetPostion(glm::vec3(-13.48f, -8.16f, 0.0f));
				computer2->SetRotation(glm::vec3(90.0f, 0.0f, 113.0f));
				computer2->SetScale(glm::vec3(0.3f, 0.3f, 0.3f));

				RenderComponent::Sptr renderer = computer2->Add<RenderComponent>();
				renderer->SetMesh(computerMesh);
				renderer->SetMaterial(computerMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = computer2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.3f, 0.61f, 0.14f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr bench1 = scene->CreateGameObject("Bench");
			{
				bench1->SetPostion(glm::vec3(-11.1f, -5.02f, 0.0f));
				bench1->SetRotation(glm::vec3(90.0f, 0.0f, 34.0f));
				bench1->SetScale(glm::vec3(2.0f, 1.0f, 1.5f));

				RenderComponent::Sptr renderer = bench1->Add<RenderComponent>();
				renderer->SetMesh(benchMesh);
				renderer->SetMaterial(benchMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = bench1->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr boxCollider = Gameplay::Physics::BoxCollider::Create();
				boxCollider->SetScale(glm::vec3(1.25f, 0.35f, 0.35f));
				physics->AddCollider(boxCollider);
			}

			TopRight->AddChild(shelf);
			TopRight->AddChild(shelf2);
			TopRight->AddChild(shelf3);
			TopRight->AddChild(shelf4);
			TopRight->AddChild(tvbox1);
			TopRight->AddChild(tvbox2);
			TopRight->AddChild(tvbox3);
			TopRight->AddChild(tvbox4);
			TopRight->AddChild(cashcounter);
			TopRight->AddChild(computer1);
			TopRight->AddChild(computer2);
			TopRight->AddChild(bench1);

			//Bot Left Store

			Gameplay::GameObject::Sptr BotLeft = scene->CreateGameObject("Bottom Left");
			
			Gameplay::GameObject::Sptr table1 = scene->CreateGameObject("diner table 1");
			{
				table1->SetPostion(glm::vec3(9.46f, 4.09f, 0.0f));
				table1->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				table1->SetScale(glm::vec3(0.3f, 0.4f, 0.3f));

				RenderComponent::Sptr renderer = table1->Add<RenderComponent>();
				renderer->SetMesh(dinertableMesh);
				renderer->SetMaterial(dinertableMat);
				
				Gameplay::Physics::RigidBody::Sptr physics = table1->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.47f, 0.99f, 0.93f));
				physics->AddCollider(box);
			}
			
			Gameplay::GameObject::Sptr table2 = scene->CreateGameObject("diner table 2");
			{
				table2->SetPostion(glm::vec3(1.84f, 7.65f, 0.0f));
				table2->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				table2->SetScale(glm::vec3(0.3f, 0.4f, 0.3f));

				RenderComponent::Sptr renderer = table2->Add<RenderComponent>();
				renderer->SetMesh(dinertableMesh);
				renderer->SetMaterial(dinertableMat);

				Gameplay::Physics::RigidBody::Sptr physics = table2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.47f, 0.99f, 0.93f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr table3 = scene->CreateGameObject("diner table 3");
			{
				table3->SetPostion(glm::vec3(1.85f, 10.68f, 0.0f));
				table3->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				table3->SetScale(glm::vec3(0.3f, 0.4f, 0.3f));

				RenderComponent::Sptr renderer = table3->Add<RenderComponent>();
				renderer->SetMesh(dinertableMesh);
				renderer->SetMaterial(dinertableMat);

				Gameplay::Physics::RigidBody::Sptr physics = table3->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.47f, 0.99f, 0.93f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr booth1 = scene->CreateGameObject("booth 1");
			{
				booth1->SetPostion(glm::vec3(10.63f, 3.86f, 0.0f));
				booth1->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				booth1->SetScale(glm::vec3(0.3f, 0.4f, 0.3f));

				RenderComponent::Sptr renderer = booth1->Add<RenderComponent>();
				renderer->SetMesh(boothMesh);
				renderer->SetMaterial(boothMat);

				Gameplay::Physics::RigidBody::Sptr physics = booth1->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.68f, 1.0f, 0.28f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr booth2 = scene->CreateGameObject("booth 2");
			{
				booth2->SetPostion(glm::vec3(8.42f, 3.89f, 0.0f));
				booth2->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
				booth2->SetScale(glm::vec3(0.3f, 0.4f, 0.3f));

				RenderComponent::Sptr renderer = booth2->Add<RenderComponent>();
				renderer->SetMesh(boothMesh);
				renderer->SetMaterial(boothMat);

				Gameplay::Physics::RigidBody::Sptr physics = booth2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.68f, 1.0f, 0.28f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr plant1 = scene->CreateGameObject("plant 1");
			{
				plant1->SetPostion(glm::vec3(-0.79f, 7.65f, 0.0f));
				plant1->SetRotation(glm::vec3(90.0f, 0.0f, 43.0f));
				plant1->SetScale(glm::vec3(0.4f, 0.4f, 0.4f));

				RenderComponent::Sptr renderer = plant1->Add<RenderComponent>();
				renderer->SetMesh(plantMesh);
				renderer->SetMaterial(plantMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = plant1->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.24f, 0.96f,0.65f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr fridge1 = scene->CreateGameObject("fridge 1");
			{
				fridge1->SetPostion(glm::vec3(16.44f, 6.81f, 0.0f));
				fridge1->SetRotation(glm::vec3(90.0f, 0.0f, -180.0f));
				fridge1->SetScale(glm::vec3(0.15f, 0.15f, 0.15f));

				RenderComponent::Sptr renderer = fridge1->Add<RenderComponent>();
				renderer->SetMesh(fridgeMesh);
				renderer->SetMaterial(fridgeMat);

				Gameplay::Physics::RigidBody::Sptr physics = fridge1->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetPosition(glm::vec3(-0.08f, 0.8f, 0.0f));
				box->SetScale(glm::vec3(0.32f, 1.31f, 0.5f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr fridge2 = scene->CreateGameObject("fridge 2");
			{
				fridge2->SetPostion(glm::vec3(16.41f, 4.2f, 0.0f));
				fridge2->SetRotation(glm::vec3(90.0f, 0.0f, -180.0f));
				fridge2->SetScale(glm::vec3(0.15f, 0.15f, 0.15f));

				RenderComponent::Sptr renderer = fridge2->Add<RenderComponent>();
				renderer->SetMesh(fridgeMesh);
				renderer->SetMaterial(fridgeMat);

				Gameplay::Physics::RigidBody::Sptr physics = fridge2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetPosition(glm::vec3(-0.08f, 0.8f, 0.0f));
				box->SetScale(glm::vec3(0.32f, 1.31f, 0.5f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr stove1 = scene->CreateGameObject("stove 1");
			{
				stove1->SetPostion(glm::vec3(14.32f, 7.03f, 0.0f));
				stove1->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				stove1->SetScale(glm::vec3(0.21f, 0.21f, 0.21f));

				RenderComponent::Sptr renderer = stove1->Add<RenderComponent>();
				renderer->SetMesh(stoveMesh);
				renderer->SetMaterial(stoveMat);

				Gameplay::Physics::RigidBody::Sptr physics = stove1->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.46f, 1.27f, 0.43f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr stove2 = scene->CreateGameObject("stove 2");
			{
				stove2->SetPostion(glm::vec3(14.32f, 5.9f, 0.0f));
				stove2->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				stove2->SetScale(glm::vec3(0.21, 0.21f, 0.21f));

				RenderComponent::Sptr renderer = stove2->Add<RenderComponent>();
				renderer->SetMesh(stoveMesh);
				renderer->SetMaterial(stoveMat);

				Gameplay::Physics::RigidBody::Sptr physics = stove2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.46f, 1.27f, 0.43f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr cashcounter2 = scene->CreateGameObject("CashCounter 2");
			{
				cashcounter2->SetPostion(glm::vec3(6.120f, 6.16f, 0.0f));
				cashcounter2->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
				cashcounter2->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				RenderComponent::Sptr renderer = cashcounter2->Add<RenderComponent>();
				renderer->SetMesh(cashMesh);
				renderer->SetMaterial(cashMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = cashcounter2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.520f, 0.740f, 1.210f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr cashcounter3 = scene->CreateGameObject("CashCounter 3");
			{
				cashcounter3->SetPostion(glm::vec3(11.41f, 9.8f, 0.0f));
				cashcounter3->SetRotation(glm::vec3(90.0f, 0.0f, -180.0f));
				cashcounter3->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				RenderComponent::Sptr renderer = cashcounter3->Add<RenderComponent>();
				renderer->SetMesh(cashMesh);
				renderer->SetMaterial(cashMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = cashcounter3->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.520f, 0.740f, 1.210f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr tablesqrt1 = scene->CreateGameObject("Square Table");
			{
				tablesqrt1->SetPostion(glm::vec3(7.68f, 9.48f, 0.0f));
				tablesqrt1->SetRotation(glm::vec3(90.0f, 0.0f, 45.0f));
				tablesqrt1->SetScale(glm::vec3(0.4f, 0.55f, 0.4f));

				RenderComponent::Sptr renderer = tablesqrt1->Add<RenderComponent>();
				renderer->SetMesh(sqrtableMesh);
				renderer->SetMaterial(sqrtableMat);

				Gameplay::Physics::RigidBody::Sptr physics = tablesqrt1->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.56f, 0.93f, 0.59f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr chair1 = scene->CreateGameObject("chair 1");
			{
				chair1->SetPostion(glm::vec3(7.15f, 10.01f, 0.0f));
				chair1->SetRotation(glm::vec3(90.0f, 0.0f, -45.0f));
				chair1->SetScale(glm::vec3(0.6f, 0.6f, 0.6f));

				RenderComponent::Sptr renderer = chair1->Add<RenderComponent>();
				renderer->SetMesh(dinerchairMesh);
				renderer->SetMaterial(dinerchairMat);

				Gameplay::Physics::RigidBody::Sptr physics = chair1->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetPosition(glm::vec3(-0.06f, 0.0f, 0.0f));
				box->SetScale(glm::vec3(0.3f, 1.07f, 0.24f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr chair2 = scene->CreateGameObject("chair 2");
			{
				chair2->SetPostion(glm::vec3(2.73f, 7.6f, 0.0f));
				chair2->SetRotation(glm::vec3(90.0f, 0.0f, -180.0f));
				chair2->SetScale(glm::vec3(0.6f, 0.6f, 0.6f));

				RenderComponent::Sptr renderer = chair2->Add<RenderComponent>();
				renderer->SetMesh(dinerchairMesh);
				renderer->SetMaterial(dinerchairMat);

				Gameplay::Physics::RigidBody::Sptr physics = chair2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetPosition(glm::vec3(-0.06f, 0.0f, 0.0f));
				box->SetScale(glm::vec3(0.3f, 1.07f, 0.24f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr chair3 = scene->CreateGameObject("chair 3");
			{
				chair3->SetPostion(glm::vec3(8.33f, 8.88f, 0.0f));
				chair3->SetRotation(glm::vec3(90.0f, 0.0f, 135.0f));
				chair3->SetScale(glm::vec3(0.6f, 0.6f, 0.6f));

				RenderComponent::Sptr renderer = chair3->Add<RenderComponent>();
				renderer->SetMesh(dinerchairMesh);
				renderer->SetMaterial(dinerchairMat);

				Gameplay::Physics::RigidBody::Sptr physics = chair3->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetPosition(glm::vec3(-0.06f, 0.0f, 0.0f));
				box->SetScale(glm::vec3(0.3f, 1.07f, 0.24f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr chair4 = scene->CreateGameObject("chair 4");
			{
				chair4->SetPostion(glm::vec3(0.92f, 7.56f, 0.0f));
				chair4->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				chair4->SetScale(glm::vec3(0.6f, 0.6f, 0.6f));

				RenderComponent::Sptr renderer = chair4->Add<RenderComponent>();
				renderer->SetMesh(dinerchairMesh);
				renderer->SetMaterial(dinerchairMat);

				Gameplay::Physics::RigidBody::Sptr physics = chair4->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetPosition(glm::vec3(-0.06f, 0.0f, 0.0f));
				box->SetScale(glm::vec3(0.3f, 1.07f, 0.24f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr chair5 = scene->CreateGameObject("chair 5");
			{
				chair5->SetPostion(glm::vec3(0.86f, 10.59f, 0.0f));
				chair5->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				chair5->SetScale(glm::vec3(0.6f, 0.6f, 0.6f));

				RenderComponent::Sptr renderer = chair5->Add<RenderComponent>();
				renderer->SetMesh(dinerchairMesh);
				renderer->SetMaterial(dinerchairMat);

				Gameplay::Physics::RigidBody::Sptr physics = chair5->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetPosition(glm::vec3(-0.06f, 0.0f, 0.0f));
				box->SetScale(glm::vec3(0.3f, 1.07f, 0.24f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr chair6 = scene->CreateGameObject("chair 6");
			{
				chair6->SetPostion(glm::vec3(2.730f, 10.62f, 0.0f));
				chair6->SetRotation(glm::vec3(90.0f, 0.0f, -180.0f));
				chair6->SetScale(glm::vec3(0.6f, 0.6f, 0.6f));

				RenderComponent::Sptr renderer = chair6->Add<RenderComponent>();
				renderer->SetMesh(dinerchairMesh);
				renderer->SetMaterial(dinerchairMat);

				Gameplay::Physics::RigidBody::Sptr physics = chair6->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetPosition(glm::vec3(-0.06f, 0.0f, 0.0f));
				box->SetScale(glm::vec3(0.3f, 1.07f, 0.24f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr poster1 = scene->CreateGameObject("poster 1");
			{
				poster1->SetPostion(glm::vec3(9.4f, 3.22f, 1.02f));
				poster1->SetRotation(glm::vec3(90.0f, 0.0f, -180.0f));
				poster1->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				RenderComponent::Sptr renderer = poster1->Add<RenderComponent>();
				renderer->SetMesh(posterMesh);
				renderer->SetMaterial(posterMat);
			}

			Gameplay::GameObject::Sptr poster2 = scene->CreateGameObject("poster 2");
			{
				poster2->SetPostion(glm::vec3(15.08f, 3.29f, 1.02f));
				poster2->SetRotation(glm::vec3(90.0f, 0.0f, -180.0f));
				poster2->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				RenderComponent::Sptr renderer = poster2->Add<RenderComponent>();
				renderer->SetMesh(posterMesh);
				renderer->SetMaterial(posterMat2);
			}

			BotLeft->AddChild(table1);
			BotLeft->AddChild(table2);
			BotLeft->AddChild(table3);
			BotLeft->AddChild(booth1);
			BotLeft->AddChild(booth2);
			BotLeft->AddChild(plant1);
			BotLeft->AddChild(fridge1);
			BotLeft->AddChild(fridge2);
			BotLeft->AddChild(stove1);
			BotLeft->AddChild(stove2);
			BotLeft->AddChild(cashcounter2);
			BotLeft->AddChild(cashcounter3);
			BotLeft->AddChild(tablesqrt1);
			BotLeft->AddChild(chair1);
			BotLeft->AddChild(chair2);
			BotLeft->AddChild(chair3);
			BotLeft->AddChild(chair4);
			BotLeft->AddChild(chair5);
			BotLeft->AddChild(chair6);
			BotLeft->AddChild(poster1);
			BotLeft->AddChild(poster2);

			//Top Left

			Gameplay::GameObject::Sptr TopLeft = scene->CreateGameObject("Top Left");

			Gameplay::GameObject::Sptr libshelf1 = scene->CreateGameObject("library shelf 1");
			{
				libshelf1->SetPostion(glm::vec3(13.63f, -4.91f, 0.0f));
				libshelf1->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				libshelf1->SetScale(glm::vec3(0.3f, 0.3f, 0.3f));

				RenderComponent::Sptr renderer = libshelf1->Add<RenderComponent>();
				renderer->SetMesh(libshelfMesh);
				renderer->SetMaterial(libshelfMat2);

				Gameplay::Physics::RigidBody::Sptr physics = libshelf1->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.38f, 1.88f, 0.89f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr libshelf2 = scene->CreateGameObject("library shelf 2");
			{
				libshelf2->SetPostion(glm::vec3(11.96f, -3.23f, 0.0f));
				libshelf2->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				libshelf2->SetScale(glm::vec3(0.3f, 0.1f, 0.3f));

				RenderComponent::Sptr renderer = libshelf2->Add<RenderComponent>();
				renderer->SetMesh(libshelfMesh);
				renderer->SetMaterial(libshelfMat);

				Gameplay::Physics::RigidBody::Sptr physics = libshelf2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.38f, 0.65f, 0.89f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr libshelf3 = scene->CreateGameObject("library shelf 3");
			{
				libshelf3->SetPostion(glm::vec3(6.75f, -9.41f, 0.0f));
				libshelf3->SetRotation(glm::vec3(90.0f, 0.0f, -180.0f));
				libshelf3->SetScale(glm::vec3(0.3f, 0.3f, 0.3));

				RenderComponent::Sptr renderer = libshelf3->Add<RenderComponent>();
				renderer->SetMesh(libshelfMesh);
				renderer->SetMaterial(libshelfMat);

				Gameplay::Physics::RigidBody::Sptr physics = libshelf3->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.38f, 1.88f, 0.89f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr libshelf4 = scene->CreateGameObject("library shelf 4");
			{
				libshelf4->SetPostion(glm::vec3(9.93f, -9.94f, 0.0f));
				libshelf4->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
				libshelf4->SetScale(glm::vec3(0.3f, 0.3f, 0.3f));

				RenderComponent::Sptr renderer = libshelf4->Add<RenderComponent>();
				renderer->SetMesh(libshelfMesh);
				renderer->SetMaterial(libshelfMat);

				Gameplay::Physics::RigidBody::Sptr physics = libshelf4->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.38f, 1.88f, 0.89f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr libshelf5 = scene->CreateGameObject("library shelf 5");
			{
				libshelf5->SetPostion(glm::vec3(11.910f, -7.82f, 0.0f));
				libshelf5->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
				libshelf5->SetScale(glm::vec3(0.3f, 0.3f, 0.3f));

				RenderComponent::Sptr renderer = libshelf5->Add<RenderComponent>();
				renderer->SetMesh(libshelfMesh);
				renderer->SetMaterial(libshelfMat2);

				Gameplay::Physics::RigidBody::Sptr physics = libshelf5->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.38f, 1.88f, 0.89f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr libshelf6 = scene->CreateGameObject("library shelf 6");
			{
				libshelf6->SetPostion(glm::vec3(9.70f, -5.94f, 0.0f));
				libshelf6->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				libshelf6->SetScale(glm::vec3(0.3f, 0.1f, 0.3f));

				RenderComponent::Sptr renderer = libshelf6->Add<RenderComponent>();
				renderer->SetMesh(libshelfMesh);
				renderer->SetMaterial(libshelfMat);

				Gameplay::Physics::RigidBody::Sptr physics = libshelf6->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.38f, 0.65f, 0.89f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr libshelf7 = scene->CreateGameObject("library shelf 7");
			{
				libshelf7->SetPostion(glm::vec3(9.74f, -3.24f, 0.0f));
				libshelf7->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				libshelf7->SetScale(glm::vec3(0.3f, 0.1f, 0.3f));

				RenderComponent::Sptr renderer = libshelf7->Add<RenderComponent>();
				renderer->SetMesh(libshelfMesh);
				renderer->SetMaterial(libshelfMat);

				Gameplay::Physics::RigidBody::Sptr physics = libshelf7->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.38f, 0.65f, 0.89f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr cashcounter4 = scene->CreateGameObject("CashCounter 4");
			{
				cashcounter4->SetPostion(glm::vec3(15.56f, -8.63f, 0.0f));
				cashcounter4->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				cashcounter4->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				RenderComponent::Sptr renderer = cashcounter4->Add<RenderComponent>();
				renderer->SetMesh(cashMesh);
				renderer->SetMaterial(cashMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = cashcounter4->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.520f, 0.740f, 1.210f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr loungechair1 = scene->CreateGameObject("Lounge Chair 1");
			{
				loungechair1->SetPostion(glm::vec3(10.62f, -7.84f, 0.0f));
				loungechair1->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
				loungechair1->SetScale(glm::vec3(0.3f, 0.3f, 0.3f));

				RenderComponent::Sptr renderer = loungechair1->Add<RenderComponent>();
				renderer->SetMesh(lchairMesh);
				renderer->SetMaterial(lchairMat);

				Gameplay::Physics::RigidBody::Sptr physics = loungechair1->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.48f, 1.08f, 0.46f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr loungechair2 = scene->CreateGameObject("Lounge Chair 2");
			{
				loungechair2->SetPostion(glm::vec3(8.7f, -7.79f, 0.0f));
				loungechair2->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				loungechair2->SetScale(glm::vec3(0.3f, 0.3f, 0.3f));

				RenderComponent::Sptr renderer = loungechair2->Add<RenderComponent>();
				renderer->SetMesh(lchairMesh);
				renderer->SetMaterial(lchairMat);

				Gameplay::Physics::RigidBody::Sptr physics = loungechair2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.48f, 1.08f, 0.46f));
				physics->AddCollider(box);
			}

			TopLeft->AddChild(libshelf1);
			TopLeft->AddChild(libshelf2);
			TopLeft->AddChild(libshelf3);
			TopLeft->AddChild(libshelf4);
			TopLeft->AddChild(libshelf5);
			TopLeft->AddChild(libshelf6);
			TopLeft->AddChild(libshelf7);
			TopLeft->AddChild(cashcounter4);
			TopLeft->AddChild(loungechair1);
			TopLeft->AddChild(loungechair2);

			// Bottom Right

			Gameplay::GameObject::Sptr BotRight = scene->CreateGameObject("Bottom Right");

			Gameplay::GameObject::Sptr toilet1 = scene->CreateGameObject("Toilet 1");
			{
				toilet1->SetPostion(glm::vec3(-7.65f, 3.75f, 0.0f));
				toilet1->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				toilet1->SetScale(glm::vec3(0.35f, 0.35f, 0.35f));

				RenderComponent::Sptr renderer = toilet1->Add<RenderComponent>();
				renderer->SetMesh(toiletMesh);
				renderer->SetMaterial(toiletMat);

				Gameplay::Physics::RigidBody::Sptr physics = toilet1->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetPosition(glm::vec3(0.0f, 0.0f, 0.08f));
				box->SetScale(glm::vec3(0.28f, 1.09f, 0.43f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr toilet2 = scene->CreateGameObject("Toilet 2");
			{
				toilet2->SetPostion(glm::vec3(-6.53f, 5.96f, 0.0f));
				toilet2->SetRotation(glm::vec3(90.0f, 0.0f, -180.0f));
				toilet2->SetScale(glm::vec3(0.35f, 0.35f, 0.35f));

				RenderComponent::Sptr renderer = toilet2->Add<RenderComponent>();
				renderer->SetMesh(toiletMesh);
				renderer->SetMaterial(toiletMat);

				Gameplay::Physics::RigidBody::Sptr physics = toilet2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetPosition(glm::vec3(0.0f, 0.0f, 0.08f));
				box->SetScale(glm::vec3(0.28f, 1.09f, 0.43f));
				physics->AddCollider(box);
			
			
			}

			Gameplay::GameObject::Sptr toilet3 = scene->CreateGameObject("Toilet 3");
			{
				toilet3->SetPostion(glm::vec3(-12.55f, 6.04f, 0.0f));
				toilet3->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
				toilet3->SetScale(glm::vec3(0.35f, 0.35f, 0.35f));

				RenderComponent::Sptr renderer = toilet3->Add<RenderComponent>();
				renderer->SetMesh(toiletMesh);
				renderer->SetMaterial(toiletMat);

				Gameplay::Physics::RigidBody::Sptr physics = toilet3->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetPosition(glm::vec3(0.0f, 0.0f, 0.08f));
				box->SetScale(glm::vec3(0.28f, 1.09f, 0.43f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr sink1 = scene->CreateGameObject("Sink 1");
			{
				sink1->SetPostion(glm::vec3(-10.01f, 0.0f, 0.0f));
				sink1->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				sink1->SetScale(glm::vec3(0.25f, 0.25f, 0.25f));

				RenderComponent::Sptr renderer = sink1->Add<RenderComponent>();
				renderer->SetMesh(sinkMesh);
				renderer->SetMaterial(sinkMat);

				Gameplay::Physics::RigidBody::Sptr physics = sink1->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.38f, 1.2f, 0.41f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr sink2 = scene->CreateGameObject("Sink 2");
			{
				sink2->SetPostion(glm::vec3(-11.08f, 0.0f, 0.0f));
				sink2->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				sink2->SetScale(glm::vec3(0.25f, 0.25f, 0.25f));

				RenderComponent::Sptr renderer = sink2->Add<RenderComponent>();
				renderer->SetMesh(sinkMesh);
				renderer->SetMaterial(sinkMat);

				Gameplay::Physics::RigidBody::Sptr physics = sink2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.38f, 1.2f, 0.41f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr sink3 = scene->CreateGameObject("Sink 3");
			{
				sink3->SetPostion(glm::vec3(-12.15f, 0.0f, 0.0f));
				sink3->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				sink3->SetScale(glm::vec3(0.25f, 0.25f, 0.25f));

				RenderComponent::Sptr renderer = sink3->Add<RenderComponent>();
				renderer->SetMesh(sinkMesh);
				renderer->SetMaterial(sinkMat);

				Gameplay::Physics::RigidBody::Sptr physics = sink3->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.38f, 1.2f, 0.41f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr tub1 = scene->CreateGameObject("Tub 1");
			{
				tub1->SetPostion(glm::vec3(-8.63f, 4.28f, 0.0f));
				tub1->SetRotation(glm::vec3(90.0f, 0.0f, -180.0f));
				tub1->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				RenderComponent::Sptr renderer = tub1->Add<RenderComponent>();
				renderer->SetMesh(tubMesh);
				renderer->SetMaterial(tubMat);

				Gameplay::Physics::RigidBody::Sptr physics = tub1->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.5f, 0.56f, 1.07f));
				physics->AddCollider(box);
			}
			
			Gameplay::GameObject::Sptr tub2 = scene->CreateGameObject("Tub 2");
			{
				tub2->SetPostion(glm::vec3(-11.75f, 1.8f, 0.0f));
				tub2->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				tub2->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				RenderComponent::Sptr renderer = tub2->Add<RenderComponent>();
				renderer->SetMesh(tubMesh);
				renderer->SetMaterial(tubMat);

				Gameplay::Physics::RigidBody::Sptr physics = tub2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.5f, 0.56f, 1.07f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr shower1 = scene->CreateGameObject("Shower 1");
			{
				shower1->SetPostion(glm::vec3(-12.49f, 4.57f, 0.0f));
				shower1->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				shower1->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				RenderComponent::Sptr renderer = shower1->Add<RenderComponent>();
				renderer->SetMesh(showerMesh);
				renderer->SetMaterial(showerMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = shower1->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.54f, 2.06f, 0.51f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr cashcounter5 = scene->CreateGameObject("CashCounter 5");
			{
				cashcounter5->SetPostion(glm::vec3(-15.56f, 1.28f, 0.0f));
				cashcounter5->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				cashcounter5->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				RenderComponent::Sptr renderer = cashcounter5->Add<RenderComponent>();
				renderer->SetMesh(cashMesh);
				renderer->SetMaterial(cashMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = cashcounter5->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.520f, 0.740f, 1.210f));
				physics->AddCollider(box);
			}

			Gameplay::GameObject::Sptr poster3 = scene->CreateGameObject("poster 3");
			{
				poster3->SetPostion(glm::vec3(-8.65, 3.21f, 0.88f));
				poster3->SetRotation(glm::vec3(90.0f, 0.0f, -180.0f));
				poster3->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				RenderComponent::Sptr renderer = poster3->Add<RenderComponent>();
				renderer->SetMesh(posterMesh);
				renderer->SetMaterial(posterMat3);
			}

			BotRight->AddChild(toilet1);
			BotRight->AddChild(toilet2);
			BotRight->AddChild(toilet3);
			BotRight->AddChild(sink1);
			BotRight->AddChild(sink2);
			BotRight->AddChild(sink3);
			BotRight->AddChild(tub1);
			BotRight->AddChild(tub2);
			BotRight->AddChild(shower1);
			BotRight->AddChild(cashcounter5);
			BotRight->AddChild(poster3);

			//Main Hall

			Gameplay::GameObject::Sptr MainHall = scene->CreateGameObject("Main Hall");

			Gameplay::GameObject::Sptr benchhall1 = scene->CreateGameObject("Bench 1");
			{
				benchhall1->SetPostion(glm::vec3(14.76f, -3.56f, 0.0f));
				benchhall1->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				benchhall1->SetScale(glm::vec3(1.21f, 1.0f, 1.5f));

				RenderComponent::Sptr renderer = benchhall1->Add<RenderComponent>();
				renderer->SetMesh(benchMesh);
				renderer->SetMaterial(benchMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = benchhall1->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr boxCollider = Gameplay::Physics::BoxCollider::Create();
				boxCollider->SetScale(glm::vec3(0.75f, 0.35f, 0.35f));
				physics->AddCollider(boxCollider);
			}

			Gameplay::GameObject::Sptr benchhall2 = scene->CreateGameObject("Bench 2");
			{
				benchhall2->SetPostion(glm::vec3(-9.67f, 8.22f, 0.0f));
				benchhall2->SetRotation(glm::vec3(90.0f, 0.0f, 107.0f));
				benchhall2->SetScale(glm::vec3(1.31f, 1.0f, 1.5f));

				RenderComponent::Sptr renderer = benchhall2->Add<RenderComponent>();
				renderer->SetMesh(benchMesh);
				renderer->SetMaterial(benchMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = benchhall2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr boxCollider = Gameplay::Physics::BoxCollider::Create();
				boxCollider->SetScale(glm::vec3(1.9f, 0.39f, 0.26f));
				physics->AddCollider(boxCollider);
			}

			Gameplay::GameObject::Sptr longfountain1 = scene->CreateGameObject("Long Fountain");
			{
				longfountain1->SetPostion(glm::vec3(10.77f, 0.0f, 0.0f));
				longfountain1->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				longfountain1->SetScale(glm::vec3(0.3f, 0.49f, 0.4f));

				RenderComponent::Sptr renderer = longfountain1->Add<RenderComponent>();
				renderer->SetMesh(longfountainMesh);
				renderer->SetMaterial(longfountainMat);

				Gameplay::Physics::RigidBody::Sptr physics = longfountain1->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr boxCollider = Gameplay::Physics::BoxCollider::Create();
				boxCollider->SetScale(glm::vec3(0.49f, 1.0f, 4.06f));
				physics->AddCollider(boxCollider);
			}

			Gameplay::GameObject::Sptr tallfountain1 = scene->CreateGameObject("Tall Fountain");
			{
				tallfountain1->SetPostion(glm::vec3(0.0f, -7.4f, 0.0f));
				tallfountain1->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				tallfountain1->SetScale(glm::vec3(0.8f, 0.8f, 0.8f));

				RenderComponent::Sptr renderer = tallfountain1->Add<RenderComponent>();
				renderer->SetMesh(tallfountainMesh);
				renderer->SetMaterial(tallfountainMat);

				Gameplay::Physics::RigidBody::Sptr physics = tallfountain1->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				//Gameplay::Physics::BoxCollider::Sptr boxCollider = Gameplay::Physics::BoxCollider::Create();
				Gameplay::Physics::CylinderCollider::Sptr boxCollider = Gameplay::Physics::CylinderCollider::Create();
				boxCollider->SetScale(glm::vec3(1.15f, 1.78f, 1.15f));
				physics->AddCollider(boxCollider);
			}

			MainHall->AddChild(benchhall1);
			MainHall->AddChild(benchhall2);
			MainHall->AddChild(longfountain1);
			MainHall->AddChild(tallfountain1);

		//Obstacles
		{

		Gameplay::GameObject::Sptr spillM = scene->CreateGameObject("Spill1");
		{
			spillM->SetPostion(glm::vec3(-11.792f, 3.142f, 0.03f));
			spillM->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
			spillM->SetScale(glm::vec3(0.790f, 0.7f, 1.f));
			// Add a render component
			RenderComponent::Sptr renderer = spillM->Add<RenderComponent>();
			renderer->SetMesh(spillMesh);
			renderer->SetMaterial(spillMaterial);
			// Add a dynamic rigid body to this monkey
			//Gameplay::Physics::RigidBody::Sptr physics = spillM->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			//box->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
			//box->SetScale(glm::vec3(0.22f, 0.37f, 0.24f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			box->SetScale(glm::vec3(0.75f, 0.001f, 0.53f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			//physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = spillM->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			//box2->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
			box2->SetScale(glm::vec3(1.f, 0.001f, 1.f));
			volume->AddCollider(box2);
			SpillBehaviour::Sptr behaviour = spillM->Add<SpillBehaviour>();


		}

		Gameplay::GameObject::Sptr spill2 = scene->CreateGameObject("Spill2");
		{
			spill2->SetPostion(glm::vec3(-14.500f, 7.044f, 0.03f));
			spill2->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			spill2->SetScale(glm::vec3(0.870f, 0.7f, 1.f));
			// Add a render component
			RenderComponent::Sptr renderer = spill2->Add<RenderComponent>();
			renderer->SetMesh(spillMesh);
			renderer->SetMaterial(spillMaterial);
			// Add a dynamic rigid body to this monkey
			//Gameplay::Physics::RigidBody::Sptr physics = spill2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			//Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			//box->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
			//box->SetScale(glm::vec3(0.22f, 0.37f, 0.24f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			//box->SetScale(glm::vec3(0.75f, 0.001f, 0.53f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			//physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = spill2->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			//box2->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
			box2->SetScale(glm::vec3(1.f, 0.001f, 1.f));
			volume->AddCollider(box2);
			SpillBehaviour::Sptr behaviour = spill2->Add<SpillBehaviour>();


		}

		Gameplay::GameObject::Sptr conveyor = scene->CreateGameObject("Conveyor");
		{
			conveyor->SetPostion(glm::vec3(-9.826f, 9.947f, 0.0f));
			conveyor->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
			conveyor->SetScale(glm::vec3(0.5f, 0.64f, 0.39f));

			RenderComponent::Sptr renderer = conveyor->Add<RenderComponent>();
			renderer->SetMesh(conveyorMesh);
			renderer->SetMaterial(conveyorMaterial);
			// Add a dynamic rigid body to this monkey
			//Gameplay::Physics::RigidBody::Sptr physics = conveyor->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			box->SetScale(glm::vec3(0.28f, 0.151f, 1.52f));
			//physics->AddCollider(box);
			Gameplay::Physics::TriggerVolume::Sptr volume = conveyor->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			box2->SetScale(glm::vec3(0.28f, 0.041f, 1.52f));
			box2->SetPosition(glm::vec3(0.0f, 0.13f, 0.0f));
			volume->AddCollider(box2);
			ConveyorBeltBehaviour::Sptr behaviour2 = conveyor->Add<ConveyorBeltBehaviour>();
		}

		Gameplay::GameObject::Sptr spill3 = scene->CreateGameObject("Spill3");
		{
			spill3->SetPostion(glm::vec3(-3.489f, 8.356f, 0.03f));
			spill3->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			spill3->SetScale(glm::vec3(1.25f, 0.7f, 1.f));
			// Add a render component
			RenderComponent::Sptr renderer = spill3->Add<RenderComponent>();
			renderer->SetMesh(spillMesh);
			renderer->SetMaterial(spillMaterial);
			// Add a dynamic rigid body to this monkey
			//Gameplay::Physics::RigidBody::Sptr physics = spill3->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			//box->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
			//box->SetScale(glm::vec3(0.22f, 0.37f, 0.24f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			box->SetScale(glm::vec3(0.75f, 0.001f, 0.53f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			//physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = spill3->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			//box2->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
			box2->SetScale(glm::vec3(1.f, 0.001f, 1.f));
			volume->AddCollider(box2);
			SpillBehaviour::Sptr behaviour = spill3->Add<SpillBehaviour>();


		}

		Gameplay::GameObject::Sptr conveyor2 = scene->CreateGameObject("Conveyor2");
		{
			conveyor2->SetPostion(glm::vec3(1.625f, 4.915f, 0.0f));
			conveyor2->SetRotation(glm::vec3(-90.0f, -180.0f, -90.0f));
			conveyor2->SetScale(glm::vec3(0.31f, 0.69f, 0.45f));

			RenderComponent::Sptr renderer = conveyor2->Add<RenderComponent>();
			renderer->SetMesh(conveyorMesh);
			renderer->SetMaterial(conveyorMaterial);
			// Add a dynamic rigid body to this monkey
			Gameplay::Physics::RigidBody::Sptr physics = conveyor2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			box->SetScale(glm::vec3(0.39f, 0.071f, 2.09f));
			box->SetPosition(glm::vec3(0.0f, 0.19f, 0.0f));
			physics->AddCollider(box);
			Gameplay::Physics::TriggerVolume::Sptr volume = conveyor2->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			box2->SetScale(glm::vec3(0.28f, 0.201f, 2.04f));
			box2->SetPosition(glm::vec3(0.0f, 0.19f, 0.0f));
			volume->AddCollider(box2);
			ConveyorBeltBehaviour::Sptr behaviour2 = conveyor2->Add<ConveyorBeltBehaviour>();
		}

		Gameplay::GameObject::Sptr spill4 = scene->CreateGameObject("Spill4");
		{
			spill4->SetPostion(glm::vec3(-4.328f, 1.298f, 0.03f));
			spill4->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			spill4->SetScale(glm::vec3(1.25f, 0.7f, 1.f));
			// Add a render component
			RenderComponent::Sptr renderer = spill4->Add<RenderComponent>();
			renderer->SetMesh(spillMesh);
			renderer->SetMaterial(spillMaterial);
			// Add a dynamic rigid body to this monkey
			//Gameplay::Physics::RigidBody::Sptr physics = spill4->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			//box->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
			//box->SetScale(glm::vec3(0.22f, 0.37f, 0.24f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			box->SetScale(glm::vec3(0.75f, 0.001f, 0.53f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			//physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = spill4->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			//box2->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
			box2->SetScale(glm::vec3(1.f, 0.001f, 1.f));
			volume->AddCollider(box2);
			SpillBehaviour::Sptr behaviour = spill4->Add<SpillBehaviour>();

		}

		Gameplay::GameObject::Sptr conveyor3 = scene->CreateGameObject("Conveyor3");
		{
			conveyor3->SetPostion(glm::vec3(-6.118f, -2.091f, 0.0f));
			conveyor3->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
			conveyor3->SetScale(glm::vec3(0.3f, 0.3f, 0.39f));

			RenderComponent::Sptr renderer = conveyor3->Add<RenderComponent>();
			renderer->SetMesh(conveyorMesh);
			renderer->SetMaterial(conveyorMaterial);
			// Add a dynamic rigid body to this monkey
			Gameplay::Physics::RigidBody::Sptr physics = conveyor3->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			box->SetScale(glm::vec3(0.28f, 0.151f, 1.52f));
			physics->AddCollider(box);
			Gameplay::Physics::TriggerVolume::Sptr volume = conveyor3->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			box2->SetScale(glm::vec3(0.28f, 0.041f, 1.52f));
			box2->SetPosition(glm::vec3(0.0f, 0.13f, 0.0f));
			volume->AddCollider(box2);
			ConveyorBeltBehaviour::Sptr behaviour2 = conveyor3->Add<ConveyorBeltBehaviour>();
		}

		Gameplay::GameObject::Sptr spill5 = scene->CreateGameObject("Spill5");
		{
			spill5->SetPostion(glm::vec3(2.946f, -2.231f, 0.03f));
			spill5->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			spill5->SetScale(glm::vec3(1.25f, 0.7f, 1.f));
			// Add a render component
			RenderComponent::Sptr renderer = spill5->Add<RenderComponent>();
			renderer->SetMesh(spillMesh);
			renderer->SetMaterial(spillMaterial);
			// Add a dynamic rigid body to this monkey
			//Gameplay::Physics::RigidBody::Sptr physics = spill5->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			//box->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
			//box->SetScale(glm::vec3(0.22f, 0.37f, 0.24f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			box->SetScale(glm::vec3(0.75f, 0.001f, 0.53f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			//physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = spill5->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			//box2->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
			box2->SetScale(glm::vec3(1.f, 0.001f, 1.f));
			volume->AddCollider(box2);
			SpillBehaviour::Sptr behaviour = spill5->Add<SpillBehaviour>();

		}

		Gameplay::GameObject::Sptr spill6 = scene->CreateGameObject("Spill6");
		{
			spill6->SetPostion(glm::vec3(14.24f, 4.38f, 0.03f));
			spill6->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			spill6->SetScale(glm::vec3(1.25f, 0.7f, 1.f));
			// Add a render component
			RenderComponent::Sptr renderer = spill6->Add<RenderComponent>();
			renderer->SetMesh(spillMesh);
			renderer->SetMaterial(spillMaterial);
			// Add a dynamic rigid body to this monkey
			//Gameplay::Physics::RigidBody::Sptr physics = spill6->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			//box->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
			//box->SetScale(glm::vec3(0.22f, 0.37f, 0.24f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			box->SetScale(glm::vec3(0.75f, 0.001f, 0.53f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			//physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = spill6->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			//box2->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
			box2->SetScale(glm::vec3(1.f, 0.001f, 1.f));
			volume->AddCollider(box2);
			SpillBehaviour::Sptr behaviour = spill6->Add<SpillBehaviour>();

		}

		Gameplay::GameObject::Sptr spill7 = scene->CreateGameObject("Spill7");
		{
			spill7->SetPostion(glm::vec3(3.22f, 9.304f, 0.03f));
			spill7->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			spill7->SetScale(glm::vec3(1.25f, 0.7f, 1.f));
			// Add a render component
			RenderComponent::Sptr renderer = spill7->Add<RenderComponent>();
			renderer->SetMesh(spillMesh);
			renderer->SetMaterial(spillMaterial);
			// Add a dynamic rigid body to this monkey
			//Gameplay::Physics::RigidBody::Sptr physics = spill7->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			//box->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
			//box->SetScale(glm::vec3(0.22f, 0.37f, 0.24f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			box->SetScale(glm::vec3(0.75f, 0.001f, 0.53f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			//physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = spill7->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			//box2->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
			box2->SetScale(glm::vec3(1.f, 0.001f, 1.f));
			volume->AddCollider(box2);
			SpillBehaviour::Sptr behaviour = spill7->Add<SpillBehaviour>();

		}

		Gameplay::GameObject::Sptr spill8 = scene->CreateGameObject("Spill8");
		{
			spill8->SetPostion(glm::vec3(10.791f, -4.363f, 0.03f));
			spill8->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			spill8->SetScale(glm::vec3(1.25f, 0.7f, 1.f));
			// Add a render component
			RenderComponent::Sptr renderer = spill8->Add<RenderComponent>();
			renderer->SetMesh(spillMesh);
			renderer->SetMaterial(spillMaterial);
			// Add a dynamic rigid body to this monkey
			//Gameplay::Physics::RigidBody::Sptr physics = spill8->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			//box->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
			//box->SetScale(glm::vec3(0.22f, 0.37f, 0.24f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			box->SetScale(glm::vec3(0.75f, 0.001f, 0.53f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			//physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = spill8->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			//box2->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
			box2->SetScale(glm::vec3(1.f, 0.001f, 1.f));
			volume->AddCollider(box2);
			SpillBehaviour::Sptr behaviour = spill8->Add<SpillBehaviour>();

		}

		Gameplay::GameObject::Sptr spill9 = scene->CreateGameObject("Spill9");
		{
			spill9->SetPostion(glm::vec3(-8.420, -7.542f, 0.03f));
			spill9->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			spill9->SetScale(glm::vec3(1.03f, 0.7f, 1.f));
			// Add a render component
			RenderComponent::Sptr renderer = spill9->Add<RenderComponent>();
			renderer->SetMesh(spillMesh);
			renderer->SetMaterial(spillMaterial);
			// Add a dynamic rigid body to this monkey
			//Gameplay::Physics::RigidBody::Sptr physics = spill9->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			//box->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
			//box->SetScale(glm::vec3(0.22f, 0.37f, 0.24f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			box->SetScale(glm::vec3(0.75f, 0.001f, 0.53f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			//physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = spill9->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			//box2->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
			box2->SetScale(glm::vec3(1.f, 0.001f, 1.f));
			volume->AddCollider(box2);
			SpillBehaviour::Sptr behaviour = spill9->Add<SpillBehaviour>();

		}

	}


		//set up robo toy
		/*Gameplay::MeshResource::Sptr roboMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("Robo/RoboWalk_000001.obj");
		Texture2D::Sptr roboTex = ResourceManager::CreateAsset<Texture2D>("textures/robo.png");
		Gameplay::Material::Sptr roboMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			roboMaterial->Name = "Robo";
			roboMaterial->Set("u_Material.AlbedoMap", roboTex);
			roboMaterial->Set("u_Material.Shininess", 0.5f);
			roboMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}
		Gameplay::GameObject::Sptr robo = scene->CreateGameObject("Robo");
		{
			robo->SetPostion(glm::vec3(0.0f, 0.0f, 0.0f));
			robo->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			robo->SetScale(glm::vec3(0.4f, 0.4f, 0.4f));

			RenderComponent::Sptr renderer = robo->Add<RenderComponent>();
			renderer->SetMesh(roboMesh);
			renderer->SetMaterial(roboMaterial);

			MorphMeshRenderer::Sptr morph1 = robo->Add<MorphMeshRenderer>();
			morph1->SetMorphMeshRenderer(roboMesh, roboMaterial);
			MorphAnimator::Sptr morph2 = robo->Add<MorphAnimator>();

			//moving

			Gameplay::MeshResource::Sptr roboMesh2 = ResourceManager::CreateAsset<Gameplay::MeshResource>("Robo/RoboWalk_000001.obj");
			Gameplay::MeshResource::Sptr roboMesh3 = ResourceManager::CreateAsset<Gameplay::MeshResource>("Robo/RoboWalk_000005.obj");
			Gameplay::MeshResource::Sptr roboMesh4 = ResourceManager::CreateAsset<Gameplay::MeshResource>("Robo/RoboWalk_000008.obj");
			Gameplay::MeshResource::Sptr roboMesh5 = ResourceManager::CreateAsset<Gameplay::MeshResource>("Robo/RoboWalk_000010.obj");
			Gameplay::MeshResource::Sptr roboMesh6 = ResourceManager::CreateAsset<Gameplay::MeshResource>("Robo/RoboWalk_000015.obj");
			Gameplay::MeshResource::Sptr roboMesh7 = ResourceManager::CreateAsset<Gameplay::MeshResource>("Robo/RoboWalk_000018.obj");
			Gameplay::MeshResource::Sptr roboMesh8 = ResourceManager::CreateAsset<Gameplay::MeshResource>("Robo/RoboWalk_000020.obj");

			std::vector<Gameplay::MeshResource::Sptr> frames;
			frames.push_back(roboMesh2);
			frames.push_back(roboMesh3);
			frames.push_back(roboMesh4);
			frames.push_back(roboMesh5);
			frames.push_back(roboMesh6);
			frames.push_back(roboMesh7);
			frames.push_back(roboMesh8);

			morph2->SetInitial();
			morph2->SetFrameTime(0.1f);
			morph2->SetFrames(frames);
		}
		//set up book
		Gameplay::MeshResource::Sptr bookMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("Book/AnimBook_000001.obj");
		Texture2D::Sptr bookTex = ResourceManager::CreateAsset<Texture2D>("textures/Book.png");
		Gameplay::Material::Sptr bookMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			bookMaterial->Name = "Book";
			bookMaterial->Set("u_Material.AlbedoMap", bookTex);
			bookMaterial->Set("u_Material.Shininess", 0.0f);
			bookMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}
		Gameplay::GameObject::Sptr book = scene->CreateGameObject("Book");
		{
			book->SetPostion(glm::vec3(1.0f, 1.0f, 0.0f));
			book->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
			book->SetScale(glm::vec3(0.4f, 0.4f, 0.4f));

			RenderComponent::Sptr renderer = book->Add<RenderComponent>();
			renderer->SetMesh(bookMesh);
			renderer->SetMaterial(bookMaterial);

			MorphMeshRenderer::Sptr morph1 = book->Add<MorphMeshRenderer>();
			morph1->SetMorphMeshRenderer(bookMesh, bookMaterial);
			MorphAnimator::Sptr morph2 = book->Add<MorphAnimator>();

			//moving

			Gameplay::MeshResource::Sptr bookMesh2 = ResourceManager::CreateAsset<Gameplay::MeshResource>("Book/AnimBook_000001.obj");
			Gameplay::MeshResource::Sptr bookMesh3 = ResourceManager::CreateAsset<Gameplay::MeshResource>("Book/AnimBook_000004.obj");
			Gameplay::MeshResource::Sptr bookMesh4 = ResourceManager::CreateAsset<Gameplay::MeshResource>("Book/AnimBook_000008.obj");
			Gameplay::MeshResource::Sptr bookMesh5 = ResourceManager::CreateAsset<Gameplay::MeshResource>("Book/AnimBook_000012.obj");
			Gameplay::MeshResource::Sptr bookMesh6 = ResourceManager::CreateAsset<Gameplay::MeshResource>("Book/AnimBook_000016.obj");

			std::vector<Gameplay::MeshResource::Sptr> frames;
			frames.push_back(bookMesh2);
			frames.push_back(bookMesh3);
			frames.push_back(bookMesh4);
			frames.push_back(bookMesh5);
			frames.push_back(bookMesh6);

			morph2->SetInitial();
			morph2->SetFrameTime(0.2f);
			morph2->SetFrames(frames);
		}
		//setup moving toy
		Gameplay::MeshResource::Sptr toyMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("toy.obj");
		Texture2D::Sptr toyTex = ResourceManager::CreateAsset<Texture2D>("textures/toy.jpg");
		// Create our material
		Gameplay::Material::Sptr toyMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			toyMaterial->Name = "Toy";
			toyMaterial->Set("u_Material.AlbedoMap", toyTex);
			toyMaterial->Set("u_Material.Shininess", 0.0f);
			toyMaterial->Set("u_Material.NormalMap", normalMapDefault);

		}
		// rolling toy collection
		{
			Gameplay::GameObject::Sptr toyM = scene->CreateGameObject("Toy");
			{
				toyM->SetPostion(glm::vec3(-0.19f, -4.25f, -0.63f));
				toyM->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				toyM->SetScale(glm::vec3(0.4f, 0.4f, 0.4f));
				// Add a render component
				RenderComponent::Sptr renderer = toyM->Add<RenderComponent>();
				renderer->SetMesh(toyMesh);
				renderer->SetMaterial(toyMaterial);
				// Add a dynamic rigid body to this monkey
				Gameplay::Physics::RigidBody::Sptr physics = toyM->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetPosition(glm::vec3(0.00f, 0.22f, 0.0f));
				box->SetScale(glm::vec3(0.28f * 0.4f, 0.24f * 0.4f, 0.57f * 0.4f));
				physics->AddCollider(box);
				Gameplay::Physics::TriggerVolume::Sptr volume = toyM->Add<Gameplay::Physics::TriggerVolume>();
				Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
				box2->SetPosition(glm::vec3(0.00f, 0.22f, 0.0f));
				box2->SetScale(glm::vec3(0.28f * 0.4f, 0.24f * 0.4f, 0.57f * 0.4f));
				volume->AddCollider(box2);
				SteeringBehaviour::Sptr behaviour2 = toyM->Add<SteeringBehaviour>();
				std::vector<glm::vec3> points;
				//points for catmull movement
				points.push_back(glm::vec3(-0.19f, -4.25f, -0.63f));
				points.push_back(glm::vec3(-7.18f, -4.25f, -0.63f));
				points.push_back(glm::vec3(-4.25f, -7.010f, -0.63f));
				points.push_back(glm::vec3(-0.67f, -7.32f, -0.63f));
				behaviour2->SetPoints(points);

				/////
				//ANIMATION STUFF////
				MorphMeshRenderer::Sptr morph1 = toyM->Add<MorphMeshRenderer>();
				morph1->SetMorphMeshRenderer(toyMesh, toyMaterial);
				MorphAnimator::Sptr morph2 = toyM->Add<MorphAnimator>();

				//moving

				Gameplay::MeshResource::Sptr toyMesh2 = ResourceManager::CreateAsset<Gameplay::MeshResource>("RollingToy/toyroll_000001.obj");
				Gameplay::MeshResource::Sptr toyMesh3 = ResourceManager::CreateAsset<Gameplay::MeshResource>("RollingToy/toyroll_000005.obj");
				Gameplay::MeshResource::Sptr toyMesh4 = ResourceManager::CreateAsset<Gameplay::MeshResource>("RollingToy/toyroll_000010.obj");
				Gameplay::MeshResource::Sptr toyMesh5 = ResourceManager::CreateAsset<Gameplay::MeshResource>("RollingToy/toyroll_000015.obj");
				Gameplay::MeshResource::Sptr toyMesh6 = ResourceManager::CreateAsset<Gameplay::MeshResource>("RollingToy/toyroll_000020.obj");

				std::vector<Gameplay::MeshResource::Sptr> frames;
				frames.push_back(toyMesh2);
				frames.push_back(toyMesh3);
				frames.push_back(toyMesh4);
				frames.push_back(toyMesh5);
				frames.push_back(toyMesh6);

				morph2->SetInitial();
				morph2->SetFrameTime(0.1f);
				morph2->SetFrames(frames);
			}
			Gameplay::GameObject::Sptr toyM2 = scene->CreateGameObject("Toy2");
			{
				toyM2->SetPostion(glm::vec3(2.00f, -4.25f, -0.63f));
				toyM2->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				toyM2->SetScale(glm::vec3(0.4f, 0.4f, 0.4f));
				// Add a render component
				RenderComponent::Sptr renderer = toyM2->Add<RenderComponent>();
				renderer->SetMesh(toyMesh);
				renderer->SetMaterial(toyMaterial);
				// Add a dynamic rigid body to this monkey
				Gameplay::Physics::RigidBody::Sptr physics = toyM2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetPosition(glm::vec3(0.00f, 0.22f, 0.0f));
				box->SetScale(glm::vec3(0.28f * 0.4f, 0.24f * 0.4f, 0.57f * 0.4f));
				physics->AddCollider(box);
				Gameplay::Physics::TriggerVolume::Sptr volume = toyM2->Add<Gameplay::Physics::TriggerVolume>();
				Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
				box2->SetPosition(glm::vec3(0.00f, 0.22f, 0.0f));
				box2->SetScale(glm::vec3(0.28f * 0.4f, 0.24f * 0.4f, 0.57f * 0.4f));
				volume->AddCollider(box2);
				FollowBehaviour::Sptr behaviour2 = toyM2->Add<FollowBehaviour>();
				behaviour2->SetTarget(toyM);

				//ANIMATION STUFF////
				MorphMeshRenderer::Sptr morph1 = toyM2->Add<MorphMeshRenderer>();
				morph1->SetMorphMeshRenderer(toyMesh, toyMaterial);
				MorphAnimator::Sptr morph2 = toyM2->Add<MorphAnimator>();

				//moving

				Gameplay::MeshResource::Sptr toyMesh2 = ResourceManager::CreateAsset<Gameplay::MeshResource>("RollingToy/toyroll_000001.obj");
				Gameplay::MeshResource::Sptr toyMesh3 = ResourceManager::CreateAsset<Gameplay::MeshResource>("RollingToy/toyroll_000005.obj");
				Gameplay::MeshResource::Sptr toyMesh4 = ResourceManager::CreateAsset<Gameplay::MeshResource>("RollingToy/toyroll_000010.obj");
				Gameplay::MeshResource::Sptr toyMesh5 = ResourceManager::CreateAsset<Gameplay::MeshResource>("RollingToy/toyroll_000015.obj");
				Gameplay::MeshResource::Sptr toyMesh6 = ResourceManager::CreateAsset<Gameplay::MeshResource>("RollingToy/toyroll_000020.obj");

				std::vector<Gameplay::MeshResource::Sptr> frames;
				frames.push_back(toyMesh2);
				frames.push_back(toyMesh3);
				frames.push_back(toyMesh4);
				frames.push_back(toyMesh5);
				frames.push_back(toyMesh6);

				morph2->SetInitial();
				morph2->SetFrameTime(0.1f);
				morph2->SetFrames(frames);

			}
		}

		//Conveyor
		Gameplay::MeshResource::Sptr conveyorMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("conveyor.obj");
		Texture2D::Sptr conveyorTex = ResourceManager::CreateAsset<Texture2D>("textures/conveyor.jpg");
		Gameplay::Material::Sptr conveyorMaterial = ResourceManager::CreateAsset<Gameplay::Material>(conveyorShader);
		{
			conveyorMaterial->Name = "Conveyor";
			conveyorMaterial->Set("u_Material.AlbedoMap", conveyorTex);
			conveyorMaterial->Set("u_Material.Shininess", 0.2f);
			conveyorMaterial->Set("u_Material.NormalMap", normalMapDefault);

		}
		Gameplay::GameObject::Sptr conveyor = scene->CreateGameObject("Conveyor");
		{
			conveyor->SetPostion(glm::vec3(-5.0f, 0.01f, 0.0f));
			conveyor->SetRotation(glm::vec3(90.0f, 0.0f, -75.0f));
			conveyor->SetScale(glm::vec3(0.3f, 0.3f, 0.3f));

			RenderComponent::Sptr renderer = conveyor->Add<RenderComponent>();
			renderer->SetMesh(conveyorMesh);
			renderer->SetMaterial(conveyorMaterial);
			// Add a dynamic rigid body to this monkey
			Gameplay::Physics::RigidBody::Sptr physics = conveyor->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			box->SetScale(glm::vec3(0.28f, 0.151f, 1.52f));
			physics->AddCollider(box);
			Gameplay::Physics::TriggerVolume::Sptr volume = conveyor->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			box2->SetScale(glm::vec3(0.28f, 0.041f, 1.52f));
			box2->SetPosition(glm::vec3(0.0f, 0.13f, 0.0f));
			volume->AddCollider(box2);
			ConveyorBeltBehaviour::Sptr behaviour2 = conveyor->Add<ConveyorBeltBehaviour>();
		}

		//spill object
		Gameplay::MeshResource::Sptr spillMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("spill.obj");
		Texture2D::Sptr spillTex = ResourceManager::CreateAsset<Texture2D>("textures/goo.png");
		// Create our material
		Gameplay::Material::Sptr spillMaterial = ResourceManager::CreateAsset<Gameplay::Material>(rackShader);
		{
			spillMaterial->Name = "Spill";
			spillMaterial->Set("u_Material.AlbedoMap", spillTex);
			spillMaterial->Set("u_Material.Shininess", 1.0f);
			spillMaterial->Set("u_Material.NormalMap", normalMapDefault);

		}
		//spill collection
		{
			Gameplay::GameObject::Sptr spillM = scene->CreateGameObject("Spill");
			{
				spillM->SetPostion(glm::vec3(4.15f, 0.39f, 0.03f));
				spillM->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				spillM->SetScale(glm::vec3(1.11f, 0.7f, 0.7f));
				// Add a render component
				RenderComponent::Sptr renderer = spillM->Add<RenderComponent>();
				renderer->SetMesh(spillMesh);
				renderer->SetMaterial(spillMaterial);
				// Add a dynamic rigid body to this monkey
				Gameplay::Physics::RigidBody::Sptr physics = spillM->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				//box->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
				//box->SetScale(glm::vec3(0.22f, 0.37f, 0.24f));
				//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
				box->SetScale(glm::vec3(0.75f, 0.001f, 0.53f));
				//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
				physics->AddCollider(box);
				//physics->SetMass(0.0f);
				Gameplay::Physics::TriggerVolume::Sptr volume = spillM->Add<Gameplay::Physics::TriggerVolume>();
				Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
				//box2->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
				box2->SetScale(glm::vec3(0.75f, 0.001f, 0.53f));
				volume->AddCollider(box2);
				SpillBehaviour::Sptr behaviour = spillM->Add<SpillBehaviour>();
				//give to our floor tiles to tag them
				//GroundBehaviour::Sptr behaviour2 = spillM->Add<GroundBehaviour>();

			}
			Gameplay::GameObject::Sptr spill2 = scene->CreateGameObject("Spill2");
			{
				spill2->SetPostion(glm::vec3(6.32f, -3.39f, 0.03f));
				spill2->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				spill2->SetScale(glm::vec3(1.35f, 0.7f, 0.7f));
				// Add a render component
				RenderComponent::Sptr renderer = spill2->Add<RenderComponent>();
				renderer->SetMesh(spillMesh);
				renderer->SetMaterial(spillMaterial);
				// Add a dynamic rigid body to this monkey
				Gameplay::Physics::RigidBody::Sptr physics = spill2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				//box->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
				//box->SetScale(glm::vec3(0.22f, 0.37f, 0.24f));
				//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
				box->SetScale(glm::vec3(1.19f, 0.001f, 0.53f));
				//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
				physics->AddCollider(box);
				//physics->SetMass(0.0f);
				Gameplay::Physics::TriggerVolume::Sptr volume = spill2->Add<Gameplay::Physics::TriggerVolume>();
				Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
				//box2->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
				box2->SetScale(glm::vec3(1.19f, 0.001f, 0.53f));
				volume->AddCollider(box2);
				SpillBehaviour::Sptr behaviour = spill2->Add<SpillBehaviour>();
				//give to our floor tiles to tag them
				//GroundBehaviour::Sptr behaviour2 = spill2->Add<GroundBehaviour>();
			}
			Gameplay::GameObject::Sptr spill3 = scene->CreateGameObject("Spill3");
			{
				spill3->SetPostion(glm::vec3(6.38, -0.68f, 0.03f));
				spill3->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				spill3->SetScale(glm::vec3(1.18f, 0.75f, 0.73f));
				// Add a render component
				RenderComponent::Sptr renderer = spill3->Add<RenderComponent>();
				renderer->SetMesh(spillMesh);
				renderer->SetMaterial(spillMaterial);
				// Add a dynamic rigid body to this monkey
				Gameplay::Physics::RigidBody::Sptr physics = spill3->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				//box->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
				//box->SetScale(glm::vec3(0.22f, 0.37f, 0.24f));
				//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
				box->SetScale(glm::vec3(1.09f, 0.001f, 0.53f));
				//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
				physics->AddCollider(box);
				//physics->SetMass(0.0f);
				Gameplay::Physics::TriggerVolume::Sptr volume = spill3->Add<Gameplay::Physics::TriggerVolume>();
				Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
				//box2->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
				box2->SetScale(glm::vec3(1.09f, 0.001f, 0.53));
				volume->AddCollider(box2);
				SpillBehaviour::Sptr behaviour = spill3->Add<SpillBehaviour>();
				//give to our floor tiles to tag them
				//GroundBehaviour::Sptr behaviour2 = spill3->Add<GroundBehaviour>();
			}
			Gameplay::GameObject::Sptr spill4 = scene->CreateGameObject("Spill4");
			{
				spill4->SetPostion(glm::vec3(12.41f, 0.94, 0.03f));
				spill4->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				spill4->SetScale(glm::vec3(0.7f, 0.7f, 1.43f));
				// Add a render component
				RenderComponent::Sptr renderer = spill4->Add<RenderComponent>();
				renderer->SetMesh(spillMesh);
				renderer->SetMaterial(spillMaterial);
				// Add a dynamic rigid body to this monkey
				Gameplay::Physics::RigidBody::Sptr physics = spill4->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				//box->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
				//box->SetScale(glm::vec3(0.22f, 0.37f, 0.24f));
				//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
				box->SetScale(glm::vec3(0.5f, 0.001f, 1.61f));
				//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
				physics->AddCollider(box);
				//physics->SetMass(0.0f);
				Gameplay::Physics::TriggerVolume::Sptr volume = spill4->Add<Gameplay::Physics::TriggerVolume>();
				Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
				//box2->SetPosition(glm::vec3(0.04f, 0.6f, 0.18f));
				box2->SetScale(glm::vec3(0.5f, 0.001f, 1.61f));
				volume->AddCollider(box2);
				SpillBehaviour::Sptr behaviour = spill4->Add<SpillBehaviour>();
				//give to our floor tiles to tag them
				//GroundBehaviour::Sptr behaviour2 = spill4->Add<GroundBehaviour>();
			}
		}
		*/
		//bin model
		Gameplay::MeshResource::Sptr binMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("BigBenClosed_000001.obj");
		Texture2D::Sptr binTex = ResourceManager::CreateAsset<Texture2D>("textures/bigben.png");
		// Create our material
		Gameplay::Material::Sptr binMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			binMaterial->Name = "Bin";
			binMaterial->Set("u_Material.AlbedoMap", binTex);
			binMaterial->Set("u_Material.Shininess", 1.0f);
			binMaterial->Set("u_Material.NormalMap", normalMapDefault);

		}

		Gameplay::GameObject::Sptr binM = scene->CreateGameObject("Bin");
		{
			binM->SetPostion(glm::vec3(-1.5f, 2.0f, 0.07f));
			binM->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
			binM->SetScale(glm::vec3(0.4f, 0.4f, 0.4f));
			// Add a render component
			RenderComponent::Sptr renderer = binM->Add<RenderComponent>();
			renderer->SetMesh(binMesh);
			renderer->SetMaterial(binMaterial);
			// Add a dynamic rigid body to this monkey
			Gameplay::Physics::RigidBody::Sptr physics = binM->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create(glm::vec3(2.0f, 2.23f, 4.25f));
			box->SetPosition(glm::vec3(0.0f, 0.4f, 0.0f));
			box->SetScale(glm::vec3(0.25f, 0.22f, 0.2f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			physics->AddCollider(box);
			//heavy
			//physics->SetMass(10.0f);

			Gameplay::Physics::TriggerVolume::Sptr volume = binM->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create(glm::vec3(2.0f, 2.23f, 4.25f));
			box2->SetPosition(glm::vec3(0.0f, 0.4f, 0.0f));
			box2->SetScale(glm::vec3(0.25f, 0.22f, 0.2f));
			volume->AddCollider(box2);
			SubmittingTrashBehaviour::Sptr behaviour2 = binM->Add<SubmittingTrashBehaviour>();

			//ANIMATION STUFF////
			MorphMeshRenderer::Sptr morph1 = binM->Add<MorphMeshRenderer>();
			morph1->SetMorphMeshRenderer(binMesh, binMaterial);
			MorphAnimator::Sptr morph2 = binM->Add<MorphAnimator>();

			//idle frames
			//std::vector <MeshResource::Sptr> frames2;
			Gameplay::MeshResource::Sptr binMesh8 = ResourceManager::CreateAsset<Gameplay::MeshResource>("BigBenClosed_000001.obj");
			std::vector<Gameplay::MeshResource::Sptr> closed;
			closed.push_back(binMesh8);
			behaviour2->getIdle(closed); //send idle frames to behaviour

			morph2->SetInitial();
			morph2->SetFrameTime(0.2f);
			morph2->SetFrames(closed);

		}
		Gameplay::GameObject::Sptr binM2 = scene->CreateGameObject("Bin Recycle");
		{
			binM2->SetPostion(glm::vec3(2.0f, 2.0f, 0.07f));
			binM2->SetRotation(glm::vec3(90.0f, 0.0f, 180.0f));
			binM2->SetScale(glm::vec3(0.4f, 0.4f, 0.4f));
			// Add a render component
			RenderComponent::Sptr renderer = binM2->Add<RenderComponent>();
			renderer->SetMesh(bin2Mesh);
			renderer->SetMaterial(bin2Material);
			// Add a dynamic rigid body to this monkey
			Gameplay::Physics::RigidBody::Sptr physics = binM2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create(glm::vec3(2.0f, 2.23f, 4.25f));
			box->SetPosition(glm::vec3(0.0f, 0.37f, 0.1f));
			box->SetScale(glm::vec3(0.2f, 0.19f, -0.08f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			physics->AddCollider(box);
			//heavy
			//physics->SetMass(10.0f);

			Gameplay::Physics::TriggerVolume::Sptr volume = binM2->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create(glm::vec3(2.0f, 2.23f, 4.25f));
			box2->SetPosition(glm::vec3(0.0f, 0.37f, 0.1f));
			box2->SetScale(glm::vec3(0.29f, 0.19f, -0.12f));

			volume->AddCollider(box2);
			SubmittingTrashBehaviour::Sptr behaviour2 = binM2->Add<SubmittingTrashBehaviour>();
			behaviour2->type = "Recycle";


		}
		//Shelf object
		/*Gameplay::MeshResource::Sptr shelfMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("shelf.obj");
		Texture2D::Sptr shelfTex = ResourceManager::CreateAsset <Texture2D>("textures/shelf.png");
		//Create Material
		Gameplay::Material::Sptr shelfMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			shelfMaterial->Name = "Shelf";
			shelfMaterial->Set("u_Material.AlbedoMap", shelfTex);
			shelfMaterial->Set("u_Material.Shininess", 0.2f);
			shelfMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}
		//shelf collection
		{
			Gameplay::GameObject::Sptr shelf = scene->CreateGameObject("Shelf1");
			{
				shelf->SetPostion(glm::vec3(4.250f, -4.690f, 0.0f));
				shelf->SetRotation(glm::vec3(90.0, 0.0f, 0.0f));
				shelf->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				RenderComponent::Sptr renderer = shelf->Add<RenderComponent>();
				renderer->SetMesh(shelfMesh);
				renderer->SetMaterial(shelfMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = shelf->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(1.04f, 1.62f, 1.54f));
				box->SetExtents(glm::vec3(0.5f, 1.0f, 1.0f));
				physics->AddCollider(box);
			}
			Gameplay::GameObject::Sptr shelf2 = scene->CreateGameObject("Shelf2");
			{
				shelf2->SetPostion(glm::vec3(6.20f, -5.69f, 0.0f));
				shelf2->SetRotation(glm::vec3(90.0, 0.0f, -90.0f));
				shelf2->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				RenderComponent::Sptr renderer = shelf2->Add<RenderComponent>();
				renderer->SetMesh(shelfMesh);
				renderer->SetMaterial(shelfMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = shelf2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(1.04f, 1.62f, 1.54f));
				box->SetExtents(glm::vec3(0.5f, 1.0f, 1.0f));
				physics->AddCollider(box);
			}
			Gameplay::GameObject::Sptr shelf3 = scene->CreateGameObject("Shelf3");
			{
				shelf3->SetPostion(glm::vec3(9.26f, -5.69f, 0.0f));
				shelf3->SetRotation(glm::vec3(90.0, 0.0f, -90.0f));
				shelf3->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				RenderComponent::Sptr renderer = shelf3->Add<RenderComponent>();
				renderer->SetMesh(shelfMesh);
				renderer->SetMaterial(shelfMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = shelf3->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(1.04f, 1.62f, 1.54f));
				box->SetExtents(glm::vec3(0.5f, 1.0f, 1.0f));
				physics->AddCollider(box);
			}
			Gameplay::GameObject::Sptr shelf4 = scene->CreateGameObject("Shelf4");
			{
				shelf4->SetPostion(glm::vec3(7.9f, 0.1f, 0.0f));
				shelf4->SetRotation(glm::vec3(90.0, 0.0f, 0.0f));
				shelf4->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				RenderComponent::Sptr renderer = shelf4->Add<RenderComponent>();
				renderer->SetMesh(shelfMesh);
				renderer->SetMaterial(shelfMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = shelf4->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(1.04f, 1.62f, 1.54f));
				box->SetExtents(glm::vec3(0.5f, 1.0f, 1.0f));
				physics->AddCollider(box);
			}
			Gameplay::GameObject::Sptr shelf5 = scene->CreateGameObject("Shelf5");
			{
				shelf5->SetPostion(glm::vec3(8.910f, 0.1f, 0.0f));
				shelf5->SetRotation(glm::vec3(90.0, 0.0f, -180.0f));
				shelf5->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				RenderComponent::Sptr renderer = shelf5->Add<RenderComponent>();
				renderer->SetMesh(shelfMesh);
				renderer->SetMaterial(shelfMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = shelf5->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(1.04f, 1.62f, 1.54f));
				box->SetExtents(glm::vec3(0.5f, 1.0f, 1.0f));
				physics->AddCollider(box);
			}
		}
		//tv box object
		Gameplay::MeshResource::Sptr tvboxMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("tvbox2.obj");
		Texture2D::Sptr tvboxTex = ResourceManager::CreateAsset<Texture2D>("textures/tvbox.png");
		//Create Material
		Gameplay::Material::Sptr tvboxMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			tvboxMaterial->Name = "TvBox";
			tvboxMaterial->Set("u_Material.AlbedoMap", tvboxTex);
			tvboxMaterial->Set("u_Material.Shininess", 0.0f);
			tvboxMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}
		//tv box collection
		{
			Gameplay::GameObject::Sptr tvbox1 = scene->CreateGameObject("tvbox1");
			{
				tvbox1->SetPostion(glm::vec3(8.37f, 2.1f, 0.0f));
				tvbox1->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				tvbox1->SetScale(glm::vec3(0.8f, 0.8f, 0.8f));

				RenderComponent::Sptr renderer = tvbox1->Add<RenderComponent>();
				renderer->SetMesh(tvboxMesh);
				renderer->SetMaterial(tvboxMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = tvbox1->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.1f, 0.98f, 0.82f));
				box->SetExtents(glm::vec3(1.7f, 1.0f, 1.0f));
				physics->AddCollider(box);

			}
			Gameplay::GameObject::Sptr tvbox2 = scene->CreateGameObject("tvbox2");
			{
				tvbox2->SetPostion(glm::vec3(8.37f, 2.51f, 0.0f));
				tvbox2->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				tvbox2->SetScale(glm::vec3(0.8f, 0.8f, 0.8f));

				RenderComponent::Sptr renderer = tvbox2->Add<RenderComponent>();
				renderer->SetMesh(tvboxMesh);
				renderer->SetMaterial(tvboxMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = tvbox2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.1f, 0.98f, 0.82f));
				box->SetExtents(glm::vec3(1.7f, 1.0f, 1.0f));
				physics->AddCollider(box);
			}
			Gameplay::GameObject::Sptr tvbox3 = scene->CreateGameObject("tvbox3");
			{
				tvbox3->SetPostion(glm::vec3(8.37f, 2.92f, 0.0f));
				tvbox3->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				tvbox3->SetScale(glm::vec3(0.8f, 0.8f, 0.8f));

				RenderComponent::Sptr renderer = tvbox3->Add<RenderComponent>();
				renderer->SetMesh(tvboxMesh);
				renderer->SetMaterial(tvboxMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = tvbox3->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.1f, 0.98f, 0.82f));
				box->SetExtents(glm::vec3(1.7f, 1.0f, 1.0f));
				physics->AddCollider(box);

			}
			Gameplay::GameObject::Sptr tvbox4 = scene->CreateGameObject("tvbox4");
			{
				tvbox4->SetPostion(glm::vec3(5.09f, -0.76f, 0.0f));
				tvbox4->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				tvbox4->SetScale(glm::vec3(0.8f, 0.8f, 0.8f));

				RenderComponent::Sptr renderer = tvbox4->Add<RenderComponent>();
				renderer->SetMesh(tvboxMesh);
				renderer->SetMaterial(tvboxMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = tvbox4->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.1f, 0.98f, 0.82f));
				box->SetExtents(glm::vec3(1.7f, 1.0f, 1.0f));
				physics->AddCollider(box);
			}
			Gameplay::GameObject::Sptr tvbox5 = scene->CreateGameObject("tvbox5");
			{
				tvbox5->SetPostion(glm::vec3(5.09f, 0.98f, 0.0f));
				tvbox5->SetRotation(glm::vec3(90.0f, 0.0f, -180.0f));
				tvbox5->SetScale(glm::vec3(0.8f, 0.8f, 0.8f));

				RenderComponent::Sptr renderer = tvbox5->Add<RenderComponent>();
				renderer->SetMesh(tvboxMesh);
				renderer->SetMaterial(tvboxMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = tvbox5->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.1f, 0.98f, 0.82f));
				box->SetExtents(glm::vec3(1.7f, 1.0f, 1.0f));
				physics->AddCollider(box);
			}
			Gameplay::GameObject::Sptr tvbox6 = scene->CreateGameObject("tvbox6");
			{
				tvbox6->SetPostion(glm::vec3(9.97f, -3.45f, 0.0f));
				tvbox6->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				tvbox6->SetScale(glm::vec3(0.8f, 0.8f, 0.8f));

				RenderComponent::Sptr renderer = tvbox6->Add<RenderComponent>();
				renderer->SetMesh(tvboxMesh);
				renderer->SetMaterial(tvboxMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = tvbox6->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.1f, 0.98f, 0.82f));
				box->SetExtents(glm::vec3(1.7f, 1.0f, 1.0f));
				physics->AddCollider(box);
			}
			Gameplay::GameObject::Sptr tvbox7 = scene->CreateGameObject("tvbox7");
			{
				tvbox7->SetPostion(glm::vec3(8.31f, -3.45f, 0.0f));
				tvbox7->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
				tvbox7->SetScale(glm::vec3(0.8f, 0.8f, 0.8f));

				RenderComponent::Sptr renderer = tvbox7->Add<RenderComponent>();
				renderer->SetMesh(tvboxMesh);
				renderer->SetMaterial(tvboxMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = tvbox7->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.1f, 0.98f, 0.82f));
				box->SetExtents(glm::vec3(1.7f, 1.0f, 1.0f));
				physics->AddCollider(box);
			}
		}
		//tv object
		Gameplay::MeshResource::Sptr tvMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("TV.obj");
		Texture2D::Sptr tvTex = ResourceManager::CreateAsset<Texture2D>("textures/tvtex.jpg");
		//Create Material
		Gameplay::Material::Sptr tvMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			tvMaterial->Name = "Tv";
			tvMaterial->Set("u_Material.AlbedoMap", tvTex);
			tvMaterial->Set("u_Material.Shininess", 0.75f);
			tvMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}
		//tv collection
		{
			Gameplay::GameObject::Sptr tv1 = scene->CreateGameObject("tv1");
			{
				tv1->SetPostion(glm::vec3(2.47f, 0.91f, 0.0f));
				tv1->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
				tv1->SetScale(glm::vec3(0.4f, 0.4f, 0.4f));

				RenderComponent::Sptr renderer = tv1->Add<RenderComponent>();
				renderer->SetMesh(tvMesh);
				renderer->SetMaterial(tvMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = tv1->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.15f, 1.01f, 0.84f));
				physics->AddCollider(box);
			}
			Gameplay::GameObject::Sptr tv2 = scene->CreateGameObject("tv2");
			{
				tv2->SetPostion(glm::vec3(4.27f, -2.89f, 0.0f));
				tv2->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				tv2->SetScale(glm::vec3(0.4f, 0.4f, 0.4f));

				RenderComponent::Sptr renderer = tv2->Add<RenderComponent>();
				renderer->SetMesh(tvMesh);
				renderer->SetMaterial(tvMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = tv2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.15f, 1.01f, 0.84f));
				physics->AddCollider(box);
			}
			Gameplay::GameObject::Sptr tv3 = scene->CreateGameObject("tv3");
			{
				tv3->SetPostion(glm::vec3(13.31f, 2.27f, 0.0f));
				tv3->SetRotation(glm::vec3(90.0f, 0.0f, -152.0f));
				tv3->SetScale(glm::vec3(0.4f, 0.4f, 0.4f));

				RenderComponent::Sptr renderer = tv3->Add<RenderComponent>();
				renderer->SetMesh(tvMesh);
				renderer->SetMaterial(tvMaterial);

				Gameplay::Physics::RigidBody::Sptr physics = tv3->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.15f, 1.01f, 0.84f));
				physics->AddCollider(box);
			}
		}
		//rectangle and trashy images for menu
		//cash counter
		Gameplay::MeshResource::Sptr cashMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("cashcounter.obj");
		Texture2D::Sptr cashTex = ResourceManager::CreateAsset<Texture2D>("textures/cash.png");
		//create Material
		Gameplay::Material::Sptr cashMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			cashMaterial->Name = "Cash";
			cashMaterial->Set("u_Material.AlbedoMap", cashTex);
			cashMaterial->Set("u_Material.Shininess", 0.75f);
			cashMaterial->Set("u_Material.NormalMap", normalMapDefault);
		}
		//cash counter
		Gameplay::GameObject::Sptr cashcounter = scene->CreateGameObject("CashCounter");
		{
			cashcounter->SetPostion(glm::vec3(12.3f, -1.79f, 0.0f));
			cashcounter->SetRotation(glm::vec3(90.0f, 0.0f, -180.0f));
			cashcounter->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

			RenderComponent::Sptr renderer = cashcounter->Add<RenderComponent>();
			renderer->SetMesh(cashMesh);
			renderer->SetMaterial(cashMaterial);

			Gameplay::Physics::RigidBody::Sptr physics = cashcounter->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Static);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			box->SetScale(glm::vec3(0.520f, 0.740f, 1.210f));
			physics->AddCollider(box);
		}
		*/
		Gameplay::MeshResource::Sptr recMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("RecOBJ.obj");
		Texture2D::Sptr recTex = ResourceManager::CreateAsset<Texture2D>("textures/Rec1.png");
		// Create our material
		Gameplay::Material::Sptr recMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			recMaterial->Name = "Rec";
			recMaterial->Set("u_Material.AlbedoMap", recTex);
			recMaterial->Set("u_Material.Shininess", 1.0f);
			recMaterial->Set("u_Material.NormalMap", normalMapDefault);

		}
		Gameplay::GameObject::Sptr recE = scene->CreateGameObject("Rec");
		{
			recE->SetPostion(glm::vec3(-1.36f, 1.22f, 7.0f));
			recE->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
			recE->SetScale(glm::vec3(1.0f, 5.8f, 4.8f));
			// Add a render component
			RenderComponent::Sptr renderer = recE->Add<RenderComponent>();

			renderer->SetMesh(recMesh);
			renderer->SetMaterial(recMaterial);
			//Gameplay::Physics::RigidBody::Sptr physics = recE->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);

		}
		Gameplay::MeshResource::Sptr trashyEMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("trashy2OBJ.obj");
		Texture2D::Sptr trashyETex = ResourceManager::CreateAsset<Texture2D>("textures/Trashy2.png");
		// Create our material
		Gameplay::Material::Sptr trashyEMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
		{
			trashyEMaterial->Name = "trashyE";
			trashyEMaterial->Set("u_Material.AlbedoMap", trashyETex);
			trashyEMaterial->Set("u_Material.Shininess", 1.0f);
			trashyEMaterial->Set("u_Material.NormalMap", normalMapDefault);

		}
		Gameplay::GameObject::Sptr trashyE = scene->CreateGameObject("TrashyE");
		{
			trashyE->SetPostion(glm::vec3(0.8f, 1.49f, 3.3f));
			trashyE->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
			trashyE->SetScale(glm::vec3(1.0f, 1.46f, 1.090f));
			// Add a render component
			RenderComponent::Sptr renderer = trashyE->Add<RenderComponent>();
			renderer->SetMesh(trashyEMesh);
			renderer->SetMaterial(trashyEMaterial);
			//Gameplay::Physics::RigidBody::Sptr physics = trashyE->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);

		}
		
		/*//Modelling Static objects
		{
			//Bench
			Gameplay::MeshResource::Sptr benchMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("bench.obj");
			Texture2D::Sptr benchTex = ResourceManager::CreateAsset<Texture2D>("textures/bench.jpg");
			Gameplay::Material::Sptr benchMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
			{
				benchMaterial->Name = "Bench";
				benchMaterial->Set("u_Material.AlbedoMap", benchTex);
				benchMaterial->Set("u_Material.Shininess", 0.0f);
				benchMaterial->Set("u_Material.NormalMap", normalMapDefault);
			}
			Gameplay::GameObject::Sptr bench = scene->CreateGameObject("Bench");
			{
				bench->SetPostion(glm::vec3(-0.02f, -2.23f, 0.0f));
				bench->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
				bench->SetScale(glm::vec3(1.0f, 1.0f, 1.0f));

				RenderComponent::Sptr renderer = bench->Add<RenderComponent>();
				renderer->SetMesh(benchMesh);
				renderer->SetMaterial(benchMaterial);
			}
			//Cash Counter
			Gameplay::GameObject::Sptr cashModel = scene->CreateGameObject("cash Modelling");
			{
				cashModel->SetPostion(glm::vec3(-6.34f, -1.61f, 0.0f));
				cashModel->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
				cashModel->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				RenderComponent::Sptr renderer = cashModel->Add<RenderComponent>();
				renderer->SetMesh(cashMesh);
				renderer->SetMaterial(cashMaterial);

			}
			//Computer
			Gameplay::MeshResource::Sptr computerMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("Computer.obj");
			Texture2D::Sptr computerTex = ResourceManager::CreateAsset<Texture2D>("textures/desktoptex.jpg");
			Gameplay::Material::Sptr computerMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
			{
				computerMaterial->Name = "Computer";
				computerMaterial->Set("u_Material.AlbedoMap", computerTex);
				computerMaterial->Set("u_Material.Shininess", 0.3f);
				computerMaterial->Set("u_Material.NormalMap", normalMapDefault);
			}
			Gameplay::GameObject::Sptr computer = scene->CreateGameObject("Desktop");
			{
				computer->SetPostion(glm::vec3(-2.61f, -1.08f, 0.0f));
				computer->SetRotation(glm::vec3(90.0f, 0.0f, 59.0f));
				computer->SetScale(glm::vec3(0.3f, 0.3f, 0.3f));

				RenderComponent::Sptr renderer = computer->Add<RenderComponent>();
				renderer->SetMesh(computerMesh);
				renderer->SetMaterial(computerMaterial);
			}
			//Conveyor
			Gameplay::MeshResource::Sptr conveyorMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("conveyor.obj");
			Texture2D::Sptr conveyorTex = ResourceManager::CreateAsset<Texture2D>("textures/conayortex.png");
			Gameplay::Material::Sptr conveyorMaterial = ResourceManager::CreateAsset<Gameplay::Material>(conveyorShader);
			{
				conveyorMaterial->Name = "Conveyor";
				conveyorMaterial->Set("u_Material.AlbedoMap", conveyorTex);
				conveyorMaterial->Set("u_Material.Shininess", 0.2f);
				conveyorMaterial->Set("u_Material.NormalMap", normalMapDefault);
				
			}
			Gameplay::GameObject::Sptr conveyor = scene->CreateGameObject("Conveyor");
			{
				conveyor->SetPostion(glm::vec3(-5.0f, 0.01f, 0.0f));
				conveyor->SetRotation(glm::vec3(90.0f, 0.0f, -75.0f));
				conveyor->SetScale(glm::vec3(0.3f, 0.3f, 0.3f));

				RenderComponent::Sptr renderer = conveyor->Add<RenderComponent>();
				renderer->SetMesh(conveyorMesh);
				renderer->SetMaterial(conveyorMaterial);
				// Add a dynamic rigid body to this monkey
				Gameplay::Physics::RigidBody::Sptr physics = conveyor->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(0.28f, 0.151f, 1.52f));
				physics->AddCollider(box);
				Gameplay::Physics::TriggerVolume::Sptr volume = conveyor->Add<Gameplay::Physics::TriggerVolume>();
				Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
				box2->SetScale(glm::vec3(0.28f, 0.041f, 1.52f));
				box2->SetPosition(glm::vec3(0.0f, 0.13f, 0.0f));
				volume->AddCollider(box2);
				ConveyorBeltBehaviour::Sptr behaviour2 = conveyor->Add<ConveyorBeltBehaviour>();
			}
			//Cup
			Gameplay::GameObject::Sptr cupModel = scene->CreateGameObject("cup Modelling");
			{
				cupModel->SetPostion(glm::vec3(-3.76f, -1.52f, 0.0f));
				cupModel->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
				cupModel->SetScale(glm::vec3(0.4f, 0.4f, 0.4f));

				RenderComponent::Sptr renderer = cupModel->Add<RenderComponent>();
				renderer->SetMesh(trashMesh);
				renderer->SetMaterial(trashMaterial);
			}
			//Mirror
			Gameplay::MeshResource::Sptr mirrorMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("Mirror.obj");
			Texture2D::Sptr mirrorTex = ResourceManager::CreateAsset<Texture2D>("textures/mirror.jpg");
			Gameplay::Material::Sptr mirrorMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
			{
				mirrorMaterial->Name = "Mirror";
				mirrorMaterial->Set("u_Material.AlbedoMap", mirrorTex);
				mirrorMaterial->Set("u_Material.Shininess", 0.6f);
				mirrorMaterial->Set("u_Material.NormalMap", normalMapDefault);
			}
			Gameplay::GameObject::Sptr mirror = scene->CreateGameObject("Mirror");
			{
				mirror->SetPostion(glm::vec3(-12.1f, -2.65f, 0.0f));
				mirror->SetRotation(glm::vec3(90.0f, -2.0f, 93.0f));
				mirror->SetScale(glm::vec3(0.4f, 0.4f, 0.4f));

				RenderComponent::Sptr renderer = mirror->Add<RenderComponent>();
				renderer->SetMesh(mirrorMesh);
				renderer->SetMaterial(mirrorMaterial);
			}
			//Plant
			Gameplay::MeshResource::Sptr plantMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("plant.obj");
			Texture2D::Sptr plantTex = ResourceManager::CreateAsset<Texture2D>("textures/planttex.png");
			Gameplay::Material::Sptr plantMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
			{
				plantMaterial->Name = "Plant";
				plantMaterial->Set("u_Material.AlbedoMap", plantTex);
				plantMaterial->Set("u_Material.Shininess", 0.0f);
				plantMaterial->Set("u_Material.NormalMap", normalMapDefault);
			}
			Gameplay::GameObject::Sptr plant = scene->CreateGameObject("Plant");
			{
				plant->SetPostion(glm::vec3(-3.17f, -1.51f, 0.0f));
				plant->SetRotation(glm::vec3(90.0f, 0.0f, 72.0f));
				plant->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				RenderComponent::Sptr renderer = plant->Add<RenderComponent>();
				renderer->SetMesh(plantMesh);
				renderer->SetMaterial(plantMaterial);
			}
			//Rack
			Gameplay::MeshResource::Sptr rackMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("rack.obj");
			Texture2D::Sptr rackTex = ResourceManager::CreateAsset<Texture2D>("textures/rack.png");
			Gameplay::Material::Sptr rackMaterial = ResourceManager::CreateAsset<Gameplay::Material>(rackShader); //CHANGE THIS SHADER
			{
				rackMaterial->Name = "Rack";
				rackMaterial->Set("u_Material.AlbedoMap", rackTex);
				rackMaterial->Set("u_Material.Shininess", 0.2f);
				rackMaterial->Set("u_Material.NormalMap", normalMapDefault);
			}
			Gameplay::GameObject::Sptr rack = scene->CreateGameObject("Rack");
			{
				rack->SetPostion(glm::vec3(-1.64f, -1.97f, 0.0f));
				rack->SetRotation(glm::vec3(90.0f, 0.0f, 63.0f));
				rack->SetScale(glm::vec3(0.3f, 0.3f, 0.3f));

				RenderComponent::Sptr renderer = rack->Add<RenderComponent>();
				renderer->SetMesh(rackMesh);
				renderer->SetMaterial(rackMaterial);
			}
			//Shelf
			Gameplay::GameObject::Sptr shelfModel = scene->CreateGameObject("Shelf Modelling");
			{
				shelfModel->SetPostion(glm::vec3(-9.54f, -1.87f, 0.0f));
				shelfModel->SetRotation(glm::vec3(90.0f, 0.0f, -90.0f));
				shelfModel->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				RenderComponent::Sptr renderer = shelfModel->Add<RenderComponent>();
				renderer->SetMesh(shelfMesh);
				renderer->SetMaterial(shelfMaterial);
			}
			//Shower
			Gameplay::MeshResource::Sptr showerMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("shower.obj");
			Texture2D::Sptr showerTex = ResourceManager::CreateAsset<Texture2D>("textures/shower.png");
			Gameplay::Material::Sptr showerMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
			{
				showerMaterial->Name = "Shower";
				showerMaterial->Set("u_Material.AlbedoMap", showerTex);
				showerMaterial->Set("u_Material.Shininess", 0.5f);
				showerMaterial->Set("u_Material.NormalMap", normalMapDefault);
			}
			Gameplay::GameObject::Sptr shower = scene->CreateGameObject("Shower");
			{
				shower->SetPostion(glm::vec3(-4.5f, -1.84f, 0.0f));
				shower->SetRotation(glm::vec3(90.0f, 0.0f, -180.0f));
				shower->SetScale(glm::vec3(0.4f, 0.4f, 0.4f));

				RenderComponent::Sptr renderer = shower->Add<RenderComponent>();
				renderer->SetMesh(showerMesh);
				renderer->SetMaterial(showerMaterial);
			}
			//Sink
			Gameplay::MeshResource::Sptr sinkMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("Sink.obj");
			Texture2D::Sptr sinkTex = ResourceManager::CreateAsset<Texture2D>("textures/sinktex.png");
			Gameplay::Material::Sptr sinkMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
			{
				sinkMaterial->Name = "Sink";
				sinkMaterial->Set("u_Material.AlbedoMap", sinkTex);
				sinkMaterial->Set("u_Material.Shininess", 0.5f);
				sinkMaterial->Set("u_Material.NormalMap", normalMapDefault);
			}
			Gameplay::GameObject::Sptr sink = scene->CreateGameObject("Sink");
			{
				sink->SetPostion(glm::vec3(-3.47f, -3.14f, 0.0f));
				sink->SetRotation(glm::vec3(90.0f, 0.0f, -34.0f));
				sink->SetScale(glm::vec3(0.25f, 0.25, 0.25f));

				RenderComponent::Sptr renderer = sink->Add<RenderComponent>();
				renderer->SetMesh(sinkMesh);
				renderer->SetMaterial(sinkMaterial);
			}
			//Tub
			Gameplay::MeshResource::Sptr tubMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("tub.obj");
			Texture2D::Sptr tubTex = ResourceManager::CreateAsset<Texture2D>("textures/tub.png");
			Gameplay::Material::Sptr tubMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
			{
				tubMaterial->Name = "Tub";
				tubMaterial->Set("u_Material.AlbedoMap", tubTex);
				tubMaterial->Set("u_Material.Shininess", 0.5f);
				tubMaterial->Set("u_Material.NormalMap", normalMapDefault);
			}
			Gameplay::GameObject::Sptr tub = scene->CreateGameObject("Tub");
			{
				tub->SetPostion(glm::vec3(-12.25f, -1.78f, 0.0f));
				tub->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				tub->SetScale(glm::vec3(0.4f, 0.4f, 0.4f));

				RenderComponent::Sptr renderer = tub->Add<RenderComponent>();
				renderer->SetMesh(tubMesh);
				renderer->SetMaterial(tubMaterial);
				Gameplay::Physics::Rigidbody::Sptr physics = tub->Add<Gameplay::Physics::RigidBodyType::Static);
				Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
				box->SetScale(glm::vec3(1.0f,1.0f,1.0f);
				box->SetExtents(glm::vec3(1.0f,1.0f,1.0f));
				physics->AdCollider(box);
			}
			//Tv
			Gameplay::GameObject::Sptr tvModel = scene->CreateGameObject("Tv Modelling");
			{
				tvModel->SetPostion(glm::vec3(-14.07f, -1.9f, 0.0f));
				tvModel->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
				tvModel->SetScale(glm::vec3(0.4f, 0.4f, 0.4f));

				RenderComponent::Sptr renderer = tvModel->Add<RenderComponent>();
				renderer->SetMesh(tvMesh);
				renderer->SetMaterial(tvMaterial);
			}
			//Tv Box
			Gameplay::GameObject::Sptr tvboxModel = scene->CreateGameObject("Tv Box Modelling");
			{
				tvboxModel->SetPostion(glm::vec3(-15.69f, -1.68f, 0.0f));
				tvboxModel->SetRotation(glm::vec3(90.0f, 0.0f, 37.0f));
				tvboxModel->SetScale(glm::vec3(0.8f, 0.8f, 0.8f));

				RenderComponent::Sptr renderer = tvboxModel->Add <RenderComponent>();
				renderer->SetMesh(tvboxMesh);
				renderer->SetMaterial(tvboxMaterial);
			}
			//Wolf statue object
			Gameplay::MeshResource::Sptr statueMesh = ResourceManager::CreateAsset<Gameplay::MeshResource>("wolfstatue.obj");
			Texture2D::Sptr statueTex = ResourceManager::CreateAsset <Texture2D>("textures/statue.jpg");
			//Create Material
			Gameplay::Material::Sptr statueMaterial = ResourceManager::CreateAsset<Gameplay::Material>(deferredForward);
			{
				statueMaterial->Name = "Statue";
				statueMaterial->Set("u_Material.AlbedoMap", statueTex);
				statueMaterial->Set("u_Material.Shininess", 1.0f);
				statueMaterial->Set("u_Material.NormalMap", normalMapDefault);
			}
			Gameplay::GameObject::Sptr wolfstatue = scene->CreateGameObject("Wolf Statue");
			{
				wolfstatue->SetPostion(glm::vec3(1.01f, -2.63f, 0.0f));
				wolfstatue->SetRotation(glm::vec3(90.0, 0.0f, -180.f));
				wolfstatue->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

				RenderComponent::Sptr renderer = wolfstatue->Add<RenderComponent>();
				renderer->SetMesh(statueMesh);
				renderer->SetMaterial(statueMaterial);
			}
		}*/

		/////////////////////////// UI //////////////////////////////
		//Font: Junk Dog
		Font::Sptr junkDogFont = ResourceManager::CreateAsset<Font>("fonts/JunkDog.otf", 35.f); //Font path, font size
		junkDogFont->Bake();

		Gameplay::GameObject::Sptr MenuUI = scene->CreateGameObject("Menu UI Canvas");
		{
			RectTransform::Sptr transform = MenuUI->Add<RectTransform>();

			Gameplay::GameObject::Sptr start = scene->CreateGameObject("Start");
			{
				RectTransform::Sptr transform = start->Add<RectTransform>();
				transform->SetMin({ 0, 0 });
				transform->SetMax({ 1280, 720 });

				GuiPanel::Sptr startPanel = start->Add<GuiPanel>();
				startPanel->SetTexture(ResourceManager::CreateAsset<Texture2D>("textures/start_Screen.png"));
			}

			Gameplay::GameObject::Sptr pause = scene->CreateGameObject("Pause");
			{
				RectTransform::Sptr transform = pause->Add<RectTransform>();
				transform->SetMin({ 0, 0 });
				transform->SetMax({ 1280, 720 });

				GuiPanel::Sptr pausePanel = pause->Add<GuiPanel>();
				pausePanel->SetTexture(ResourceManager::CreateAsset<Texture2D>("textures/pause.png"));
				//pausePanel->SetColor(glm::vec4(1.f, 1.f, 1.f, 0.f));
				pausePanel->IsEnabled = false;
			}

			Gameplay::GameObject::Sptr win = scene->CreateGameObject("Win");
			{
				RectTransform::Sptr transform = win->Add<RectTransform>();
				transform->SetMin({ 0, 0 });
				transform->SetMax({ 1280, 720 });

				GuiPanel::Sptr winPanel = win->Add<GuiPanel>();
				winPanel->SetTexture(ResourceManager::CreateAsset<Texture2D>("textures/WIN.png"));
				//winPanel->SetColor(glm::vec4(1.f, 1.f, 1.f, 0.f));
				winPanel->IsEnabled = false;
			}

			Gameplay::GameObject::Sptr end = scene->CreateGameObject("Fail");
			{
				RectTransform::Sptr transform = end->Add<RectTransform>();
				transform->SetMin({ 0, 0 });
				transform->SetMax({ 1280, 720 });

				GuiPanel::Sptr losePanel = end->Add<GuiPanel>();
				losePanel->SetTexture(ResourceManager::CreateAsset<Texture2D>("textures/fail.png"));
				//losePanel->SetColor(glm::vec4(1.f, 1.f, 1.f, 0.f));
				losePanel->IsEnabled = false;
			}

			MenuUI->AddChild(start);
		}


		Gameplay::GameObject::Sptr objectiveUI = scene->CreateGameObject("Objective UI Canvas"); //UI for Time and Garbage Remaining
		{
			RectTransform::Sptr transform = objectiveUI->Add<RectTransform>();
			transform->SetMin({ 10, 10 });
			transform->SetMax({ 200, 200 });
			transform->SetPosition({ 640, 50 });
			transform->SetSize({ 35,35 });

			GuiPanel::Sptr canPanel = objectiveUI->Add<GuiPanel>();
			canPanel->SetColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.f));
			canPanel->SetTexture(ResourceManager::CreateAsset<Texture2D>("textures/ui/ui-clock.png"));
			//canPanel->IsEnabled = false;


			Gameplay::GameObject::Sptr timeText = scene->CreateGameObject("Time Text");
			{
				RectTransform::Sptr timetransform = timeText->Add<RectTransform>();
				timetransform->SetMax({ 130, 240 });

				GuiText::Sptr text = timeText->Add<GuiText>();
				text->SetText("0:00");
				text->SetFont(junkDogFont);
				text->SetColor(glm::vec4(1.f, 1.f, 1.f, 1.f));
				text->SetTextScale(3.0f);
				text->IsEnabled = false;

			}
			objectiveUI->AddChild(timeText);

			Gameplay::GameObject::Sptr trashRemain = scene->CreateGameObject("Trash Remaining");
			{
				RectTransform::Sptr remainTransform = trashRemain->Add<RectTransform>();
				remainTransform->SetMax({ 130,293 });
				//remainTransform->SetSize({ 0,0 });

				GuiText::Sptr text = trashRemain->Add<GuiText>();
				text->SetText("0 Trash Remaining!");
				text->SetFont(junkDogFont);
				text->SetColor(glm::vec4(1.f, 1.f, 1.f, 1.f));
				text->SetTextScale(1.0f);
				text->IsEnabled = false;
			}
			//trashRemain->Get<GuiText>()->IsEnabled = false;
			objectiveUI->AddChild(trashRemain);
		}

		Gameplay::GameObject::Sptr feedbackUI = scene->CreateGameObject("Feedback UI");
		{
			RectTransform::Sptr feedbackTransform = feedbackUI->Add<RectTransform>();
			feedbackTransform->SetPosition({ 690, 750 });
			feedbackTransform->SetSize({ 35,35 });

			GuiPanel::Sptr feedbackPanel = feedbackUI->Add<GuiPanel>();
			feedbackPanel->SetColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.f));

			//Temporary for Feedback, lets make this prettier later
			Gameplay::GameObject::Sptr pickupFeedback = scene->CreateGameObject("Pickup Feedback");
			{
				pickupFeedback->Add<RectTransform>();

				GuiText::Sptr text = pickupFeedback->Add<GuiText>();
				text->SetText("Press E to Pickup Trash!");
				text->SetFont(junkDogFont);
				text->SetColor(glm::vec4(1.f, 1.f, 1.f, 1.f));
				text->IsEnabled = false;
			}

			Gameplay::GameObject::Sptr submitFeedback = scene->CreateGameObject("Submit Feedback");
			{
				submitFeedback->Add<RectTransform>();

				GuiText::Sptr text = submitFeedback->Add<GuiText>();
				text->SetText("Press E to Dump the Trash!");
				text->SetFont(junkDogFont);
				text->SetColor(glm::vec4(1.f, 1.f, 1.f, 1.f));
				text->IsEnabled = false;
			}

			Gameplay::GameObject::Sptr returnFeedback = scene->CreateGameObject("Return Feedback");
			{
				returnFeedback->Add<RectTransform>()
					->SetMax({ 0, -80.f });

				GuiText::Sptr text = returnFeedback->Add<GuiText>();
				text->SetText("Return to the Dumpster!");
				text->SetFont(junkDogFont);
				text->SetColor(glm::vec4(1.f, 1.f, 1.f, 1.f));
				text->IsEnabled = false;
			}
			//returnFeedback->Get<GuiText>()->IsEnabled = false;
			feedbackUI->AddChild(pickupFeedback);
			feedbackUI->AddChild(submitFeedback);
			feedbackUI->AddChild(returnFeedback);
		}

		Gameplay::GameObject::Sptr inventoryUI = scene->CreateGameObject("Inventory UI");
		{
			RectTransform::Sptr transform = inventoryUI->Add<RectTransform>();
			transform->SetMax(app.GetWindowSize());
			transform->SetPosition(glm::vec2(app.GetWindowSize().x - 100.f, app.GetWindowSize().y - 80));
			transform->SetSize(glm::vec2(25.f, 40.f));

			GuiPanel::Sptr invPanel = inventoryUI->Add<GuiPanel>();
			//invPanel->IsEnabled = false;

		}

		Gameplay::GameObject::Sptr LfountainParticles1 = scene->CreateGameObject("Long Foutain Particles 1");
		{
			ParticleSystem::Sptr particleManager = LfountainParticles1->Add<ParticleSystem>();
			particleManager->AddEmitter(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 10.0f), 10.0f, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
			particleManager->Render();
		}

		GuiBatcher::SetDefaultTexture(ResourceManager::CreateAsset<Texture2D>("textures/ui/ui-sprite.png"));
		GuiBatcher::SetDefaultBorderRadius(8);

		// Save the asset manifest for all the resources we just loaded
		ResourceManager::SaveManifest("scene-manifest.json");
		// Save the scene to a JSON file
		scene->Save("scene.json");
		
		// Send the scene to the application
		app.LoadScene(scene);
		scene->IsPlaying = false;

		////starting variables
		scene->score = 0;
		scene->trash = 0;
		scene->held = 0;
	}


}

void DefaultSceneLayer::_CreateTrash()
{
	//auto& scene = Application::Get().CurrentScene();
	//cup collection CUPS TRASH
	{
		Gameplay::GameObject::Sptr trashM = _currentScene->CreateGameObject("Trash1");
		{
			trashM->SetPostion(glm::vec3(3.487f, 5.735f, 0.0f));
			trashM->SetRotation(glm::vec3(90.0f, 0.0f, -92.0f));
			trashM->SetScale(glm::vec3(0.9f, 0.59f, 0.73f));
			// Add a render component
			RenderComponent::Sptr renderer = trashM->Add<RenderComponent>();
			renderer->SetMesh(bagtrashMesh);
			renderer->SetMaterial(bagtrashMaterial);
			// Add a dynamic rigid body to this monkey
			Gameplay::Physics::RigidBody::Sptr physics = trashM->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			box->SetPosition(glm::vec3(0.00f, 0.15f, 0.0f));
			box->SetScale(glm::vec3(0.44f, 0.3f, 0.38f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			//box->SetScale(glm::vec3(0.3f, 0.210f, 0.130f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = trashM->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			box2->SetPosition(glm::vec3(0.00f, 0.25f, -0.05f));
			box2->SetRotation(glm::vec3(0.0f, -3.0f, 0.0f));
			box2->SetScale(glm::vec3(0.66f, 0.21f, 0.58f));
			volume->AddCollider(box2);
			CollectTrashBehaviour::Sptr behaviour2 = trashM->Add<CollectTrashBehaviour>();

		}
		all_trash.push_back(trashM);

		Gameplay::GameObject::Sptr trash2 = _currentScene->CreateGameObject("Trash2");
		{
			trash2->SetPostion(glm::vec3(-7.883f, -0.103f, 0.f));
			trash2->SetRotation(glm::vec3(90.0f, 0.0f, -92.0f));
			trash2->SetScale(glm::vec3(0.9f, 0.59f, 0.73f));
			// Add a render component
			RenderComponent::Sptr renderer = trash2->Add<RenderComponent>();
			renderer->SetMesh(bagtrashMesh);
			renderer->SetMaterial(bagtrashMaterial);
			// Add a dynamic rigid body to this monkey
			Gameplay::Physics::RigidBody::Sptr physics = trash2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			box->SetPosition(glm::vec3(0.00f, 0.15f, 0.0f));
			box->SetScale(glm::vec3(0.44f, 0.3f, 0.38f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			//box->SetScale(glm::vec3(0.3f, 0.210f, 0.130f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = trash2->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			box2->SetPosition(glm::vec3(0.00f, 0.25f, -0.05f));
			box2->SetRotation(glm::vec3(0.0f, -3.0f, 0.0f));
			box2->SetScale(glm::vec3(0.66f, 0.21f, 0.58f));
			volume->AddCollider(box2);
			CollectTrashBehaviour::Sptr behaviour2 = trash2->Add<CollectTrashBehaviour>();

		}
		all_trash.push_back(trash2);

		Gameplay::GameObject::Sptr trash3 = _currentScene->CreateGameObject("Trash3");
		{
			trash3->SetPostion(glm::vec3(-16.344f, 2.197f, 0.0f));
			trash3->SetRotation(glm::vec3(90.0f, 0.0f, -92.0f));
			trash3->SetScale(glm::vec3(0.9f, 0.59f, 0.73f));
			// Add a render component
			RenderComponent::Sptr renderer = trash3->Add<RenderComponent>();
			renderer->SetMesh(bagtrashMesh);
			renderer->SetMaterial(bagtrashMaterial);
			// Add a dynamic rigid body to this monkey
			Gameplay::Physics::RigidBody::Sptr physics = trash3->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			box->SetPosition(glm::vec3(0.00f, 0.15f, 0.0f));
			box->SetScale(glm::vec3(0.44f, 0.3f, 0.38f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			//box->SetScale(glm::vec3(0.3f, 0.210f, 0.130f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = trash3->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			box2->SetPosition(glm::vec3(0.00f, 0.25f, -0.05f));
			box2->SetRotation(glm::vec3(0.0f, -3.0f, 0.0f));
			box2->SetScale(glm::vec3(0.66f, 0.21f, 0.58f));
			volume->AddCollider(box2);
			CollectTrashBehaviour::Sptr behaviour2 = trash3->Add<CollectTrashBehaviour>();

		}
		all_trash.push_back(trash3);
		Gameplay::GameObject::Sptr trash4 = _currentScene->CreateGameObject("Trash4"); //has cup PLACEHOLDER
		{
			trash4->SetPostion(glm::vec3(-11.836, 5.937f, 0.0f));
			trash4->SetRotation(glm::vec3(90.0f, 0.0f, -92.0f));
			trash4->SetScale(glm::vec3(0.9f, 0.59f, 0.73f));
			// Add a render component
			RenderComponent::Sptr renderer = trash4->Add<RenderComponent>();
			renderer->SetMesh(bagtrashMesh);
			renderer->SetMaterial(bagtrashMaterial);
			// Add a dynamic rigid body to this monkey
			Gameplay::Physics::RigidBody::Sptr physics = trash4->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			box->SetPosition(glm::vec3(0.00f, 0.15f, 0.0f));
			box->SetScale(glm::vec3(0.44f, 0.3f, 0.38f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			//box->SetScale(glm::vec3(0.3f, 0.210f, 0.130f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = trash4->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			box2->SetPosition(glm::vec3(0.00f, 0.25f, -0.05f));
			box2->SetRotation(glm::vec3(0.0f, -3.0f, 0.0f));
			box2->SetScale(glm::vec3(0.66f, 0.21f, 0.58f));
			volume->AddCollider(box2);
			CollectTrashBehaviour::Sptr behaviour2 = trash4->Add<CollectTrashBehaviour>();

		}
		all_trash.push_back(trash4);
		Gameplay::GameObject::Sptr CupM = _currentScene->CreateGameObject("Recycle1"); //has cup PLACEHOLDER
		{
			CupM->SetPostion(glm::vec3(-6.399, 3.569f, 0.06f));
			CupM->SetRotation(glm::vec3(120.0f, 0.0f, 0.0f));
			CupM->SetScale(glm::vec3(0.82f, 0.73f, 0.78f));
			// Add a render component
			RenderComponent::Sptr renderer = CupM->Add<RenderComponent>();
			renderer->SetMesh(trashMesh);
			renderer->SetMaterial(trashMaterial);
			// Add a dynamic rigid body to this monkey
			Gameplay::Physics::RigidBody::Sptr physics = CupM->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			box->SetPosition(glm::vec3(0.00f, 0.05f, 0.0f));
			box->SetScale(glm::vec3(0.14f, 0.09f, 0.21f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			//box->SetScale(glm::vec3(0.3f, 0.210f, 0.130f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = CupM->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			box2->SetPosition(glm::vec3(0.00f, 0.05f, 0.0f));
			box2->SetScale(glm::vec3(0.4f, 0.15f, 0.4f));
			volume->AddCollider(box2);
			CollectTrashBehaviour::Sptr behaviour2 = CupM->Add<CollectTrashBehaviour>();
			behaviour2->type = "Recycle";

		}
		all_trash.push_back(CupM);

		Gameplay::GameObject::Sptr trash5 = _currentScene->CreateGameObject("Trash5"); //has cup PLACEHOLDER
		{
			trash5->SetPostion(glm::vec3(-3.404, -9.582f, 0.0f));
			trash5->SetRotation(glm::vec3(90.0f, 0.0f, -92.0f));
			trash5->SetScale(glm::vec3(0.9f, 0.59f, 0.73f));
			// Add a render component
			RenderComponent::Sptr renderer = trash5->Add<RenderComponent>();
			renderer->SetMesh(bagtrashMesh);
			renderer->SetMaterial(bagtrashMaterial);
			// Add a dynamic rigid body to this monkey
			Gameplay::Physics::RigidBody::Sptr physics = trash5->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			box->SetPosition(glm::vec3(0.00f, 0.15f, 0.0f));
			box->SetScale(glm::vec3(0.44f, 0.3f, 0.38f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			//box->SetScale(glm::vec3(0.3f, 0.210f, 0.130f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = trash5->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			box2->SetPosition(glm::vec3(0.00f, 0.25f, -0.05f));
			box2->SetRotation(glm::vec3(0.0f, -3.0f, 0.0f));
			box2->SetScale(glm::vec3(0.66f, 0.21f, 0.58f));
			volume->AddCollider(box2);
			CollectTrashBehaviour::Sptr behaviour2 = trash5->Add<CollectTrashBehaviour>();

		}
		all_trash.push_back(trash5);
		Gameplay::GameObject::Sptr trash6 = _currentScene->CreateGameObject("Trash6"); //has cup PLACEHOLDER
		{
			trash6->SetPostion(glm::vec3(16.289, 5.601f, 0.0f));
			trash6->SetRotation(glm::vec3(90.0f, 0.0f, -92.0f));
			trash6->SetScale(glm::vec3(0.9f, 0.59f, 0.73f));
			// Add a render component
			RenderComponent::Sptr renderer = trash6->Add<RenderComponent>();
			renderer->SetMesh(bagtrashMesh);
			renderer->SetMaterial(bagtrashMaterial);
			// Add a dynamic rigid body to this monkey
			Gameplay::Physics::RigidBody::Sptr physics = trash6->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			box->SetPosition(glm::vec3(0.00f, 0.15f, 0.0f));
			box->SetScale(glm::vec3(0.44f, 0.3f, 0.38f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			//box->SetScale(glm::vec3(0.3f, 0.210f, 0.130f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = trash6->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			box2->SetPosition(glm::vec3(0.00f, 0.25f, -0.05f));
			box2->SetRotation(glm::vec3(0.0f, -3.0f, 0.0f));
			box2->SetScale(glm::vec3(0.66f, 0.21f, 0.58f));
			volume->AddCollider(box2);
			CollectTrashBehaviour::Sptr behaviour2 = trash6->Add<CollectTrashBehaviour>();

		}
		all_trash.push_back(trash6);
		Gameplay::GameObject::Sptr trash7 = _currentScene->CreateGameObject("Trash7"); //has cup PLACEHOLDER
		{
			trash7->SetPostion(glm::vec3(12.801f, 9.244f, 0.0f));
			trash7->SetRotation(glm::vec3(90.0f, 0.0f, -76.0f));
			trash7->SetScale(glm::vec3(0.9f, 0.59f, 0.73f));
			// Add a render component
			RenderComponent::Sptr renderer = trash7->Add<RenderComponent>();
			renderer->SetMesh(bagtrashMesh);
			renderer->SetMaterial(bagtrashMaterial);
			// Add a dynamic rigid body to this monkey
			Gameplay::Physics::RigidBody::Sptr physics = trash7->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			box->SetPosition(glm::vec3(0.00f, 0.15f, 0.0f));
			box->SetScale(glm::vec3(0.44f, 0.3f, 0.38f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			//box->SetScale(glm::vec3(0.3f, 0.210f, 0.130f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = trash7->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			box2->SetPosition(glm::vec3(0.00f, 0.25f, -0.05f));
			box2->SetRotation(glm::vec3(0.0f, -3.0f, 0.0f));
			box2->SetScale(glm::vec3(0.66f, 0.21f, 0.58f));
			volume->AddCollider(box2);
			CollectTrashBehaviour::Sptr behaviour2 = trash7->Add<CollectTrashBehaviour>();

		}
		all_trash.push_back(trash7);
		Gameplay::GameObject::Sptr Cup2 = _currentScene->CreateGameObject("Recycle2"); //has cup PLACEHOLDER
		{
			Cup2->SetPostion(glm::vec3(-0.789, 10.168f, 0.06f));
			Cup2->SetRotation(glm::vec3(120.0f, 0.0f, 0.0f));
			Cup2->SetScale(glm::vec3(0.82f, 0.73f, 0.78f));
			// Add a render component
			RenderComponent::Sptr renderer = Cup2->Add<RenderComponent>();
			renderer->SetMesh(trashMesh);
			renderer->SetMaterial(trashMaterial);
			// Add a dynamic rigid body to this monkey
			Gameplay::Physics::RigidBody::Sptr physics = Cup2->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			box->SetPosition(glm::vec3(0.00f, 0.05f, 0.0f));
			box->SetScale(glm::vec3(0.14f, 0.09f, 0.21f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			//box->SetScale(glm::vec3(0.3f, 0.210f, 0.130f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = Cup2->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			box2->SetPosition(glm::vec3(0.00f, 0.05f, 0.0f));
			box2->SetScale(glm::vec3(0.4f, 0.15f, 0.4f));
			volume->AddCollider(box2);
			CollectTrashBehaviour::Sptr behaviour2 = Cup2->Add<CollectTrashBehaviour>();
			behaviour2->type = "Recycle";

		}
		all_trash.push_back(Cup2);
		Gameplay::GameObject::Sptr trash8 = _currentScene->CreateGameObject("Trash8"); //has cup PLACEHOLDER
		{
			trash8->SetPostion(glm::vec3(13.064, -9.650f, 0.0f));
			trash8->SetRotation(glm::vec3(90.0f, 0.0f, -92.0f));
			trash8->SetScale(glm::vec3(0.9f, 0.59f, 0.73f));
			// Add a render component
			RenderComponent::Sptr renderer = trash8->Add<RenderComponent>();
			renderer->SetMesh(bagtrashMesh);
			renderer->SetMaterial(bagtrashMaterial);
			// Add a dynamic rigid body to this monkey
			Gameplay::Physics::RigidBody::Sptr physics = trash8->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			box->SetPosition(glm::vec3(0.00f, 0.15f, 0.0f));
			box->SetScale(glm::vec3(0.44f, 0.3f, 0.38f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			//box->SetScale(glm::vec3(0.3f, 0.210f, 0.130f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = trash8->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			box2->SetPosition(glm::vec3(0.00f, 0.25f, -0.05f));
			box2->SetRotation(glm::vec3(0.0f, -3.0f, 0.0f));
			box2->SetScale(glm::vec3(0.66f, 0.21f, 0.58f));
			volume->AddCollider(box2);
			CollectTrashBehaviour::Sptr behaviour2 = trash8->Add<CollectTrashBehaviour>();

		}
		all_trash.push_back(trash8);
		Gameplay::GameObject::Sptr trash9 = _currentScene->CreateGameObject("Trash9"); //has cup PLACEHOLDER
		{
			trash9->SetPostion(glm::vec3(7.801, -3.236f, 0.0f));
			trash9->SetRotation(glm::vec3(90.0f, 0.0f, -92.0f));
			trash9->SetScale(glm::vec3(0.9f, 0.59f, 0.73f));
			// Add a render component
			RenderComponent::Sptr renderer = trash9->Add<RenderComponent>();
			renderer->SetMesh(bagtrashMesh);
			renderer->SetMaterial(bagtrashMaterial);
			// Add a dynamic rigid body to this monkey
			Gameplay::Physics::RigidBody::Sptr physics = trash9->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			box->SetPosition(glm::vec3(0.00f, 0.15f, 0.0f));
			box->SetScale(glm::vec3(0.44f, 0.3f, 0.38f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			//box->SetScale(glm::vec3(0.3f, 0.210f, 0.130f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = trash9->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			box2->SetPosition(glm::vec3(0.00f, 0.25f, -0.05f));
			box2->SetRotation(glm::vec3(0.0f, -3.0f, 0.0f));
			box2->SetScale(glm::vec3(0.66f, 0.21f, 0.58f));
			volume->AddCollider(box2);
			CollectTrashBehaviour::Sptr behaviour2 = trash9->Add<CollectTrashBehaviour>();

		}
		all_trash.push_back(trash9);
		Gameplay::GameObject::Sptr Cup3 = _currentScene->CreateGameObject("Recycle3"); //has cup PLACEHOLDER
		{
			Cup3->SetPostion(glm::vec3(4.647, -9.695f, 0.06f));
			Cup3->SetRotation(glm::vec3(120.0f, 0.0f, 0.0f));
			Cup3->SetScale(glm::vec3(0.82f, 0.73f, 0.78f));
			// Add a render component
			RenderComponent::Sptr renderer = Cup3->Add<RenderComponent>();
			renderer->SetMesh(trashMesh);
			renderer->SetMaterial(trashMaterial);
			// Add a dynamic rigid body to this monkey
			Gameplay::Physics::RigidBody::Sptr physics = Cup3->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			box->SetPosition(glm::vec3(0.00f, 0.05f, 0.0f));
			box->SetScale(glm::vec3(0.14f, 0.09f, 0.21f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			//box->SetScale(glm::vec3(0.3f, 0.210f, 0.130f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = Cup3->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			box2->SetPosition(glm::vec3(0.00f, 0.05f, 0.0f));
			box2->SetScale(glm::vec3(0.4f, 0.15f, 0.4f));
			volume->AddCollider(box2);
			CollectTrashBehaviour::Sptr behaviour2 = Cup3->Add<CollectTrashBehaviour>();
			behaviour2->type = "Recycle";

		}
		all_trash.push_back(Cup3);
		Gameplay::GameObject::Sptr Cup4 = _currentScene->CreateGameObject("Recycle4"); //has cup PLACEHOLDER
		{
			Cup4->SetPostion(glm::vec3(-16.379, -9.667f, 0.06f));
			Cup4->SetRotation(glm::vec3(120.0f, 0.0f, 0.0f));
			Cup4->SetScale(glm::vec3(0.82f, 0.73f, 0.78f));
			// Add a render component
			RenderComponent::Sptr renderer = Cup4->Add<RenderComponent>();
			renderer->SetMesh(trashMesh);
			renderer->SetMaterial(trashMaterial);
			// Add a dynamic rigid body to this monkey
			Gameplay::Physics::RigidBody::Sptr physics = Cup4->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			box->SetPosition(glm::vec3(0.00f, 0.05f, 0.0f));
			box->SetScale(glm::vec3(0.14f, 0.09f, 0.21f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			//box->SetScale(glm::vec3(0.3f, 0.210f, 0.130f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = Cup4->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			box2->SetPosition(glm::vec3(0.00f, 0.05f, 0.0f));
			box2->SetScale(glm::vec3(0.4f, 0.15f, 0.4f));
			volume->AddCollider(box2);
			CollectTrashBehaviour::Sptr behaviour2 = Cup4->Add<CollectTrashBehaviour>();
			behaviour2->type = "Recycle";

		}
		all_trash.push_back(Cup4);
		Gameplay::GameObject::Sptr trash10 = _currentScene->CreateGameObject("Trash10"); //has cup PLACEHOLDER
		{
			trash10->SetPostion(glm::vec3(-5.428, -4.582f, 0.0f));
			trash10->SetRotation(glm::vec3(90.0f, 0.0f, -92.0f));
			trash10->SetScale(glm::vec3(0.9f, 0.59f, 0.73f));
			// Add a render component
			RenderComponent::Sptr renderer = trash10->Add<RenderComponent>();
			renderer->SetMesh(bagtrashMesh);
			renderer->SetMaterial(bagtrashMaterial);
			// Add a dynamic rigid body to this monkey
			Gameplay::Physics::RigidBody::Sptr physics = trash10->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			box->SetPosition(glm::vec3(0.00f, 0.15f, 0.0f));
			box->SetScale(glm::vec3(0.44f, 0.3f, 0.38f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			//box->SetScale(glm::vec3(0.3f, 0.210f, 0.130f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = trash10->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			box2->SetPosition(glm::vec3(0.00f, 0.25f, -0.05f));
			box2->SetRotation(glm::vec3(0.0f, -3.0f, 0.0f));
			box2->SetScale(glm::vec3(0.66f, 0.21f, 0.58f));
			volume->AddCollider(box2);
			CollectTrashBehaviour::Sptr behaviour2 = trash10->Add<CollectTrashBehaviour>();

		}
		all_trash.push_back(trash10);

		Gameplay::GameObject::Sptr trash11 = _currentScene->CreateGameObject("Trash11"); //has cup PLACEHOLDER
		{
			trash11->SetPostion(glm::vec3(-11.654, -9.563f, 0.0f));
			trash11->SetRotation(glm::vec3(90.0f, 0.0f, -92.0f));
			trash11->SetScale(glm::vec3(0.9f, 0.59f, 0.73f));
			// Add a render component
			RenderComponent::Sptr renderer = trash11->Add<RenderComponent>();
			renderer->SetMesh(bagtrashMesh);
			renderer->SetMaterial(bagtrashMaterial);
			// Add a dynamic rigid body to this monkey
			Gameplay::Physics::RigidBody::Sptr physics = trash11->Add<Gameplay::Physics::RigidBody>(RigidBodyType::Kinematic);
			Gameplay::Physics::BoxCollider::Sptr box = Gameplay::Physics::BoxCollider::Create();
			box->SetPosition(glm::vec3(0.00f, 0.15f, 0.0f));
			box->SetScale(glm::vec3(0.44f, 0.3f, 0.38f));
			//box->SetPosition(glm::vec3(0.02f, 0.5f, 0.0f));
			//box->SetScale(glm::vec3(0.3f, 0.210f, 0.130f));
			//box->SetExtents(glm::vec3(0.8, 2.68, 0.83));
			physics->AddCollider(box);
			//physics->SetMass(0.0f);
			Gameplay::Physics::TriggerVolume::Sptr volume = trash11->Add<Gameplay::Physics::TriggerVolume>();
			Gameplay::Physics::BoxCollider::Sptr box2 = Gameplay::Physics::BoxCollider::Create();
			box2->SetPosition(glm::vec3(0.00f, 0.25f, -0.05f));
			box2->SetRotation(glm::vec3(0.0f, -3.0f, 0.0f));
			box2->SetScale(glm::vec3(0.66f, 0.21f, 0.58f));
			volume->AddCollider(box2);
			CollectTrashBehaviour::Sptr behaviour2 = trash11->Add<CollectTrashBehaviour>();

		}
		all_trash.push_back(trash11);
	}
}

void DefaultSceneLayer::RandomizePositions()
{
	
	std::vector<glm::vec3> positions;
	
	//randomize/shuffle
	auto rd = std::random_device{};
	auto rng = std::default_random_engine{ rd() };
	std::shuffle(std::begin(positions), std::end(positions), rng);

	//actually setting the new positions
	for (int i = 0; i < all_trash.size(); i++)
	{
		all_trash[i]->SetPostion(positions[i]);
	}
}
