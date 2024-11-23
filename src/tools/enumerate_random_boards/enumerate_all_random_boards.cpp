#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <unordered_set>
#include <random>
#include <algorithm>
#include <ios>
#include <iomanip>
#include "./../../engine/board.hpp"
#include "./../../engine/util.hpp"

/*
    @brief array for calculating hash code for book
*/
size_t hash_rand_player_book[4][65536];
size_t hash_rand_opponent_book[4][65536];

/*
    @brief initialize hash array for book randomly
*/
void book_hash_init_rand(){
    int i, j;
    for (i = 0; i < 4; ++i){
        for (j = 0; j < 65536; ++j){
            hash_rand_player_book[i][j] = 0;
            while (pop_count_uint(hash_rand_player_book[i][j]) < 9)
                hash_rand_player_book[i][j] = myrand_ull();
            hash_rand_opponent_book[i][j] = 0;
            while (pop_count_uint(hash_rand_opponent_book[i][j]) < 9)
                hash_rand_opponent_book[i][j] = myrand_ull();
        }
    }
}

/*
    @brief Hash function for book

    @param board                board
    @return hash code
*/
struct Book_hash {
    size_t operator()(Board board) const{
        const uint16_t *p = (uint16_t*)&board.player;
        const uint16_t *o = (uint16_t*)&board.opponent;
        return 
            hash_rand_player_book[0][p[0]] ^ 
            hash_rand_player_book[1][p[1]] ^ 
            hash_rand_player_book[2][p[2]] ^ 
            hash_rand_player_book[3][p[3]] ^ 
            hash_rand_opponent_book[0][o[0]] ^ 
            hash_rand_opponent_book[1][o[1]] ^ 
            hash_rand_opponent_book[2][o[2]] ^ 
            hash_rand_opponent_book[3][o[3]];
    }
};

std::unordered_set<uint64_t> all_silhouettes;
std::unordered_set<Board, Book_hash> all_boards;

inline void first_update_representative_board(Board *res, Board *sym){
    uint64_t vp = vertical_mirror(sym->player);
    uint64_t vo = vertical_mirror(sym->opponent);
    if (res->player > vp || (res->player == vp && res->opponent > vo)){
        res->player = vp;
        res->opponent = vo;
    }
}

inline void update_representative_board(Board *res, Board *sym){
    if (res->player > sym->player || (res->player == sym->player && res->opponent > sym->opponent))
        sym->copy(res);
    uint64_t vp = vertical_mirror(sym->player);
    uint64_t vo = vertical_mirror(sym->opponent);
    if (res->player > vp || (res->player == vp && res->opponent > vo)){
        res->player = vp;
        res->opponent = vo;
    }
}

inline uint64_t get_representative_silhouette(uint64_t silhouette){
    uint64_t res = silhouette;
    res = std::min(res, vertical_mirror(silhouette));
    
    silhouette = black_line_mirror(silhouette);
    res = std::min(res, silhouette);
    res = std::min(res, vertical_mirror(silhouette));

    silhouette = horizontal_mirror(silhouette);
    res = std::min(res, silhouette);
    res = std::min(res, vertical_mirror(silhouette));

    silhouette = white_line_mirror(silhouette);
    res = std::min(res, silhouette);
    res = std::min(res, vertical_mirror(silhouette));

    return res;

}

inline Board get_representative_board(Board b){
    Board res = b;
    first_update_representative_board(&res, &b);
    b.board_black_line_mirror();
    update_representative_board(&res, &b);
    b.board_horizontal_mirror();
    update_representative_board(&res, &b);
    b.board_white_line_mirror();
    update_representative_board(&res, &b);
    return res;
}

std::string fill0(int n, int d){
    std::stringstream ss;
    ss << std::setfill('0') << std::right << std::setw(d) << n;
    return ss.str();
}

