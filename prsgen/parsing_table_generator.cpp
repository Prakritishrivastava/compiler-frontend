/**
    Compiler Phase 2: PRSGEN
    parsing_table_generator.cpp
    Purpose: Generates and maintains the logic of creating the parsing table used by the parser. That is composed of three main components: Computing first and follow sets and construction of the table.

    @author(s) Omar Swidan, Mostafa Yousry
    @version 1.0
    @date 21/4/2020
*/

#include "parsing_table_generator.h"
#include "grammar_parser.h"
#include <utility>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <numeric>
using std::vector;
using std::string;
using std::unordered_set;
using std::make_pair;
using std::cout;
using std::endl;
using std::unordered_map;
const string epsilon = EPSILON_EXPRESSION;

/******** Public Methods ********/

ParsingTableGenerator::ParsingTableGenerator(std::unordered_set<std::string>& terminals_,
        std::vector<NonTerminal>& non_terminals_)
        :terminals_(terminals_), non_terminals_(non_terminals_)
{
    for (auto&& non_terminal:non_terminals_) {
        name_non_terminal_.insert(make_pair(non_terminal.getName_(), &non_terminal));
    }
    //for each production element if it's a non terminal then add it to the map with the value to be the set of its parents
    for (auto&& non_terminal:non_terminals_) {
        for (auto&& production  : non_terminal.getProduction_rules_()) {
            for (auto&& production_element: production) {
                //If the element is a non terminal
                if (name_non_terminal_.count(production_element)) {
                    //Check if this non terminal production element is added to the map before
                    if (non_terminal_non_terminal_parents.count(production_element)) {
                        non_terminal_non_terminal_parents[production_element].insert(
                                non_terminal.getName_());
                    }
                    else {
                        non_terminal_non_terminal_parents.
                                insert(
                                make_pair(production_element, unordered_set<string>{non_terminal.getName_()}));
                    }
                }
            }
        }
    }
}

/**
 * It's assumed in this function that the non terminals vector member variable is ordered so that the last non terminal
 * corresponds to the last production in the grammar and that its first set contains no other non terminal
 */
void ParsingTableGenerator::computeFirst()
{
    for (int i = static_cast<int>(non_terminals_.size()-1); i>=0; i--) {
        //Get all production rules of the current non terminal
        const vector<vector<string>>& productions = non_terminals_[i].getProduction_rules_();
        unordered_set<string> first;
        for (auto&& production: productions) {
            int j = 0;
            //loop on the elements of each production which can be a terminal, a non terminal or an epsilon
            for (auto&& production_element: production) {
                if (production_element==epsilon) {
                    first.insert(epsilon);
                    break;
                }
                //If this condition is true then we should break the loop as if a terminal is found then no other non terminal or terminal
                //after it will be added to the first set
                if (terminals_.count(production_element)) {
                    first.insert(production_element);
                    break;
                }
                //if epsilon is in all the non terminals in the production then add epsilon the first set
                if (j==production.size()-1 && name_non_terminal_[production_element]->getFirst_().count(epsilon))
                    first.insert(epsilon);
                //Has to be copied to  variable as the getter returns a const
                unordered_set<string> production_non_terminal_first = name_non_terminal_[production_element]->getFirst_();
                //Add to the first the first of Yi
                first.merge(production_non_terminal_first);
                //Break if Yi doesn't  derive epsilon
                if (!name_non_terminal_[production_element]->getFirst_().count(epsilon)) break;
                j++;
            }
        }
        //add first to non terminal
        non_terminals_[i].setFirst_(first);
    }
}

void ParsingTableGenerator::computeFollow()
{
    for (int i = 0; i<non_terminals_.size(); ++i) {
        unordered_set<string> follow;
        if (i==0) follow.insert("$");
        //Get a vector of the parents of this non terminal to iterate on the productions that contains the current non terminal
        const unordered_set<string>& non_terminal_parents = non_terminal_non_terminal_parents[non_terminals_[i].getName_()];
        //Loop on the parents of the current non terminal
        for (auto&& non_terminal_parent_name : non_terminal_parents) {
            NonTerminal* non_terminal_parent = name_non_terminal_[non_terminal_parent_name];
            //Loop on the productions of this parent to get compute the follow set
            for (auto&& production: non_terminal_parent->getProduction_rules_()) {
                fill_follow_from_production(follow, production, non_terminals_[i],
                        *non_terminal_parent);
            }
        }
        non_terminals_[i].setFollow_(follow);
    }
}

/**
 * This function assumes that the grammar isn't ambiguous
 */
