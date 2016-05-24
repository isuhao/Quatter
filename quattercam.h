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

    SharedPtr<Camera> camera_;
    SharedPtr<Viewport> viewport_;
    SharedPtr<RenderPath> effectRenderPath_;

    Vector3 GetWorldPosition();
    Quaternion GetRotation();
    Vector2 GetDollyRotation();
private:
    MasterControl* masterControl_;
    SharedPtr<Node> rootNode_;

    float distance_;
    Vector2 dollyRotation_;
    Vector3 targetPosition_;
    Vector3 lastTargetPosition_;

    void SetupViewport();

    void Rotate(Vector2 rotation);
//    void SetDollyRotation(Vector2 dollyRotation){ dollyRotation_ = dollyRotation;}
//    void Rotate(Vector2 rotation){ SetDollyRotation(GetDollyRotation() + rotation);
//    void Focus(Vector3 targetPosition, float distance, float duration);
};

#endif // TEMPLATECAM_H
