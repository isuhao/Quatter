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

#ifndef YAD_H
#define YAD_H

#include <Urho3D/Urho3D.h>
#include "luckey.h"

#define YAD_FULLBRIGHT 0.5f
#define YAD_DIMMED 0.1f

class Yad : public LogicComponent
{
    friend class InputMaster;
    URHO3D_OBJECT(Yad, LogicComponent);

public:
    Yad(Context* context);
    static void RegisterObject(Context* context);
    void OnNodeSet(Node* node) override;

private:
    SharedPtr<AnimatedModel> model_;
    SharedPtr<Material> material_;
    SharedPtr<Light> light_;
    bool hidden_;
    bool dimmed_;

    void Dim();
    void Hide();
    void Reveal();
    void Restore();
};

#endif // YAD_H
