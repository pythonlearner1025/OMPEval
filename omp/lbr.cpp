// lbr.cpp
#include "lbr.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <bitset>  


namespace omp {

LBR::LBR() {
    construct_card_lookup();
}

void LBR::construct_card_lookup() {
    int idx = 0;
    for (int i = 0; i < 51; ++i) {
        for (int j = i + 1; j < 52; ++j) {
            card_lookup_table[idx++] = {i, j};
        }
    }
}

std::vector<std::array<int, 2>> LBR::construct_hands_lookup(const std::vector<double>& opp_range) {
    if (opp_range.size() != 1326) {
        throw std::runtime_error("Opponent range size must be 1326");
    }
    std::vector<std::array<int, 2>> hands;
    hands.reserve(opp_range.size());
    for (size_t i = 0; i < opp_range.size(); ++i) {
        hands.push_back(card_lookup_table[i]);
    }
    return hands;
}

double LBR::wprollout(const std::vector<int>& player_hand,
                      const std::vector<double>& opp_range,
                      const std::vector<int>& board,
                      const std::vector<int>& deck_cards) {
    if (player_hand.empty() || board.size() > 5 || deck_cards.empty()) {
        throw std::runtime_error("Invalid input to wprollout");
    }

    auto hands_lookup_table = construct_hands_lookup(opp_range);

    if (hands_lookup_table.size() != opp_range.size()) {
        throw std::runtime_error("Invalid hands lookup table size");
    }
    
    std::vector<int> remaining_cards;
    std::set_difference(deck_cards.begin(), deck_cards.end(),
                        board.begin(), board.end(),
                        std::back_inserter(remaining_cards));
    
    int cards_to_deal = 5 - board.size();
    if (cards_to_deal < 0 || cards_to_deal > remaining_cards.size()) {
        throw std::runtime_error("Invalid number of cards to deal");
    }

    std::vector<bool> v(remaining_cards.size());
    std::fill(v.end() - cards_to_deal, v.end(), true);

    // precompute initial hand
    Hand initial_player_hand = Hand::empty();
    for (int card : player_hand) initial_player_hand += Hand(card);

    // Preallocate memory for new_board
    std::vector<int> new_board;
    new_board.reserve(5);

    // Use bitset for faster card lookup
    std::bitset<52> board_bitset;
    for (int card : board) board_bitset.set(card);
    
    double total_won = 0.0;
    int total_boards = 0;
    
    do {
        new_board = board;

        for (size_t i = 0; i < v.size(); ++i) {
            if (v[i]) {
                new_board.push_back(remaining_cards[i]);
                board_bitset.set(remaining_cards[i]);
            }
        } 
        
        Hand my_hand_board = initial_player_hand;
        Hand opp_hand_board = Hand::empty();
        
        // Initialize both hands with the board cards
        for (int card : new_board) {
            my_hand_board += Hand(card);
            opp_hand_board += Hand(card);
        }

        int my_strength = evaluator.evaluate(my_hand_board);
        
        double won = 0.0;
        for (size_t j = 0; j < opp_range.size(); ++j) {
            double prob = opp_range[j];
            const auto& opp_hand = hands_lookup_table[j];
            
            if (prob > 0 && 
                !board_bitset.test(opp_hand[0]) && !board_bitset.test(opp_hand[1])) {

                Hand current_opp_hand = opp_hand_board;

                // Add opponent's cards
                for (int card : opp_hand) current_opp_hand += Hand(card);
                
                int opp_strength = evaluator.evaluate(current_opp_hand);
                if (my_strength > opp_strength) won += prob;
            }
        }
        total_won += won;
        ++total_boards;

        // Reset board_bitset for next iteration
        for (size_t i = board.size(); i < new_board.size(); ++i) {
            board_bitset.reset(new_board[i]);
        }

    } while (std::next_permutation(v.begin(), v.end()));
    
    if (total_boards == 0) {
        throw std::runtime_error("No valid boards evaluated");
    }

    return total_won / total_boards;
}

} // namespace omp