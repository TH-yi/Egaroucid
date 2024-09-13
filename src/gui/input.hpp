﻿/*
    Egaroucid Project

    @file input.hpp
        Input scenes
    @date 2021-2024
    @author Takuto Yamana
    @license GPL-3.0 license
*/

#pragma once
#include <iostream>
#include <future>
#include "./../engine/engine_all.hpp"
#include "function/function_all.hpp"

class Import_transcript : public App::Scene {
private:
    Button single_back_button;
    Button back_button;
    Button import_button;
    Button import_from_position_button;
    bool done;
    bool failed;
    std::string transcript;
    std::vector<History_elem> n_history;
    bool imported_from_position;
    TextAreaEditState text_area;

public:
    Import_transcript(const InitData& init) : IScene{ init } {
        single_back_button.init(BACK_BUTTON_SX, BACK_BUTTON_SY, BACK_BUTTON_WIDTH, BACK_BUTTON_HEIGHT, BACK_BUTTON_RADIUS, language.get("common", "back"), 25, getData().fonts.font, getData().colors.white, getData().colors.black);
        back_button.init(BUTTON3_1_SX, BUTTON3_SY, BUTTON3_WIDTH, BUTTON3_HEIGHT, BUTTON3_RADIUS, language.get("common", "back"), 25, getData().fonts.font, getData().colors.white, getData().colors.black);
        import_button.init(BUTTON3_2_SX, BUTTON3_SY, BUTTON3_WIDTH, BUTTON3_HEIGHT, BUTTON3_RADIUS, language.get("in_out", "import"), 25, getData().fonts.font, getData().colors.white, getData().colors.black);
        import_from_position_button.init(BUTTON3_3_SX, BUTTON3_SY, BUTTON3_WIDTH, BUTTON3_HEIGHT, BUTTON3_RADIUS, language.get("in_out", "import_from_this_position"), 15, getData().fonts.font, getData().colors.white, getData().colors.black);
        done = false;
        failed = false;
        imported_from_position = false;
        transcript.clear();
    }

    void update() override {
        if (System::GetUserActions() & UserAction::CloseButtonClicked) {
            changeScene(U"Close", SCENE_FADE_TIME);
        }
        Scene::SetBackground(getData().colors.green);
        const int icon_width = SCENE_ICON_WIDTH;
        getData().resources.icon.scaled((double)icon_width / getData().resources.icon.width()).draw(X_CENTER - icon_width / 2, 20);
        getData().resources.logo.scaled((double)icon_width / getData().resources.logo.width()).draw(X_CENTER - icon_width / 2, 20 + icon_width);
        int sy = 20 + icon_width + 50;
        if (!done) {
            getData().fonts.font(language.get("in_out", "input_transcript")).draw(25, Arg::topCenter(X_CENTER, sy), getData().colors.white);
            text_area.active = true;
            SimpleGUI::TextArea(text_area, Vec2{X_CENTER - 300, sy + 40}, SizeF{600, 130}, SimpleGUI::PreferredTextAreaMaxChars);
            getData().fonts.font(language.get("in_out", "you_can_paste_with_ctrl_v")).draw(13, Arg::topCenter(X_CENTER, sy + 175), getData().colors.white);
            bool return_pressed = false;
            if (text_area.text.size()) {
                if (text_area.text[text_area.text.size() - 1] == '\n') {
                    return_pressed = true;
                }
            }
            transcript = text_area.text.replaced(U"\r", U"").replaced(U"\n", U"").replaced(U" ", U"").narrow();
            back_button.draw();
            import_button.draw();
            import_from_position_button.draw();
            if (back_button.clicked() || KeyEscape.pressed()) {
                changeScene(U"Main_scene", SCENE_FADE_TIME);
            }
            if (import_button.clicked() || KeyEnter.pressed()) {
                History_elem history_elem;
                Board h_bd;
                h_bd.reset();
                history_elem.set(h_bd, BLACK, GRAPH_IGNORE_VALUE, -1, -1, -1, "");
                n_history.emplace_back(history_elem);
                n_history = import_transcript_processing(n_history, history_elem, transcript, &failed);
                done = true;
            }
            if (import_from_position_button.clicked()) {
                for (History_elem history_elem : getData().graph_resources.nodes[getData().graph_resources.branch]) {
                    if (getData().history_elem.board.n_discs() >= history_elem.board.n_discs()) {
                        n_history.emplace_back(history_elem);
                    }
                }
                n_history = import_transcript_processing(n_history, getData().history_elem, transcript, &failed);
                done = true;
                imported_from_position = true;
            }
        }
        else {
            if (!failed) {
                if (!imported_from_position) {
                    getData().graph_resources.init();
                    getData().graph_resources.nodes[0] = n_history;
                    getData().graph_resources.n_discs = getData().graph_resources.nodes[0].back().board.n_discs();
                    getData().game_information.init();
                }
                else {
                    getData().graph_resources.nodes[getData().graph_resources.branch] = n_history;
                }
                std::string opening_name, n_opening_name;
                for (int i = 0; i < (int)getData().graph_resources.nodes[getData().graph_resources.branch].size(); ++i) {
                    n_opening_name.clear();
                    n_opening_name = opening.get(getData().graph_resources.nodes[getData().graph_resources.branch][i].board, getData().graph_resources.nodes[getData().graph_resources.branch][i].player ^ 1);
                    if (n_opening_name.size()) {
                        opening_name = n_opening_name;
                    }
                    getData().graph_resources.nodes[getData().graph_resources.branch][i].opening_name = opening_name;
                }
                getData().graph_resources.n_discs = getData().graph_resources.nodes[getData().graph_resources.branch].back().board.n_discs();
                getData().graph_resources.need_init = false;
                getData().history_elem = getData().graph_resources.nodes[getData().graph_resources.branch].back();
                changeScene(U"Main_scene", SCENE_FADE_TIME);
            }
            else {
                getData().fonts.font(language.get("in_out", "import_failed")).draw(25, Arg::topCenter(X_CENTER, sy), getData().colors.white);
                single_back_button.draw();
                if (single_back_button.clicked() || KeyEscape.pressed()) {
                    getData().graph_resources.need_init = false;
                    changeScene(U"Main_scene", SCENE_FADE_TIME);
                }
            }
        }
    }

