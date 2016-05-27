#include "board.h"
#include "effectmaster.h"
#include "quattercam.h"

namespace Urho3D {
template <> unsigned MakeHash(const IntVector2& value)
  {
    return LucKey::IntVector2ToHash(value);
  }
}

Board::Board(): Object(MC->GetContext()),
    squares_{},
    selectedSquare_{nullptr},
    lastSelectedSquare_{nullptr}
{
    rootNode_ = MC->world_.scene_->CreateChild("Board");
    model_ = rootNode_->CreateComponent<StaticModel>();
    model_->SetModel(MC->cache_->GetResource<Model>("Resources/Models/Board.mdl"));
    model_->SetMaterial(MC->cache_->GetResource<Material>("Resources/Materials/Board.xml"));
    model_->SetCastShadows(true);

    //Fill board with squares
    for (int i{0}; i < BOARD_WIDTH; ++i) for (int j{0}; j < BOARD_HEIGHT; ++j){

        Square* square{new Square};
        square->coords_ = IntVector2(i, j);
        square->node_ = rootNode_->CreateChild("Square");
        square->node_->SetPosition(CoordsToPosition(square->coords_));
        square->free_ = true;
        square->piece_ = nullptr;

        Node* slotNode{square->node_->CreateChild("Slot")};
        slotNode->SetPosition(Vector3::UP * 0.05f);
        square->slot_ = slotNode->CreateComponent<AnimatedModel>();
        square->slot_->SetModel(MC->GetModel("Slot"));
        square->slot_->SetMaterial(MC->GetMaterial("Glow")->Clone());
        square->slot_->GetMaterial()->SetShaderParameter("MatDiffColor", Color(0.0f, 0.0f, 0.0f, 0.0f));

        Node* lightNode{slotNode->CreateChild("Light")};
        lightNode->SetPosition(Vector3::UP * 0.23f);
        square->light_ = lightNode->CreateComponent<Light>();
        square->light_->SetColor(COLOR_GLOW);
        square->light_->SetBrightness(0.023f);
        square->light_->SetRange(2.0f);
        square->light_->SetCastShadows(false);

        squares_[square->coords_] = square;

    }

    SubscribeToEvent(E_SCENEUPDATE, URHO3D_HANDLER(Board, HandleSceneUpdate));
}
void Board::Reset()
{
    for (Square* s: squares_.Values()){

        s->free_ = true;
        s->piece_ = nullptr;
        s->light_->SetEnabled(true);

    }

    DeselectAll();
}

void Board::Refuse()
{
    if (selectedSquare_){
        Material* glow{selectedSquare_->slot_->GetMaterial()};
        glow->SetShaderParameter("MatDiffColor", Color(1.0f, 0.0f, 0.0f, 1.0f));
        if (selectedSquare_->free_)
            FX->FadeTo(glow, COLOR_GLOW, 0.23f);
        else
            FX->FadeTo(glow,Color(1.0f, 0.8f, 0.0f, 0.5f), 0.23f);
    }
}

bool Board::IsEmpty() const
{
    for (Square* s: squares_.Values())
        if (!s->free_) return false;

    return true;
}

Vector3 Board::CoordsToPosition(IntVector2 coords)
{
    return Vector3(0.5f + coords.x_ - BOARD_WIDTH/2,
                   GetThickness(),
                   0.5f + coords.y_ - BOARD_HEIGHT/2);
}

void Board::HandleSceneUpdate(StringHash eventType, VariantMap& eventData)
{
    for (Square* s: squares_.Values()){

        s->slot_->SetMorphWeight(0, MC->Sine(2.3f, 0.0f, 1.0f));

    }
}

bool Board::PutPiece(Piece* piece, Square* square)
{
    if (piece && square->free_){

        MC->DeselectPiece();

        square->piece_ = piece;
        square->free_ = false;
        square->light_->SetEnabled(false);

        piece->Put(square->node_->GetWorldPosition()
                   + Vector3(Random(-0.05f, 0.05f),
                             0.0f,
                             Random(-0.05f, 0.05f)));
        DeselectAll();
        if (CheckQuatter())
            MC->Quatter();

        return true;

    } else return false;
}

