/*
    Egaroucid Project

    @file board_info.hpp
        Board structure of Egaroucid
    @date 2021-2025
    @author Takuto Yamana
    @license GPL-3.0 license
*/

#pragma once
#include "./../engine/engine_all.hpp"

#define INVALID_CELL -1

struct Board_info {
    Board board;
    uint_fast8_t player;
    std::vector<Board> boards;
    std::vector<int> players;
    int ply_vec;

    void reset() {
        board.reset();
        player = BLACK;
        boards.clear();
        players.clear();
        boards.emplace_back(board);
        players.emplace_back(player);
        ply_vec = 0;
    }

    Board_info copy() {
        Board_info res;
        res.board = board.copy();
        res.player = player;
        res.boards = boards;
        res.players = players;
        res.ply_vec = ply_vec;
        return res;
    }

    void set_board(const std::string &board_state) {
        board.reset();
        for (int i = 0; i < board_state.size(); ++i) {
            int x = i % HW; // 列
            int y = i / HW; // 行
            char piece = board_state[i];
            if (piece == 'X') {
                board.place_piece(BLACK, x, y); // 黑棋
            } else if (piece == 'O') {
                board.place_piece(WHITE, x, y); // 白棋
            }
        }
    }

    void swap_player() {
        player = (player == BLACK) ? WHITE : BLACK;
    }
};
