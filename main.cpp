#define STB_IMAGE_IMPLEMENTATION

#include <glad/gl.h> 
#include <GLFW/glfw3.h> 
#include <iostream>
#include <fstream>
#include <sstream>
#include "stb_image.h"
#include <vector>
#include "Piece.h"
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include "TextRenderer.h"

// Deklaracije funkcija
unsigned int createShaderProgram(const char* vertexPath, const char* fragmentPath);
unsigned int loadTexture(const char* path);
void setupChessboardVAO(unsigned int& VAO, unsigned int& VBO, unsigned int& EBO);
void drawChessboard(unsigned int shader, unsigned int VAO, unsigned int texture);
std::string readShaderFile(const char* filePath);
void mouseToOpenGL(GLFWwindow* window, double xpos, double ypos, float& xOut, float& yOut);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
std::vector<std::unique_ptr<Piece>> initializeChessPieces();
void drawPieces(const std::vector<std::unique_ptr<Piece>>& pieces, unsigned int shader, unsigned int pieceVAO);
void setupPieceVAO(unsigned int& VAO, unsigned int& VBO, unsigned int& EBO);
void drawPossibleMoves(const std::vector<Position>& moves, unsigned int shader, unsigned int VAO);
void setupMoveVAO(unsigned int& VAO, unsigned int& VBO, unsigned int& EBO);
std::string toChessNotation(int row, int col);
bool isCheckmate(const std::vector<std::unique_ptr<Piece>>& pieces, std::vector<std::vector<Piece*>>& board, Color kingColor);
void drawTimer(float whiteTimeLeft, float blackTimeLeft, bool isWhiteTurn);
unsigned int createTextShader();

Piece* selectedPiece = nullptr;
std::vector<std::unique_ptr<Piece>> pieces;
std::vector<std::vector<Piece*>> board(8, std::vector<Piece*>(8, nullptr));
bool isWhiteTurn = true;
float whiteTimeLeft = 25 * 60.0f;
float blackTimeLeft = 25 * 60.0f;
double lastTime = glfwGetTime();
bool isPaused = false;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        isPaused = !isPaused; // Prebacivanje između pauze i pokretanja
        std::cout << (isPaused ? "Timer paused." : "Timer resumed.") << std::endl;
    }
}

// Provjera OpenGL grešaka
void checkGLError(const std::string& context) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL Error at " << context << ": " << error << std::endl;
    }
}

std::string pieceTypeToString(PieceType type) {
    switch (type) {
    case PieceType::King: return "King";
    case PieceType::Queen: return "Queen";
    case PieceType::Rook: return "Rook";
    case PieceType::Bishop: return "Bishop";
    case PieceType::Knight: return "Knight";
    case PieceType::Pawn: return "Pawn";
    default: return "Unknown";
    }
}



