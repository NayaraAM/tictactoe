#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <array>
#include <random>
#include <chrono>
#include <cstdlib> 
#include <ctime>

// Classe TicTacToe
class TicTacToe {
private:
    std::array<std::array<char, 3>, 3> board; // board = Tabuleiro do jogo
    std::mutex board_mutex; // Mutex para controle de acesso ao tabuleiro
    std::condition_variable turn_cv; // turn_cv = coordenadas - Variável de condição para alternância de turnos
    char current_player; // Jogador atual ('X' ou 'O')
    bool game_over; // Estado do jogo
    char winner; // Vencedor do jogo

public:
    TicTacToe() {
        // Inicializar o tabuleiro e as variáveis do jogo
        for (auto& row : this->board)
            row.fill(' ');
        this->current_player = 'X';
        this->game_over = false;
        this->winner = ' ';
    }

    void display_board() {
        // Exibir o tabuleiro no console
        system("clear");
        std::cout << "Tabuleiro:\n";
        for (int i = 0; i < 3; ++i) {
            std::cout << " ";
            for (int j = 0; j < 3; ++j) {
                std::cout << this->board[i][j];
                if (j < 2) std::cout << " | ";
            }
            std::cout << "\n";
            if (i < 2) std::cout << "---+---+---\n";
        }
        std::cout << std::endl;
    }

    bool make_move(char player, int row, int col) {
        // Implementar a lógica para realizar uma jogada no tabuleiro
        // Utilizar mutex para controle de acesso
        // Utilizar variável de condição para alternância de turnos
        std::unique_lock<std::mutex> lock(this->board_mutex);
        this->turn_cv.wait(lock, [this, player]() {
            return player == this->current_player && !this->game_over;
        });

        if (this->board[row][col] != ' ' || this->game_over) {
            return false; // Casa ocupada ou jogo já acabou
        }

        this->board[row][col] = player;
        this->display_board();

        if (this->check_win(player)) {
            this->game_over = true;
            this->winner = player;
        } else if (this->check_draw()) {
            this->game_over = true;
            this->winner = 'D';
        } else {
            this->current_player = (player == 'X') ? 'O' : 'X';
        }

        this->turn_cv.notify_all(); // Notifica o outro jogador
        return true;
    }

    bool check_win(char player) {
        // Verificar se o jogador atual venceu o jogo
        for (int i = 0; i < 3; ++i)
            if (this->board[i][0] == player &&
                this->board[i][1] == player &&
                this->board[i][2] == player)
                return true;

        for (int j = 0; j < 3; ++j)
            if (this->board[0][j] == player &&
                this->board[1][j] == player &&
                this->board[2][j] == player)
                return true;

        if (this->board[0][0] == player &&
            this->board[1][1] == player &&
            this->board[2][2] == player)
            return true;

        if (this->board[0][2] == player &&
            this->board[1][1] == player &&
            this->board[2][0] == player)
            return true;

        return false;
    }

    bool check_draw() {
        // Verificar se houve um empate
        for (auto& row : this->board)
            for (char c : row)
                if (c == ' ')
                    return false;
        return true;
    }

    bool is_game_over() {
        // Retornar se o jogo terminou
        return this->game_over;
    }

    char get_winner() {
        // Retornar o vencedor do jogo ('X', 'O', ou 'D' para empate)
        return this->winner;
    }
};

// Classe Player
class Player {
private:
    TicTacToe& game; // Referência para o jogo
    char symbol; // Símbolo do jogador ('X' ou 'O')
    std::string strategy; // Estratégia do jogador

public:
    Player(TicTacToe& g, char s, std::string strat) 
        : game(g), symbol(s), strategy(strat) {}

    void play() {
        // Executar jogadas de acordo com a estratégia escolhida
        if (this->strategy == "sequencial") {
            this->play_sequential();
        } else {
            this->play_random();
        }
    }

private:
    void play_sequential() {
        // Implementar a estratégia sequencial de jogadas
        while (!this->game.is_game_over()) {
            for (int i = 0; i < 3 && !this->game.is_game_over(); ++i) {
                for (int j = 0; j < 3 && !this->game.is_game_over(); ++j) {
                    this->game.make_move(this->symbol, i, j);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
        }
    }

    void play_random() {
        // Implementar a estratégia aleatória de jogadas
        std::uniform_int_distribution<int> dist(0, 2);
        while (!this->game.is_game_over()) {
            int row = rand() % 3;
            int col = rand() % 3;
            if (this->game.make_move(this->symbol, row, col)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
};

// Função principal
int main() {
    // Inicializar o jogo e os jogadores
    TicTacToe jogo;

    Player jogadorX(jogo, 'X', "sequencial");
    Player jogadorO(jogo, 'O', "aleatorio");

    // Criar as threads para os jogadores
    std::thread t1(&Player::play, &jogadorX);
    std::thread t2(&Player::play, &jogadorO);

    // Aguardar o término das threads
    t1.join();
    t2.join();

    // Exibir o resultado final do jogo
    char vencedor = jogo.get_winner();
    if (vencedor == 'D') {
        std::cout << "\nO jogo terminou em empate!\n";
    } else {
        std::cout << "\nO jogador " << vencedor << " venceu o jogo!\n";
    }

    return 0;
}
