#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <string>
#include <vector>
#include <math.h>
#include <unordered_set>
#include "board.hpp"

using namespace std;

#define n_phases 15
#define phase_n_stones 4
#define n_patterns 13
#define n_eval (n_patterns + 3)
#define max_surround 80
#define max_canput 50
#define max_evaluate_idx 59049

int sa_phase;

#define p31 3
#define p32 9
#define p33 27
#define p34 81
#define p35 243
#define p36 729
#define p37 2187
#define p38 6561
#define p39 19683
#define p310 59049

#define sc_w 6400
#define step 100

#define n_data 30000000

#define n_raw_params (50 + 3)

double alpha;
#define beta 0.1

const int pattern_sizes[n_patterns] = {8, 8, 8, 5, 6, 7, 8, 10, 10, 10, 10, 9, 10};
const int eval_sizes[n_eval] = {p38, p38, p38, p35, p36, p37, p38, p310, p310, p310, p310, p39, p310, max_surround * max_surround, max_surround * max_canput * 2, max_surround * max_canput * 2};
double eval_arr[n_phases][n_eval][max_evaluate_idx];
int test_data[n_data / 6 + 10000][n_raw_params];
double test_labels[n_data / 6 + 10000];
int nums;
double scores;
vector<int> test_memo[n_eval][max_evaluate_idx];
vector<double> test_scores, pre_calc_scores;
unordered_set<int> used_idxes[n_eval];
vector<int> used_idxes_vector[n_eval];
int rev_idxes[n_patterns][max_evaluate_idx];


inline unsigned long long tim(){
    return chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
}

int xorx=123456789, xory=362436069, xorz=521288629, xorw=88675123;
inline double myrandom(){
    int t = (xorx^(xorx<<11));
    xorx = xory;
    xory = xorz;
    xorz = xorw;
    xorw = (xorw^(xorw>>19))^(t^(t>>8));
    return (double)(xorw) / 2147483648.0;
}

inline int myrandrange(int s, int e){
    return s + (int)(myrandom() * (e - s));
}

void initialize_param(){
    int phase_idx, pattern_idx, pattern_elem, dense_idx, canput, sur0, sur1, i, j, k;
    for (phase_idx = 0; phase_idx < n_phases; ++phase_idx){
        cerr << "=";
        for (pattern_idx = 0; pattern_idx < n_eval; ++pattern_idx){
            for (pattern_elem = 0; pattern_elem < eval_sizes[pattern_idx]; ++pattern_elem)
                eval_arr[phase_idx][pattern_idx][pattern_elem] = 0;
        }
    }
    cerr << endl;
}

void input_param(){
    ifstream ifs("f_param.txt");
    if (ifs.fail()){
        cerr << "evaluation file not exist" << endl;
        exit(1);
    }
    string line;
    int t =0;
    int phase_idx, pattern_idx, pattern_elem, dense_idx, canput, sur0, sur1, i, j, k;
    for (phase_idx = 0; phase_idx < n_phases; ++phase_idx){
        cerr << "=";
        for (pattern_idx = 0; pattern_idx < n_eval; ++pattern_idx){
            for (pattern_elem = 0; pattern_elem < eval_sizes[pattern_idx]; ++pattern_elem){
                ++t;
                getline(ifs, line);
                eval_arr[phase_idx][pattern_idx][pattern_elem] = stoi(line);
            }
        }
    }
    cerr << t << endl;
}

void input_param_onephase(string file){
    ifstream ifs(file);
    if (ifs.fail()){
        cerr << "evaluation file not exist" << endl;
        exit(1);
    }
    string line;
    int t =0;
    int pattern_idx, pattern_elem, dense_idx, canput, sur0, sur1, i, j, k;
    for (pattern_idx = 0; pattern_idx < n_eval; ++pattern_idx){
        cerr << "=";
        for (pattern_elem = 0; pattern_elem < eval_sizes[pattern_idx]; ++pattern_elem){
            ++t;
            getline(ifs, line);
            eval_arr[sa_phase][pattern_idx][pattern_elem] = stoi(line);
        }
    }
    cerr << t << endl;
}

inline int calc_sur0_sur1(int arr[]){
    return arr[50] * max_surround + arr[51];
}

inline int calc_sur0_canput(int arr[]){
    return arr[50] * max_surround + arr[52];
}

inline int calc_sur1_canput(int arr[]){
    return arr[51] * max_surround + arr[52];
}