int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Postavljanje opcija za GLFW proyor, važno zbog fiksnog pipelin - a i nove verzije OpenGL - a
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow* window = glfwCreateWindow(800, 900, "Chessboard", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window); // OpenGL funkcije djeluju na trenutni kontekst, a ovdje ga mi postavimo kao trenutni 
    glfwSetKeyCallback(window, keyCallback); // Registracija funkcije povratnog poziva(CallBack funkcija) kada korisnik pritisne taster da vrati taj info
    if (!gladLoaderLoadGL()) {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return -1;
    }

    checkGLError("GLAD Initialization");

    // 2. Učitavanje šejdera i resursa
    unsigned int shaderProgram = createShaderProgram("basic.vert", "basic.frag");
    if (shaderProgram == 0) return -1;

    glEnable(GL_BLEND); // omogucava konfigurisanje blendovanja boja sto znaci da mozemo da spajamo boje, transparentnost, opacity... 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    unsigned int textShader = createShaderProgram("text.vert", "text.frag");
    if (textShader == 0) return -1;

    unsigned int texture = loadTexture("res/chessboard.png");
    if (texture == 0) return -1;

    unsigned int VAO, VBO, EBO; // osnovni objekti u OpenGL - VAO skladisti informacije o tjemenima, VBO - teksture, boje, koordinate, EBO - indekse za crtanje(reodlsijed kojim se crta)
    setupChessboardVAO(VAO, VBO, EBO);

    unsigned int pieceVAO, pieceVBO, pieceEBO;
    setupPieceVAO(pieceVAO, pieceVBO, pieceEBO);

    unsigned int moveVAO, moveVBO, moveEBO;
    setupMoveVAO(moveVAO, moveVBO, moveEBO);

    pieces = initializeChessPieces();

    // Inicijalizacija TextRenderer-a
    TextRenderer textRenderer(800, 900);
    textRenderer.loadFont("Montserrat-Regular.ttf");
    textRenderer.setProjection(textShader, 800, 100);

    glfwSetMouseButtonCallback(window, mouseButtonCallback); // Opet CallBack funckija, vraca info o kliknutom misu

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime(); // Skladistimo vrijeme koje je proslo od posledje iteracije
        double deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        if (!isPaused) {
            if (isWhiteTurn) {
                whiteTimeLeft -= deltaTime;
                if (whiteTimeLeft <= 0) {
                    std::cout << "Time's up! Black wins!" << std::endl;
                    glfwSetWindowShouldClose(window, true);
                }
            }
            else {
                blackTimeLeft -= deltaTime;
                if (blackTimeLeft <= 0) {
                    std::cout << "Time's up! White wins!" << std::endl;
                    glfwSetWindowShouldClose(window, true);
                }
            }
        }

        // Brisanje ekrana
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Crtanje tekstualnog interfejsa
        glViewport(0, 800, 800, 100); // Prostor za tekst, odnosno gornji prozor
        glUseProgram(textShader);

        int whiteMinutes = static_cast<int>(whiteTimeLeft / 60);
        int whiteSeconds = static_cast<int>(fmod(whiteTimeLeft, 60.0f));
        std::string whiteTime = "White Timer: " + std::to_string(whiteMinutes) + ":" +
            (whiteSeconds < 10 ? "0" : "") + std::to_string(whiteSeconds);

        int blackMinutes = static_cast<int>(blackTimeLeft / 60);
        int blackSeconds = static_cast<int>(fmod(blackTimeLeft, 60.0f));
        std::string blackTime = "Black Timer: " + std::to_string(blackMinutes) + ":" +
            (blackSeconds < 10 ? "0" : "") + std::to_string(blackSeconds);

        // Ispisuje preostalo vrijeme za oba igrača
        textRenderer.renderText(textShader, whiteTime, 10.0f, 80.0f, 0.8f, glm::vec3(1.0f, 1.0f, 1.0f));
        textRenderer.renderText(textShader, blackTime, 10.0f, 50.0f, 0.8f, glm::vec3(1.0f, 1.0f, 1.0f));
        
        
        textRenderer.renderText(textShader, "Nikola Pejanovic RA 237-2021", 10.0f, 20.0f, 0.4f, glm::vec3(0.8f, 0.3f, 0.2f));

        std::string currentPlayer = isWhiteTurn ? "White's Turn" : "Black's Turn";
        float textWidth = textRenderer.calculateTextWidth(currentPlayer, 0.8f);
        float xPosition = 800.0f - textWidth - 10.0f; // Desna strana sa marginom od 10 piksela
        textRenderer.renderText(textShader, currentPlayer, xPosition, 40.0f, 0.8f, glm::vec3(1.0f, 1.0f, 1.0f));


        // Crtanje šahovske table
        glViewport(0, 0, 800, 800); // Prostor za tablu
        drawChessboard(shaderProgram, VAO, texture);
        drawPieces(pieces, shaderProgram, pieceVAO);

        // Crtanje mogućih poteza
        if (selectedPiece) {
            drawPossibleMoves(selectedPiece->getPossibleMoves(), shaderProgram, moveVAO);
        }

        // Mjenjamo bafere i procesiramo događaje
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 5. Oslobađanje resursa
    glDeleteProgram(shaderProgram);
    glDeleteProgram(textShader);
    glDeleteTextures(1, &texture);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &pieceVAO);
    glDeleteBuffers(1, &pieceVBO);
    glDeleteBuffers(1, &pieceEBO);
    glDeleteVertexArrays(1, &moveVAO);
    glDeleteBuffers(1, &moveVBO);
    glDeleteBuffers(1, &moveEBO);

    glfwTerminate();
    return 0;
}


unsigned int createTextShader() {
    return createShaderProgram("text.vert", "text.frag");
}