    void draw() const override {

    }

private:
    std::vector<History_elem> import_transcript_processing(std::vector<History_elem> n_history, History_elem strt_elem, std::string transcript, bool* failed) {
        Board h_bd = strt_elem.board;
        String transcript_str = Unicode::Widen(transcript).replace(U"\r", U"").replace(U"\n", U"").replace(U" ", U"");
        if (transcript_str.size() % 2 != 0 && transcript_str.size() >= 120) {
            *failed = true;
        }
        else {
            int y, x;
            uint64_t legal;
            Flip flip;
            History_elem history_elem;
            int player = strt_elem.player;
            //history_elem.set(h_bd, player, GRAPH_IGNORE_VALUE, -1, -1, -1, "");
            //n_history.emplace_back(history_elem);
            for (int i = 0; i < (int)transcript_str.size(); i += 2) {
                x = (int)transcript_str[i] - (int)'a';
                if (x < 0 || HW <= x) {
                    x = (int)transcript_str[i] - (int)'A';
                    if (x < 0 || HW <= x) {
                        *failed = true;
                        break;
                    }
                }
                y = (int)transcript_str[i + 1] - (int)'1';
                if (y < 0 || HW <= y) {
                    *failed = true;
                    break;
                }
                y = HW_M1 - y;
                x = HW_M1 - x;
                legal = h_bd.get_legal();
                if (1 & (legal >> (y * HW + x))) {
                    calc_flip(&flip, &h_bd, y * HW + x);
                    h_bd.move_board(&flip);
                    player ^= 1;
                    if (h_bd.get_legal() == 0ULL) {
                        h_bd.pass();
                        player ^= 1;
                        if (h_bd.get_legal() == 0ULL) {
                            h_bd.pass();
                            player ^= 1;
                            if (i != transcript_str.size() - 2) {
                                *failed = true;
                                break;
                            }
                        }
                    }
                }
                else {
                    *failed = true;
                    break;
                }
                n_history.back().next_policy = y * HW + x;
                history_elem.set(h_bd, player, GRAPH_IGNORE_VALUE, -1, y * HW + x, -1, "");
                n_history.emplace_back(history_elem);
            }
        }
        return n_history;
    }
};

class Import_board : public App::Scene {
private:
    Button single_back_button;
    Button back_button;
    Button import_button;
    bool done;
    bool failed;
    Board board;
    int player;
    std::string board_str;
    TextAreaEditState text_area;

public:
    Import_board(const InitData& init) : IScene{ init } {
        single_back_button.init(BACK_BUTTON_SX, BACK_BUTTON_SY, BACK_BUTTON_WIDTH, BACK_BUTTON_HEIGHT, BACK_BUTTON_RADIUS, language.get("common", "back"), 25, getData().fonts.font, getData().colors.white, getData().colors.black);
        back_button.init(GO_BACK_BUTTON_BACK_SX, GO_BACK_BUTTON_SY, GO_BACK_BUTTON_WIDTH, GO_BACK_BUTTON_HEIGHT, GO_BACK_BUTTON_RADIUS, language.get("common", "back"), 25, getData().fonts.font, getData().colors.white, getData().colors.black);
        import_button.init(GO_BACK_BUTTON_GO_SX, GO_BACK_BUTTON_SY, GO_BACK_BUTTON_WIDTH, GO_BACK_BUTTON_HEIGHT, GO_BACK_BUTTON_RADIUS, language.get("in_out", "import"), 25, getData().fonts.font, getData().colors.white, getData().colors.black);
        done = false;
        failed = false;
        board_str.clear();
    }

    void update() override {
        if (System::GetUserActions() & UserAction::CloseButtonClicked) {
            changeScene(U"Close", SCENE_FADE_TIME);
        }
        Scene::SetBackground(getData().colors.green);
        const int icon_width = SCENE_ICON_WIDTH;
        getData().resources.icon.scaled((double)icon_width / getData().resources.icon.width()).draw(X_CENTER - icon_width / 2, 20);
        getData().resources.logo.scaled((double)icon_width / getData().resources.logo.width()).draw(X_CENTER - icon_width / 2, 20 + icon_width);
        int sy = 20 + icon_width + 50;
        if (!done) {
            getData().fonts.font(language.get("in_out", "input_board")).draw(25, Arg::topCenter(X_CENTER, sy), getData().colors.white);
            text_area.active = true;
            SimpleGUI::TextArea(text_area, Vec2{X_CENTER - 300, sy + 40}, SizeF{600, 130}, SimpleGUI::PreferredTextAreaMaxChars);
            getData().fonts.font(language.get("in_out", "you_can_paste_with_ctrl_v")).draw(13, Arg::topCenter(X_CENTER, sy + 175), getData().colors.white);
            bool return_pressed = false;
            if (text_area.text.size()) {
                if (text_area.text[text_area.text.size() - 1] == '\n') {
                    return_pressed = true;
                }
            }
            board_str = text_area.text.replaced(U"\r", U"").replaced(U"\n", U"").replaced(U" ", U"").narrow();
            back_button.draw();
            import_button.draw();
            if (back_button.clicked() || KeyEscape.pressed()) {
                changeScene(U"Main_scene", SCENE_FADE_TIME);
            }
            if (import_button.clicked() || KeyEnter.pressed()) {
                failed = import_board_processing();
                done = true;
            }
        }
        else {
            if (!failed) {
                getData().graph_resources.init();
                History_elem history_elem;
                history_elem.reset();
                getData().graph_resources.nodes[0].emplace_back(history_elem);
                history_elem.player = player;
                history_elem.board = board;
                getData().graph_resources.nodes[0].emplace_back(history_elem);
                getData().graph_resources.n_discs = board.n_discs();
                getData().game_information.init();
                getData().graph_resources.need_init = false;
                getData().history_elem = getData().graph_resources.nodes[0].back();
                changeScene(U"Main_scene", SCENE_FADE_TIME);
            }
            else {
                getData().fonts.font(language.get("in_out", "import_failed")).draw(25, Arg::topCenter(X_CENTER, sy), getData().colors.white);
                single_back_button.draw();
                if (single_back_button.clicked() || KeyEscape.pressed()) {
                    changeScene(U"Main_scene", SCENE_FADE_TIME);
                }
            }
        }
    }

