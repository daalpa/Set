#include <array>
#include <algorithm>
#include <cassert>
#include <exception>
#include <iostream>
#include <iterator>
#include <random>
#include <string>
#include <vector>

using namespace std;

struct Card {
    Card(int _value = 0) { setValue(_value);};
    void setValue(int _value) {
        if(0 <= _value && _value <= NO_CARDS) {
            value = _value;
        }
        else {
            throw out_of_range("Invalid argument");
        }
    }
    
    const static int NO_PROPERTIES = 4;
    const static int NO_VALUES = 3;
    const static int NO_CARDS = 81;
    
    int getValue(int property) const {
        if(!(0 <= property && property < NO_PROPERTIES)) {
            throw out_of_range("Trying to access inexistent member.");
        }
        int rtn = value;
        while(property-->0) {
            rtn /= NO_VALUES;
        }
        return rtn %NO_VALUES;
    }
    bool operator==(const Card& rhs) const { return value == rhs.value; }
    int value;
};



using Deck = array<Card,Card::NO_CARDS>;
static const int NO_CARDS_IN_SET = 3;
using Set  = array<Card,NO_CARDS_IN_SET>;
static const int INITIAL_NO_CARDS_TABLE = 12;

void setDeck(Deck& deck) {
 int count = 0;
 for_each(deck.begin(),deck.end(), [&count](Card& card) {
     card.setValue(count++); });
}

bool propertyShared(const Set& set, int property_no) {
    return (set[0].getValue(property_no) == set[1].getValue(property_no)) &&
           (set[0].getValue(property_no) == set[2].getValue(property_no));
}

bool propertyDifferent(const Set& set, int property_no) {
    return (set[0].getValue(property_no) != set[1].getValue(property_no)) &&
           (set[0].getValue(property_no) != set[2].getValue(property_no)) &&
           (set[1].getValue(property_no) != set[2].getValue(property_no));
}

//An easier way, would be to sum the values of the properties
//if not a multiple of 3, return false
bool isSet(const Set& set) {
    for(int i = 0; i < Card::NO_PROPERTIES; i++) {
        if(!propertyShared(set,i) && !propertyDifferent(set,i)) {
            return false;
        }
    }
    return true;
}

void findSets(vector<Set> &sets, const vector<Card>& table) {
    sets.resize(0);
    for(size_t i = 0; i < table.size(); i++) {
        for(size_t j = i+1; j < table.size(); j++) {
            for(size_t k = j+1; k < table.size(); k++) {
                Set set = {{table[i],table[j],table[k]}};
                if(isSet(set)) {
                    sets.push_back(set);
                }
            }
        }
    }
}

pair<bool,Card> isThereUniqueValue(const vector<Card>& table) {
    array<array<int,Card::NO_VALUES>, Card::NO_PROPERTIES> count;
    
    //set to 0
    for(auto& row : count) {
        for(auto& entry : row) {
            entry = 0; } }
            
    //count properties
    for(const auto& card : table) { 
     for(int i = 0; i < Card::NO_PROPERTIES; i++) {
         count[i][card.getValue(i)]++;
     }
    }
    
    int iteration = 0;
    for(const auto& property : count) {
        auto unique_value = find(property.cbegin(), property.cend(), 1);
        if(unique_value != property.cend()) {
            int value = distance(property.cbegin(),unique_value);
            auto card_it = find_if(table.cbegin(),table.cend(), [&iteration,value](const Card& card) {
                return card.getValue(iteration) == value;});
            assert(card_it != table.cend());
            return make_pair(true,*card_it);
        }
        iteration++;
    }
    return make_pair(false, Card(0));
}

bool isCardOnSet(const Set& set, const Card& card) {
    return (card == set[0]) || (card == set[1]) || (card == set[2]);
}

void removeSetFromTable(vector<Card>& table, const Set &set, Deck::iterator next_card) {
    for(const auto& card : set) {
        auto card_to_remove = find(table.begin(),table.end(),card);
        if(card_to_remove == table.end()) throw invalid_argument("The card on the set, is not on the table");
        *card_to_remove = *next_card;
        ++next_card;
    }
}

void removeSetFromTableWithoutReplacement(vector<Card>& table, const Set &set) {
    auto it_end = table.end();
    for(const auto& card : set) {
        auto card_to_remove = find(table.begin(),it_end,card);
        if(card_to_remove == it_end) throw invalid_argument("The card on the set, is not on the table");
        std::swap(*card_to_remove,*(--it_end));
    }
    table.resize(table.size()-set.size());
}

static int no_unique = 0;
static int unique_part_set = 0;
static int unique_not_part_set = 0;
static int no_tables = 0;
static int no_set = 0;
static int max_table_size = 0;
static int table_size_15 = 0;
static int table_size_18 = 0;

void game() {
 Deck deck;
 setDeck(deck);
 shuffle(deck.begin(),deck.end(),mt19937(random_device{}()));
 vector<Card> table;
 for_each(deck.begin(),deck.begin()+INITIAL_NO_CARDS_TABLE, 
     [&table](Card card) { table.push_back(card);});
 vector<Set> sets;
 for(auto it = deck.begin()+INITIAL_NO_CARDS_TABLE;
     it != deck.end();
     it+= NO_CARDS_IN_SET) {
        no_tables++;
        auto unique = isThereUniqueValue(table);
        if(unique.first) no_unique++;
        findSets(sets,table);
        if(!sets.empty() && unique.first) { // is there a set?
            int index = 0, count = 0;
            for(const auto& set : sets) {
                if(isCardOnSet(set,unique.second)) {unique_part_set++; index = count;}
                else                               {unique_not_part_set++; }
                count ++;
            }
            if(table.size() == INITIAL_NO_CARDS_TABLE) {
                removeSetFromTable(table,sets[index],it);
            }
            else {
                removeSetFromTableWithoutReplacement(table,sets[index]);
            }
        }
        else if(!sets.empty()) {
            if(table.size() == INITIAL_NO_CARDS_TABLE) {
                removeSetFromTable(table,sets[0],it);
            }
            else {
                removeSetFromTableWithoutReplacement(table,sets[0]);
            }
        }
        else  {
            for(int i = 0; i < NO_CARDS_IN_SET; i++) {
                table.push_back(*(it+i));
            }
            no_set++;
        }
        max_table_size = max(int(table.size()),max_table_size);
        if(table.size() == 15) { table_size_15++;}
        if(table.size() == 18) { table_size_18++;}
 }
}

int main() {
    const int NO_GAMES = 100000;
    try{
        for(int i = 0; i < NO_GAMES; i++) {
            game();
        }
    } catch(exception& e) {
            cerr << e.what() << endl;
    }
    cout << "Played " << NO_GAMES << " games\n";
    cout << no_unique << " unique cards found\n";
    cout << unique_part_set << " times, they were part of a set.\n";
    cout << unique_not_part_set << " times, they were not part of a set\n";
    cout << "Probability of unique card being part of a set: " << unique_part_set/double(unique_part_set+unique_not_part_set) << "\n";
    cout << "Probability of unique appearing: " << no_unique/double(no_tables) << "\n";
    cout << "Probability of no set: " << no_set/double(no_tables) << "\n";
    cout << "Probability of 15 cards: " << table_size_15/double(no_tables) << "\n";
    cout << "Probability of 18 cards: " << table_size_18/double(no_tables) << "\n";
    cout << "Max table size: " << max_table_size << "\n";
    return 0;
}