#include "Piece.h"
#include "Position.h"
#include <vector>
#include <iostream>

bool Piece::isAt(int row, int col) const {
    return currentPosition.getRow() == row && currentPosition.getColumn() == col;
}

void Piece::setPosition(int row, int col, const std::string& nazivPolja, std::vector<std::vector<Piece*>>& board) {

    printChessboard(board);

    if (isCaptured) {
        std::cerr << "Cannot move a captured piece: " << name << "\n";
        return;
    }

    auto it = std::find_if(possibleMoves.begin(), possibleMoves.end(), [row, col](const Position& pos) {
        return pos.getRow() == row && pos.getColumn() == col;
        });

    if (it == possibleMoves.end()) {
        std::cerr << "Invalid move for piece: " << name << " to position (" << row << ", " << col << ")\n";
        return;
    }

    Piece* targetPiece = board[row][col];

    if (targetPiece != nullptr && targetPiece->getColor() != color) {
        std::cout << "Piece " << name << " ate " << targetPiece->getName() << std::endl;
        targetPiece->capture(board);
        board[row][col] = nullptr;
    }
    else if (targetPiece != nullptr && targetPiece->getColor() == color) {
        std::cerr << "Cannot move to a position occupied by a friendly piece: (" << row << ", " << col << ")\n";
        return;
    }

    int oldRow = currentPosition.getRow();
    int oldCol = currentPosition.getColumn();

    currentPosition.setRow(row);
    currentPosition.setColumn(col);
    currentPosition.setNazivPolja(nazivPolja);

    currentPosition.setXGL(-0.875f + col * 0.25f);
    currentPosition.setYGL(0.875f - row * 0.25f);

    hasMoved = true;

    board[oldRow][oldCol] = nullptr;
    board[row][col] = this;

}

void Piece::calculatePossibleMoves(const std::vector<std::vector<Piece*>>& board) {

    if (isCaptured) {
        possibleMoves.clear();
        return;
    }

    possibleMoves.clear();

    int row = currentPosition.getRow();
    int col = currentPosition.getColumn();

    switch (type) {
    case PieceType::Pawn:
        calculatePawnMoves(board, row, col);
        break;
    case PieceType::Rook:
        calculateLinearMoves(board, row, col, { {0, 1}, {1, 0}, {0, -1}, {-1, 0} });
        break;
    case PieceType::Bishop:
        calculateLinearMoves(board, row, col, { {1, 1}, {1, -1}, {-1, 1}, {-1, -1} });
        break;
    case PieceType::Queen:
        calculateLinearMoves(board, row, col, { {0, 1}, {1, 0}, {0, -1}, {-1, 0}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1} });
        break;
    case PieceType::King:
        calculateKingMoves(board, row, col);
        break;
    case PieceType::Knight:
        calculateKnightMoves(board, row, col);
        break;
    }
}

bool Piece::isKingInCheck(const std::vector<std::unique_ptr<Piece>>& pieces, const std::vector<std::vector<Piece*>>& board, Color kingColor) {
    Position kingPosition;
    for (const auto& piece : pieces) {
        if (piece->getType() == PieceType::King && piece->getColor() == kingColor) {
            kingPosition = piece->getCurrentPosition();
            break;
        }
    }

    for (const auto& piece : pieces) {
        if (piece->getColor() != kingColor) {
            piece->calculatePossibleMoves(board);
            const auto& moves = piece->getPossibleMoves();
            for (const auto& move : moves) {
                if (move.getRow() == kingPosition.getRow() && move.getColumn() == kingPosition.getColumn()) {
                    return true;
                }
            }
        }
    }

    return false;
}

void Piece::printChessboard(const std::vector<std::vector<Piece*>>& board) const {
    /*std::cout << "Current Chessboard State:\n";
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            Piece* piece = board[row][col];
            if (piece) {
                std::cout << "[" << row << "," << col << " - "
                    << (piece->getColor() == Color::White ? "White" : "Black") << " "
                    << piece->getName() << "] ";
            }
            else {
                std::cout << "[" << row << "," << col << " - Empty] ";
            }
        }
        std::cout << "\n";
    }
    std::cout << "\n";*/
}

void Piece::calculatePawnMoves(const std::vector<std::vector<Piece*>>& board, int row, int col) {
    int direction = (color == Color::White) ? -1 : 1;

    // Pomjeraj jedno polje unapred ako je prazno
    if (row + direction >= 0 && row + direction < 8 && board[row + direction][col] == nullptr) {
        possibleMoves.emplace_back(row + direction, col, "");
    }

    // Pomjeraj dva polja unapred samo ako je pijun na početnom položaju i oba polja su prazna
    int startingRow = (color == Color::White) ? 6 : 1;
    if (row == startingRow && board[row + direction][col] == nullptr && board[row + 2 * direction][col] == nullptr) {
        possibleMoves.emplace_back(row + 2 * direction, col, "");
    }

    // Napad koso na levo
    if (row + direction >= 0 && row + direction < 8 && col > 0 &&
        board[row + direction][col - 1] != nullptr &&
        board[row + direction][col - 1]->getColor() != color) {
        possibleMoves.emplace_back(row + direction, col - 1, "");
    }

    // Napad koso na desno
    if (row + direction >= 0 && row + direction < 8 && col < 7 &&
        board[row + direction][col + 1] != nullptr &&
        board[row + direction][col + 1]->getColor() != color) {
        possibleMoves.emplace_back(row + direction, col + 1, "");
    }
}

