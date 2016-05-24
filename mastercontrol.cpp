#include "mastercontrol.h"
#include "inputmaster.h"
#include "quattercam.h"
#include "board.h"

URHO3D_DEFINE_APPLICATION_MAIN(MasterControl);

MasterControl* MasterControl::instance_ = NULL;

MasterControl* MasterControl::GetInstance()
{
    return MasterControl::instance_;
}

MasterControl::MasterControl(Context *context):
    Application(context),
    musicGain_{1.0f},
    gamePhase_{GamePhase::PLAYER1PICKS}
{
    instance_ = this;
}

void MasterControl::Setup()
{
    engineParameters_["WindowTitle"] = "Quatter";
    engineParameters_["LogName"] = GetSubsystem<FileSystem>()->GetAppPreferencesDir("urho3d", "logs")+"Quatter.log";
    engineParameters_["ResourcePaths"] = "Data;CoreData;Resources";
    engineParameters_["WindowIcon"] = "icon.png";

//    engineParameters_["FullScreen"] = false;
//    engineParameters_["WindowWidth"] = 960;
//    engineParameters_["WindowHeight"] = 540;
}
void MasterControl::Start()
{
    new InputMaster();
    cache_ = GetSubsystem<ResourceCache>();

    CreateScene();

    //Play music
    Sound* music{cache_->GetResource<Sound>("Resources/Music/Angelight - The Knowledge River.ogg")};
    music->SetLooped(true);
    Node* musicNode{world.scene->CreateChild("Music")};
    musicSource_ = musicNode->CreateComponent<SoundSource>();
    musicSource_->SetSoundType(SOUND_MUSIC);
    musicSource_->SetGain(musicGain_);
    musicSource_->Play(music);
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
    world.camera = new QuatterCam();
    CreateLights();

    //Create table
    Node* tableNode = world.scene->CreateChild("Table");
    tableNode->SetRotation(Quaternion(23.5f, Vector3::UP));
    tableNode->SetScale(19.0f);
    StaticModel* tableModel = tableNode->CreateComponent<StaticModel>();
    tableModel->SetModel(cache_->GetResource<Model>("Models/Plane.mdl"));
    tableModel->SetMaterial(cache_->GetResource<Material>("Materials/Stone.xml"));
    tableModel->GetMaterial()->SetShaderParameter("MatDiffColor", Vector4(0.32f, 0.40f, 0.42f, 1.0f));

    //Create board and pieces
    world.board_ = new Board();
    tableNode->SetPosition(Vector3::DOWN * world.board_->GetHeight());

    for (int p = 0; p < 16; ++p){
        Piece* newPiece = new Piece(std::bitset<4>(p));
        world.pieces_.Push(SharedPtr<Piece>(newPiece));
        newPiece->SetPosition(Quaternion(360.0f/16 * p, Vector3::UP) * Vector3::RIGHT * 6.66f +
                              Vector3::DOWN * world.board_->GetHeight());
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
    downwardsLight->SetBrightness(0.88f);
    downwardsLight->SetColor(Color(0.8f, 0.9f, 0.95f));
    downwardsLight->SetCastShadows(true);
    downwardsLight->SetShadowIntensity(0.23f);
    downwardsLight->SetShadowBias(BiasParameters(0.000025f, 0.5f));
    downwardsLight->SetShadowCascade(CascadeParameters(1.0f, 5.0f, 23.0f, 100.0f, 0.8f));

    //Create a point light.
    Node* pointLightNode_ = world.scene->CreateChild("PointLight");
    pointLightNode_->SetPosition(Vector3(0.0f, -1.0f, -13.0f));
    Light* pointLight = pointLightNode_->CreateComponent<Light>();
    pointLight->SetLightType(LIGHT_POINT);
    pointLight->SetBrightness(0.23f);
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

void MasterControl::ToggleMusic()
{


    if (musicSource_->GetGain() == 0.0f){
        ValueAnimation* fadeIn_{new ValueAnimation(context_)};
        fadeIn_->SetKeyFrame(0.0f, 0.0f);
        fadeIn_->SetKeyFrame(1.0f, 0.5f * musicGain_);
        fadeIn_->SetKeyFrame(2.3f, musicGain_);
        musicSource_->SetAttributeAnimation("Gain", fadeIn_, WM_ONCE);
    }
    else{
        musicGain_ = musicSource_->GetGain();
        ValueAnimation* fadeOut_ = new ValueAnimation(context_);
        fadeOut_->SetKeyFrame(0.0f, musicGain_);
        fadeOut_->SetKeyFrame(1.0f, 0.5f * musicGain_);
        fadeOut_->SetKeyFrame(2.3f, 0.1f * musicGain_);
        fadeOut_->SetKeyFrame(5.0f, 0.0f);
        musicSource_->SetAttributeAnimation("Gain", fadeOut_, WM_ONCE);
    }
}

void MasterControl::MusicGainUp(float step)
{
    musicGain_ = Clamp(musicGain_ + step, 0.0f, 1.0f);

    ValueAnimation* fadeIn_{new ValueAnimation(context_)};
    fadeIn_->SetKeyFrame(0.0f, musicSource_->GetGain());
    fadeIn_->SetKeyFrame(0.23f, musicGain_);
    musicSource_->SetAttributeAnimation("Gain", fadeIn_, WM_ONCE);
}
void MasterControl::MusicGainDown(float step)
{
    musicGain_ = Clamp(musicGain_ - step, 0.0f, 1.0f);

    ValueAnimation* fadeOut_{new ValueAnimation(context_)};
    fadeOut_->SetKeyFrame(0.0f, musicSource_->GetGain());
    fadeOut_->SetKeyFrame(0.23f, musicGain_);
    musicSource_->SetAttributeAnimation("Gain", fadeOut_, WM_ONCE);
}
