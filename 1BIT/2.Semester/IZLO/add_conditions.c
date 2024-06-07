#include <stddef.h>
#include "cnf.h"

//
// LOGIN: <XNOVAKF00>
//

// Tato funkce by mela do formule pridat klauzule predstavujici podminku 1)
// Křižovatky jsou reprezentovany cisly 0, 1, ..., num_of_crossroads-1
// Cislo num_of_streets predstavuje pocet ulic a proto i pocet kroku cesty
// Pole streets ma velikost num_of_streets a obsahuje vsechny existujuci ulice
//    - pro 0 <= i < num_of_streets predstavuje streets[i] jednu existujici
//      ulici od krizovatky streets[i].crossroad_from ke krizovatce streets[i].crossroad_to
void at_least_one_valid_street_for_each_step(CNF* formula, unsigned num_of_crossroads, unsigned num_of_streets, const Street* streets) {
    assert(formula != NULL);
    assert(num_of_crossroads > 0);
    assert(num_of_streets > 0);
    assert(streets != NULL);

    for(unsigned step = 0; step < num_of_streets; ++step)
    {
        Clause *c = create_new_clause(formula);
        
        for(unsigned street = 0; street < num_of_streets; ++street)
        {
            add_literal_to_clause(c, true, step, streets[street].crossroad_from, streets[street].crossroad_to);
        }
    }
}

// Tato funkce by mela do formule pridat klauzule predstavujici podminku 2)
// Křižovatky jsou reprezentovany cisly 0, 1, ..., num_of_crossroads-1
// Cislo num_of_streets predstavuje pocet ulic a proto i pocet kroku cesty
void at_most_one_street_for_each_step(CNF* formula, unsigned num_of_crossroads, unsigned num_of_streets) {
    assert(formula != NULL);
    assert(num_of_crossroads > 0);
    assert(num_of_streets > 0);

    for(unsigned step = 0; step < num_of_streets; ++step)
    {
        for(unsigned crossroad_from_a = 0; crossroad_from_a < num_of_crossroads; crossroad_from_a++)
        {
            for(unsigned crossroad_to_a = 0; crossroad_to_a < num_of_crossroads; ++crossroad_to_a)
            {
                for(unsigned crossroad_from_b = 0; crossroad_from_b < num_of_crossroads; crossroad_from_b++)
                {
                    for(unsigned crossroad_to_b = 0; crossroad_to_b < num_of_crossroads; ++crossroad_to_b)
                    {
                        if(!((crossroad_from_a == crossroad_from_b) && (crossroad_to_a == crossroad_to_b)))
                        {
                            Clause *c = create_new_clause(formula);
                            add_literal_to_clause(c, false, step, crossroad_from_a, crossroad_to_a);
                            add_literal_to_clause(c, false, step, crossroad_from_b, crossroad_to_b);
                        }                        
                    }
                }
            }
        }
    }
}

// Tato funkce by mela do formule pridat klauzule predstavujici podminku 3)
// Křižovatky jsou reprezentovany cisly 0, 1, ..., num_of_crossroads-1
// Cislo num_of_streets predstavuje pocet ulic a proto i pocet kroku cesty
void streets_connected(CNF* formula, unsigned num_of_crossroads, unsigned num_of_streets) {
    assert(formula != NULL);
    assert(num_of_crossroads > 0);
    assert(num_of_streets > 0);

    for(unsigned step = 0; step < num_of_streets - 1; ++step)
    {
        for(unsigned prev_crossroad_from = 0; prev_crossroad_from < num_of_crossroads; ++prev_crossroad_from)
        {
            for(unsigned prev_crossroad_to = 0; prev_crossroad_to < num_of_crossroads; ++prev_crossroad_to)
            {
                Clause *c = create_new_clause(formula);
                add_literal_to_clause(c, false, step, prev_crossroad_from, prev_crossroad_to);
                for(unsigned next_crossroad_from = 0; next_crossroad_from < num_of_crossroads; ++next_crossroad_from)
                {
                    for(unsigned next_crossroad_to = 0; next_crossroad_to < num_of_crossroads; ++next_crossroad_to)
                    {
                        if(prev_crossroad_to == next_crossroad_from)
                        {
                            add_literal_to_clause(c, true, step+1, next_crossroad_from, next_crossroad_to);
                        }
                    }
                }
            }
        }
    }
}

// Tato funkce by mela do formule pridat klauzule predstavujici podminku 4)
// Křižovatky jsou reprezentovany cisly 0, 1, ..., num_of_crossroads-1
// Cislo num_of_streets predstavuje pocet ulic a proto i pocet kroku cesty
void streets_do_not_repeat(CNF* formula, unsigned num_of_crossroads, unsigned num_of_streets) {
    assert(formula != NULL);
    assert(num_of_crossroads > 0);
    assert(num_of_streets > 0);
    
    for (unsigned i = 0; i < num_of_streets; ++i) {
        // pro kazdy krok i
        for (unsigned j = 0; j < num_of_streets; ++j) {
            if (i != j) {
                // pro kazdy jiny krok j
                for (unsigned z = 0; z < num_of_crossroads; ++z) {
                    for (unsigned k = 0; k < num_of_crossroads; ++k) {
                        // pro kazdu dvojici krizovatek (z, k)
                        Clause* cl = create_new_clause(formula);
                        add_literal_to_clause(cl, false, i, z, k);
                        add_literal_to_clause(cl, false, j, z, k);
                    }
                }
            }
        }
    }
}
