#define _GLIBCXX_USE_CXX11_ABI 0
#ifndef GLOBALPLACER_H
#define GLOBALPLACER_H

#include "Placement.h"
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <cmath>

#define BINROW 15
#define BINCOL 15
#define OUTERITER 3



class GlobalPlacer 
{
public:
    GlobalPlacer(Placement &placement):_placement(placement), _binCol(BINCOL), _binRow(BINROW), _numIter(OUTERITER) {
		_fWLReviseGlobal = 1;
		_gWLReviseGlobal = 1;
		_fDenseReviseGlobal = 1;
		_gDenseReviseGlobal = 1;
		_alpha = 0.001; ////////////////////////// need to notice this ////////////
		_binNum = _binRow * _binCol;
		_binW = (double)(_placement.boundryRight() - _placement.boundryLeft()) / (double)_binCol;
		_binH = (double)(_placement.boundryTop() - _placement.boundryBottom()) / (double)_binRow;
		_W = placement.boundryRight() - placement.boundryLeft();
		_H = placement.boundryTop() - placement.boundryBottom();
		_gamma = (_W+_H)/1200.0;
		
	};
	void place();
    void plotPlacementResult( const string outfilename, bool isPrompt = false );
	double rand_legal_pos_x(double x, double w);
	double rand_legal_pos_y(double y, double h);
	double initplace1(vector<double> &x);
	double initplace2(vector<double> &x);
	double calinitDensityFG(const vector<double> &x, double& density_grad);
	double calinitWLFG(const vector<double> &x, double& wire_grad);
	double sigmoid(double x){return 1/(1+fastExp2(-_alpha*x));}
	float fastExp2(register float x);
	

private:
    Placement& _placement;
    void plotBoxPLT( ofstream& stream, double x1, double y1, double x2, double y2 );
	int _numIter;
	int _stepBound;
	int _binRow;
	int _binCol;
	int _binNum;
	double _binW;
	double _binH;
	double _alpha;
	double _currlambda;
	double _initWL;
	double _gamma;
	double _W;
	double _H;
	double _fWLReviseGlobal;
	double _gWLReviseGlobal;
	double _fDenseReviseGlobal;
	double _gDenseReviseGlobal;
	
};

#endif // GLOBALPLACER_H