void fill_silhouette(uint64_t silhouette, const std::vector<int> &cell_list, int cell_list_strt_idx, int n_filled) {
    if (n_filled == 0) { // silhouette filled
        //all_silhouettes.emplace(silhouette);
        all_silhouettes.emplace(get_representative_silhouette(silhouette));
    }
    for (int i = cell_list_strt_idx; i < (int)cell_list.size(); ++i) {
        silhouette ^= 1ULL << cell_list[i];
            fill_silhouette(silhouette, cell_list, i + 1, n_filled - 1);
        silhouette ^= 1ULL << cell_list[i];
    }
}

void fill_discs(uint64_t silhouette, int n_discs, int n_white_min_discs, int n_white_max_discs) {
    for (int n_white = n_white_min_discs; n_white <= n_white_max_discs; ++n_white) {
        int n_black = n_discs - n_white;
    }
}

#define N_BOARDS_PER_FILE 10000

int main(int argc, char *argv[]){
    if (argc < 2){
        std::cerr << "input [n_discs]" << std::endl;
        return 1;
    }

    mobility_init();
    flip_init();
    book_hash_init_rand();

    int n_discs = atoi(argv[1]);

    if (n_discs < 4) {
        n_discs = 4;
    }

    uint64_t avoid_cells = 0xC3C300000000C3C3ULL;

    std::vector<std::vector<int>> cell_distance_list;
    for (int i = 0; i < HW; ++i) {
        cell_distance_list.emplace_back(std::vector<int>());
    }
    for (int cell = 0; cell < HW2; ++cell) {
        if ((avoid_cells & (1ULL << cell)) == 0) {
            int x = cell % HW;
            int y = cell / HW;
            int d = (std::max(std::abs(2 * x - (HW - 1)), std::abs(2 * y - (HW - 1))) - 1) / 2;
            cell_distance_list[d].emplace_back(cell);
        }
    }
    std::vector<int> always_filled_cells;
    std::vector<int> randomly_filled_cells;
    int n_randomly_filled = 0;
    int n_filled = 0;
    for (int d = 0; d < HW; ++d) {
        if (n_filled + cell_distance_list[d].size() <= n_discs) {
            for (int &cell: cell_distance_list[d]) {
                always_filled_cells.emplace_back(cell);
            }
            n_filled += cell_distance_list[d].size();
        } else if (n_filled < n_discs) {
            n_randomly_filled = n_discs - n_filled;
            for (int &cell: cell_distance_list[d]) {
                randomly_filled_cells.emplace_back(cell);
            }
            n_filled = n_discs;
        }
    }
    std::cerr << "always filled " << always_filled_cells.size() << " : ";
    for (const int &cell: always_filled_cells) {
        std::cerr << idx_to_coord(cell) << " ";
    }
    std::cerr << std::endl;
    std::cerr << "randomly filled " << n_randomly_filled << " / " << randomly_filled_cells.size() <<  " : ";
    for (const int &cell: randomly_filled_cells) {
        std::cerr << idx_to_coord(cell) << " ";
    }
    std::cerr << std::endl;

    uint64_t silhouette = 0ULL;
    for (const int &cell: always_filled_cells) {
        silhouette ^= 1ULL << cell;
    }
    bit_print_board(silhouette);

    fill_silhouette(silhouette, randomly_filled_cells, 0, n_randomly_filled);
    std::cerr << "n_silhouettes " << all_silhouettes.size() << std::endl;

    int d2_n_discs = n_discs / 2;
    int imb = d2_n_discs / 3;
    int n_white_min_discs = d2_n_discs - imb;
    int n_white_max_discs = d2_n_discs + imb;
    if (n_discs % 2 == 1) {
        ++d2_n_discs;
        imb = d2_n_discs / 3;
        n_white_min_discs = std::min(n_white_min_discs, d2_n_discs - imb);
        n_white_max_discs = std::min(n_white_max_discs, d2_n_discs + imb);
    }
    std::cerr << "white min_discs " << n_white_min_discs << " max_discs " << n_white_max_discs << std::endl;
    for (uint64_t sil: all_silhouettes) {
        fill_discs(sil, n_discs, n_white_min_discs, n_white_max_discs);
    }

    return 0;
}