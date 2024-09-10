// lbr.h
#pragma once

#include <vector>
#include <array>
#include "HandEvaluator.h"

namespace omp {

class LBR {
public:
    LBR();
    double wprollout(const std::vector<int>& player_hand,
                     const std::vector<double>& opp_range,
                     const std::vector<int>& board,
                     const std::vector<int>& deck_cards);

private:
    HandEvaluator evaluator;
    std::array<std::array<int, 2>, 1326> card_lookup_table;
    
    void construct_card_lookup();
    std::vector<std::array<int, 2>> construct_hands_lookup(const std::vector<double>& opp_range);
};

} // namespace omp