    void draw() const override {

    }

private:
    bool import_board_processing() {
        String board_str_str = Unicode::Widen(board_str).replace(U"\r", U"").replace(U"\n", U"").replace(U" ", U"");
        bool failed_res = false;
        int bd_arr[HW2];
        Board bd;
        if (board_str_str.size() != HW2 + 1) {
            failed_res = true;
        }
        else {
            for (int i = 0; i < HW2; ++i) {
                if (board_str_str[i] == '0' || board_str_str[i] == 'B' || board_str_str[i] == 'b' || board_str_str[i] == 'X' || board_str_str[i] == 'x' || board_str_str[i] == '*')
                    bd_arr[i] = BLACK;
                else if (board_str_str[i] == '1' || board_str_str[i] == 'W' || board_str_str[i] == 'w' || board_str_str[i] == 'O' || board_str_str[i] == 'o')
                    bd_arr[i] = WHITE;
                else if (board_str_str[i] == '.' || board_str_str[i] == '-')
                    bd_arr[i] = VACANT;
                else {
                    failed_res = true;
                    break;
                }
            }
            if (board_str_str[HW2] == '0' || board_str_str[HW2] == 'B' || board_str_str[HW2] == 'b' || board_str_str[HW2] == 'X' || board_str_str[HW2] == 'x' || board_str_str[HW2] == '*')
                player = 0;
            else if (board_str_str[HW2] == '1' || board_str_str[HW2] == 'W' || board_str_str[HW2] == 'w' || board_str_str[HW2] == 'O' || board_str_str[HW2] == 'o')
                player = 1;
            else
                failed_res = true;
        }
        if (!failed_res) {
            board.translate_from_arr(bd_arr, player);
            if (!board.is_end() && board.get_legal() == 0){
                board.pass();
            }
        }
        return failed_res;
    }
};

class Edit_board : public App::Scene {
private:
    Button back_button;
    Button set_button;
    Radio_button player_radio;
    Radio_button disc_radio;
    bool done;
    bool failed;
    History_elem history_elem;

public:
    Edit_board(const InitData& init) : IScene{ init } {
        back_button.init(BUTTON2_VERTICAL_SX, BUTTON2_VERTICAL_1_SY, BUTTON2_VERTICAL_WIDTH, BUTTON2_VERTICAL_HEIGHT, BUTTON2_VERTICAL_RADIUS, language.get("common", "back"), 25, getData().fonts.font, getData().colors.white, getData().colors.black);
        set_button.init(BUTTON2_VERTICAL_SX, BUTTON2_VERTICAL_2_SY, BUTTON2_VERTICAL_WIDTH, BUTTON2_VERTICAL_HEIGHT, BUTTON2_VERTICAL_RADIUS, language.get("in_out", "import"), 25, getData().fonts.font, getData().colors.white, getData().colors.black);
        done = false;
        failed = false;
        history_elem = getData().history_elem;
        Radio_button_element radio_button_elem;
        player_radio.init();
        radio_button_elem.init(480, 120, getData().fonts.font, 15, language.get("common", "black"), true);
        player_radio.push(radio_button_elem);
        radio_button_elem.init(480, 140, getData().fonts.font, 15, language.get("common", "white"), false);
        player_radio.push(radio_button_elem);
        disc_radio.init();
        radio_button_elem.init(480, 210, getData().fonts.font, 15, language.get("edit_board", "black"), true);
        disc_radio.push(radio_button_elem);
        radio_button_elem.init(480, 230, getData().fonts.font, 15, language.get("edit_board", "white"), false);
        disc_radio.push(radio_button_elem);
        radio_button_elem.init(480, 250, getData().fonts.font, 15, language.get("edit_board", "empty"), false);
        disc_radio.push(radio_button_elem);

    }

