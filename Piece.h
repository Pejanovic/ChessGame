#ifndef PIECE_H
#define PIECE_H

#include <string>
#include <vector>
#include <utility>
#include <memory>
#include "Position.h"

enum class PieceType { Pawn, Rook, Knight, Bishop, Queen, King };
enum class Color { White, Black };

class Piece {
private:
    std::string name;                          // Naziv figure
    PieceType type;                            // Tip figure
    Color color;                               // Boja figure
    std::vector<Position> possibleMoves;       // Lista mogućih poteza
    Position currentPosition;                  // Trenutna pozicija figure
    std::string imagePath;                     // Putanja do slike figure
    bool hasMoved;                             // Da li se figura pomjerala
    int pointValue;                            // Vrednost figure u poenima
    bool isCaptured;
    bool isBlackKingInCheck;
    bool isWhiteKingInCheck;
    bool checkmate;

    void calculatePawnMoves(const std::vector<std::vector<Piece*>>& board, int row, int col);
    void calculateLinearMoves(const std::vector<std::vector<Piece*>>& board, int row, int col, const std::vector<std::pair<int, int>>& directions);
    void calculateKingMoves(const std::vector<std::vector<Piece*>>& board, int row, int col);
    void calculateKnightMoves(const std::vector<std::vector<Piece*>>& board, int row, int col);

public:
    Piece(const std::string& name, PieceType type, Color color, const Position& initialPosition, const std::string& imagePath, int pointValue)
        : name(name), type(type), color(color), currentPosition(initialPosition), imagePath(imagePath), hasMoved(false), pointValue(pointValue), isCaptured(false), isBlackKingInCheck(false), isWhiteKingInCheck(false),
            checkmate(false) {}

    std::string getName() const { return name; }
    PieceType getType() const { return type; }
    Color getColor() const { return color; }
    Position getCurrentPosition() const { return currentPosition; }
    const std::vector<Position>& getPossibleMoves() const { return possibleMoves; }
    std::string getImagePath() const { return imagePath; }
    int getPointValue() const { return pointValue; }
    bool getIsCaptured() const { return isCaptured; }
    void printChessboard(const std::vector<std::vector<Piece*>>& board) const;
    void setPossibleMoves(const std::vector<Position>& moves) {
        possibleMoves = moves;
    }
    void setCurrentPosition(const Position& pos) {
        currentPosition = pos;
    }

   
    bool isAt(int row, int col) const;
    void setPosition(int row, int col, const std::string& nazivPolja, std::vector<std::vector<Piece*>>& board);
    void calculatePossibleMoves(const std::vector<std::vector<Piece*>>& board);
    void capture(std::vector<std::vector<Piece*>>& board);
    bool isCheckmate(const std::vector<std::unique_ptr<Piece>>& pieces, std::vector<std::vector<Piece*>>& board, Color kingColor);
    static bool isKingInCheck(const std::vector<std::unique_ptr<Piece>>& pieces, const std::vector<std::vector<Piece*>>& board, Color kingColor);
    std::vector<Position> filterMovesToAvoidCheck(
        const std::vector<Position>& possibleMoves,
        const std::vector<std::unique_ptr<Piece>>& pieces,
        std::vector<std::vector<Piece*>>& board,
        Color kingColor
    );

    void clearPossibleMoves();

};

#endif // PIECE_H
