#ifndef MASTERCONTROL_H
#define MASTERCONTROL_H

#include <Urho3D/Urho3D.h>
#include "luckey.h"

namespace Urho3D {
class Drawable;
class Node;
class Scene;
class Sprite;
}

using namespace Urho3D;

class QuatterCam;
class InputMaster;
class EffectMaster;
class Board;
class Piece;

enum class GameState{PLAYER1PICKS, PLAYER2PUTS, PLAYER2PICKS, PLAYER1PUTS, QUATTER};
enum MusicState{MUSIC_SONG1, MUSIC_SONG2, MUSIC_OFF};
enum SelectionMode{SM_CAMERA, SM_STEP};

typedef struct GameWorld
{
    SharedPtr<QuatterCam> camera_;
    SharedPtr<Scene> scene_;
    SharedPtr<Board> board_;
    Vector< SharedPtr<Piece> > pieces_;
    struct {
        SharedPtr<Node> sceneCursor_;
        SharedPtr<Cursor> uiCursor_;
        PODVector<RayQueryResult> hitResults_;
    } cursor_;
} GameWorld;

#define MC MasterControl::GetInstance()
#define FX MC->effectMaster_
#define CAMERA MC->world_.camera_
#define BOARD MC->world_.board_
#define NUM_PIECES 16
#define TABLE_DEPTH 0.21f
#define RESET_DURATION 1.23f

#define COLOR_GLOW MC->GetMaterial("Glow")->GetShaderParameter("MatDiffColor").GetColor()

class MasterControl : public Application
{
    URHO3D_OBJECT(MasterControl, Application);
    friend class InputMaster;
public:
    MasterControl(Context* context);
    static MasterControl* GetInstance();

    EffectMaster* effectMaster_;
    GameWorld world_;
    SharedPtr<ResourceCache> cache_;
    SharedPtr<Graphics> graphics_;

    virtual void Setup();
    virtual void Start();
    virtual void Stop();

    void Exit();
    void CreateLights();
    inline GameState GetGameState() const noexcept { return gameState_; }
    inline bool InPickState() const noexcept { return gameState_ == GameState::PLAYER1PICKS || gameState_ == GameState::PLAYER2PICKS; }
    inline bool InPutState() const noexcept { return gameState_ == GameState::PLAYER1PUTS || gameState_ == GameState::PLAYER2PUTS; }
    inline bool InPlayer1State() const noexcept { return gameState_ == GameState::PLAYER1PICKS || gameState_ == GameState::PLAYER1PUTS; }
    inline bool InPlayer2State() const noexcept { return gameState_ == GameState::PLAYER2PICKS || gameState_ == GameState::PLAYER2PUTS; }

    float AttributesToAngle(int attributes) const { return (360.0f/NUM_PIECES * attributes) + 180.0f/NUM_PIECES + 23.5f; }
    Vector3 AttributesToPosition(int attributes) const {
        return Quaternion(AttributesToAngle(attributes), Vector3::UP) * Vector3::BACK * 7.0f
                + Vector3::DOWN * TABLE_DEPTH;
    }
    Piece* GetSelectedPiece() const;
    Piece* GetPickedPiece() const;
    int CountFreePieces();

    Material* GetMaterial(String name) const { return cache_->GetResource<Material>("Materials/"+name+".xml"); }
    Model* GetModel(String name) const { return cache_->GetResource<Model>("Models/"+name+".mdl"); }
    Texture* GetTexture(String name) const { return cache_->GetResource<Texture>("Textures/"+name+".png"); }
    Sound* GetMusic(String name) const;
    Sound* GetSample(String name) const;

    float Sine(const float freq, const float min, const float max, const float shift = 0.0f);
    float Cosine(const float freq, const float min, const float max, const float shift = 0.0f);

    void Quatter();
    void DeselectPiece();

    void NextMusicState();
    void TakeScreenshot();
private:
    static MasterControl* instance_;
    InputMaster* inputMaster_;
    SharedPtr<Node> leafyLightNode_;
    SharedPtr<Light> leafyLight_;

    SharedPtr<SoundSource> musicSource1_;
    SharedPtr<SoundSource> musicSource2_;
    float musicGain_;

    GameState gameState_;
    MusicState musicState_;
    MusicState previousMusicState_;

    Piece* selectedPiece_;
    Piece* lastSelectedPiece_;
    Piece* pickedPiece_;
    SelectionMode selectionMode_;

    void CreateScene();
    void Reset();
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void NextPhase();

    void CameraSelectPiece();
    void StepSelectPiece(bool next);
    void SelectPrevious();

    void MusicGainUp(float step);
    void MusicGainDown(float step);

    void SelectPiece(Piece* piece);
    bool SelectLastPiece();

    float lastReset_;
    bool Lame() { return GetSubsystem<Time>()->GetElapsedTime() - lastReset_ < RESET_DURATION; }
};

#endif // MASTERCONTROL_H
