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

enum class GamePhase{PLAYER1PICKS, PLAYER2PUTS, PLAYER2PICKS, PLAYER1PUTS, QUATTER};

typedef struct GameWorld
{
    SharedPtr<QuatterCam> camera;
    SharedPtr<Scene> scene;
    SharedPtr<Board> board_;
    Vector< SharedPtr<Piece> > pieces_;
    struct {
        SharedPtr<Node> sceneCursor;
        SharedPtr<Cursor> uiCursor;
        PODVector<RayQueryResult> hitResults;
    } cursor;
} GameWorld;

#define MC MasterControl::GetInstance()
#define CAMERA MC->world.camera
#define NUM_PIECES 16

class MasterControl : public Application
{
    URHO3D_OBJECT(MasterControl, Application);
    friend class InputMaster;
public:
    MasterControl(Context* context);
    static MasterControl* GetInstance();

    EffectMaster* effectMaster_;
    GameWorld world;
    SharedPtr<ResourceCache> cache_;
    SharedPtr<Graphics> graphics_;

    /// Setup before engine initialization. Modifies the engine paramaters.
    virtual void Setup();
    /// Setup after engine initialization.
    virtual void Start();
    /// Cleanup after the main loop. Called by Application.
    virtual void Stop();

    void Exit();
    void CreateLights();
    inline GamePhase GetGamePhase() const noexcept { return gamePhase_; }

    float AttributesToAngle(int attributes) const { return (360.0f/NUM_PIECES * attributes) + 180.0f/NUM_PIECES + 23.5f; }
    Vector3 AttributesToPosition(int attributes) const {
        return Quaternion(AttributesToAngle(attributes), Vector3::UP) * Vector3::FORWARD * 7.0f
                + Vector3::DOWN * 0.21f;
    }
    Piece* GetSelectedPiece() const;
    Piece* GetPickedPiece() const;
    int CountFreePieces();

    Material* GetMaterial(String name) const { return cache_->GetResource<Material>("Materials/"+name+".xml"); }
    Model* GetModel(String name) const { return cache_->GetResource<Model>("Models/"+name+".mdl"); }
    Texture* GetTexture(String name) const { return cache_->GetResource<Texture>("Textures/"+name+".png"); }


    float Sine(const float freq, const float min, const float max, const float shift = 0.0f);
    float Cosine(const float freq, const float min, const float max, const float shift = 0.0f);

    void Quatter();
private:
    static MasterControl* instance_;
    InputMaster* inputMaster_;
    SharedPtr<Node> leafyLightNode_;
    SharedPtr<Light> leafyLight_;

    SharedPtr<SoundSource> musicSource_;
    float musicGain_;

    GamePhase gamePhase_;
    Node* movingLightNode_;

    void CreateScene();
    void NextPhase();
    void ToggleMusic();
    void MusicGainUp(float step);
    void MusicGainDown(float step);

    void HandleUpdate(StringHash eventType, VariantMap& eventData);
};

#endif // MASTERCONTROL_H
