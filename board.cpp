#include "board.h"

namespace Urho3D {
template <> unsigned MakeHash(const IntVector2& value)
  {
    return LucKey::IntVector2ToHash(value);
  }
}

Board::Board(): Object(MC->GetContext()),
    squares_{}
{
    rootNode_ = MC->world.scene->CreateChild("Board");
    model_ = rootNode_->CreateComponent<StaticModel>();
    model_->SetModel(MC->cache_->GetResource<Model>("Resources/Models/Board.mdl"));
    model_->SetMaterial(MC->cache_->GetResource<Material>("Resources/Materials/Board.xml"));
    model_->SetCastShadows(true);

    for (int i{0}; i < BOARD_HEIGHT; ++i)
        for (int j{0}; j < BOARD_WIDTH; ++j){
            Square* square{new Square};
            square->coords_ = IntVector2(i, j);
            square->node_ = rootNode_->CreateChild("Square");
            square->node_->SetPosition(SquarePosition(square->coords_));
            square->free_ = true;
            square->selected_ = false;

            Node* lightNode{square->node_->CreateChild("Light")};
            lightNode->SetPosition(Vector3::UP * 0.23f);
            square->light_ = square->node_->CreateComponent<Light>();
            square->light_->SetColor(Color(0.0f, 0.8f, 0.5f));
            square->light_->SetBrightness(0.5f);
            square->light_->SetRange(5.0f);
            square->light_->SetEnabled(false);

            StaticModel* slotModel{square->node_->CreateComponent<StaticModel>()};
//            slotModel->SetModel(MC->GetModel("Slot")->Clone());
//            slotModel->SetMaterial(MC->GetMaterial("Slot.xml"));
            slotModel->SetCastShadows(true);

            squares_[square->coords_] = square;
        }
}

Vector3 Board::SquarePosition(IntVector2 coords)
{
    return Vector3(0.5f + coords.x_ - BOARD_WIDTH/2,
                   0.0f,
                   0.5f + coords.y_ - BOARD_HEIGHT/2);
}

bool Board::CheckQuatter()
{
    //Check rows
    for (int i{0}; i < BOARD_HEIGHT; ++i){
        Piece::Attributes matching{NUM_PIECES - 1};
        Piece::Attributes first{};
        for (int j{0}; j < BOARD_WIDTH; ++j){
            IntVector2 coords(i, j);
            if (squares_[coords]->piece_){
                auto attributes(squares_[coords]->piece_->GetAttributes());
                if (j == 0) {
                    first = attributes;
                } else {
                    for (int a{0}; a < NUM_ATTRIBUTES; ++a)
                        if (first[a] != attributes[a])
                            matching[a] = false;
                }
            //Full row required
            } else {
                matching.reset();
                break;
            }
        }
        //Quatter!
        if (matching.any()){
            return true;
        }
    }

    //Check columns
    for (int j{0}; j < BOARD_WIDTH; ++j){
        Piece::Attributes matching{NUM_PIECES - 1};
        Piece::Attributes first{};
        for (int i{0}; i < BOARD_HEIGHT; ++i){
            IntVector2 coords(i, j);
            if (squares_[coords]->piece_){
                auto attributes(squares_[coords]->piece_->GetAttributes());
                if (j == 0) {
                    first = attributes;
                } else {
                    for (int a{0}; a < NUM_ATTRIBUTES; ++a)
                        if (first[a] != attributes[a])
                            matching[a] = false;
                }
            //Full column required
            } else {
                matching.reset();
                break;
            }
        }
        //Quatter!
        if (matching.any()){
            return true;
        }
    }
    //Check diagonals
    for (bool direction : {true, false}){
        Piece::Attributes matching{NUM_PIECES - 1};
        Piece::Attributes first{};
        for (int i{0}; i < BOARD_WIDTH; ++i){
            IntVector2 coords(i, direction ? i : (BOARD_WIDTH - i - 1));
            if (squares_[coords]->piece_){
                auto attributes(squares_[coords]->piece_->GetAttributes());
                if (i == 0) {
                    first = attributes;
                } else {
                    for (int a{0}; a < NUM_ATTRIBUTES; ++a)
                        if (first[a] != attributes[a])
                            matching[a] = false;
                }
                //Full line required
            } else {
                matching.reset();
                break;
            }
        }
        //Quatter!
        if (matching.any()){
            return true;
        }
    }
    //Check blocks
    for (int k{0}; k < BOARD_WIDTH - 1; ++k){
        for (int l{0}; l < BOARD_HEIGHT - 1; ++l){
            Piece::Attributes matching{NUM_PIECES - 1};
            Piece::Attributes first{};
            for (int m : {0, 1}) for (int n : {0, 1}){
                IntVector2 coords(k + m, l + n);
                if (squares_[coords]->piece_){
                    auto attributes(squares_[coords]->piece_->GetAttributes());
                    if (m == 0 && n == 0) {
                        first = attributes;
                    } else {
                        for (int a{0}; a < NUM_ATTRIBUTES; ++a)
                            if (first[a] != attributes[a])
                                matching[a] = false;
                    }
                    //Full line required
                } else {
                    matching.reset();
                    break;
                }
            }
            //Quatter!
            if (matching.any()){
                return true;
            }
        }
    }
    //No Quatter
    return false;
}

void Board::PutPiece(Piece* piece, IntVector2 coords)
{
    Square* square{squares_[coords]};
    square->piece_ = piece;
    piece->Put(square->node_->GetWorldPosition() + GetThickness() * Vector3::UP);
}
Square* Board::GetSelectedSquare()
{
    for (Square* s : squares_.Values()){
        if (s->selected_)
            return s;
    }
}
