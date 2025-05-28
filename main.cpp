#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>

using namespace std;

const int ROWS = 6;
const int COLS = 6;
const int CELL_SIZE = 80;
const int OFFSET = 50;
const int BTN_SIZE = 50;
const int BTN_SPACING = 10;
const int WIN_WIDTH = 700;
const int WIN_HEIGHT = 800;

enum Owner { NO_ONE, PLAYER, BOT };

struct Tile {
    int number;
    Owner state = NO_ONE;
    sf::RectangleShape box;
    sf::Text text;
};

vector<vector<int>> boardNumbers = {
    {1, 2, 3, 4, 5, 6},
    {7, 8, 9, 10, 12, 14},
    {15, 16, 18, 20, 21, 24},
    {25, 27, 28, 30, 32, 35},
    {36, 40, 42, 45, 48, 49},
    {54, 56, 63, 64, 72, 81}
};

bool isWinningMove(const vector<vector<Tile>>& board, Owner current) {
    for (int i = 0; i < ROWS; ++i) {
        int row = 0, col = 0;
        for (int j = 0; j < COLS; ++j) {
            row = (board[i][j].state == current) ? row + 1 : 0;
            col = (board[j][i].state == current) ? col + 1 : 0;
            if (row == 4 || col == 4) return true;
        }
    }
    return false;
}

void resetGameState(vector<vector<Tile>>& board, vector<int>& selectors, Owner& turn,
                    int& prevF, int& currF, Owner& result, int& prevProd, int& currProd) {
    for (auto& row : board)
        for (auto& tile : row)
            tile.state = NO_ONE;

    selectors.assign(10, 1);
    turn = PLAYER;
    result = NO_ONE;
    prevF = currF = 1;
    prevProd = currProd = -1;
}

void setupBoard(vector<vector<Tile>>& board, sf::Font& font) {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            int num = boardNumbers[i][j];
            Tile& tile = board[i][j];
            tile.number = num;
            tile.box.setSize(sf::Vector2f(CELL_SIZE, CELL_SIZE));
            tile.box.setPosition(OFFSET + j * CELL_SIZE, OFFSET + i * CELL_SIZE);
            tile.box.setFillColor(sf::Color(30, 30, 100));
            tile.box.setOutlineColor(sf::Color::Black);
            tile.box.setOutlineThickness(1);

            tile.text.setFont(font);
            tile.text.setCharacterSize(16);
            tile.text.setFillColor(sf::Color::White);
            tile.text.setString(to_string(num));
            tile.text.setPosition(tile.box.getPosition().x + 15, tile.box.getPosition().y + 15);
        }
    }
}

