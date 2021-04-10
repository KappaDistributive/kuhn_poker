#include <iostream>
#include <vector>

namespace {
enum class Action {
  fold,
  check,
  call,
  bet,
};

enum class Card : int {
  jack,
  queen,
  king,
};

int
calculate_pot(const std::vector<Action>& actions) {
  int pot{2};  // antees
  for (auto action : actions) {
    if (action == Action::bet || action == Action::call) {
      pot++;
    }
  }

  return pot;
}

bool alice_won(const std::vector<Action>& actions,
               const Card& alice_card,
               const Card& bob_card) {

               }

int
play_round(const Card& alice_card,
           const Card& bob_card,
           Action(*alice_strategy)
             (const std::vector<Action>& moves, const Card& card),
           Action(*bob_strategy)
             (const std::vector<Action>& moves, const Card& card)) {
  std::vector<Action> actions;

  for (size_t move{0}; move < 4; move++) {
    Action action;
    if (move % 2 == 0) {
      action = alice_strategy(actions, alice_card);
    } else {
      action = bob_strategy(actions, bob_card);
    }
    actions.push_back(action);
    if (action == Action::fold) {
      break;
    }
  }
  return 0;
}
// TODO(stefan.mesken): define game
// TODO(stefan.mesken): logging
// TODO(stefan.mesken): simulate EV

namespace strategy {
// TODO(stefan.mesken): define strategy
/*
A strategy is a function
(bets, position?, card) -> Action
*/

// Action naive(const std::vector<int>& bets, )
};  // namespace strategy

};  // namespace


int main() {
  std::cout << (Card::queen < Card::king) << std::endl;
}
