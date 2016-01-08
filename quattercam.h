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
    QuatterCam(Context *context, MasterControl* masterControl);

    SharedPtr<Camera> camera_;
    SharedPtr<Viewport> viewport_;
    SharedPtr<RenderPath> effectRenderPath_;

    Vector3 GetWorldPosition();
    Quaternion GetRotation();
private:
    MasterControl* masterControl_;
    SharedPtr<Node> rootNode_;

    void SetupViewport();
};

#endif // TEMPLATECAM_H