    void update() override {
        if (System::GetUserActions() & UserAction::CloseButtonClicked) {
            changeScene(U"Close", SCENE_FADE_TIME);
        }
        int board_arr[HW2];
        history_elem.board.translate_to_arr(board_arr, history_elem.player);
        for (int cell = 0; cell < HW2; ++cell) {
            int x = BOARD_SX + (cell % HW) * BOARD_CELL_SIZE;
            int y = BOARD_SY + (cell / HW) * BOARD_CELL_SIZE;
            Rect cell_region(x, y, BOARD_CELL_SIZE, BOARD_CELL_SIZE);
            if (cell_region.leftPressed()) {
                board_arr[cell] = disc_radio.checked;
            }
        }
        history_elem.board.translate_from_arr(board_arr, history_elem.player);
        if (KeyB.pressed()) {
            disc_radio.checked = BLACK;
        }
        else if (KeyW.pressed()) {
            disc_radio.checked = WHITE;
        }
        else if (KeyE.pressed()) {
            disc_radio.checked = VACANT;
        }
        Scene::SetBackground(getData().colors.green);
        getData().fonts.font(language.get("in_out", "edit_board")).draw(25, 480, 20, getData().colors.white);
        getData().fonts.font(language.get("in_out", "player")).draw(20, 480, 80, getData().colors.white);
        getData().fonts.font(language.get("in_out", "color")).draw(20, 480, 170, getData().colors.white);
        draw_board(getData().fonts, getData().colors, history_elem);
        player_radio.draw();
        disc_radio.draw();
        back_button.draw();
        set_button.draw();
        if (back_button.clicked() || KeyEscape.pressed()) {
            changeScene(U"Main_scene", SCENE_FADE_TIME);
        }
        if (set_button.clicked() || KeyEnter.pressed()) {
            if (history_elem.player != player_radio.checked){
                history_elem.board.pass();
                history_elem.player = player_radio.checked;
            }
            history_elem.v = GRAPH_IGNORE_VALUE;
            history_elem.level = -1;
            getData().history_elem = history_elem;
            if (!history_elem.board.is_end() && history_elem.board.get_legal() == 0){
                history_elem.board.pass();
            }
            int n_discs = history_elem.board.n_discs();
            int insert_place = (int)getData().graph_resources.nodes[getData().graph_resources.branch].size();
            int replace_place = -1;
            for (int i = 0; i < (int)getData().graph_resources.nodes[getData().graph_resources.branch].size(); ++i) {
                int node_n_discs = getData().graph_resources.nodes[getData().graph_resources.branch][i].board.n_discs();
                if (node_n_discs == n_discs) {
                    replace_place = i;
                    insert_place = -1;
                    break;
                }
                else if (node_n_discs > n_discs) {
                    insert_place = i;
                    break;
                }
            }
            history_elem.policy = -1; // reset last policy
            if (replace_place - 1 >= 0){
                uint64_t f_discs = getData().graph_resources.nodes[getData().graph_resources.branch][replace_place - 1].board.player | getData().graph_resources.nodes[getData().graph_resources.branch][replace_place - 1].board.opponent;
                uint64_t discs = history_elem.board.player | history_elem.board.opponent;
                if (pop_count_ull(discs ^ f_discs) == 1){
                    int last_policy = ctz(discs ^ f_discs);
                    history_elem.policy = last_policy;
                }
            } else if (insert_place - 1 >= 0 && insert_place - 1 < getData().graph_resources.nodes[getData().graph_resources.branch].size()){
                uint64_t f_discs = getData().graph_resources.nodes[getData().graph_resources.branch][insert_place - 1].board.player | getData().graph_resources.nodes[getData().graph_resources.branch][insert_place - 1].board.opponent;
                uint64_t discs = history_elem.board.player | history_elem.board.opponent;
                if (pop_count_ull(discs ^ f_discs) == 1){
                    int last_policy = ctz(discs ^ f_discs);
                    history_elem.policy = last_policy;
                }
            } else{
                for (int i = 0; i < (int)getData().graph_resources.nodes[0].size(); ++i) {
                    int node_n_discs = getData().graph_resources.nodes[0][i].board.n_discs();
                    if (node_n_discs + 1 == n_discs){
                        uint64_t f_discs = getData().graph_resources.nodes[0][i].board.player | getData().graph_resources.nodes[0][i].board.opponent;
                        uint64_t discs = history_elem.board.player | history_elem.board.opponent;
                        if (pop_count_ull(discs ^ f_discs) == 1){
                            int last_policy = ctz(discs ^ f_discs);
                            history_elem.policy = last_policy;
                        }
                    }
                }
            }
            if (replace_place != -1) {
                std::cerr << "replace" << std::endl;
                getData().graph_resources.nodes[getData().graph_resources.branch][replace_place] = history_elem;
            }
            else {
                std::cerr << "insert" << std::endl;
                getData().graph_resources.nodes[getData().graph_resources.branch].insert(getData().graph_resources.nodes[getData().graph_resources.branch].begin() + insert_place, history_elem);
            }
            getData().graph_resources.n_discs = n_discs;
            getData().graph_resources.need_init = false;
            getData().history_elem = getData().graph_resources.nodes[getData().graph_resources.branch].back();
            changeScene(U"Main_scene", SCENE_FADE_TIME);
        }
    }

    void draw() const override {

    }
};

