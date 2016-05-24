#include "piece.h"

Piece::Piece(std::bitset<4> attributes): Object(MC->GetContext()),
    state_{PieceState::FREE}
{
    rootNode_ = MC->world.scene->CreateChild("Piece");
    rootNode_->SetRotation(Quaternion(Random(360.0f), Vector3::UP));

    attributes_ = attributes;
    int attr{static_cast<int>(attributes.to_ulong())};

    StaticModel* pieceModel = rootNode_->CreateComponent<StaticModel>();
    pieceModel->SetCastShadows(true);
    switch (attr){
    case 0: case 8: pieceModel->SetModel(MC->GetModel("Piece_SSS"));
        break;
    case 1: case 9: pieceModel->SetModel(MC->GetModel("Piece_SSH"));
        break;
    case 2: case 10: pieceModel->SetModel(MC->GetModel("Piece_SRS"));
        break;
    case 3: case 11: pieceModel->SetModel(MC->GetModel("Piece_SRH"));
        break;
    case 4: case 12: pieceModel->SetModel(MC->GetModel("Piece_TSS"));
        break;
    case 5: case 13: pieceModel->SetModel(MC->GetModel("Piece_TSH"));
        break;
    case 6: case 14: pieceModel->SetModel(MC->GetModel("Piece_TRS"));
        break;
    case 7: case 15: pieceModel->SetModel(MC->GetModel("Piece_TRH"));
        break;
    }
    if (attributes[3]){
        pieceModel->SetMaterial(MC->cache_->GetResource<Material>("Resources/Materials/Wood_dark.xml"));
    }
    else pieceModel->SetMaterial(MC->cache_->GetResource<Material>("Resources/Materials/Wood_light.xml"));


    StaticModel* outlineModel = rootNode_->CreateComponent<StaticModel>();
    outlineModel->SetCastShadows(true);
    outlineModel->SetEnabled(!attr);
    switch(attr){
    case 0: case 8: case 1: case 9:
        outlineModel->SetModel(MC->GetModel("Piece_SS_outline"));
        break;
    case 2: case 10: case 3: case 11:
        outlineModel->SetModel(MC->GetModel("Piece_SR_outline"));
        break;
    case 4: case 12: case 5: case 13:
        outlineModel->SetModel(MC->GetModel("Piece_TS_outline"));
        break;
    case 6: case 14: case 7: case 15:
        outlineModel->SetModel(MC->GetModel("Piece_TR_outline"));
        break;
    }
    outlineModel->SetMaterial(MC->cache_->GetResource<Material>("Resources/Materials/Glow.xml"));
}

