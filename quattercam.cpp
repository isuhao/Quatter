#include "quattercam.h"

QuatterCam::QuatterCam():
    Object(MC->GetContext()),
    distance_{12.0f},
    dollyRotation_{Vector2::ZERO},
    targetPosition_{Vector3::ZERO},
    lastTargetPosition_{Vector3::ZERO}
{
    rootNode_ = MC->world.scene->CreateChild("Camera");
    camera_ = rootNode_->CreateComponent<Camera>();
    camera_->SetFarClip(1024.0f);
    rootNode_->SetPosition(Vector3(3.0f, 10.0f, -8.0f));
    rootNode_->LookAt(Vector3::ZERO);

    Zone* zone{rootNode_->CreateComponent<Zone>()};
    zone->SetFogColor(Color(0.05f, 0.1f, 0.23f));
    zone->SetFogStart(23.0f);
    zone->SetFogEnd(128.0f);
    zone->SetAmbientColor(Color(0.23f, 0.23f, 0.23f));

    SetupViewport();

    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(QuatterCam, HandleSceneUpdate));
}

void QuatterCam::SetupViewport()
{
    //Set up a viewport to the Renderer subsystem so that the 3D scene can be seen
    SharedPtr<Viewport> viewport(new Viewport(context_, MC->world.scene, camera_));
    viewport_ = viewport;

    //Add anti-asliasing and bloom
    effectRenderPath_ = viewport_->GetRenderPath();
    effectRenderPath_->Append(MC->cache_->GetResource<XMLFile>("PostProcess/FXAA3.xml"));
    effectRenderPath_->SetEnabled("FXAA3", true);
    effectRenderPath_->Append(MC->cache_->GetResource<XMLFile>("PostProcess/BloomHDR.xml"));
    effectRenderPath_->SetShaderParameter("BloomHDRThreshold", 0.42f);
    effectRenderPath_->SetShaderParameter("BloomHDRMix", Vector2(1.0f, 1.0f));
    effectRenderPath_->SetEnabled("BloomHDR", true);

    Renderer* renderer{GetSubsystem<Renderer>()};
    viewport_->SetRenderPath(effectRenderPath_);
    renderer->SetViewport(0, viewport_);
}

void QuatterCam::HandleSceneUpdate(StringHash eventType, VariantMap& eventData)
{
    //Update distance
    Vector3 relativeToTarget{(rootNode_->GetPosition() - targetPosition_).Normalized()};
    if (relativeToTarget.Length() != distance_){
            rootNode_->SetPosition(distance_ * relativeToTarget + targetPosition_);
            camera_->SetFov(Clamp(60.0f + distance_, 23.0f, 110.0f));
    }
}

void QuatterCam::Rotate(Vector2 rotation)
{
    rootNode_->LookAt(targetPosition_);
    rootNode_->RotateAround(targetPosition_,
                            Quaternion(rotation.x_, Vector3::UP) * Quaternion(rotation.y_, rootNode_->GetRight()), TS_WORLD);

    //Clamp pitch
    if (GetPitch() > PITCH_MAX)
        rootNode_->RotateAround(targetPosition_,
                                Quaternion(-(GetPitch() - PITCH_MAX), rootNode_->GetRight()), TS_WORLD);
    else if (GetPitch() < PITCH_MIN)
        rootNode_->RotateAround(targetPosition_,
                                Quaternion(-GetPitch() + PITCH_MIN, rootNode_->GetRight()), TS_WORLD);
}
void QuatterCam::Zoom(float distance)
{
    distance_ = Clamp(distance_ - distance, ZOOM_MIN, ZOOM_MAX);
}
