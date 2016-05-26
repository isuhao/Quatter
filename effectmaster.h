#ifndef EFFECTMASTER_H
#define EFFECTMASTER_H

#include <Urho3D/Urho3D.h>
#include "master.h"

class EffectMaster : public Master
{
    URHO3D_OBJECT(EffectMaster, Master);
public:
    EffectMaster();
    void FadeOut(Material* material) {
        FadeTo(material, material->GetShaderParameter("MatDiffColor").GetColor() * 0.0f);
    }
    void FadeOut(Light* light) { FadeTo(light, 0.0f); }

    void FadeTo(Material* material, Color color, float duration = 0.23f);
    void FadeTo(Light* light, float brightness, float duration = 0.23f);
    void FadeTo(SoundSource* soundSource, float gain, float duration = 2.3f);
    void FadeOut(SoundSource* soundSource, float duration = 5.0f);
    void TransformTo(Node* node, Vector3 pos, Quaternion rot = Quaternion::IDENTITY, float duration = 1.0f);
};

#endif // EFFECTMASTER_H
