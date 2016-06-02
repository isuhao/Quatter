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

#include "quattercam.h"
#include <initializer_list>

QuatterCam::QuatterCam():
    Object(MC->GetContext()),
    distance_{12.0f},
    targetDistance_{distance_},
    targetPosition_{Vector3::UP * 0.42f},
    smoothTargetPosition_{targetPosition_}
{
    rootNode_ = MC->world_.scene_->CreateChild("Camera");
    for (bool p : {true, false}){
        Node* pocketNode{rootNode_->CreateChild("Pocket")};
        pocketNode->SetPosition(Vector3(p?2.0f:-2.0f, 1.5f, 3.2f));
        pocketNode->SetRotation(Quaternion(-70.0f, Vector3::RIGHT));
        if (p)
            pockets_.first_ = pocketNode;
        else
            pockets_.second_ = pocketNode;
    }

    camera_ = rootNode_->CreateComponent<Camera>();
    camera_->SetFarClip(1024.0f);
    rootNode_->SetPosition(Vector3(3.0f, 10.0f, -8.0f));
    rootNode_->LookAt(Vector3::ZERO);

    Zone* zone{rootNode_->CreateComponent<Zone>()};
    zone->SetFogColor(Color(0.20f, 0.17f, 0.13f));
    zone->SetFogStart(50.0f);
    zone->SetFogEnd(64.0f);
    zone->SetAmbientColor(Color(0.2f, 0.2f, 0.2f));

    SetupViewport();

    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(QuatterCam, HandleSceneUpdate));
}

void QuatterCam::SetupViewport()
{
    //Set up a viewport to the Renderer subsystem so that the 3D scene can be seen
    SharedPtr<Viewport> viewport(new Viewport(context_, MC->world_.scene_, camera_));
    viewport_ = viewport;

    //Add anti-asliasing and bloom
    effectRenderPath_ = viewport_->GetRenderPath();
    effectRenderPath_->Append(MC->cache_->GetResource<XMLFile>("PostProcess/FXAA3.xml"));
    effectRenderPath_->SetEnabled("FXAA3", true);
    effectRenderPath_->Append(MC->cache_->GetResource<XMLFile>("PostProcess/BloomHDR.xml"));
    effectRenderPath_->SetShaderParameter("BloomHDRThreshold", 0.4f);
    effectRenderPath_->SetShaderParameter("BloomHDRMix", Vector2(1.0f, 1.25f));
    effectRenderPath_->SetEnabled("BloomHDR", true);

    Renderer* renderer{GetSubsystem<Renderer>()};
    viewport_->SetRenderPath(effectRenderPath_);
    renderer->SetViewport(0, viewport_);
}

void QuatterCam::HandleSceneUpdate(StringHash eventType, VariantMap& eventData)
{
    float t{eventData[SceneUpdate::P_TIMESTEP].GetFloat()};
    //Update distance
    if (targetDistance_ != distance_){
        distance_ = 0.1f * (9.0f * distance_ + targetDistance_);
    }

    Vector3 relativeToTarget{(rootNode_->GetPosition() - targetPosition_).Normalized()};
    if (relativeToTarget.Length() != distance_){
            rootNode_->SetPosition(distance_ * relativeToTarget + targetPosition_);
            camera_->SetFov(Clamp(60.0f + distance_, 23.0f, 110.0f));
    }
    //Spin pockets
    float spinSpeed{23.0f};
    pockets_.first_->Rotate(Quaternion(t * spinSpeed, Vector3::UP));
    pockets_.second_->Rotate(Quaternion(-t * spinSpeed, Vector3::UP));
    //Reposition pockets
    pockets_.first_->SetPosition(Vector3(-2.2f - 0.06f * (ZOOM_MAX - distance_), 1.4f, 3.2f));
    pockets_.second_->SetPosition(Vector3(2.2f + 0.06f * (ZOOM_MAX - distance_), 1.4f, 3.2f));
}

void QuatterCam::Rotate(Vector2 rotation)
{
    rootNode_->LookAt(targetPosition_);
    rootNode_->RotateAround(targetPosition_,
                            Quaternion(rotation.x_, Vector3::UP) * Quaternion(rotation.y_, rootNode_->GetRight()), TS_WORLD);

    //Clamp pitch
    if (GetPitch() > PITCH_MAX)
        rootNode_->RotateAround(targetPosition_,
                                Quaternion(PITCH_MAX - GetPitch(), rootNode_->GetRight()), TS_WORLD);
    else if (GetPitch() < PITCH_MIN)
        rootNode_->RotateAround(targetPosition_,
                                Quaternion(PITCH_MIN - GetPitch(), rootNode_->GetRight()), TS_WORLD);
}
void QuatterCam::Zoom(float delta)
{
    targetDistance_ = Clamp(targetDistance_ - delta, ZOOM_MIN, ZOOM_MAX);
}

