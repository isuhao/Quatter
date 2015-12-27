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

class QuartoCam;
class InputMaster;

typedef struct GameWorld
{
    SharedPtr<QuartoCam> camera;
    SharedPtr<Scene> scene;
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
private:
    Node* movingLightNode_;

    void CreateScene();

    void HandleUpdate(StringHash eventType, VariantMap& eventData);
};

#endif // MASTERCONTROL_H
