/* Quatter
// Copyright (C) 2017 LucKey Productions (luckeyproductions.nl)
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

#include "guimaster.h"

#include "quattercam.h"
//#include <initializer_list>

void QuatterCam::RegisterObject(Context *context)
{
    context->RegisterFactory<QuatterCam>();
}

QuatterCam::QuatterCam(Context* context) : LogicComponent(context),
    distance_{16.0f},
    aimDistance_{distance_},
    targetPosition_{TARGET_BOARD * 23.0f},
    aimTargetPosition_{targetPosition_}
{
}

void QuatterCam::OnNodeSet(Node *node)
{ if (!node) return;

    node_->SetPosition(Vector3(3.0f, 8.0f, -10.0f).Normalized() * distance_);
    node_->LookAt(targetPosition_);
    camera_ = node_->CreateComponent<Camera>();
    camera_->SetFarClip(1024.0f);
    UpdateFov();

    Zone* zone{node_->CreateComponent<Zone>()};
    zone->SetFogColor(Color(0.20f, 0.17f, 0.13f));
    zone->SetFogStart(50.0f);
    zone->SetFogEnd(64.0f);
    zone->SetAmbientColor(Color(0.2f, 0.2f, 0.2f));
    zone->SetZoneTexture(GetScene()->GetChild("Sky")->GetComponent<Skybox>()->GetMaterial(0)->GetTexture(TU_DIFFUSE));

    CreatePockets();
    SetupViewport();
}

void QuatterCam::CreatePockets()
{
    for (bool left : { true, false }){

        Node* pocketNode{ node_->CreateChild("Pocket", LOCAL) };
        pocketNode->SetPosition(Vector3(left ? -2.0f : 2.0f, 1.5f, 3.2f));
        pocketNode->SetRotation(Quaternion(-70.0f, Vector3::RIGHT));

        if (left)
            pockets_.first_ = pocketNode;
        else
            pockets_.second_ = pocketNode;
    }
}

void QuatterCam::SetupViewport()
{
    viewport_ = new Viewport(context_, GetScene(), camera_);

    //Add anti-asliasing and bloom
    effectRenderPath_ = viewport_->GetRenderPath();
    effectRenderPath_->Append(CACHE->GetResource<XMLFile>("PostProcess/FXAA3.xml"));
    effectRenderPath_->SetEnabled("FXAA3", true);
    effectRenderPath_->Append(CACHE->GetResource<XMLFile>("PostProcess/BloomHDR.xml"));
    effectRenderPath_->SetShaderParameter("BloomHDRThreshold", 0.4f);
    effectRenderPath_->SetShaderParameter("BloomHDRMix", Vector2(0.0f, 2.3f));
    effectRenderPath_->SetEnabled("BloomHDR", true);

    RENDERER->SetViewport(0, viewport_);
}

void QuatterCam::UpdateFov()
{
    camera_->SetFov(Clamp(60.0f + distance_, 23.0f, 110.0f));
}

void QuatterCam::Update(float timeStep)
{
    if (effectRenderPath_->GetShaderParameter("BloomHDRMix").GetVector2().y_ != 0.7f)
        effectRenderPath_->SetShaderParameter("BloomHDRMix", Vector2(Clamp(TIME->GetElapsedTime() * 0.5f - 1.0f, 0.0f, 1.0f), Max(0.7f, 2.3f - TIME->GetElapsedTime() * 0.25f)));

    if (MC->InMenu()) {

        if (aimTargetPosition_.y_ > 1.0f) {

            node_->RotateAround(targetPosition_, Quaternion(timeStep, Vector3::UP), TS_WORLD);

        } else {

            if (node_->GetWorldPosition().y_ - aimTargetPosition_.y_ > 0.0f)
                node_->Translate(Vector3::DOWN * (node_->GetWorldPosition().y_ - aimTargetPosition_.y_) * timeStep * 4.2f);

            int side{ GetSubsystem<GUIMaster>()->GetActiveSide() };//Floor(TIME->GetElapsedTime() * 0.5f) };
            Vector3 tableSideCrossProduct{ node_->GetDirection().CrossProduct(Quaternion(-90.0f * side, Vector3::UP) * MC->world_.tableNode_->GetDirection()) };

            if (tableSideCrossProduct.Length()) {

                node_->RotateAround(targetPosition_, Quaternion(tableSideCrossProduct.Length() * 235.0f * timeStep, tableSideCrossProduct), TS_WORLD);
                node_->LookAt(targetPosition_, 0.5f * (node_->GetUp() + Vector3::UP));
            }

            aimDistance_ = ZOOM_MENU * (1.0f + 0.23f * tableSideCrossProduct.Length());
        }
    }

    //Update distance
    if (distance_ != aimDistance_)
        distance_ = 0.1f * (9.0f * distance_ + aimDistance_);

    //Update target position
    if (targetPosition_ != aimTargetPosition_)
        targetPosition_ = 0.2f * (4.0f * targetPosition_ + aimTargetPosition_);

    Vector3 relativeToTarget{ (node_->GetPosition() - targetPosition_).Normalized() };
    if (relativeToTarget.Length() != distance_) {

            node_->SetPosition(distance_ * relativeToTarget + targetPosition_);
            UpdateFov();
    }

    //Spin pockets
    UpdatePockets(timeStep);
}

void QuatterCam::UpdatePockets(float timeStep)
{
    float spinSpeed{ 23.0f };

    pockets_.first_->Rotate(Quaternion(timeStep * spinSpeed, Vector3::UP));
    pockets_.second_->Rotate(Quaternion(-timeStep * spinSpeed, Vector3::UP));
    //Reposition pockets
    pockets_.first_->SetPosition(Vector3(-2.2f - 0.06f * (ZOOM_MAX - distance_), 1.4f - GetPitch() * 0.01f, 3.2f));
    pockets_.second_->SetPosition(Vector3(2.2f + 0.06f * (ZOOM_MAX - distance_), 1.4f - GetPitch() * 0.01f, 3.2f));
}

void QuatterCam::Rotate(Vector2 rotation)
{
    if (MC->InMenu())
        return;

    node_->LookAt(targetPosition_);
    node_->RotateAround(targetPosition_,
                        Quaternion(rotation.x_, Vector3::UP) * Quaternion(rotation.y_, node_->GetRight()),
                        TS_WORLD);

    Vector3 camRight{ node_->GetRight() };

    //Clamp pitch
    if (GetPitch() > PITCH_MAX)
        node_->RotateAround(targetPosition_,
                                Quaternion(PITCH_MAX - GetPitch(), camRight),
                                TS_WORLD);
    else if (GetPitch() < PITCH_MIN)
        node_->RotateAround(targetPosition_,
                                Quaternion(PITCH_MIN - GetPitch(), camRight),
                                TS_WORLD);
}

void QuatterCam::Zoom(float delta)
{
    if (MC->InMenu())
        return;

    aimDistance_ = Clamp(aimDistance_ - delta, ZOOM_MIN, ZOOM_MAX);
}

void QuatterCam::TargetMenu()
{
    aimTargetPosition_ = TARGET_MENU;
}

void QuatterCam::TargetBoard()
{
    aimTargetPosition_ = TARGET_BOARD;
}
