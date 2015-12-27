#include "piece.h"

Piece::Piece(Context* context, MasterControl *masterControl, std::bitset<4> attributes): Object(context),
    masterControl_{masterControl}
{
    rootNode_ = masterControl_->world.scene->CreateChild("Piece");
    rootNode_->SetRotation(Quaternion(Random(360.0f), Vector3::UP));

    attributes_ = attributes;

    StaticModel* model = rootNode_->CreateComponent<StaticModel>();
    model->SetCastShadows(true);

    switch (attributes.to_ulong()){
    case 0: case 8: model->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/Piece_SSS.mdl"));
        break;
    case 1: case 9: model->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/Piece_SSH.mdl"));
        break;
    case 2: case 10: model->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/Piece_SRS.mdl"));
        break;
    case 3: case 11: model->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/Piece_SRH.mdl"));
        break;
    case 4: case 12: model->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/Piece_TSS.mdl"));
        break;
    case 5: case 13: model->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/Piece_TSH.mdl"));
        break;
    case 6: case 14: model->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/Piece_TRS.mdl"));
        break;
    case 7: case 15: model->SetModel(masterControl_->cache_->GetResource<Model>("Resources/Models/Piece_TRH.mdl"));
        break;
    }

    if (attributes[3]){
        model->SetMaterial(masterControl_->cache_->GetResource<Material>("Resources/Materials/Wood_dark.xml"));
    }
    else model->SetMaterial(masterControl_->cache_->GetResource<Material>("Resources/Materials/Wood_light.xml"));
}