class Import_game : public App::Scene {
private:
    std::vector<Game_abstract> games;
    std::vector<Button> import_buttons;
    Scroll_manager scroll_manager;
    Button back_button;
    bool failed;

public:
    Import_game(const InitData& init) : IScene{ init } {
        back_button.init(BACK_BUTTON_SX, BACK_BUTTON_SY, BACK_BUTTON_WIDTH, BACK_BUTTON_HEIGHT, BACK_BUTTON_RADIUS, language.get("common", "back"), 25, getData().fonts.font, getData().colors.white, getData().colors.black);
        failed = false;
        const String csv_path = Unicode::Widen(getData().directories.document_dir) + U"games/summary.csv";
        const CSV csv{ csv_path };
        if (csv) {
            for (size_t row = 0; row < csv.rows(); ++row) {
                Game_abstract game_abstract;
                game_abstract.date = csv[row][0];
                game_abstract.black_player = csv[row][1];
                game_abstract.white_player = csv[row][2];
                game_abstract.memo = csv[row][3];
                game_abstract.black_score = ParseOr<int32>(csv[row][4], GAME_DISCS_UNDEFINED);
                game_abstract.white_score = ParseOr<int32>(csv[row][5], GAME_DISCS_UNDEFINED);
                games.emplace_back(game_abstract);
            }
        }
        reverse(games.begin(), games.end());
        for (int i = 0; i < (int)games.size(); ++i){
            Button button;
            button.init(0, 0, IMPORT_GAME_BUTTON_WIDTH, IMPORT_GAME_BUTTON_HEIGHT, IMPORT_GAME_BUTTON_RADIUS, language.get("in_out", "import"), 15, getData().fonts.font, getData().colors.white, getData().colors.black);
            import_buttons.emplace_back(button);
        }
        scroll_manager.init(770, IMPORT_GAME_SY + 8, 10, IMPORT_GAME_HEIGHT * IMPORT_GAME_N_GAMES_ON_WINDOW, 20, (int)games.size(), IMPORT_GAME_N_GAMES_ON_WINDOW, IMPORT_GAME_SX, 73, IMPORT_GAME_WIDTH + 10, IMPORT_GAME_HEIGHT * IMPORT_GAME_N_GAMES_ON_WINDOW);
    }

    void update() override {
        if (System::GetUserActions() & UserAction::CloseButtonClicked) {
            changeScene(U"Close", SCENE_FADE_TIME);
        }
        getData().fonts.font(language.get("in_out", "input_game")).draw(25, Arg::topCenter(X_CENTER, 10), getData().colors.white);
        back_button.draw();
        if (back_button.clicked() || KeyEscape.pressed()) {
            getData().graph_resources.need_init = false;
            changeScene(U"Main_scene", SCENE_FADE_TIME);
        }
        if (failed) {
            getData().fonts.font(language.get("in_out", "import_failed")).draw(20, Arg::center(X_CENTER, Y_CENTER), getData().colors.white);
        }
        else if (games.size() == 0) {
            getData().fonts.font(language.get("in_out", "no_game_available")).draw(20, Arg::center(X_CENTER, Y_CENTER), getData().colors.white);
        }
        else {
            int sy = IMPORT_GAME_SY;
            int strt_idx_int = scroll_manager.get_strt_idx_int();
            if (strt_idx_int > 0) {
                getData().fonts.font(U"︙").draw(15, Arg::bottomCenter = Vec2{ X_CENTER, sy }, getData().colors.white);
            }
            sy += 8;
            for (int i = strt_idx_int; i < std::min((int)games.size(), strt_idx_int + IMPORT_GAME_N_GAMES_ON_WINDOW); ++i) {
                Rect rect;
                rect.y = sy;
                rect.x = IMPORT_GAME_SX;
                rect.w = IMPORT_GAME_WIDTH;
                rect.h = IMPORT_GAME_HEIGHT;
                int winner = -1;
                if (games[i].black_score != GAME_DISCS_UNDEFINED && games[i].white_score != GAME_DISCS_UNDEFINED) {
                    if (games[i].black_score > games[i].white_score) {
                        winner = IMPORT_GAME_WINNER_BLACK;
                    }
                    else if (games[i].black_score < games[i].white_score) {
                        winner = IMPORT_GAME_WINNER_WHITE;
                    }
                    else {
                        winner = IMPORT_GAME_WINNER_DRAW;
                    }
                }
                if (i % 2){
                    rect.draw(getData().colors.dark_green).drawFrame(1.0, getData().colors.white);
                } else{
                    rect.draw(getData().colors.green).drawFrame(1.0, getData().colors.white);
                }
                String date = games[i].date.substr(0, 10).replace(U"_", U"/");
                getData().fonts.font(date).draw(15, IMPORT_GAME_SX + 10, sy + 2, getData().colors.white);
                // player (black)
                Rect black_player_rect;
                black_player_rect.w = IMPORT_GAME_PLAYER_WIDTH;
                black_player_rect.h = IMPORT_GAME_PLAYER_HEIGHT;
                black_player_rect.y = sy + 1;
                black_player_rect.x = IMPORT_GAME_SX + IMPORT_GAME_DATE_WIDTH;
                if (winner == IMPORT_GAME_WINNER_BLACK) {
                    black_player_rect.draw(getData().colors.darkred);
                }
                else if (winner == IMPORT_GAME_WINNER_WHITE) {
                    black_player_rect.draw(getData().colors.darkblue);
                }
                else if (winner == IMPORT_GAME_WINNER_DRAW) {
                    black_player_rect.draw(getData().colors.chocolate);
                }
                int upper_center_y = black_player_rect.y + black_player_rect.h / 2;
                for (int font_size = 15; font_size >= 12; --font_size){
                    if (getData().fonts.font(games[i].black_player).region(font_size, Vec2{0, 0}).w <= IMPORT_GAME_PLAYER_WIDTH - 4){
                        getData().fonts.font(games[i].black_player).draw(font_size, Arg::rightCenter(black_player_rect.x + IMPORT_GAME_PLAYER_WIDTH - 2, upper_center_y), getData().colors.white);
                        break;
                    } else if (font_size == 12){
                        String player = games[i].black_player;
                        while (getData().fonts.font(player).region(font_size, Vec2{0, 0}).w > IMPORT_GAME_PLAYER_WIDTH - 4){
                            for (int i = 0; i < 4; ++i){
                                player.pop_back();
                            }
                            player += U"...";
                        }
                        getData().fonts.font(player).draw(font_size, Arg::rightCenter(black_player_rect.x + IMPORT_GAME_PLAYER_WIDTH - 2, upper_center_y), getData().colors.white);
                    }
                }
                // score
                String black_score = U"??";
                String white_score = U"??";
                if (games[i].black_score != GAME_DISCS_UNDEFINED && games[i].white_score != GAME_DISCS_UNDEFINED) {
                    black_score = Format(games[i].black_score);
                    white_score = Format(games[i].white_score);
                }
                double hyphen_w = getData().fonts.font(U"-").region(15, Vec2{0, 0}).w;
                getData().fonts.font(black_score).draw(15, Arg::rightCenter(black_player_rect.x + IMPORT_GAME_PLAYER_WIDTH + IMPORT_GAME_SCORE_WIDTH / 2 - hyphen_w / 2 - 1, upper_center_y), getData().colors.white);
                getData().fonts.font(U"-").draw(15, Arg::center(black_player_rect.x + IMPORT_GAME_PLAYER_WIDTH + IMPORT_GAME_SCORE_WIDTH / 2, upper_center_y), getData().colors.white);
                getData().fonts.font(white_score).draw(15, Arg::leftCenter(black_player_rect.x + IMPORT_GAME_PLAYER_WIDTH + IMPORT_GAME_SCORE_WIDTH / 2 + hyphen_w / 2 + 1, upper_center_y), getData().colors.white);
                // player (white)
                Rect white_player_rect;
                white_player_rect.w = IMPORT_GAME_PLAYER_WIDTH;
                white_player_rect.h = IMPORT_GAME_PLAYER_HEIGHT;
                white_player_rect.y = sy + 1;
                white_player_rect.x = black_player_rect.x + IMPORT_GAME_PLAYER_WIDTH + IMPORT_GAME_SCORE_WIDTH;
                if (winner == IMPORT_GAME_WINNER_BLACK) {
                    white_player_rect.draw(getData().colors.darkblue);
                }
                else if (winner == IMPORT_GAME_WINNER_WHITE) {
                    white_player_rect.draw(getData().colors.darkred);
                }
                else if (winner == IMPORT_GAME_WINNER_DRAW) {
                    white_player_rect.draw(getData().colors.chocolate);
                }
                for (int font_size = 15; font_size >= 12; --font_size){
                    if (getData().fonts.font(games[i].white_player).region(font_size, Vec2{0, 0}).w <= IMPORT_GAME_PLAYER_WIDTH - 4){
                        getData().fonts.font(games[i].white_player).draw(font_size, Arg::leftCenter(white_player_rect.x + 2, upper_center_y), getData().colors.white);
                        break;
                    } else if (font_size == 12){
                        String player = games[i].white_player;
                        while (getData().fonts.font(player).region(font_size, Vec2{0, 0}).w > IMPORT_GAME_PLAYER_WIDTH - 4){
                            for (int i = 0; i < 4; ++i){
                                player.pop_back();
                            }
                            player += U"...";
                        }
                        getData().fonts.font(player).draw(font_size, Arg::leftCenter(white_player_rect.x + 2, upper_center_y), getData().colors.white);
                    }
                }
                getData().fonts.font(games[i].memo).draw(12, IMPORT_GAME_SX + 10, black_player_rect.y + black_player_rect.h, getData().colors.white);
                import_buttons[i].move(IMPORT_GAME_BUTTON_SX, sy + IMPORT_GAME_BUTTON_SY);
                import_buttons[i].draw();
                if (import_buttons[i].clicked()){
                    import_game(i);
                }
                sy += IMPORT_GAME_HEIGHT;
            }
            if (strt_idx_int + IMPORT_GAME_N_GAMES_ON_WINDOW < (int)games.size()) {
                getData().fonts.font(U"︙").draw(15, Arg::bottomCenter = Vec2{ X_CENTER, 415}, getData().colors.white);
            }
            scroll_manager.draw();
            scroll_manager.update();
        }
    }

