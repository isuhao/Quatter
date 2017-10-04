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

#include "yad.h"
#include "effectmaster.h"

void Yad::RegisterObject(Context *context)
{
    context->RegisterFactory<Yad>();
}

Yad::Yad(Context* context) : LogicComponent(context),
    model_{},
    material_{},
    light_{},
    hidden_{true},
    dimmed_{false}
{
}

void Yad::OnNodeSet(Node *node)
{ if (!node) return;

    node_->AddTag("Yad");

    model_ = node_->CreateComponent<AnimatedModel>();
    model_->SetModel(MC->GetModel("Yad"));
    material_ = MC->GetMaterial("Glow")->Clone();
    model_->SetMaterial(material_);

    Node* lightNode{ node_->CreateChild("Light") };
    lightNode->SetPosition(Vector3::UP * 0.23f);
    light_ = lightNode->CreateComponent<Light>();
    light_->SetLightType(LIGHT_POINT);
    light_->SetCastShadows(true);
    light_->SetColor(COLOR_GLOW);
    light_->SetRange(1.0f);
    light_->SetBrightness(YAD_FULLBRIGHT);

}

void Yad::Dim()
{
    FX->FadeTo(light_, YAD_DIMMED);
}
void Yad::Hide()
{
    hidden_ = true;
    FX->FadeOut(light_);
    FX->FadeOut(material_, 0.1f);
}
void Yad::Reveal()
{
    hidden_ = false;
    Restore();
}
void Yad::Restore()
{
    FX->FadeTo(light_, YAD_FULLBRIGHT);
    FX->FadeTo(material_, COLOR_GLOW, 0.1f);
}