// funkcija za ucitavanje sadrzaja datoteke sejdera u string 
std::string readShaderFile(const char* filePath) { 
    std::ifstream file(filePath, std::ios::binary); // otvaramo datoteku
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string code = buffer.str();

    // Uklanjanje BOM ako postoji, BOM (Byte Order Mark) je specijalni niz bajtova na početku datoteke koji označava njeno enkodiranje, obično UTF-8.
    if (code.size() >= 3 &&
        static_cast<unsigned char>(code[0]) == 0xEF &&
        static_cast<unsigned char>(code[1]) == 0xBB &&
        static_cast<unsigned char>(code[2]) == 0xBF) {
        code = code.substr(3);
    }

    return code;
}

// Kreiranje šejder programa
unsigned int createShaderProgram(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCode = readShaderFile(vertexPath);
    std::string fragmentCode = readShaderFile(fragmentPath);

    const char* vertexShaderSource = vertexCode.c_str();
    const char* fragmentShaderSource = fragmentCode.c_str();

    // Kompajliranje Vertex Shader-a
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); // Prosledjujemo GLSL kod sejderu 
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glBindAttribLocation(shaderProgram, 0, "aPos"); // Ova funkcija mapira varijable iz GLSL koda na indekse koje se koriste u aplikaciji. 0 i 1
    glBindAttribLocation(shaderProgram, 1, "aTexCoord");

    glLinkProgram(shaderProgram); // Povezemo sejder programe frag i vert

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void drawTimer(float whiteTimeLeft, float blackTimeLeft, bool isWhiteTurn) {

    int whiteMinutes = (int)(whiteTimeLeft / 60);
    int whiteSeconds = (int)((int)whiteTimeLeft % 60);
    int blackMinutes = (int)(blackTimeLeft / 60);
    int blackSeconds = (int)((int)blackTimeLeft % 60);

    std::cout << "White Time: " << whiteMinutes << ":" << (whiteSeconds < 10 ? "0" : "") << whiteSeconds << "\n";
    std::cout << "Black Time: " << blackMinutes << ":" << (blackSeconds < 10 ? "0" : "") << blackSeconds << std::endl;

}

void setupMoveVAO(unsigned int& VAO, unsigned int& VBO, unsigned int& EBO) {
    float vertices[] = {
        // Pozicije      // Teksturne koordinate
        -0.05f, -0.05f,  0.0f, 0.0f, // Donji lijevi ugao
         0.05f, -0.05f,  1.0f, 0.0f, // Donji desni
         0.05f,  0.05f,  1.0f, 1.0f, // Gornji desni
        -0.05f,  0.05f,  0.0f, 1.0f  // Gornji lijevi
    };

    unsigned int indices[] = {
        0, 1, 2, // dva trougla = kvadrat
        2, 3, 0
    };

    glGenVertexArrays(1, &VAO); // kreirako i cuvao verteks podatke
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO); // sve dalje se odnosi na ovaj vao tj aktiviramo ga

    glBindBuffer(GL_ARRAY_BUFFER, VBO); // aktiviramo vbo kao trenutni bafer za podatke
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // prenosimo podatke iz matrice verteksa u GPU

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0); // postavljamo kako OpenGL tretira podatke, 0 - indeks atributa, 2 - komponent x i y, GL_FLOAT - tip podatka, sirina svakog verteksa
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void drawChessboard(unsigned int shader, unsigned int VAO, unsigned int texture) {
    glUseProgram(shader);

    glm::mat4 model = glm::mat4(1.0f); // Matrica bez transformacija, sto znaci objekat ce biti nacrtan na svojoj poziciji
    unsigned int modelLoc = glGetUniformLocation(shader, "model"); // Traži lokaciju uniformne promenljive model u šejder programu (npr. u vertex shader-u). Uniformna promjenljiva se obicno koristi za transformaciju objekata u prostoru
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); // Postavlja vrijednosti uniformne matrice model u sejder programa

    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0);
}



unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Postavlja parametre za teksturu npr. teskura se ponavlja ako su koordinate izvan opsega -1 i 1
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // kako se tekstura ponasa pri uvecanju ili umanjenju
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }
    else {
        std::cerr << "Failed to load texture: " << path << std::endl;
        stbi_image_free(data);
        return 0;
    }

    return textureID;
}