    void draw() const override {

    }

private:
    void import_game(int idx) {
        const String json_path = Unicode::Widen(getData().directories.document_dir) + U"games/" + games[idx].date + U".json";
        JSON game_json = JSON::Load(json_path);
        if (not game_json) {
            std::cerr << "can't open game" << std::endl;
            failed = true;
            return;
        }
        if (game_json[GAME_BLACK_PLAYER].getType() == JSONValueType::String) {
            getData().game_information.black_player_name = game_json[GAME_BLACK_PLAYER].getString();
        }
        if (game_json[GAME_WHITE_PLAYER].getType() == JSONValueType::String) {
            getData().game_information.white_player_name = game_json[GAME_WHITE_PLAYER].getString();
        }
        if (game_json[GAME_MEMO].getType() == JSONValueType::String) {
            getData().game_information.memo = game_json[GAME_MEMO].getString();
        }
        getData().graph_resources.nodes[GRAPH_MODE_NORMAL].clear();
        getData().graph_resources.nodes[GRAPH_MODE_INSPECT].clear();
        for (int n_discs = 4; n_discs <= HW2; ++n_discs) {
            String n_discs_str = Format(n_discs);
            History_elem history_elem;
            bool error_found = false;
            if (game_json[n_discs_str][GAME_BOARD_PLAYER].getType() == JSONValueType::Number) {
                history_elem.board.player = game_json[n_discs_str][GAME_BOARD_PLAYER].get<uint64_t>();
            }
            else {
                error_found = true;
            }
            if (game_json[n_discs_str][GAME_BOARD_OPPONENT].getType() == JSONValueType::Number) {
                history_elem.board.opponent = game_json[n_discs_str][GAME_BOARD_OPPONENT].get<uint64_t>();
            }
            else {
                error_found = true;
            }
            if (game_json[n_discs_str][GAME_LEVEL].getType() == JSONValueType::Number) {
                history_elem.level = game_json[n_discs_str][GAME_LEVEL].get<int>();
            }
            else {
                error_found = true;
            }
            if (game_json[n_discs_str][GAME_PLAYER].getType() == JSONValueType::Number) {
                history_elem.player = game_json[n_discs_str][GAME_PLAYER].get<int>();
            }
            else {
                error_found = true;
            }
            if (game_json[n_discs_str][GAME_POLICY].getType() == JSONValueType::Number) {
                history_elem.policy = game_json[n_discs_str][GAME_POLICY].get<int>();
            }
            else {
                error_found = true;
            }
            if (game_json[n_discs_str][GAME_NEXT_POLICY].getType() == JSONValueType::Number) {
                history_elem.next_policy = game_json[n_discs_str][GAME_NEXT_POLICY].get<int>();
            }
            else {
                error_found = true;
            }
            if (game_json[n_discs_str][GAME_VALUE].getType() == JSONValueType::Number) {
                history_elem.v = game_json[n_discs_str][GAME_VALUE].get<int>();
            }
            else {
                error_found = true;
            }
            if (!error_found) {
                getData().graph_resources.nodes[GRAPH_MODE_NORMAL].emplace_back(history_elem);
            }
        }
        std::string opening_name, n_opening_name;
        for (int i = 0; i < (int)getData().graph_resources.nodes[GRAPH_MODE_NORMAL].size(); ++i) {
            n_opening_name.clear();
            n_opening_name = opening.get(getData().graph_resources.nodes[GRAPH_MODE_NORMAL][i].board, getData().graph_resources.nodes[GRAPH_MODE_NORMAL][i].player ^ 1);
            if (n_opening_name.size()) {
                opening_name = n_opening_name;
            }
            getData().graph_resources.nodes[GRAPH_MODE_NORMAL][i].opening_name = opening_name;
        }
        getData().graph_resources.n_discs = getData().graph_resources.nodes[GRAPH_MODE_NORMAL].back().board.n_discs();
        getData().graph_resources.need_init = false;
        getData().history_elem = getData().graph_resources.nodes[GRAPH_MODE_NORMAL].back();
        changeScene(U"Main_scene", SCENE_FADE_TIME);
    }
};


