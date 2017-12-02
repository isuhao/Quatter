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



#ifndef GUIMASTER_H
#define GUIMASTER_H

#include <Urho3D/Urho3D.h>
#include "master.h"

class GUIMaster : public Master
{
    URHO3D_OBJECT(GUIMaster, Master);
public:
    GUIMaster(Context* context);
    void EnterMenu();

    int GetActiveSide() const { return activeSide_; }
    void SetLogoVisible(bool enable);
private:
    int activeSide_;
    Sprite* logoSprite_;

    void Init3DGUI();
    void CreateLogo();

    Vector<Node*> faces_;
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleClick(StringHash eventType, VariantMap& eventData);
};

#endif // GUIMASTER_H
