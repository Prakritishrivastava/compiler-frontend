// Created by omar_swidan on 23/03/20.
//

#ifndef LEXGEN_NFA_H
#define LEXGEN_NFA_H

using namespace std;

#include <iostream>
#include<stack>
#include <string>
#include <unordered_set>
#include "nfa_state.h"

class NFA {
    typedef enum state_type
    {start_state, normal_state,acceptance_state}state_type;
public:

    NFA();
    NFAState regex_to_nfa();

private:
    NFAState concat(NFAState first_nfa_state,NFAState second_nfa_state);
    NFAState or_combiner(NFAState first_nfa_state,NFAState second_nfa_state);
    NFAState kleene(NFAState nfa_state);
    NFAState plus(NFAState nfa_state);
    NFAState construct_one_transition_state(string transition);
    string infix_to_postfix(string regex,vector<string>input_table);
    NFAState postfix_to_NFA(string postfix,vector<string>input_table);
    int precedence_decision(string operator_symbol);

    bool is_operator(char character);
    bool is_operand(char character);

};


#endif //LEXGEN_NFA_H