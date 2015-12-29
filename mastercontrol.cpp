#include "mastercontrol.h"
#include "inputmaster.h"
#include "quartocam.h"
#include "board.h"

URHO3D_DEFINE_APPLICATION_MAIN(MasterControl);

MasterControl::MasterControl(Context *context):
    Application(context),
    gamePhase_{GamePhase::PLAYER1PICKS}
{
}

void MasterControl::Setup()
{
    engineParameters_["WindowTitle"] = "Quarto";
    engineParameters_["LogName"] = GetSubsystem<FileSystem>()->GetAppPreferencesDir("urho3d", "logs")+"Quarto.log";
//    engineParameters_["FullScreen"] = false;
//    engineParameters_["WindowWidth"] = 960;
//    engineParameters_["WindowHeight"] = 540;
}
void MasterControl::Start()
{
    new InputMaster(context_, this);
    cache_ = GetSubsystem<ResourceCache>();

    CreateScene();

    //Play music
    Sound* music = cache_->GetResource<Sound>("Resources/Music/Angelight - The Knowledge River.ogg");
    music->SetLooped(true);
    Node* musicNode = world.scene->CreateChild("Music");
    SoundSource* musicSource = musicNode->CreateComponent<SoundSource>();
    musicSource->SetSoundType(SOUND_MUSIC);
    musicSource->Play(music);
}
void MasterControl::Stop()
{
    engine_->DumpResources(true);
}
void MasterControl::Exit()
{
    engine_->Exit();
}

void MasterControl::CreateScene()
{
    world.scene = new Scene(context_);
    world.scene->CreateComponent<Octree>();
    world.camera = new QuartoCam(context_, this);
    CreateLights();

    Node* tableNode = world.scene->CreateChild("Table");
    tableNode->SetPosition(Vector3::DOWN * 0.4f);
    tableNode->SetRotation(Quaternion(23.5f, Vector3::UP));
    tableNode->SetScale(23.0f);
    StaticModel* tableModel = tableNode->CreateComponent<StaticModel>();
    tableModel->SetModel(cache_->GetResource<Model>("Models/Plane.mdl"));
    tableModel->SetMaterial(cache_->GetResource<Material>("Resources/Materials/Basic.xml"));

    //Create board and pieces
    new Board(context_, this);
    for (int p = 0; p < 16; ++p){
        Piece* newPiece = new Piece(context_, this, std::bitset<4>(p));
        newPiece->SetPosition(Quaternion(360.0f/16 * p, Vector3::UP) * Vector3::RIGHT * 9.0f +
                              Vector3::DOWN * 0.4f);
    }
}


void MasterControl::CreateLights()
{
    //Add a directional light to the world. Enable cascaded shadows on it
    Node* downardsLightNode = world.scene->CreateChild("DirectionalLight");
    downardsLightNode->SetPosition(Vector3(-5.0f, 23.0f, -2.0f));
    downardsLightNode->LookAt(Vector3(0.0f, 0.0f, 0.0f));
    Light* downwardsLight = downardsLightNode->CreateComponent<Light>();
    downwardsLight->SetLightType(LIGHT_DIRECTIONAL);
    downwardsLight->SetBrightness(1.0f);
    downwardsLight->SetColor(Color(0.8f, 0.9f, 0.95f));
    downwardsLight->SetCastShadows(true);
    downwardsLight->SetShadowIntensity(0.23f);
    downwardsLight->SetShadowBias(BiasParameters(0.000025f, 0.5f));
    downwardsLight->SetShadowCascade(CascadeParameters(1.0f, 5.0f, 23.0f, 100.0f, 0.8f));

    //Create a point light. Enable cascaded shadows on it
    Node* pointLightNode_ = world.scene->CreateChild("PointLight");
    pointLightNode_->SetPosition(Vector3(0.0f, -1.0f, -13.0f));
    Light* pointLight = pointLightNode_->CreateComponent<Light>();
    pointLight->SetLightType(LIGHT_POINT);
    pointLight->SetBrightness(0.5f);
    pointLight->SetRange(42.0f);
    pointLight->SetColor(Color(0.75f, 1.0f, 0.75f));
}

void MasterControl::NextPhase()
{
    switch (gamePhase_)    {
    case GamePhase::PLAYER1PICKS: gamePhase_ = GamePhase::PLAYER2PUTS;
        break;
    case GamePhase::PLAYER2PUTS: gamePhase_ = GamePhase::PLAYER2PICKS;
        break;
    case GamePhase::PLAYER2PICKS: gamePhase_ = GamePhase::PLAYER1PUTS;
        break;
    case GamePhase::PLAYER1PUTS: gamePhase_ = GamePhase::PLAYER1PICKS;
        break;
    }
}
