#ifndef POSITION_H
#define POSITION_H

#include <string>

// Forward deklaracija klase Piece (izbegava kružnu zavisnost)
class Piece;

class Position {
private:
    float xGL;
    float yGL;
    std::string nazivPolja; // Polje na šahovskoj tabli (npr. A1, B2)
    int row;                // Red na šahovskoj tabli
    int column;             // Kolona na šahovskoj tabli
    Piece* occupyingPiece;

public:
    Position(float xGL, float yGL, const std::string& nazivPolja, int row, int column)
        : xGL(xGL), yGL(yGL), nazivPolja(nazivPolja), row(row), column(column), occupyingPiece(nullptr) {}

    Position( int row, int column)
        : row(row), column(column) {}

    Position(int row, int column, const std::string& nazivPolja)
        : row(row), column(column), nazivPolja(nazivPolja),
        xGL(-0.875f + column * 0.25f), yGL(0.875f - row * 0.25f),
        occupyingPiece(nullptr) {}

    Position() : xGL(0), yGL(0), nazivPolja(""), row(-1), column(-1), occupyingPiece(nullptr) {}

    float getXGL() const { return xGL; }
    void setXGL(float newXGL) { xGL = newXGL; }

    float getYGL() const { return yGL; }
    void setYGL(float newYGL) { yGL = newYGL; }

    std::string getNazivPolja() const { return nazivPolja; }
    void setNazivPolja(const std::string& newNazivPolja) { nazivPolja = newNazivPolja; }

    int getRow() const { return row; }
    void setRow(int newRow) { row = newRow; }

    int getColumn() const { return column; }
    void setColumn(int newColumn) { column = newColumn; }

    Piece* getOccupyingPiece() const { return occupyingPiece; }
    void setOccupyingPiece(Piece* piece) { occupyingPiece = piece; }

    bool operator==(const Position& other) const {
        return this->row == other.row && this->column == other.column;
    }

    bool operator!=(const Position& other) const {
        return !(*this == other);
    }

};

#endif // POSITION_H