void input_test_data(int strt){
    int i, j, k;
    ifstream ifs("big_data.txt");
    if (ifs.fail()){
        cerr << "evaluation file not exist" << endl;
        exit(1);
    }
    string line;
    int phase, player, score;
    int t = 0, u = 0;
    nums = 0;
    const int pattern_nums[50] = {
        0, 0, 0, 0,
        1, 1, 1, 1,
        2, 2, 2, 2,
        3, 3, 3, 3,
        4, 4, 4, 4,
        5, 5, 5, 5,
        6, 6,
        7, 7, 7, 7,
        8, 8, 8, 8,
        9, 9, 9, 9,
        10, 10, 10, 10,
        11, 11, 11, 11,
        12, 12, 12, 12
    };
    for (j = 0; j < n_eval; ++j){
        used_idxes[j].clear();
        for (k = 0; k < max_evaluate_idx; ++k)
            test_memo[j][k].clear();
    }
    test_scores.clear();
    for (i = 0; i < strt; ++i)
        getline(ifs, line);
    while (getline(ifs, line) && t < n_data){
        ++t;
        if ((t & 0b1111111111111111) == 0b1111111111111111)
            cerr << '\r' << t;
        istringstream iss(line);
        iss >> phase;
        if (phase == sa_phase){
            ++u;
            iss >> player;
            for (i = 0; i < n_raw_params; ++i)
                iss >> test_data[nums][i];
            iss >> score;
            for (i = 0; i < 50; ++i)
                used_idxes[pattern_nums[i]].emplace(test_data[nums][i]);
            used_idxes[13].emplace(calc_sur0_sur1(test_data[nums]));
            used_idxes[14].emplace(calc_sur0_canput(test_data[nums]));
            used_idxes[15].emplace(calc_sur1_canput(test_data[nums]));
            test_labels[nums] = score * step;
            for (i = 0; i < 50; ++i)
                test_memo[pattern_nums[i]][test_data[nums][i]].push_back(nums);
            test_memo[13][calc_sur0_sur1(test_data[nums])].push_back(nums);
            test_memo[14][calc_sur0_canput(test_data[nums])].push_back(nums);
            test_memo[15][calc_sur1_canput(test_data[nums])].push_back(nums);
            test_scores.push_back(0);
            pre_calc_scores.push_back(0);
            ++nums;
        }
    }
    cerr << '\r' << t << endl;
    cerr << "loaded data" << endl;
    for (i = 0; i < n_eval; ++i){
        for (auto elem: used_idxes[i])
            used_idxes_vector[i].push_back(elem);
    }

    cerr << "n_data " << u << endl;

    alpha = beta / (double)u;

    u = 0;
    for (i = 0; i < n_eval; ++i)
        u += eval_sizes[i];
    cerr << "n_all_param " << u << endl;
    u = 0;
    for (i = 0; i < n_eval; ++i){
        u += (int)used_idxes[i].size();
    }
    cerr << "used_param " << u << endl;
    
    ifs.close();
}

void output_param(){
    int phase_idx, pattern_idx, pattern_elem, dense_idx, canput, sur0, sur1, i, j;
    for (phase_idx = 0; phase_idx < n_phases; ++phase_idx){
        cerr << "=";
        for (pattern_idx = 0; pattern_idx < n_eval; ++pattern_idx){
            for (pattern_elem = 0; pattern_elem < eval_sizes[pattern_idx]; ++pattern_elem){
                cout << round(eval_arr[phase_idx][pattern_idx][pattern_elem]) << endl;
            }
        }
    }
    cerr << endl;
}

void output_param_onephase(){
    int pattern_idx, pattern_elem, dense_idx, canput, sur0, sur1, i, j;
    cerr << "=";
    for (pattern_idx = 0; pattern_idx < n_eval; ++pattern_idx){
        for (pattern_elem = 0; pattern_elem < eval_sizes[pattern_idx]; ++pattern_elem){
            cout << round(eval_arr[sa_phase][pattern_idx][pattern_elem]) << endl;
        }
    }
    cerr << endl;
}

inline double loss(double x, int siz){
    //double sq_size = sqrt((double)siz);
    //double tmp = (double)x / sq_size;
    return (double)x / (double)siz * (double)x;
}