bool Board::CheckQuatter()
{
    bool checkBlocks{true};

    //Check rows
    for (int i{0}; i < BOARD_WIDTH; ++i){
        Piece::Attributes matching{};
        matching.flip();
        Piece::Attributes first{};
        for (int j{0}; j < BOARD_HEIGHT; ++j){
            IntVector2 coords(i, j);
            Piece* piece{squares_[coords].Get()->piece_};
            if (piece){
                Piece::Attributes attributes{piece->GetAttributes()};
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
    for (int j{0}; j < BOARD_HEIGHT; ++j){
        Piece::Attributes matching{};
        matching.flip();
        Piece::Attributes first{};
        for (int i{0}; i < BOARD_WIDTH; ++i){
            IntVector2 coords(i, j);
            Piece* piece{squares_[coords].Get()->piece_};
            if (piece){
                Piece::Attributes attributes(piece->GetAttributes());
                if (i == 0) {
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
        Piece::Attributes matching{};
        matching.flip();
        Piece::Attributes first{};
        for (int i{0}; i < BOARD_WIDTH; ++i){
            IntVector2 coords(i, direction ? i : (BOARD_WIDTH - i - 1));
            Piece* piece{squares_[coords].Get()->piece_};
            if (piece){
                Piece::Attributes attributes(piece->GetAttributes());
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
    //Check 2x2 blocks
    if (checkBlocks){
        for (int k{0}; k < BOARD_WIDTH - 1; ++k){
            for (int l{0}; l < BOARD_HEIGHT - 1; ++l){
                Piece::Attributes matching{};
                matching.flip();
                Piece::Attributes first{};
                for (int m : {0, 1}) for (int n : {0, 1}){
                    IntVector2 coords(k + m, l + n);
                    Piece* piece{squares_[coords].Get()->piece_};
                    if (piece){
                        Piece::Attributes attributes(piece->GetAttributes());
                        if (m == 0 && n == 0) {
                            first = attributes;
                        } else {
                            for (int a{0}; a < NUM_ATTRIBUTES; ++a)
                                if (first[a] != attributes[a])
                                    matching[a] = false;
                        }
                        //Full block required
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
    }

    //No Quatter
    return false;
}

Square* Board::GetNearestSquare(Vector3 pos, bool free)
{
    Square* nearest{};
    for (Square* s : squares_.Values()){
        if (!nearest ||
            LucKey::Distance(s->node_->GetWorldPosition(), pos) <
            LucKey::Distance(nearest->node_->GetWorldPosition(), pos))
            if (s->free_ || !free)
                nearest = s;
    }
    return nearest;
}
void Board::SelectNearestFreeSquare(Vector3 pos)
{
    Square* square{GetNearestSquare(pos, true)};
    if (square) Select(square);
}
void Board::SelectNearestSquare(Vector3 pos)
{
    Square* square{GetNearestSquare(pos, false)};
    if (square) Select(square);
}
bool Board::SelectLast()
{
    if (lastSelectedSquare_ && lastSelectedSquare_ != selectedSquare_) {
        Select(lastSelectedSquare_);
        return true;
    } else return false;
}

void Board::Select(Square* square)
{
    if (selectedSquare_)
        Deselect(selectedSquare_);

    selectedSquare_ = square;
    square->selected_ = true;

    //Fade in slot and light
    if (square->free_){
        FX->FadeTo(square->slot_->GetMaterial(),
                   COLOR_GLOW);
    } else {
        FX->FadeTo(square->slot_->GetMaterial(),
                   Color(1.0f, 0.8f, 0.0f, 0.5f));
    }

    FX->FadeTo(square->light_, 0.42f);
}
void Board::Deselect(Square* square)
{
    if (!square) return;

    if (selectedSquare_ == square){
        lastSelectedSquare_ = selectedSquare_;
        selectedSquare_ = nullptr;
    }

    square->selected_ = false;

    //Fade out slot and light
    FX->FadeOut(square->slot_->GetMaterial());
    FX->FadeTo(square->light_, 0.023f);
}
void Board::DeselectAll()
{
    for (Square* s: squares_.Values()){
        Deselect(s);
    }
}

void Board::Step(IntVector2 step)
{
    if (selectedSquare_){
        IntVector2 newCoords{selectedSquare_->coords_ + step};
        if (squares_.Contains(newCoords)){
            Select(squares_[newCoords].Get());
        }
    } else if (lastSelectedSquare_)
        SelectLast();
    else {
        SelectNearestFreeSquare(CAMERA->GetPosition());
    }
}
