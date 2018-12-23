#include "GlobalPlacer.h"
#include "ExampleFunction.h"
#include "NumericalOptimizer.h"

GlobalPlacer::GlobalPlacer(Placement &placement)
	:_placement(placement)
{

}

// Randomly place modules implemented by TA
void GlobalPlacer::randomPlace(vector<double> &coordinate){
	double w = _placement.boundryRight() - _placement.boundryLeft();
	double h = _placement.boundryTop() - _placement.boundryBottom();
	for (size_t i = 0; i < _placement.numModules(); ++i ){
		double wx = _placement.module(i).width(), 
			   hx = _placement.module(i).height();
		coordinate[ 2 * i ] = (int) rand() % (int)(w - wx) + _placement.boundryLeft();
		coordinate[2 * i+1] = (int) rand() % (int)(h - hx) + _placement.boundryBottom();
		_placement.module(i).setPosition( coordinate[2*i], coordinate[2*i+1] );
	}
}


void GlobalPlacer::place()
{
	ExampleFunction ef(_placement); // require to define the object function and gradient function
    NumericalOptimizer no(ef);

    vector<double> coordinate(_placement.numModules() * 2); // solution vector, size: num_blocks*2 
                                                            // each 2 variables represent the X and Y dimensions of a block
	srand(0);
	randomPlace(coordinate);

    no.setX(coordinate);    // set solution
    no.setNumIteration(200000000/this->_placement.numModules()); // user-specified parameter
    no.setStepSizeBound(13); // user-specified parameter
    for(int j=0; j<10; ++j){

        no.solve(); // Conjugate Gradient solver
        for(size_t i=0; i<_placement.numModules(); ++i){
            double x = no.x(2 * i);
            double y = no.x(2 * i + 1);
            if( !_placement.module(i).isFixed() ){
                double width  = _placement.module(i).width();
                double height = _placement.module(i).height();
                if(x + width  > _placement.boundryRight() ) x = _placement.boundryRight() - width;
                if(x - width  < _placement.boundryLeft()  ) x = _placement.boundryLeft();
                if(y + height > _placement.boundryTop()   ) y = _placement.boundryTop() - height;
                if(y - height < _placement.boundryBottom()) y = _placement.boundryBottom();
            }else{
                x = _placement.module(i).x();
                y = _placement.module(i).y();
            }
            _placement.module(i).setPosition( x, y );
            coordinate[2*i] = x;
            coordinate[2*i+1] = y;
        }
        no.setNumIteration(1200000/this->_placement.numModules());             // user-specified parameter
        if( j % 2 ) no.setStepSizeBound( ceil( _placement.boundryRight() - _placement.boundryLeft() )   / 100 );  // user-specified parameter
        else        no.setStepSizeBound( ceil( _placement.boundryTop()   - _placement.boundryBottom() ) / 100 );  // user-specified parameter

        
        no.setX(coordinate);
        ef.addlambda();
    }
}


void GlobalPlacer::plotPlacementResult( const string outfilename, bool isPrompt )
{
    ofstream outfile( outfilename.c_str() , ios::out );
    outfile << " " << endl;
    outfile << "set title \"wirelength = " << _placement.computeHpwl() << "\"" << endl;
    outfile << "set size ratio 1" << endl;
    outfile << "set nokey" << endl << endl;
    outfile << "plot[:][:] '-' w l lt 3 lw 2, '-' w l lt 1" << endl << endl;
    outfile << "# bounding box" << endl;
    plotBoxPLT( outfile, _placement.boundryLeft(), _placement.boundryBottom(), _placement.boundryRight(), _placement.boundryTop() );
    outfile << "EOF" << endl;
    outfile << "# modules" << endl << "0.00, 0.00" << endl << endl;
    for( size_t i = 0; i < _placement.numModules(); ++i ){
        Module &module = _placement.module(i);
        plotBoxPLT( outfile, module.x(), module.y(), module.x() + module.width(), module.y() + module.height() );
    }
    outfile << "EOF" << endl;
    outfile << "pause -1 'Press any key to close.'" << endl;
    outfile.close();

    if( isPrompt ){
        char cmd[ 200 ];
        sprintf( cmd, "gnuplot %s", outfilename.c_str() );
        if( !system( cmd ) ) { cout << "Fail to execute: \"" << cmd << "\"." << endl; }
    }
}

void GlobalPlacer::plotBoxPLT( ofstream& stream, double x1, double y1, double x2, double y2 )
{
    stream << x1 << ", " << y1 << endl << x2 << ", " << y1 << endl
           << x2 << ", " << y2 << endl << x1 << ", " << y2 << endl
           << x1 << ", " << y1 << endl << endl;
}
