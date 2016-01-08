#include "quattercam.h"

QuatterCam::QuatterCam(Context *context, MasterControl *masterControl):
    Object(context)
{
    masterControl_ = masterControl;

    rootNode_ = masterControl_->world.scene->CreateChild("Camera");
    camera_ = rootNode_->CreateComponent<Camera>();
    camera_->SetFarClip(1024.0f);
    rootNode_->SetPosition(Vector3(0.0f, 5.0f, -13.0f));
    rootNode_->LookAt(Vector3::UP);

    Zone* zone = rootNode_->CreateComponent<Zone>();
    zone->SetFogStart(23.0f);
    zone->SetFogEnd(42.0f);

    SetupViewport();
}

void QuatterCam::SetupViewport()
{
    //Set up a viewport to the Renderer subsystem so that the 3D scene can be seen
    SharedPtr<Viewport> viewport(new Viewport(context_, masterControl_->world.scene, camera_));
    viewport_ = viewport;

    //Add anti-asliasing and bloom
    effectRenderPath_ = viewport_->GetRenderPath()->Clone();
    effectRenderPath_->Append(masterControl_->cache_->GetResource<XMLFile>("PostProcess/FXAA3.xml"));
    effectRenderPath_->SetEnabled("FXAA3", true);
    effectRenderPath_->Append(masterControl_->cache_->GetResource<XMLFile>("PostProcess/Bloom.xml"));
    effectRenderPath_->SetShaderParameter("BloomThreshold", 0.23f);
    effectRenderPath_->SetShaderParameter("BloomMix", Vector2(1.75f, 1.25f));
    effectRenderPath_->SetEnabled("Bloom", true);

    Renderer* renderer = GetSubsystem<Renderer>();
    viewport_->SetRenderPath(effectRenderPath_);
    renderer->SetViewport(0, viewport_);
}

Vector3 QuatterCam::GetWorldPosition()
{
    return rootNode_->GetWorldPosition();
}

Quaternion QuatterCam::GetRotation()
{
    return rootNode_->GetRotation();
}