// VAO, VBO i EBO za šahovsku tablu
void setupChessboardVAO(unsigned int& VAO, unsigned int& VBO, unsigned int& EBO) {
    float vertices[] = {
        // Pozicije      // Teksturne koordinate
        -1.0f, -1.0f,   0.0f, 0.0f, // Donji lijevi
         1.0f, -1.0f,   1.0f, 0.0f, // Donji desni
         1.0f,  1.0f,   1.0f, 1.0f, // Gornji desni
        -1.0f,  1.0f,   0.0f, 1.0f  // Gornji lijevi
    };


    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0); // aPos
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))); // aTexCoord
    glEnableVertexAttribArray(1);


    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void mouseToOpenGL(GLFWwindow* window, double xpos, double ypos, float& xOut, float& yOut) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Provjeri da li je kliknut region šahovske table
    if (ypos < 100 || ypos > 900 || xpos > 800) {
        xOut = yOut = -1.0f; // Klik je izvan šahovske table
        return;
    }

    // Mapiranje iz piksel prostora (800x800 tabla) u OpenGL prostor (-1.0 do 1.0)
    xOut = 2.0f * (float)xpos / 800.0f - 1.0f;          // X koordinata na osnovu širine šahovske table
    yOut = 1.0f - 2.0f * (float)(ypos - 100) / 800.0f;  // Y koordinata na osnovu visine table (od 100 do 900)
}

bool canMove(Color pieceColor) {
    // Provjerava da li figura može da igra na osnovu poteza
    return (pieceColor == Color::White && isWhiteTurn) || (pieceColor == Color::Black && !isWhiteTurn);
}


void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        float xGL, yGL;
        mouseToOpenGL(window, xpos, ypos, xGL, yGL);

        if (xGL == -1.0f || yGL == -1.0f) {
            std::cout << "Click outside chessboard!" << std::endl;
            return;
        }

        // Mapiranje OpenGL koordinata na redove i kolone šahovske table
        int col = (int)((xGL + 1.0f) / (2.0f / 8.0f)); // 8 kolona
        int row = (int)((1.0f - yGL) / (2.0f / 8.0f)); // 8 redova

        if (col >= 0 && col < 8 && row >= 0 && row < 8) {
            if (selectedPiece == nullptr) {
                for (const auto& piece : pieces) {
                    if (piece && piece->isAt(row, col) && !piece->getIsCaptured()) {

                        if (!canMove(piece->getColor())) {
                            std::cout << "Not your turn!" << std::endl;
                            return;
                        }

                        selectedPiece = piece.get();

                        // Dodaj ispis za selektovanu figuru
                        std::cout << "Selected piece: " << piece->getName() << "\n";
                        std::cout << "Color: " << (piece->getColor() == Color::White ? "White" : "Black") << "\n";
                        std::cout << "Current Position: "
                            << toChessNotation(piece->getCurrentPosition().getRow(),
                                piece->getCurrentPosition().getColumn()) << "\n";
                        std::cout << "Possible Moves: ";
                        piece->calculatePossibleMoves(board);
                        for (const auto& move : piece->getPossibleMoves()) {
                            std::cout << toChessNotation(move.getRow(), move.getColumn()) << " ";
                        }
                        std::cout << "\n";

                        // Ako je kralj u šahu, filtriraj poteze
                        if (Piece::isKingInCheck(pieces, board, selectedPiece->getColor())) {
                            auto filteredMoves = selectedPiece->filterMovesToAvoidCheck(
                                selectedPiece->getPossibleMoves(),
                                pieces,
                                board,
                                selectedPiece->getColor()
                            );


                            // Postavi filtrirane poteze kao validne poteze
                            selectedPiece->setPossibleMoves(filteredMoves);
                        }

                        break;
                    }
                }
            }
            else {
                // Provjeri da li je kliknuto na već selektovanu figuru
                if (selectedPiece->isAt(row, col)) {
                    std::cout << "Deselected piece: " << selectedPiece->getName() << std::endl;
                    selectedPiece = nullptr; // Deselektovanje figure
                    return;
                }

                // Pokušaj pomeranja figure
                std::string nazivPolja = "";
                auto previousPosition = selectedPiece->getCurrentPosition();
                int oldRow = previousPosition.getRow();
                int oldCol = previousPosition.getColumn();

                selectedPiece->setPosition(row, col, nazivPolja, board);

                // Provjera da li je figura stvarno pomjerena
                auto newPosition = selectedPiece->getCurrentPosition();
                if (newPosition.getRow() == oldRow && newPosition.getColumn() == oldCol) {
                    // Figura nije pomjerena
                    std::cout << "Invalid move! Try again." << std::endl;
                    return;
                }

                // Ispis pomeranja figure
                std::cout << selectedPiece->getName() << " moved from "
                    << toChessNotation(oldRow, oldCol) << " to "
                    << toChessNotation(row, col) << "." << std::endl;

                // Provjera šaha ili šah-mata
                if (Piece::isKingInCheck(pieces, board, Color::White)) {
                    std::cout << "White King is in check!" << std::endl;
                    if (isCheckmate(pieces, board, Color::White)) {
                        std::cout << "Checkmate! Black wins!" << std::endl;
                        glfwSetWindowShouldClose(window, true);
                        return;
                    }
                }

                if (Piece::isKingInCheck(pieces, board, Color::Black)) {
                    std::cout << "Black King is in check!" << std::endl;
                    if (isCheckmate(pieces, board, Color::Black)) {
                        std::cout << "Checkmate! White wins!" << std::endl;
                        glfwSetWindowShouldClose(window, true);
                        return;
                    }
                }

                // Prebacivanje poteza na drugog igrača
                isWhiteTurn = !isWhiteTurn;
                selectedPiece = nullptr;
            }
        }
    }
}



