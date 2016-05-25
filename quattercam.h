#ifndef QUATTERCAM_H
#define QUATTERCAM_H

#include <Urho3D/Urho3D.h>
#include "mastercontrol.h"

namespace Urho3D {
class Drawable;
class Node;
class Scene;
class Sprite;
class Viewport;
class RenderPath;
class Camera;
}

using namespace Urho3D;

#define PITCH_MIN 13.0f
#define PITCH_MAX 80.0f
#define PITCH_EDGE 5.0f
#define ZOOM_MIN 5.0f
#define ZOOM_MAX 23.0f
#define ZOOM_EDGE 2.3f

class QuatterCam : public Object
{
    URHO3D_OBJECT(QuatterCam, Object);
    friend class MasterControl;
    friend class InputMaster;
public:
    QuatterCam();

    SharedPtr<Camera> camera_;
    SharedPtr<Viewport> viewport_;
    SharedPtr<RenderPath> effectRenderPath_;

    float GetPitch() const { return rootNode_->GetRotation().EulerAngles().x_; }
    float GetYaw() const { return rootNode_->GetRotation().EulerAngles().y_ + 180.0f; }
    float GetDistance() const { return distance_; }
    void Zoom(float distance);
    Node* GetPocket(bool right) const { return right ? pockets_.second_ : pockets_.first_; }
private:
    SharedPtr<Node> rootNode_;
    Pair<SharedPtr<Node>,
         SharedPtr<Node>> pockets_;

    float distance_;
    Vector3 targetPosition_;
    Vector3 smoothTargetPosition_;

    void SetupViewport();

    void Rotate(Vector2 rotation);
    void HandleSceneUpdate(StringHash eventType, VariantMap& eventData);
};

#endif // QUATTERCAM_H
