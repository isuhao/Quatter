#ifndef EFFECTMASTER_H
#define EFFECTMASTER_H

#include <Urho3D/Urho3D.h>
#include "master.h"

#define WAYPOINTS 42

class EffectMaster : public Master
{
    URHO3D_OBJECT(EffectMaster, Master);
public:
    EffectMaster();

    void FadeTo(Material* material, Color color, float duration = 0.23f);
    void FadeTo(Light* light, float brightness, float duration = 0.23f);
    void FadeTo(SoundSource* soundSource, float gain, float duration = 2.3f);

    void FadeOut(Material* material) {FadeTo(material, material->GetShaderParameter("MatDiffColor").GetColor() * 0.0f); }
    void FadeOut(Light* light) { FadeTo(light, 0.0f); }
    void FadeOut(SoundSource* soundSource, float duration = 5.0f);

    void TransformTo(Node* node, Vector3 pos, Quaternion rot = Quaternion::IDENTITY, float duration = 1.0f);
    void ArchTo(Node* node, Vector3 pos, Quaternion rot, float archHeight = 2.3f, float duration = 1.0f, bool stall = false);
    float Arch(float t) const noexcept { return 1.0f - pow(2.0f * (t-0.5f), 4.0f); }
};

#endif // EFFECTMASTER_H