std::string toChessNotation(int row, int col) {
    char colChar = 'A' + col;  // Pretvara broj u odgovarajući karakter (0 -> 'A', 1 -> 'B', ...)
    int rowNum = 8 - row;  // Redovi se numerišu od 1 do 8, pa je potrebno invertovati broj

    return std::string(1, colChar) + std::to_string(rowNum);  // Kombinovanje kolone i reda
}



std::vector<std::unique_ptr<Piece>> initializeChessPieces() {
    std::vector<std::unique_ptr<Piece>> pieces;

    for (int i = 0; i < 8; ++i) {
        pieces.push_back(std::make_unique<Piece>("Pawn", PieceType::Pawn, Color::White, Position(-0.875f + i * 0.25f, -0.625f, "A2", 6, i), "res/white_pawn.png", 1));
        board[6][i] = pieces.back().get(); // Postavljanje belih pjesaka na tablu
    }

    // Bijele figure
    pieces.push_back(std::make_unique<Piece>("Rook", PieceType::Rook, Color::White, Position(-0.875f, -0.875f, "A1", 7, 0), "res/white_rook.png", 5));
    board[7][0] = pieces.back().get();
    pieces.push_back(std::make_unique<Piece>("Knight", PieceType::Knight, Color::White, Position(-0.625f, -0.875f, "B1", 7, 1), "res/white_horse.png", 3));
    board[7][1] = pieces.back().get();
    pieces.push_back(std::make_unique<Piece>("Bishop", PieceType::Bishop, Color::White, Position(-0.375f, -0.875f, "C1", 7, 2), "res/white_bishop.png", 3));
    board[7][2] = pieces.back().get();
    pieces.push_back(std::make_unique<Piece>("Queen", PieceType::Queen, Color::White, Position(-0.125f, -0.875f, "D1", 7, 3), "res/white_queen.png", 9));
    board[7][3] = pieces.back().get();
    pieces.push_back(std::make_unique<Piece>("King", PieceType::King, Color::White, Position(0.125f, -0.875f, "E1", 7, 4), "res/white_king.png", 10));
    board[7][4] = pieces.back().get();
    pieces.push_back(std::make_unique<Piece>("Bishop", PieceType::Bishop, Color::White, Position(0.375f, -0.875f, "F1", 7, 5), "res/white_bishop.png", 3));
    board[7][5] = pieces.back().get();
    pieces.push_back(std::make_unique<Piece>("Knight", PieceType::Knight, Color::White, Position(0.625f, -0.875f, "G1", 7, 6), "res/white_horse.png", 3));
    board[7][6] = pieces.back().get();
    pieces.push_back(std::make_unique<Piece>("Rook", PieceType::Rook, Color::White, Position(0.875f, -0.875f, "H1", 7, 7), "res/white_rook.png", 5));
    board[7][7] = pieces.back().get();

    // Crni pjesaci
    for (int i = 0; i < 8; ++i) {
        pieces.push_back(std::make_unique<Piece>("Pawn", PieceType::Pawn, Color::Black, Position(-0.875f + i * 0.25f, 0.625f, "A7", 1, i), "res/black_pawn.png", 1));
        board[1][i] = pieces.back().get();
    }

    // Crne figure
    pieces.push_back(std::make_unique<Piece>("Rook", PieceType::Rook, Color::Black, Position(-0.875f, 0.875f, "A8", 0, 0), "res/black_rook.png", 5));
    board[0][0] = pieces.back().get();
    pieces.push_back(std::make_unique<Piece>("Knight", PieceType::Knight, Color::Black, Position(-0.625f, 0.875f, "B8", 0, 1), "res/black_horse.png", 3));
    board[0][1] = pieces.back().get();
    pieces.push_back(std::make_unique<Piece>("Bishop", PieceType::Bishop, Color::Black, Position(-0.375f, 0.875f, "C8", 0, 2), "res/black_bishop.png", 3));
    board[0][2] = pieces.back().get();
    pieces.push_back(std::make_unique<Piece>("Queen", PieceType::Queen, Color::Black, Position(-0.125f, 0.875f, "D8", 0, 3), "res/black_queen.png", 9));
    board[0][3] = pieces.back().get();
    pieces.push_back(std::make_unique<Piece>("King", PieceType::King, Color::Black, Position(0.125f, 0.875f, "E8", 0, 4), "res/black_king.png", 10));
    board[0][4] = pieces.back().get();
    pieces.push_back(std::make_unique<Piece>("Bishop", PieceType::Bishop, Color::Black, Position(0.375f, 0.875f, "F8", 0, 5), "res/black_bishop.png", 3));
    board[0][5] = pieces.back().get();
    pieces.push_back(std::make_unique<Piece>("Knight", PieceType::Knight, Color::Black, Position(0.625f, 0.875f, "G8", 0, 6), "res/black_horse.png", 3));
    board[0][6] = pieces.back().get();
    pieces.push_back(std::make_unique<Piece>("Rook", PieceType::Rook, Color::Black, Position(0.875f, 0.875f, "H8", 0, 7), "res/black_rook.png", 5));
    board[0][7] = pieces.back().get();

    return pieces;
}


