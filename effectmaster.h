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

#ifndef EFFECTMASTER_H
#define EFFECTMASTER_H

#include <Urho3D/Urho3D.h>
#include "master.h"

#define WAYPOINTS 42

class EffectMaster : public Master
{
    URHO3D_OBJECT(EffectMaster, Master);
public:
    EffectMaster(Context* context);

    void FadeTo(Material* material, Color color, float duration = 0.23f, float delay = 0.0f);
    void FadeTo(Light* light, float brightness, float duration = 0.23f);
    void FadeTo(SoundSource* soundSource, float gain, float duration = 2.3f);

    void FadeOut(Material* material, float duration = 0.23f) { FadeTo(material, material->GetShaderParameter("MatDiffColor").GetColor() * 0.0f, duration); }
    void FadeOut(Light* light) { FadeTo(light, 0.0f); }
    void FadeOut(SoundSource* soundSource, float duration = 5.0f);

    void TransformTo(Node* node, Vector3 pos, Quaternion rot = Quaternion::IDENTITY, float duration = 1.0f);
    void ArchTo(Node* node, Vector3 pos, Quaternion rot, float archHeight = 2.3f, float duration = 1.0f, float delay = 0.0f);
    float Arch(float t) const noexcept { return 1.0f - pow(2.0f * (t-0.5f), 4.0f); }
};

#endif // EFFECTMASTER_H
