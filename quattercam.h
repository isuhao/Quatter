/* Quatter
// Copyright (C) 2016 LucKey Productions (luckeyproductions.nl)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

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
#define PITCH_EDGE 10.0f
#define ZOOM_MIN 5.0f
#define ZOOM_MAX 23.0f
#define ZOOM_EDGE 7.0f

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
    float GetYaw() const { return rootNode_->GetRotation().EulerAngles().y_; }
    float GetDistance() const { return distance_; }
    Vector3 GetPosition() const { return rootNode_->GetPosition(); }
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
