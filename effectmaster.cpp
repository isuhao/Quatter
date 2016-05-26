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

void EffectMaster::FadeTo(SoundSource* soundSource, float gain, float duration)
{
    ValueAnimation* fade{new ValueAnimation(context_)};
    fade->SetKeyFrame(0.0f, soundSource->GetGain());
    fade->SetKeyFrame(0.42f * duration, Lerp(soundSource->GetGain(), gain, 0.5f));
    fade->SetKeyFrame(duration, gain);
    soundSource->SetAttributeAnimation("Gain", fade, WM_ONCE);
}
void EffectMaster::FadeOut(SoundSource* soundSource, float duration)
{
    float lastGain{soundSource->GetGain()};

    ValueAnimation* fade{new ValueAnimation(context_)};
    fade->SetKeyFrame(0.0f, lastGain);
    fade->SetKeyFrame(0.2f * duration, 0.5f * lastGain);
    fade->SetKeyFrame(0.46f * duration, 0.1f * lastGain);
    fade->SetKeyFrame(duration, 0.0f);
    soundSource->SetAttributeAnimation("Gain", fade, WM_ONCE);
}

void EffectMaster::TransformTo(Node* node, Vector3 pos, Quaternion rot, float duration)
{
    ValueAnimation* posAnim{new ValueAnimation(context_)};
    posAnim->SetKeyFrame(0.0f, node->GetPosition());
    posAnim->SetKeyFrame(duration, pos);
    node->SetAttributeAnimation("Position", posAnim, WM_ONCE);

    ValueAnimation* rotAnim{new ValueAnimation(context_)};
    rotAnim->SetKeyFrame(0.0f, node->GetRotation());
    rotAnim->SetKeyFrame(duration, rot);
    node->SetAttributeAnimation("Rotation", rotAnim, WM_ONCE);
}