int main() {
    sf::RenderWindow app(sf::VideoMode(WIN_WIDTH, WIN_HEIGHT), "Factor Grid Game");
    sf::Font font;
    font.loadFromFile("arial.ttf");

    vector<vector<Tile>> board(ROWS, vector<Tile>(COLS));
    vector<int> selectors(10, 1);
    Owner currentPlayer = PLAYER, winner = NO_ONE;
    int prevFactor = 1, currFactor = 1;
    int prevResult = -1, currResult = -1;
    bool gameStarted = false;

    setupBoard(board, font);

    sf::Text status("", font, 24);
    status.setFillColor(sf::Color::White);
    status.setPosition(OFFSET, 10);

    sf::Text notice("", font, 18);
    notice.setFillColor(sf::Color::Red);
    notice.setPosition(OFFSET, WIN_HEIGHT - 60);

    sf::CircleShape playButton(60);
    playButton.setPosition((WIN_WIDTH - 120) / 2, (WIN_HEIGHT - 120) / 2);
    playButton.setFillColor(sf::Color(255, 140, 0));

    sf::ConvexShape playIcon;
    playIcon.setPointCount(3);
    playIcon.setPoint(0, sf::Vector2f(0, 0));
    playIcon.setPoint(1, sf::Vector2f(0, 40));
    playIcon.setPoint(2, sf::Vector2f(35, 20));
    playIcon.setFillColor(sf::Color::White);
    playIcon.setPosition((WIN_WIDTH - 35) / 2, (WIN_HEIGHT - 40) / 2);

    sf::CircleShape markerPrev(15, 3), markerCurr(15, 3);
    markerPrev.setFillColor(sf::Color(255, 165, 0)); markerPrev.setRotation(180);
    markerCurr.setFillColor(sf::Color::Cyan);

    while (app.isOpen()) {
        sf::Event evt;
        while (app.pollEvent(evt)) {
            if (evt.type == sf::Event::Closed)
                app.close();

            if (!gameStarted && evt.type == sf::Event::MouseButtonPressed) {
                sf::Vector2f mouse(evt.mouseButton.x, evt.mouseButton.y);
                if (playButton.getGlobalBounds().contains(mouse)) {
                    resetGameState(board, selectors, currentPlayer, prevFactor, currFactor, winner, prevResult, currResult);
                    gameStarted = true;
                    continue;
                }
            }

            if (!gameStarted || winner != NO_ONE) continue;

            if (evt.type == sf::Event::KeyPressed && currentPlayer == PLAYER) {
                if (evt.key.code == sf::Keyboard::Left && currFactor > 1) currFactor--;
                else if (evt.key.code == sf::Keyboard::Right && currFactor < 9) currFactor++;
                else if (evt.key.code == sf::Keyboard::A && prevFactor > 1) prevFactor--;
                else if (evt.key.code == sf::Keyboard::D && prevFactor < 9) prevFactor++;
                else if (evt.key.code == sf::Keyboard::Enter) {
                    int result = prevFactor * currFactor;
                    bool placed = false;
                    for (auto& row : board) {
                        for (auto& tile : row) {
                            if (tile.number == result && tile.state == NO_ONE) {
                                tile.state = PLAYER;
                                prevResult = currResult;
                                currResult = result;
                                currentPlayer = BOT;
                                notice.setString("");
                                if (isWinningMove(board, PLAYER)) winner = PLAYER;
                                placed = true;
                                break;
                            }
                        }
                        if (placed) break;
                    }
                    if (!placed) notice.setString("Invalid move. Try again.");
                }
            }
        }

        // Bot Move
        if (gameStarted && currentPlayer == BOT && winner == NO_ONE) {
            bool done = false;
            for (int f = 1; f <= 9 && !done; ++f) {
                int result = prevFactor * f;
                for (auto& row : board) {
                    for (auto& tile : row) {
                        if (tile.number == result && tile.state == NO_ONE) {
                            tile.state = BOT;
                            prevFactor = f;
                            prevResult = currResult;
                            currResult = result;
                            currentPlayer = PLAYER;
                            if (isWinningMove(board, BOT)) winner = BOT;
                            done = true;
                            break;
                        }
                    }
                    if (done) break;
                }
            }
            if (!done) winner = PLAYER;
        }

        // Triangle indicators
        float markerY = OFFSET + ROWS * CELL_SIZE + 60;
        float xPrev = OFFSET + (prevFactor - 1) * (BTN_SIZE + BTN_SPACING) + BTN_SIZE / 2;
        float xCurr = OFFSET + (currFactor - 1) * (BTN_SIZE + BTN_SPACING) + BTN_SIZE / 2;
        markerPrev.setOrigin(markerPrev.getRadius(), markerPrev.getRadius());
        markerCurr.setOrigin(markerCurr.getRadius(), markerCurr.getRadius());
        markerPrev.setPosition(xPrev, markerY - markerPrev.getRadius() - 5);
        markerCurr.setPosition(xCurr, markerY + BTN_SIZE + markerCurr.getRadius() + 5);

        // Draw
        app.clear(sf::Color(40, 60, 100));

        if (!gameStarted) {
            app.draw(playButton);
            app.draw(playIcon);
        } else {
            for (auto& row : board)
                for (auto& tile : row) {
                    tile.box.setFillColor(
                        tile.state == PLAYER ? sf::Color::Green :
                        tile.state == BOT ? sf::Color(200, 50, 50) :
                        sf::Color(30, 30, 100)
                    );
                    app.draw(tile.box);
                    app.draw(tile.text);
                }

            for (int i = 1; i <= 9; ++i) {
                float x = OFFSET + (i - 1) * (BTN_SIZE + BTN_SPACING);
                sf::RectangleShape btn(sf::Vector2f(BTN_SIZE, BTN_SIZE));
                btn.setPosition(x, markerY);
                btn.setFillColor(sf::Color(180, 180, 0));
                app.draw(btn);

                sf::Text label(to_string(i), font, 20);
                label.setFillColor(sf::Color::Black);
                label.setPosition(x + 15, markerY + 10);
                app.draw(label);
            }

            app.draw(markerPrev);
            app.draw(markerCurr);

            status.setString(
                winner == PLAYER ? "Victory!" :
                winner == BOT ? "Bot wins!" :
                "R & L: Cyan | A/D: Yellow | Enter: Play"
            );
            app.draw(status);
            app.draw(notice);
        }

        app.display();
    }

    return 0;
}
