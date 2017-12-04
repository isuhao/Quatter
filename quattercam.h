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

#define TARGET_BOARD Vector3::UP * 0.42f
#define TARGET_MENU Vector3::DOWN * 10.0f
#define PITCH_MIN 13.0f
#define PITCH_MAX 80.0f
#define PITCH_EDGE 10.0f
#define PITCH_MENU 5.0f
#define ZOOM_MIN 5.0f
#define ZOOM_MAX 23.0f
#define ZOOM_EDGE 7.0f
#define ZOOM_MENU 21.0f

#define CAMERA MC->world_.camera_

class QuatterCam : public LogicComponent
{
    URHO3D_OBJECT(QuatterCam, LogicComponent);
    friend class MasterControl;

public:
    QuatterCam(Context* context);
    static void RegisterObject(Context* context);
    virtual void OnNodeSet(Node* node);
    void Update(float timeStep) override;

    Vector3 GetPosition() const { return node_->GetPosition(); }
    Node* GetPocket(bool left) const { return left ? pockets_.first_ : pockets_.second_; }

    void Rotate(Vector2 rotation);
    float GetPitch() const { return node_->GetRotation().EulerAngles().x_; }
    float GetYaw() const { return node_->GetRotation().EulerAngles().y_; }

    void SetDistance(float distance) { aimDistance_ = Clamp(distance, ZOOM_MIN, ZOOM_MAX); }
    void Zoom(float delta);
    void ZoomToBoard() { SetDistance(ZOOM_MIN + 1.0f); }
    void ZoomToTable() { SetDistance(13.0f); }
    float GetDistance() const { return distance_; }

    Ray GetScreenRay(float x, float y) const { return camera_->GetScreenRay(x, y); }
    void TargetMenu();
    void TargetBoard();
private:
    SharedPtr<Camera> camera_;
    SharedPtr<Viewport> viewport_;
    SharedPtr<RenderPath> effectRenderPath_;

    Pair<SharedPtr<Node>,
         SharedPtr<Node>> pockets_;

    float distance_;
    float aimDistance_;
    Vector3 targetPosition_;
    Vector3 aimTargetPosition_;

    void SetupViewport();
    void CreatePockets();
    void UpdatePockets(float timeStep);
    void UpdateFov();
};

#endif // QUATTERCAM_H
