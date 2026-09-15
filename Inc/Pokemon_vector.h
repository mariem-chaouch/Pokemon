#include "Pokemon.h"
#include <vector>
using std::vector;

class Pokemon_Vector {
protected:
   vector<Pokemon*> pokemons;
    public:
    Pokemon_Vector();
    virtual ~Pokemon_Vector();

    virtual void display() const = 0;
};