void drawPieces(const std::vector<std::unique_ptr<Piece>>& pieces, unsigned int shader, unsigned int pieceVAO) {
    for (const auto& piece : pieces) {
        // Provjeri da li postoji figura i preskoči crtanje uhvaćenih figura
        if (!piece || piece->getIsCaptured()) {
            continue;
        }

        unsigned int texture = loadTexture(piece->getImagePath().c_str());
        if (texture == 0) {
            std::cerr << "Texture not loaded for piece: " << piece->getName() << std::endl;
            continue;
        }

        glUseProgram(shader);
        glBindTexture(GL_TEXTURE_2D, texture);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(piece->getCurrentPosition().getXGL(), piece->getCurrentPosition().getYGL(), 0.0f)); // zasto matrica translacije, pa jednostavno svaka figura mora da ide na svoje mjesto na osnovu koordinata
        unsigned int modelLoc = glGetUniformLocation(shader, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(pieceVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glDeleteTextures(1, &texture);
    }
}


void setupPieceVAO(unsigned int& VAO, unsigned int& VBO, unsigned int& EBO) {
    float vertices[] = {
        // Pozicije      // Teksturne koordinate
        -0.1f, -0.1f,   0.0f, 0.0f, // Donji levi
         0.1f, -0.1f,   1.0f, 0.0f, // Donji desni
         0.1f,  0.1f,   1.0f, 1.0f, // Gornji desni
        -0.1f,  0.1f,   0.0f, 1.0f  // Gornji levi
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void drawPossibleMoves(const std::vector<Position>& moves, unsigned int shader, unsigned int VAO) {
    glUseProgram(shader);

    // Učitaj lokacije uniformi samo jednom
    unsigned int colorLoc = glGetUniformLocation(shader, "moveColor");
    unsigned int modelLoc = glGetUniformLocation(shader, "model");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(VAO);

    for (const auto& move : moves) {
        // Provjeri da li je na potezu neprijateljska figura
        Piece* occupyingPiece = board[move.getRow()][move.getColumn()];
        bool isAttackMove = occupyingPiece != nullptr && occupyingPiece->getColor() != selectedPiece->getColor();

        // Postavi boju poteza
        glm::vec4 moveColor = isAttackMove
            ? glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) // Crvena za napad
            : glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Crna za slobodne poteze
        glUniform4fv(colorLoc, 1, glm::value_ptr(moveColor));

        // Postavi model matricu za pozicioniranje na odgovarajuće mjesto na tabli
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(move.getXGL(), move.getYGL(), 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Nacrtaj potez
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
    glUseProgram(0);
}



bool isCheckmate(const std::vector<std::unique_ptr<Piece>>& pieces, std::vector<std::vector<Piece*>>& board, Color kingColor) {
    if (!Piece::isKingInCheck(pieces, board, kingColor)) {
        std::cout << "Kralj nije u šahu." << std::endl;
        return false;
    }

    std::cout << "Kralj je u šahu." << std::endl;

    Position kingPosition;
    std::vector<Position> kingMoves;

    // Pronađi kralja i njegove poteze
    for (const auto& piece : pieces) {
        if (piece->getType() == PieceType::King && piece->getColor() == kingColor) {
            kingPosition = piece->getCurrentPosition();
            piece->calculatePossibleMoves(board);
            kingMoves = piece->getPossibleMoves();
            break;
        }
    }

    // Lambda za provjeru da li je polje napadnuto od strane protivnika
    auto isSquareAttackedByOpponent = [&](int r, int c, Color friendColor) {
        for (const auto& p : pieces) {
            if (p->getColor() != friendColor) {
                p->calculatePossibleMoves(board);
                for (const auto& m : p->getPossibleMoves()) {
                    if (m.getRow() == r && m.getColumn() == c) {
                        return true;
                    }
                }
            }
        }
        return false;
    };

    // Pronađi sve napadačke figure
    std::vector<Piece*> attackingPieces;
    for (const auto& piece : pieces) {
        if (piece->getColor() != kingColor) {
            piece->calculatePossibleMoves(board);
            for (const auto& move : piece->getPossibleMoves()) {
                if (move.getRow() == kingPosition.getRow() && move.getColumn() == kingPosition.getColumn()) {
                    attackingPieces.push_back(piece.get());
                }
            }
        }
    }

    // Ako ima više napadača, šah-mat
    if (attackingPieces.size() > 1) {
        std::cout << "Više od jednog napadača. Šah-mat." << std::endl;
        return true;
    }

    Piece* attackingPiece = attackingPieces.front();
    Position attackerPosition = attackingPiece->getCurrentPosition();

    // 1. Provjera da li kralj može pobjeći ili pojesti napadača
    for (const auto& move : kingMoves) {
        int row = move.getRow();
        int col = move.getColumn();

        // Ako je potez upravo na mesto napadača, pokušaj da pojedeš napadača
        if (move == attackerPosition) {
            // Privremeno skloni napadača
            Piece* tempAttacker = board[row][col];
            board[row][col] = nullptr;

            // Provjeri da li je polje i dalje pod napadom
            bool isUnderAttack = isSquareAttackedByOpponent(row, col, kingColor);

            // Vrati napadača privremeno (da kasnije možeš simulirati potez)
            board[row][col] = tempAttacker;

            if (!isUnderAttack) {
                // Simuliraj da kralj jede napadača
                Piece* temp = board[row][col];
                board[row][col] = board[kingPosition.getRow()][kingPosition.getColumn()];
                board[kingPosition.getRow()][kingPosition.getColumn()] = nullptr;
                attackingPiece->clearPossibleMoves();
                Position oldAttackerPosition = attackingPiece->getCurrentPosition();
                attackingPiece->setCurrentPosition(Position(-1, -1));

                bool stillInCheck = Piece::isKingInCheck(pieces, board, kingColor);

                // Vrati stanje
                board[kingPosition.getRow()][kingPosition.getColumn()] = board[row][col];
                board[row][col] = temp;

                attackingPiece->setCurrentPosition(oldAttackerPosition);

                if (!stillInCheck) {
                    // Kralj može bezbedno pojesti napadača
                    return false;
                }

            }
        } else {
            bool isUnderAttack = isSquareAttackedByOpponent(row, col, kingColor);
            if (!isUnderAttack) {
                // Simuliraj potez kralja
                Piece* temp = board[row][col];
                board[row][col] = board[kingPosition.getRow()][kingPosition.getColumn()];
                board[kingPosition.getRow()][kingPosition.getColumn()] = nullptr;

                bool stillInCheck = Piece::isKingInCheck(pieces, board, kingColor);

                // Vrati tablu u prethodno stanje
                board[kingPosition.getRow()][kingPosition.getColumn()] = board[row][col];
                board[row][col] = temp;

                if (!stillInCheck) {
                    return false; // Kralj može povjeci
                }
            }
        }
    }

    // 2. Provjera da li neko može pojesti napadača (osim kralja, jer smo to već obradili)
    for (const auto& piece : pieces) {
        if (piece->getColor() == kingColor && piece->getType() != PieceType::King) {
            piece->calculatePossibleMoves(board);
            for (const auto& move : piece->getPossibleMoves()) {
                if (move == attackerPosition) {
                    // Simuliraj potez
                    Piece* temp = board[move.getRow()][move.getColumn()];
                    Position oldPos = piece->getCurrentPosition();

                    board[move.getRow()][move.getColumn()] = piece.get();
                    board[oldPos.getRow()][oldPos.getColumn()] = nullptr;

                    // Ako hvatamo neprijateljsku figuru - napadača,
                    // privremeno je ukloni iz kalkulacija
                    Position oldAttackerPos = Position(-1, -1);
                    bool capturedEnemy = false;

                    if (temp != nullptr && temp->getColor() != kingColor) {
                        oldAttackerPos = temp->getCurrentPosition();
                        temp->setCurrentPosition(Position(-1, -1));
                        capturedEnemy = true;
                    }

                    bool stillInCheck = Piece::isKingInCheck(pieces, board, kingColor);

                    // Vrati tablu u prethodno stanje
                    board[oldPos.getRow()][oldPos.getColumn()] = piece.get();
                    board[move.getRow()][move.getColumn()] = temp;

                    // Vrati napadača na staru poziciju, ako smo ga pomjerali
                    if (capturedEnemy && temp != nullptr) {
                        temp->setCurrentPosition(oldAttackerPos);
                    }

                    if (!stillInCheck) {
                        // Neka figura može pojesti napadača i spasiti kralja
                        return false;
                    }
                }
            }
        }
    }


    // 3. Ako napadač može biti blokiran (važi za lovca, topa, kraljicu)
    if (attackingPiece->getType() == PieceType::Rook ||
        attackingPiece->getType() == PieceType::Bishop ||
        attackingPiece->getType() == PieceType::Queen) {
        Position blockPosition = attackerPosition;

        int rowDirection = (kingPosition.getRow() > blockPosition.getRow()) ? 1 : (kingPosition.getRow() < blockPosition.getRow()) ? -1 : 0;
        int colDirection = (kingPosition.getColumn() > blockPosition.getColumn()) ? 1 : (kingPosition.getColumn() < blockPosition.getColumn()) ? -1 : 0;

        while (blockPosition != kingPosition) {
            blockPosition.setRow(blockPosition.getRow() + rowDirection);
            blockPosition.setColumn(blockPosition.getColumn() + colDirection);

            if (blockPosition.getRow() < 0 || blockPosition.getRow() >= (int)board.size() ||
                blockPosition.getColumn() < 0 || blockPosition.getColumn() >= (int)board[0].size()) {
                break;
            }

            for (const auto& piece : pieces) {
                if (piece->getColor() == kingColor && piece->getType() != PieceType::King) {
                    piece->calculatePossibleMoves(board);
                    for (const auto& m : piece->getPossibleMoves()) {
                        if (m == blockPosition) {
                            Piece* temp = board[m.getRow()][m.getColumn()];
                            Position oldPos = piece->getCurrentPosition();

                            board[m.getRow()][m.getColumn()] = piece.get();
                            board[oldPos.getRow()][oldPos.getColumn()] = nullptr;

                            bool isStillInCheck = Piece::isKingInCheck(pieces, board, kingColor);

                            // Vrati stanje
                            board[oldPos.getRow()][oldPos.getColumn()] = piece.get();
                            board[m.getRow()][m.getColumn()] = temp;

                            if (!isStillInCheck) {
                                return false; // Napad može biti blokiran
                            }
                        }
                    }
                }
            }
        }
    }

    // Ako ništa od navedenog ne pomaže, to je šah-mat
    std::cout << "Šah-mat." << std::endl;
    return true;
}
