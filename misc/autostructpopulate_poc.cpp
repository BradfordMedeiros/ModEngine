#include <iostream>
#include <cstddef>
#include <vector>
#include <map>

struct Amount {
    double amount;

    char   letter;
};

enum FillerType { FILLER_CHAR, FILLER_DOUBLE, FILLER_SHORT };
struct Filler {
    size_t offset;
    FillerType type;
    std::string lookupName;
};  




int main(){
    std::vector<Filler> fillers;
    fillers.push_back(Filler{
        .offset = offsetof(Amount, letter),
        .type = FILLER_CHAR,
        .lookupName = "category",
    });
    fillers.push_back(Filler{
        .offset = offsetof(Amount, amount),
        .type = FILLER_DOUBLE,
        .lookupName = "price",
    });


    Amount amount1 {
            .amount = 234.4,

        .letter = 'A',
    };
    

    std::map<char, double> values = { { 'A', 20 }, { 'B', 123.3 } };

    for (auto &filler : fillers){
        if (filler.lookupName == "price"){
            if (filler.type == FILLER_DOUBLE){
                char* address = ((char*) (&amount1)) + filler.offset;
                double* amount = (double*) address;
                *amount = values.at('A');
                
                std::cout << amount1.amount << std::endl;
                std::cout << amount1.letter << std::endl;
            }
        }
    }
}