class Import_bitboard : public App::Scene {
private:
    Button single_back_button;
    Button back_button;
    Button import_button;
    TextAreaEditState text_area[2];
    std::string player_string;
    std::string opponent_string;
    Radio_button player_radio;
    Board board;
    bool done;
    bool failed;

public:
    Import_bitboard(const InitData& init) : IScene{ init } {
        single_back_button.init(BACK_BUTTON_SX, BACK_BUTTON_SY, BACK_BUTTON_WIDTH, BACK_BUTTON_HEIGHT, BACK_BUTTON_RADIUS, language.get("common", "back"), 25, getData().fonts.font, getData().colors.white, getData().colors.black);
        back_button.init(GO_BACK_BUTTON_BACK_SX, GO_BACK_BUTTON_SY, GO_BACK_BUTTON_WIDTH, GO_BACK_BUTTON_HEIGHT, GO_BACK_BUTTON_RADIUS, language.get("common", "back"), 25, getData().fonts.font, getData().colors.white, getData().colors.black);
        import_button.init(GO_BACK_BUTTON_GO_SX, GO_BACK_BUTTON_SY, GO_BACK_BUTTON_WIDTH, GO_BACK_BUTTON_HEIGHT, GO_BACK_BUTTON_RADIUS, language.get("in_out", "import"), 25, getData().fonts.font, getData().colors.white, getData().colors.black);
        Radio_button_element radio_button_elem;
        player_radio.init();
        radio_button_elem.init(X_CENTER, 70 + SCENE_ICON_WIDTH + 175, getData().fonts.font, 20, language.get("common", "black"), true);
        player_radio.push(radio_button_elem);
        radio_button_elem.init(X_CENTER, 70 + SCENE_ICON_WIDTH + 201, getData().fonts.font, 20, language.get("common", "white"), false);
        player_radio.push(radio_button_elem);
        done = false;
        failed = false;
        text_area[0].active = true;
    }