void Piece::calculateLinearMoves(const std::vector<std::vector<Piece*>>& board, int row, int col, const std::vector<std::pair<int, int>>& directions) {
    for (const auto& dir : directions) {
        int newRow = row;
        int newCol = col;

        while (true) {
            newRow += dir.first;
            newCol += dir.second;

            if (newRow < 0 || newRow >= 8 || newCol < 0 || newCol >= 8) break;

            if (board[newRow][newCol] == nullptr) {
                possibleMoves.emplace_back(newRow, newCol, "");
            }
            else {
                if (board[newRow][newCol]->getColor() != color) {
                    possibleMoves.emplace_back(newRow, newCol, "");
                }
                break;
            }

            if (board[newRow][newCol] != nullptr && board[newRow][newCol]->getColor() == color) {
                break;
            }
        }
    }
}

void Piece::calculateKingMoves(const std::vector<std::vector<Piece*>>& board, int row, int col) {
    const std::vector<std::pair<int, int>> directions = {
        {0, 1}, {1, 0}, {0, -1}, {-1, 0},
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
    };

    for (const auto& dir : directions) {
        int newRow = row + dir.first;
        int newCol = col + dir.second;

        if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8) {
            if (board[newRow][newCol] == nullptr || board[newRow][newCol]->getColor() != color) {
                possibleMoves.emplace_back(newRow, newCol, "");
            }
        }
    }
}


void Piece::calculateKnightMoves(const std::vector<std::vector<Piece*>>& board, int row, int col) {
    const std::vector<std::pair<int, int>> moves = {
        {2, 1}, {2, -1}, {-2, 1}, {-2, -1},
        {1, 2}, {1, -2}, {-1, 2}, {-1, -2}
    };

    for (const auto& move : moves) {
        int newRow = row + move.first;
        int newCol = col + move.second;

        if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8) {
            if (board[newRow][newCol] == nullptr || board[newRow][newCol]->getColor() != color) {
                possibleMoves.emplace_back(newRow, newCol, "");
            }
        }
    }
}

void Piece::capture(std::vector<std::vector<Piece*>>& board) {
    isCaptured = true;
    currentPosition.setXGL(9999);
    currentPosition.setYGL(9999);
    possibleMoves.clear();

    int row = currentPosition.getRow();
    int col = currentPosition.getColumn();
    if (row >= 0 && row < 8 && col >= 0 && col < 8) {
        board[row][col] = nullptr;
    }
}

std::vector<Position> Piece::filterMovesToAvoidCheck(
    const std::vector<Position>& possibleMoves,
    const std::vector<std::unique_ptr<Piece>>& pieces,
    std::vector<std::vector<Piece*>>& board,
    Color kingColor) {

    std::vector<Position> validMoves;

    // Pronađi kralja
    Position kingPosition;
    for (const auto& piece : pieces) {
        if (piece->getType() == PieceType::King && piece->getColor() == kingColor) {
            kingPosition = piece->getCurrentPosition();
            break;
        }
    }

    Position currentPos = this->getCurrentPosition();

    for (const auto& move : possibleMoves) {
        int targetRow = move.getRow();
        int targetCol = move.getColumn();

        // Sačuvaj stanje mete
        Piece* capturedPiece = board[targetRow][targetCol];

        // Simuliraj potez
        board[targetRow][targetCol] = board[currentPos.getRow()][currentPos.getColumn()];
        board[currentPos.getRow()][currentPos.getColumn()] = nullptr;

        // Ako hvatamo protivničku figuru, privremeno je ukloni iz kalkulacija
        Position oldCapturedPos(-1, -1);
        bool capturedEnemy = false;
        if (capturedPiece != nullptr && capturedPiece->getColor() != kingColor) {
            // Sačuvaj staru poziciju
            oldCapturedPos = capturedPiece->getCurrentPosition();
            // Postavi je van table
            capturedPiece->setCurrentPosition(Position(-1, -1));
            capturedEnemy = true;
        }

        // Provjeri da li je kralj i dalje u šahu nakon ovog poteza
        bool stillInCheck = Piece::isKingInCheck(pieces, board, kingColor);

        // Vrati tablu u prethodno stanje
        board[currentPos.getRow()][currentPos.getColumn()] = board[targetRow][targetCol];
        board[targetRow][targetCol] = capturedPiece;

        // Vrati napadaču staru poziciju ako je bio "uhvaćen"
        if (capturedEnemy) {
            capturedPiece->setCurrentPosition(oldCapturedPos);
        }

        // Ako nakon poteza kralj nije u šahu, onda je taj potez validan
        if (!stillInCheck) {
            validMoves.push_back(move);
        }
    }

    return validMoves;
}


void Piece::clearPossibleMoves() {
    possibleMoves.clear();
}



