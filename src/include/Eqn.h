#ifndef EqnH
#define EqnH

#include "CParSet.h"
#include <stdio.h>
#include <iostream>

void deqn(std::vector<double>& x0,
            std::vector<double>& xp,
            std::vector<double>& Inputs,
            std::vector<double>& Outputs,
            CCellPar& pars,
            CSubPar& subPar,
            CReachPar& reachPar,
            std::vector<double>& Conc,
            bool& useNewHydrology,
            double Msoil);

void reqn(std::vector<double>& x0,
            std::vector<double>& xp,
            std::vector<double>& Inputs,
            std::vector<double>& Outputs,
            CCellPar& pars,
            CSubPar& subPar,
            CReachPar& reachPar,
            std::vector<double>& Conc,
            bool& Abstracted,
            double Msoil);

class EqnException
{
    int code, timestep, reach;
    double flow, volume, tc;
    double input, value;
    std::string type, eqn;

public:
    EqnException(const int i_code,
                    const int i_reach,
                    const std::string& i_type,
                    const std::string& i_eqn,
                    const double i_flow,
                    const double i_vol,
                    const double i_tc,
                    const double i_input = -999.0,
                    const double i_value = -999.0)
                                        :   code(i_code),
                                            reach(i_reach),
											flow(i_flow),
											volume(i_vol),
											tc(i_tc),
											input(i_input),
											value(i_value),
											type(i_type),
                                            eqn(i_eqn)
                                            { }

    const int get_code() const { return code; }
    std::ostream& print(std::ostream& os) const;
    void TimeStep(const int ts) { timestep = ts; }
};

inline std::ostream& operator<<(std::ostream& os, const EqnException& e)
{
    return e.print(os);
}

#endif