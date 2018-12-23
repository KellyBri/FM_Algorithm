#ifndef EXAMPLEFUNCTION_H
#define EXAMPLEFUNCTION_H

#include <cmath>
#include <cstring>
#include "Placement.h"
#include "NumericalOptimizerInterface.h"

class ExampleFunction : public NumericalOptimizerInterface
{
public:
    ExampleFunction(Placement &);

    void evaluateFG(const vector<double> &x, double &f, vector<double> &g);
    void evaluateF(const vector<double> &x, double &f);
    unsigned dimension();
    void addlambda();
    
private:
    Placement &placement;
    unsigned dim;
    double gamma, lambda;
    double *ex, *negex, *ey, *negey;
    int binNum;
    double binHeight, binWidth;
    double *density_gx, *density_gy, *binDensity, *Mb;
    double maxDensity;
};
#endif // EXAMPLEFUNCTION_H
