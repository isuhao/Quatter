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
class Board;
class Piece;

enum class GamePhase{PLAYER1PICKS, PLAYER2PUTS, PLAYER2PICKS, PLAYER1PUTS};

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

class MasterControl : public Application
{
    URHO3D_OBJECT(MasterControl, Application);
    friend class InputMaster;
public:
    MasterControl(Context* context);
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
private:
    GamePhase gamePhase_;
    Node* movingLightNode_;

    void CreateScene();
    void NextPhase();

    void HandleUpdate(StringHash eventType, VariantMap& eventData);
};

#endif // MASTERCONTROL_H
