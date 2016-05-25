#include "piece.h"

Piece::Piece(Attributes attributes): Object(MC->GetContext()),
    state_{PieceState::FREE},
    attributes_{attributes}
{
    rootNode_ = MC->world.scene->CreateChild("Piece"+GetCodon(4));
    rootNode_->SetRotation(Quaternion(Random(360.0f), Vector3::UP));


    StaticModel* pieceModel{rootNode_->CreateComponent<StaticModel>()};
    pieceModel->SetCastShadows(true);
    pieceModel->SetModel(MC->GetModel("Piece_"+GetCodon(3)));
    if (attributes[3]){
        pieceModel->SetMaterial(MC->cache_->GetResource<Material>("Resources/Materials/Wood_light.xml"));
    }
    else pieceModel->SetMaterial(MC->cache_->GetResource<Material>("Resources/Materials/Wood_dark.xml"));


    outlineModel_ = rootNode_->CreateComponent<StaticModel>();
    outlineModel_->SetCastShadows(false);
    outlineModel_->SetModel(MC->GetModel("Piece_"+GetCodon(2)+"_outline"));
    outlineModel_->SetMaterial(MC->cache_->GetResource<Material>("Resources/Materials/Glow.xml"));

    Select();
}

String Piece::GetCodon(int length) const
{
    if (length > attributes_.size() || length < 1)
        length = static_cast<int>(attributes_.size());

    String codon{};

        codon += attributes_[0] ? "T" : "S"; //Tall  : Short
    if (length > 1)
        codon += attributes_[1] ? "R" : "S"; //Round : Square
    if (length > 2)
        codon += attributes_[2] ? "H" : "S"; //Hole  : Solid
    if (length == NUM_ATTRIBUTES)
        codon += attributes_[3] ? "L" : "D"; //Light : Dark

    return codon;
}

void Piece::Select()
{
    outlineModel_->SetEnabled(true);
}
void Piece::Deselect()
{
    outlineModel_->SetEnabled(false);
}