void ParsingTableGenerator::constructParsingTable()
{
    for (auto&& non_terminal : non_terminals_) {
        unordered_map<string, int> current_parsing_table_entry;
        const unordered_set<string>& current_non_terminal_follow = name_non_terminal_[non_terminal.getName_()]->getFollow_();
        //For each terminal in the follow set of the parent non terminal add an entry in the parsing table with synch in it
        //to be used in error handling. Take Care that the value in the entry is overriden if the first production element is an epsilon
        //or a non terminal that derives to an epsilon
        fill_parsing_table_entry_with_keys_and_value(current_parsing_table_entry,
                current_non_terminal_follow,
                synch);
        //j is the index of the production in the non terminal class
        int j = 0;
        for (auto&& production : non_terminal.getProduction_rules_()) {
            //If the first element in the production is a terminal then it's first set contains non more than this terminal
            if (terminals_.count(production[0])) {
                current_parsing_table_entry[production[0]] = j++;
                continue;
            }
            //If the current element in the production is an epsilon or if the first of the non terminal derives to epsilon
            if (production[0]==epsilon) {
                //Add an entry for for each terminal in the current non terminal's (the parent of the production) follow set with the production
                fill_parsing_table_entry_with_keys_and_value(current_parsing_table_entry,
                        current_non_terminal_follow,
                        j);
                j++;
                continue;
            }
            const unordered_set<string>& production_element_non_terminal_first = name_non_terminal_[production[0]]->getFirst_();
            //if the current production element is a non terminal then for each terminal in its first set add an entry in the
            //current parent non terminal's parsing table entry
            fill_parsing_table_entry_with_keys_and_value(current_parsing_table_entry,
                    production_element_non_terminal_first,
                    j);
            //Remove the epsilon if it was added due to its presence in the first set
            current_parsing_table_entry.erase(epsilon);
            if (production_element_non_terminal_first.count(epsilon)) {
                //Add an entry for for each terminal in the current non terminal's (the parent of the production) follow set with the production
                fill_parsing_table_entry_with_keys_and_value(current_parsing_table_entry,
                        current_non_terminal_follow,
                        j);
            }
            j++;
        }
        non_terminal.setParse_table_entry_(current_parsing_table_entry);
    }
}

void ParsingTableGenerator::writeParsingTable(const std::string& out_file_relative_path)
{
    std::ofstream transition_table_file;
    transition_table_file.open(out_file_relative_path);
    transition_table_file << "$";
    for (auto&& terminal: terminals_) {
        transition_table_file << terminal << "$";
    }
    transition_table_file << "\n";
    for (auto&& non_terminal  : non_terminals_) {
        transition_table_file << non_terminal.getName_() << "$";
        for (auto&& terminal: terminals_) {
            int production_rule_index = non_terminal.GetProductionRuleIndex(terminal);
            if (production_rule_index==synch) {
                transition_table_file << "synch" << "$";
            }
            else if (production_rule_index!=empty) {
                std::vector<std::string> production_rule = non_terminal.GetProductionRule(production_rule_index);
                std::string production_rule_string = std::accumulate(production_rule.begin(), production_rule.end(),
                        std::string(""),
                        [](std::string& ss, std::string& s) {
                          return ss.empty() ? s : ss+" "+s;
                        });
                transition_table_file << production_rule_string << "$";
            }
            else {
                transition_table_file << " " << "$";
            }
        }
        transition_table_file << endl;
    }
}

/******** Private Methods ********/

void ParsingTableGenerator::fill_follow_from_production(std::unordered_set<std::string>& follow,
        const vector<string>& production,
        const NonTerminal& current, const NonTerminal& parent)
{
    //Loop on the elements of this production
    for (int j = 0; j<production.size(); ++j) {
        //This check is done to locate the current non terminal in the current production
        //Take care that not all of the productions of this parent have to contain this non terminal as only the parent is stored
        //so it's an at least once relationship
        if (production[j]==current.getName_()) {
            //Check if the non terminal is the last element in the production
            if (j==production.size()-1) {
                unordered_set<string> parent_follow = parent.getFollow_();
                follow.merge(parent_follow);
                //Break and move to the next production
                break;
            }
            if (terminals_.count(production[j+1])) {
                //Insert the terminal as is as in the follow set as it first is the same as its name
                follow.insert(production[j+1]);
                //Continue iterating in case the current non terminal is repeated in the production again
                j++;
                continue;
            }
            unordered_set<string> right_sibling_first = name_non_terminal_[production[j+1]]->getFirst_();
            //Check if the right sibling is the last element in the production and it contains an epsilon in its first set
            if (j+1==production.size()-1 && right_sibling_first.count(epsilon)) {
                unordered_set<string> parent_follow = parent.getFollow_();
                follow.merge(parent_follow);
            }
            right_sibling_first.erase(epsilon);
            //Add the right sibling's first to the current non terminal's follow set
            follow.merge(right_sibling_first);
        }
    }
}

void ParsingTableGenerator::fill_parsing_table_entry_with_keys_and_value(
        std::unordered_map<std::string, int>& parsing_table_entry, const std::unordered_set<std::string>& keys,
        const int value)
{
    for (auto&& key : keys) {
        parsing_table_entry[key] = value;
    }

}
