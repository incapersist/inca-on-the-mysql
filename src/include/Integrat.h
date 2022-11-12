#ifndef IntegratH
#define IntegratH

#include "CParSet.h"
#include "CBaseSeries.h"
#include <stdio.h>

void dascru(void (*f)(std::vector<double>&, std::vector<double>&, std::vector<double>&, std::vector<double>&,
                        CCellPar&, CSubPar&, CReachPar&,
            std::vector<double>&, bool&, double), int, int, double, int,
            std::vector<double>&, std::vector<double>&, std::vector<double>&, std::vector<double>&,
            CCellPar&, CSubPar&, CReachPar&, 
            std::vector<double>&, bool&, double);

#endif