inline double calc_score(int phase, int i){
    return
        eval_arr[phase][0][test_data[i][0]] + 
        eval_arr[phase][0][test_data[i][1]] + 
        eval_arr[phase][0][test_data[i][2]] + 
        eval_arr[phase][0][test_data[i][3]] + 
        eval_arr[phase][1][test_data[i][4]] + 
        eval_arr[phase][1][test_data[i][5]] + 
        eval_arr[phase][1][test_data[i][6]] + 
        eval_arr[phase][1][test_data[i][7]] + 
        eval_arr[phase][2][test_data[i][8]] + 
        eval_arr[phase][2][test_data[i][9]] + 
        eval_arr[phase][2][test_data[i][10]] + 
        eval_arr[phase][2][test_data[i][11]] + 
        eval_arr[phase][3][test_data[i][12]] + 
        eval_arr[phase][3][test_data[i][13]] + 
        eval_arr[phase][3][test_data[i][14]] + 
        eval_arr[phase][3][test_data[i][15]] + 
        eval_arr[phase][4][test_data[i][16]] + 
        eval_arr[phase][4][test_data[i][17]] + 
        eval_arr[phase][4][test_data[i][18]] + 
        eval_arr[phase][4][test_data[i][19]] + 
        eval_arr[phase][5][test_data[i][20]] + 
        eval_arr[phase][5][test_data[i][21]] + 
        eval_arr[phase][5][test_data[i][22]] + 
        eval_arr[phase][5][test_data[i][23]] + 
        eval_arr[phase][6][test_data[i][24]] + 
        eval_arr[phase][6][test_data[i][25]] + 
        eval_arr[phase][7][test_data[i][26]] + 
        eval_arr[phase][7][test_data[i][27]] + 
        eval_arr[phase][7][test_data[i][28]] + 
        eval_arr[phase][7][test_data[i][29]] + 
        eval_arr[phase][8][test_data[i][30]] + 
        eval_arr[phase][8][test_data[i][31]] + 
        eval_arr[phase][8][test_data[i][32]] + 
        eval_arr[phase][8][test_data[i][33]] + 
        eval_arr[phase][9][test_data[i][34]] + 
        eval_arr[phase][9][test_data[i][35]] + 
        eval_arr[phase][9][test_data[i][36]] + 
        eval_arr[phase][9][test_data[i][37]] + 
        eval_arr[phase][10][test_data[i][38]] + 
        eval_arr[phase][10][test_data[i][39]] + 
        eval_arr[phase][10][test_data[i][40]] + 
        eval_arr[phase][10][test_data[i][41]] + 
        eval_arr[phase][11][test_data[i][42]] + 
        eval_arr[phase][11][test_data[i][43]] + 
        eval_arr[phase][11][test_data[i][44]] + 
        eval_arr[phase][11][test_data[i][45]] + 
        eval_arr[phase][12][test_data[i][46]] + 
        eval_arr[phase][12][test_data[i][47]] + 
        eval_arr[phase][12][test_data[i][48]] + 
        eval_arr[phase][12][test_data[i][49]] + 
        //eval_arr[phase][13][test_data[i][50]] + 
        //eval_arr[phase][13][test_data[i][51]] + 
        //eval_arr[phase][13][test_data[i][52]] + 
        //eval_arr[phase][13][test_data[i][53]] + 
        eval_arr[phase][13][calc_sur0_sur1(test_data[i])] + 
        eval_arr[phase][14][calc_sur0_canput(test_data[i])] + 
        eval_arr[phase][15][calc_sur1_canput(test_data[i])];
}

inline int calc_pop(int a, int b, int s){
    return (a / pow3[s - 1 - b]) % 3;
}

inline int calc_rev_idx(int pattern_idx, int pattern_size, int idx){
    int res = 0;
    if (pattern_idx <= 7 || pattern_idx == 12){
        for (int i = 0; i < pattern_size; ++i)
            res += pow3[i] * calc_pop(idx, i, pattern_size);
    } else if (pattern_idx == 8){
        res += p39 * calc_pop(idx, 0, pattern_size);
        res += p38 * calc_pop(idx, 4, pattern_size);
        res += p37 * calc_pop(idx, 7, pattern_size);
        res += p36 * calc_pop(idx, 9, pattern_size);
        res += p35 * calc_pop(idx, 1, pattern_size);
        res += p34 * calc_pop(idx, 5, pattern_size);
        res += p33 * calc_pop(idx, 8, pattern_size);
        res += p32 * calc_pop(idx, 2, pattern_size);
        res += p31 * calc_pop(idx, 6, pattern_size);
        res += calc_pop(idx, 3, pattern_size);
    } else if (pattern_idx == 9){
        res += p39 * calc_pop(idx, 5, pattern_size);
        res += p38 * calc_pop(idx, 4, pattern_size);
        res += p37 * calc_pop(idx, 3, pattern_size);
        res += p36 * calc_pop(idx, 2, pattern_size);
        res += p35 * calc_pop(idx, 1, pattern_size);
        res += p34 * calc_pop(idx, 0, pattern_size);
        res += p33 * calc_pop(idx, 9, pattern_size);
        res += p32 * calc_pop(idx, 8, pattern_size);
        res += p31 * calc_pop(idx, 7, pattern_size);
        res += calc_pop(idx, 6, pattern_size);
    } else if (pattern_idx == 10){
        res += p39 * calc_pop(idx, 0, pattern_size);
        res += p38 * calc_pop(idx, 1, pattern_size);
        res += p37 * calc_pop(idx, 2, pattern_size);
        res += p36 * calc_pop(idx, 3, pattern_size);
        res += p35 * calc_pop(idx, 7, pattern_size);
        res += p34 * calc_pop(idx, 8, pattern_size);
        res += p33 * calc_pop(idx, 9, pattern_size);
        res += p32 * calc_pop(idx, 4, pattern_size);
        res += p31 * calc_pop(idx, 5, pattern_size);
        res += calc_pop(idx, 6, pattern_size);
    } else if (pattern_idx == 11){
        res += p38 * calc_pop(idx, 0, pattern_size);
        res += p37 * calc_pop(idx, 3, pattern_size);
        res += p36 * calc_pop(idx, 6, pattern_size);
        res += p35 * calc_pop(idx, 1, pattern_size);
        res += p34 * calc_pop(idx, 4, pattern_size);
        res += p33 * calc_pop(idx, 7, pattern_size);
        res += p32 * calc_pop(idx, 2, pattern_size);
        res += p31 * calc_pop(idx, 5, pattern_size);
        res += calc_pop(idx, 8, pattern_size);
    } else if (pattern_idx == 13){
        res += p39 * calc_pop(idx, 0, pattern_size);
        res += p38 * calc_pop(idx, 5, pattern_size);
        res += p37 * calc_pop(idx, 7, pattern_size);
        res += p36 * calc_pop(idx, 8, pattern_size);
        res += p35 * calc_pop(idx, 9, pattern_size);
        res += p34 * calc_pop(idx, 1, pattern_size);
        res += p33 * calc_pop(idx, 6, pattern_size);
        res += p32 * calc_pop(idx, 2, pattern_size);
        res += p31 * calc_pop(idx, 3, pattern_size);
        res += calc_pop(idx, 4, pattern_size);
    }
    return res;
}

