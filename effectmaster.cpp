#include "effectmaster.h"

using namespace Urho3D;

EffectMaster::EffectMaster() : Master()
{

}

void EffectMaster::FadeTo(Material* material, Color color, float duration)
{
    Color startColor{material->GetShaderParameter("MatDiffColor").GetColor()};
    ValueAnimation* fade{new ValueAnimation(context_)};
    fade->SetKeyFrame(0.0f, startColor);
    fade->SetKeyFrame(duration, color);
    material->SetShaderParameterAnimation("MatDiffColor", fade, WM_ONCE);
}

void EffectMaster::FadeTo(Light* light, float brightness, float duration)
{
    ValueAnimation* fade{new ValueAnimation(context_)};
    fade->SetKeyFrame(0.0f, light->GetBrightness());
    fade->SetKeyFrame(duration, brightness);
    light->SetAttributeAnimation("Brightness Multiplier", fade, WM_ONCE);
}





