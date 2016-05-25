#ifndef TEMPLATECAM_H
#define TEMPLATECAM_H

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

class QuatterCam : public Object
{
    URHO3D_OBJECT(QuatterCam, Object);
    friend class MasterControl;
    friend class InputMaster;
public:
    QuatterCam();

#define PITCH_MIN 13.0f
#define PITCH_MAX 80.0f
#define ZOOM_MIN 5.0f
#define ZOOM_MAX 23.0f

    SharedPtr<Camera> camera_;
    SharedPtr<Viewport> viewport_;
    SharedPtr<RenderPath> effectRenderPath_;

    float GetPitch() const { return rootNode_->GetRotation().EulerAngles().x_; }
    float GetYaw() const { return rootNode_->GetRotation().EulerAngles().y_ + 180.0f; }
    float GetDistance() const { return distance_; }
    void Zoom(float distance);
private:
    SharedPtr<Node> rootNode_;

    float distance_;
    Vector2 dollyRotation_;
    Vector3 targetPosition_;
    Vector3 lastTargetPosition_;

    void SetupViewport();

    void Rotate(Vector2 rotation);
    void HandleSceneUpdate(StringHash eventType, VariantMap& eventData);
};

#endif // TEMPLATECAM_H