inline void scoring_mae(){
    int i, j, score;
    double avg_score, res = 0.0;
    avg_score = 0;
    for (i = 0; i < nums; ++i){
        score = pre_calc_scores[i];
        avg_score += fabs(test_labels[i] - (double)score) / nums;
    }
    cerr << " " << avg_score << "                   ";
}

inline double scoring_next_step(int pattern, int idx){
    double score, res = 0.0, err;
    int data_size = nums;
    for (const int &i: test_memo[pattern][idx]){
        score = pre_calc_scores[i];
        err = test_labels[i] - score;
        res += err;
    }
    return res;
}

inline void next_step(){
    int pattern, rev_idx;
    double err;
    for (int i = 0; i < nums; ++i)
        pre_calc_scores[i] = calc_score(sa_phase, i);
    for (pattern = 0; pattern < n_eval; ++pattern){
        for (const int &idx: used_idxes_vector[pattern]){
            if (pattern < n_patterns){
                rev_idx = rev_idxes[pattern][idx];
                if (idx < rev_idx){
                    err = scoring_next_step(pattern, idx) + scoring_next_step(pattern, rev_idx);
                    eval_arr[sa_phase][pattern][idx] += 2.0 * alpha * err;
                    eval_arr[sa_phase][pattern][rev_idx] += 2.0 * alpha * err;
                } else if (idx == rev_idx){
                    err = scoring_next_step(pattern, idx);
                    eval_arr[sa_phase][pattern][idx] += 2.0 * alpha * err;
                }
            } else{
                err = scoring_next_step(pattern, idx);
                eval_arr[sa_phase][pattern][idx] += 2.0 * alpha * err;
            }
        }
    }
}

void sd(unsigned long long tl){
    cerr << "alpha " << alpha << endl;
    unsigned long long strt = tim(), now = tim();
    int t = 0;
    for (;;){
        ++t;
        next_step();
        if ((t & 0b0) == 0){
            now = tim();
            if (now - strt > tl)
                break;
            cerr << "\r " << t << " " << (int)((double)(now - strt) / tl * 1000);
            scoring_mae();
        }
    }
    cerr << t;
    scoring_mae();
    cerr << endl;
}

void init(){
    int i, j;
    for (i = 0; i < n_patterns; ++i){
        for (j = 0; j < eval_sizes[i]; ++j)
            rev_idxes[i][j] = calc_rev_idx(i, pattern_sizes[i], j);
    }
}

int main(int argc, char *argv[]){
    sa_phase = atoi(argv[1]);
    cerr << sa_phase << endl;
    int i, j;

    unsigned long long hour = 0;
    unsigned long long minute = 10;
    unsigned long long second = 0;
    minute += hour * 60;
    second += minute * 60;

    board_init();
    init();
    //initialize_param();
    input_param_onephase((string)(argv[2]));
    input_test_data(0);

    sd(second * 1000);

    output_param_onephase();

    return 0;
}