    void update() override {
        if (System::GetUserActions() & UserAction::CloseButtonClicked) {
            changeScene(U"Close", SCENE_FADE_TIME);
        }
        Scene::SetBackground(getData().colors.green);
        const int icon_width = SCENE_ICON_WIDTH;
        getData().resources.icon.scaled((double)icon_width / getData().resources.icon.width()).draw(X_CENTER - icon_width / 2, 20);
        getData().resources.logo.scaled((double)icon_width / getData().resources.logo.width()).draw(X_CENTER - icon_width / 2, 20 + icon_width);
        int sy = 20 + icon_width + 50;
        if (!done) {
            getData().fonts.font(language.get("in_out", "input_bitboard")).draw(25, Arg::topCenter(X_CENTER, sy), getData().colors.white);
            const int text_area_y[2] = {sy + 40, sy + 90};
            constexpr int text_area_h = 40;
            constexpr int circle_radius = 15;
            for (int i = 0; i < 2; ++i){
                SimpleGUI::TextArea(text_area[i], Vec2{X_CENTER - 300, text_area_y[i]}, SizeF{600, text_area_h}, SimpleGUI::PreferredTextAreaMaxChars);
                if (player_radio.checked == i){
                    Circle(X_CENTER + 330, text_area_y[i] + text_area_h / 2, circle_radius).draw(getData().colors.black);
                } else{
                    Circle(X_CENTER + 330, text_area_y[i] + text_area_h / 2, circle_radius).draw(getData().colors.white);
                }
            }
            getData().fonts.font(language.get("in_out", "player")).draw(20, Arg::rightCenter(X_CENTER - 310, text_area_y[0] + text_area_h / 2), getData().colors.white);
            getData().fonts.font(language.get("in_out", "opponent")).draw(20, Arg::rightCenter(X_CENTER - 310, text_area_y[1] + text_area_h / 2), getData().colors.white);
            getData().fonts.font(language.get("in_out", "you_can_paste_with_ctrl_v")).draw(13, Arg::topCenter(X_CENTER, sy + 140), getData().colors.white);
            getData().fonts.font(language.get("in_out", "player")).draw(25, Arg::rightCenter(X_CENTER - 5, 70 + SCENE_ICON_WIDTH + 188), getData().colors.white);
            player_radio.draw();
            for (int i = 0; i < 2; ++i){
                std::string str = text_area[i].text.narrow();
                if (str.find("\t") != std::string::npos){
                    text_area[i].active = false;
                    text_area[(i + 1) % 2].active = true;
                    int tab_place = str.find("\t");
                    std::string txt0;
                    for (int j = 0; j < tab_place; ++j){
                        txt0 += str[j];
                    }
                    std::string txt1;
                    for (int j = tab_place + 1; j < (int)str.size(); ++j){
                        txt1 += str[j];
                    }
                    text_area[i].text = Unicode::Widen(txt0);
                    text_area[i].cursorPos = text_area[i].text.size();
                    text_area[i].rebuildGlyphs();
                    text_area[(i + 1) % 2].text += Unicode::Widen(txt1);
                    text_area[(i + 1) % 2].cursorPos = text_area[(i + 1) % 2].text.size();
                    text_area[(i + 1) % 2].rebuildGlyphs();
                }
            }
            bool return_pressed = false;
            for (int i = 0; i < 2; ++i){
                if (text_area[i].text.size()) {
                    if (text_area[i].text[text_area[i].text.size() - 1] == '\n') {
                        return_pressed = true;
                    }
                }
            }
            player_string = text_area[0].text.replaced(U"\r", U"").replaced(U"\n", U"").replaced(U" ", U"").narrow();
            opponent_string = text_area[1].text.replaced(U"\r", U"").replaced(U"\n", U"").replaced(U" ", U"").narrow();
            back_button.draw();
            import_button.draw();
            if (back_button.clicked() || KeyEscape.pressed()) {
                changeScene(U"Main_scene", SCENE_FADE_TIME);
            }
            if (import_button.clicked() || KeyEnter.pressed()) {
                failed = import_bitboard_processing();
                done = true;
            }
        }
        else {
            if (!failed) {
                getData().graph_resources.init();
                History_elem history_elem;
                history_elem.player = player_radio.checked;
                history_elem.board = board;
                getData().graph_resources.nodes[0].emplace_back(history_elem);
                getData().graph_resources.n_discs = board.n_discs();
                getData().game_information.init();
                getData().graph_resources.need_init = false;
                getData().history_elem = getData().graph_resources.nodes[0].back();
                changeScene(U"Main_scene", SCENE_FADE_TIME);
            }
            else {
                getData().fonts.font(language.get("in_out", "import_failed")).draw(25, Arg::topCenter(X_CENTER, sy), getData().colors.white);
                single_back_button.draw();
                if (single_back_button.clicked() || KeyEscape.pressed()) {
                    changeScene(U"Main_scene", SCENE_FADE_TIME);
                }
            }
        }
    }

    void draw() const override {

    }

private:
    bool import_bitboard_processing() {
        if (player_string[0] == '0' && player_string[1] == 'x'){
            player_string = player_string.substr(2);
        }
        if (opponent_string[0] == '0' && opponent_string[1] == 'x'){
            opponent_string = opponent_string.substr(2);
        }
        while (player_string.size() < 16){
            player_string = "0" + player_string;
        }
        while (opponent_string.size() < 16){
            opponent_string = "0" + opponent_string;
        }
        board.player = 0;
        board.opponent = 0;
        for (int i = 0; i < 16; ++i){
            player_string[i] |= 0x20; // lower case
            opponent_string[i] |= 0x20; // lower case
            if (player_string[i] < '0' || ('9' < player_string[i] && player_string[i] < 'a') || 'f' < player_string[i]){ // out of range
                std::cerr << "player out of range" << std::endl;
                return true;
            }
            if (opponent_string[i] < '0' || ('9' < opponent_string[i] && opponent_string[i] < 'a') || 'f' < opponent_string[i]){ // out of range
                std::cerr << "opponent out of range" << std::endl;
                return true;
            }
            int p_int = player_string[i] % 87 % 48; // 0-9, a-f hex to decimal
            int o_int = opponent_string[i] % 87 % 48; // 0-9, a-f hex to decimal
            if (p_int & o_int){ // 2 discs on same square
                std::cerr << "both discs on same square" << std::endl;
                return true;
            }
            for (int j = 0; j < 4; ++j){
                int idx = HW2_M1 - (i * 4 + 3 - j);
                board.player |= (uint64_t)(p_int & 1) << idx;
                board.opponent |= (uint64_t)(o_int & 1) << idx;
                p_int /= 2;
                o_int /= 2;
            }
        }
        if (!board.is_end() && board.get_legal() == 0){
            board.pass();
        }
        return false;
    }
};