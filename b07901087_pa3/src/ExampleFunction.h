#define _GLIBCXX_USE_CXX11_ABI 0
#ifndef EXAMPLEFUNCTION_H
#define EXAMPLEFUNCTION_H

#include "NumericalOptimizerInterface.h"
#include "Placement.h"
#include <vector>
#include <iostream>
#include <cmath>
#include <cstdlib>

#define ALPHA 0.001



class ExampleFunction : public NumericalOptimizerInterface
{
public:
    ExampleFunction(Placement& placement, int binCol, int binRow, double WLNorm ):
	_placement(placement),
	_alpha(ALPHA)
	{
		_fDenseRevise = 1;
		_gDenseRevise = 1;
		_fWLRevise = 1;
		_gWLRevise = 1;
		_fDensity = 0;
		_fWL = 0;
		_gDensity = 0;
		_gWL = 0;
		_WLNorm = WLNorm;
		_lambda = 0;
		_binRow = binRow;
		_binCol = binCol;
		_binNum = binRow * binCol;
		_moduleNum = placement.numModules();
		_W = placement.boundryRight() - placement.boundryLeft();
		_H = placement.boundryTop() - placement.boundryBottom();
		_gamma = (_W+_H)/1200.0;
		_binW = _W*1.0/_binCol; // x direction
		_binH = _H*1.0/_binRow; // y direction
		_A = _W*_H;
		// double tempA=0.0;
		// for(int i = 0; i < _moduleNum; ++i){
			// tempA += placement.module(i).area(); 
		// }
		
		_Mb = _moduleNum /_binNum; //need modification
		
		_module_densex_in_bin.resize(_moduleNum);
		_module_densey_in_bin.resize(_moduleNum);
		_binDensities.resize(_binNum);
		
	}
	
	

    void evaluateFG(const vector<double> &x, double &f, vector<double> &g);
    void evaluateF(const vector<double> &x, double &f);
    unsigned dimension();
	void setLambda(double x) { cout << "setLambda is called, new lambda is: "<< x << endl; _lambda = x; }
	void setAlpha(double x) { _alpha = x; }
	void initial(const vector<double> &x, double &f, vector<double> &g);
	void calWLFG(const vector<double> &x, double &f, vector<double> &g);
	void calWLF(const vector<double> &x, double &f);
	double sigmoid(double x);
	void calDensityFG(const vector<double> &x, double &f, vector<double> &g);
	void calDensityF(const vector<double> &x, double &f);
	void setWLNorm(double x) { _WLNorm = x; }
	void setMb(double x) { _Mb = x; }
	double getfWL() {return _fWL; }
	double getfDensity() {return _fDensity; }
	double getgWL() {return _gWL; }
	double getgDensity() {return _gDensity; }
	double getLambda() {return _lambda; }
	void setfDenseRevise(double x){ _fDenseRevise = x; }
	void setgDenseRevise(double x){ _gDenseRevise = x; }
	void setfWLRevise(double x){_fWLRevise = x;}
	void setgWLRevise(double x){_gWLRevise = x; }
	double getfDenseRevise(){ return _fDenseRevise; }
	double getgDenseRevise(){ return _gDenseRevise; }
	double getfWLRevise(){ return _fWLRevise; }
	double getgWLRevise(){ return _gWLRevise; }
	float fastExp3(register float x);
	//void   setNorm(double x) {
	
	
	
	
	
	
private:
	vector<double> _binDensities;
	vector<double> _module_densex_in_bin; // need init
	vector<double> _module_densey_in_bin;
	double 		_lambda;
	double 		_W;
	double 		_H;
	double 		_A;
	double 		_alpha;
	int	   		_moduleNum;
	int			_binRow;
	int 		_binCol;
	int			_binNum;
	double		_binW;
	double 		_binH;
	double		_gamma;
	double		_Mb;
	double      _WLNorm;
	double		_fDensity;
	double		_fWL;
	double		_gDensity;
	double		_gWL;
	double		_fDenseRevise;
	double 		_gDenseRevise;
	double		_fWLRevise;
	double		_gWLRevise;
	Placement& 	_placement;
	
	// need init
	
	

};
#endif // EXAMPLEFUNCTION_H
