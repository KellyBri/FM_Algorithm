#ifndef EXAMPLEFUNCTION_H
#define EXAMPLEFUNCTION_H

#include <cmath>
#include "Placement.h"
#include "NumericalOptimizerInterface.h"

class ExampleFunction : public NumericalOptimizerInterface
{
public:
    ExampleFunction(Placement &);

    void evaluateFG(const vector<double> &x, double &f, vector<double> &g);
    void evaluateF(const vector<double> &x, double &f);
    unsigned dimension();
    
private:
    Placement &placement;
    unsigned dim;
    double gamma;
    double *ex, *negex, *ey, *negey; 
    
};
#endif // EXAMPLEFUNCTION_H
