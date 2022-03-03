#include "ExampleFunction.h"
#include <stdint.h>
float ExampleFunction::fastExp3(register float x)  // cubic spline approximation
{
    union { float f; int32_t i; } reinterpreter;

    reinterpreter.i = (int32_t)(12102203.0f*x) + 127*(1 << 23);
    int32_t m = (reinterpreter.i >> 7) & 0xFFFF;  // copy mantissa
    // empirical values for small maximum relative error (8.34e-5):
    reinterpreter.i +=
         ((((((((1277*m) >> 14) + 14825)*m) >> 14) - 79749)*m) >> 11) - 626;
    return reinterpreter.f;
}

void ExampleFunction::calWLFG(const vector<double> &x, double &f, vector<double> &g){ // use LSE function
	// cout << "====================== cal WLFG =========================" << endl;
	f = 0;
	for(int i = 0; i < _placement.numModules()*2; ++i){ // 
		g[i] = 0;
	}
	// for all net
	// for each summation part of f
	double part1 = 0;
	double part2 = 0;
	double part3 = 0;
	double part4 = 0;
	for(int i = 0; i <  _placement.numNets(); ++i){
		// read all module position(use pin or center?) of a net, do summation for four part, 
		Net thisnet = _placement.net(i);
		
		for(int j = 0; j <  thisnet.numPins(); ++j){
			int midx = thisnet.pin(j).moduleId();
			part1 += exp(x[2*midx] / _gamma);
			part2 += exp(-1.0*x[2*midx] / _gamma);
			part3 += exp(x[2*midx+1] / _gamma);
			part4 += exp(-1.0*x[2*midx+1] / _gamma);
		}
		// add this sum to final answer
		f += log(part1) + log(part2) + log(part3) + log(part4);
		
		// compute g(gradient)
		for(int j = 0; j <  thisnet.numPins(); ++j){
		// take log of each part and sum them all
			int midx = thisnet.pin(j).moduleId();
			g[midx*2] += exp(x[2*midx] / _gamma) / part1 ;
			g[midx*2] += (-1.0) * exp(-1.0*x[2*midx] / _gamma) / part2 ;
			g[midx*2+1] += exp(x[2*midx+1] / _gamma) / part3 ;
			g[midx*2+1] += (-1.0) * exp(-1.0*x[2*midx+1] / _gamma) / part4 ;
		}
		
		part1 = part2 = part3 = part4 =0;
	}
	if (_placement.numModules() * 2 != g.size()){ cerr << "g size is incorrect!" << endl; }
	for(int i = 0; i < g.size(); ++i){
		g[i] = g[i]* _gWLRevise / _WLNorm;
		_gWL += abs(g[i]);
	}
	f = _fWLRevise* f * _gamma / _WLNorm;
	_fWL = f;
	/////////// this is WL check part ////////////////////
	// cout << "cal WLFG is finished, " << "f: " << _fWL << endl;
	// cout << "g: " << _gWL << endl;
	// for(int i = 0; i < g.size(); ++i){
		// if (i % 3000 == 0) cout << "g[" << i << "] = " << g[i] << endl;
	// }
	
	// exit(0);
	// for (unsigned i = 0; i < 2*_moduleNum; i++) {
		// if (i % 3000 == 0) cout << "x[" << i << "] = " << x[i] << endl;
	// }
	// cout << "==================== cal WLFG is finished ========================" << endl;
}

void ExampleFunction::calWLF(const vector<double> &x, double &f){ // use LSE function
	f = 0;
	// cout << "=============== go into calWLF ===============" << endl;
	// for all net
	// for each summation part of f
	double part1;
	double part2;
	double part3;
	double part4;
	// tmp
	// double tmppart1;
	// double tmppart2;
	// double tmppart3;
	// double tmppart4;
	for(int i = 0; i <  _placement.numNets(); ++i){
		// read all module position(use pin or center?) of a net, do summation for four part, 
		Net thisnet = _placement.net(i);
		
		for(int j = 0; j <  thisnet.numPins(); ++j){
		// take log of each part and sum them all
			int midx = thisnet.pin(j).moduleId();
			// int x_idx = idx*2;
			// int y_idx = idx*2+1;
			// tmppart1 = exp(x[2*midx] / _gamma);
			// tmppart2 = exp(-1.0*x[2*midx] / _gamma);
			// tmppart3 = exp(x[2*midx+1] / _gamma);
			// tmppart4 = exp(-1.0*x[2*midx+1] / _gamma);
			part1 += exp(x[2*midx] / _gamma);
			part2 += exp(-1.0*x[2*midx] / _gamma);
			part3 += exp(x[2*midx+1] / _gamma);
			part4 += exp(-1.0*x[2*midx+1] / _gamma);
			// if (j % 1000==0)cout << "tmppart1: " << tmppart1 << endl;
			
		
		}
		// add this sum to final answer
		f += log(part1) + log(part2) + log(part3) + log(part4);
		part1 = part2 = part3 = part4 =0;
	}
	f = _fWLRevise* f * _gamma / _WLNorm;
	_fWL = f;
	// for (unsigned i = 0; i < 2*_moduleNum; i++) {
		// if (i % 3000 == 0) cout << "x[" << i << "] = " << x[i] << endl;
	// }
	// cout << "cal WLF is finished, " << "f: " << _fWL << endl;
	// cout << "================== cal WLF is finished =========================" << endl;
}

double ExampleFunction::sigmoid(double x){
	return 1/(1+fastExp3(-_alpha*x));
}

// double ExampleFunction::sigmoid_grad(double x){
	// return 1/(1+exp(-_alpha*x));
// }



void ExampleFunction::calDensityFG(const vector<double> &x, double &f, vector<double> &g){ // use sigmoid function
	for(int i = 0; i < _binNum; ++i) { _binDensities[i] = 0.0; }
	for( int i = 0; i < _moduleNum; ++i) { _module_densex_in_bin[i] = 0.0; }
	for( int i = 0; i < _moduleNum; ++i) { _module_densey_in_bin[i] = 0.0; }
	// for each module, cal overlap and get density
	// cout << "================= go into cal Dense FG ===================" << endl;
	double lboundary = _placement.boundryLeft();
	double bboundary = _placement.boundryBottom();
	for(int i = 0; i < _binCol; ++i){
		double gridL = _binW * i + lboundary;
		double gridR = gridL + _binW;
		for (int j = 0; j < _binRow; ++j){
			double gridB = _binH * j+  bboundary;
			double gridT = gridB + _binH;
			for(int k = 0; k < _moduleNum; ++k){ // f
				int mxidx = k * 2;
				int myidx = mxidx + 1;
				// Module thismodule = _placement.module(k); // use ref to catch or use Module to catch?
				double mleft = x[mxidx] - lboundary;
				double mlower = x[myidx] - bboundary;
				int lHidx = (int)(mleft / _binW);
				int bVidx = (int)(mlower / _binH);
			
				if (i > lHidx-3 && i < lHidx+4 && j > bVidx-3 && j < bVidx+4){ //the module fall in the bin
					_module_densex_in_bin[k] = sigmoid(x[mxidx]-gridL)*sigmoid(gridR-x[mxidx]);
					_module_densey_in_bin[k] = sigmoid(x[myidx]-gridB)*sigmoid(gridT-x[myidx]);
					_binDensities[i*_binCol + j] += _module_densex_in_bin[k] * _module_densey_in_bin[k];
				}
				else{
					_module_densex_in_bin[k] = 0;
					_module_densey_in_bin[k] = 0;
				}
			}
			if (_module_densex_in_bin.size() != _placement.numModules()){cout << "num is not correct" << endl; exit(0);}
			for(int k = 0; k < _moduleNum; ++k){ // cal gradient
				double mleft = x[2*k] - lboundary;
				double mlower = x[2*k+1] - bboundary;
				int lHidx = (int)(mleft / _binW);
				int bVidx = (int)(mlower / _binH);
				if (i > lHidx-3 && i < lHidx+4 && j > bVidx-3 && j < bVidx+4){ //the module fall in the bin
					if (_module_densex_in_bin[k] == 0 || _module_densey_in_bin[k] == 0){
						cerr << "density is strange!" << endl;
						exit(0);
					}
					double constant = _gDenseRevise* 2.0*(_binDensities[i*_binCol+j]-_Mb)*_lambda/(_Mb*_binNum); //normalize
					double tmp1 = constant* _alpha * ( fastExp3(-_alpha*(x[2*k]-gridL)) - fastExp3(-_alpha*(gridR-x[2*k])) ) * _module_densex_in_bin[k]*_module_densex_in_bin[k]*_module_densey_in_bin[k];
					double tmp2 = constant* _alpha * ( fastExp3(-_alpha*(x[2*k+1]-gridB)) - fastExp3(-_alpha*(gridT-x[2*k+1])) ) * _module_densey_in_bin[k]*_module_densey_in_bin[k]*_module_densex_in_bin[k];
					g[2*k] += tmp1;
					g[2*k+1] += tmp2;
					_gDensity += abs(tmp1);
					_gDensity += abs(tmp2);
				}
				else{
					_module_densex_in_bin[k] = 0;
					_module_densey_in_bin[k] = 0;
				}
				
			}
		}
	}
	// cout << "end of loop" << endl;
	double tmp = 0;
	for(int i = 0; i<_binNum; ++i){
		tmp += (_binDensities[i]-_Mb)*(_binDensities[i]-_Mb);
	}
	f += _fDenseRevise* _lambda*tmp/(_Mb*_binNum);  //normalize
	_fDensity = _fDenseRevise* _lambda*tmp/(_Mb*_binNum);
	// cout << "Dense FG result:" << endl;
	// cout << "fDensity: " << _fDensity << endl;
	// cout << "g after density: " << _gDensity << endl;
	// for(int i = 0; i < g.size(); ++i){
		// if (i % 3000 == 0) cout << "g[" << i << "] = " << g[i] << endl;
	// }
	// for (unsigned i = 0; i < 2*_moduleNum; i++) {
		// if (i % 3000 == 0) cout << "x[" << i << "] = " << x[i] << endl;
	// }
	// cout << "==================== Dense FG is finished =======================" << endl;
	// exit(0);
}

void ExampleFunction::calDensityF(const vector<double> &x, double &f){ // use sigmoid function
	for(int i = 0; i < _binNum; ++i) { _binDensities[i] = 0.0; }
	//for( int i = 0; i < g.size(); ++i) { _gDense[i] = 0; }
	for( int i = 0; i < _moduleNum; ++i) { _module_densex_in_bin[i] = 0.0; }
	for( int i = 0; i < _moduleNum; ++i) { _module_densey_in_bin[i] = 0.0; }
	// cout << "================= go into cal Dense F ===================" << endl;
	double lboundary = _placement.boundryLeft();
	double bboundary = _placement.boundryBottom();
	for(int i = 0; i < _binCol; ++i){
		double gridL = _binW * i + lboundary;
		double gridR = gridL + _binW;
		for (int j = 0; j < _binRow; ++j){
			double gridB = _binH * j + bboundary;
			double gridT = gridB + _binH;
			for(int k = 0; k < _moduleNum; ++k){ // f
				double mleft = x[k*2] - lboundary;
				double mlower = x[k*2+1] - bboundary;
				int lHidx = (int)(mleft / _binW);
				// int rHidx = (int)(mright / _binW) + 1;
				int bVidx = (int)(mlower / _binH);
				// int tVidx = (int)(mupper / _binH) + 1;
				if (i > lHidx-3 && i < lHidx+4 && j > bVidx-3 && j < bVidx+4){ //the module fall in the bin 3, 4
					_module_densex_in_bin[k] = sigmoid(x[k*2]-gridL)*sigmoid(gridR-x[k*2]);
					_module_densey_in_bin[k] = sigmoid(x[k*2+1]-gridB)*sigmoid(gridT-x[k*2+1]);
					_binDensities[i*_binCol + j] += _module_densex_in_bin[k] * _module_densey_in_bin[k]; 
				}
				else{
					_module_densex_in_bin[k] = 0;
					_module_densey_in_bin[k] = 0;
				}
			}
		}
	}
	double tmp1 = 0;
	for(int i = 0; i<_binNum; ++i){
		tmp1 += (_binDensities[i]-_Mb)*(_binDensities[i]-_Mb);
	}
	f += _fDenseRevise* _lambda*tmp1/(_Mb*_binNum);  //normalize
	_fDensity = _fDenseRevise* _lambda*tmp1/(_Mb*_binNum);
	// cout << "fDensity: " << _fDensity << endl;
	// for (unsigned i = 0; i < 2*_moduleNum; i++) {
		// if (i % 3000 == 0) cout << "x[" << i << "] = " << x[i] << endl;
	// }
	// cout << "===========================================" << endl;
}

void ExampleFunction::evaluateFG(const vector<double> &x, double &f, vector<double> &g)
{
    // f = 3*x[0]*x[0] + 2*x[0]*x[1] + 2*x[1]*x[1] + 7; // objective function
    // g[0] = 6*x[0] + 2*x[1]; // gradient function of X
    // g[1] = 2*x[0] + 4*x[1]; // gradient function of Y
	// cout << "evaluate FG" << endl;
	_fWL = 0;
	_gWL = 0;
	_fDensity = 0;
	_gDensity = 0;
	f = 0;
	for(int i = 0; i < _placement.numModules()*2; ++i){ // 
		g[i] = 0;
	}
	calWLFG(x, f, g);
	calDensityFG(x, f, g);
	// cout << endl << endl << endl << endl << endl;
}

void ExampleFunction::evaluateF(const vector<double> &x, double &f)
{
    // f = 3*x[0]*x[0] + 2*x[0]*x[1] + 2*x[1]*x[1] + 7; // objective function
	// cout << "cal F" << endl;
	_fWL = 0;
	//_gWL = 0;
	_fDensity = 0;
	//_gDensity = 0;
	f = 0;
	calWLF(x, f);
	calDensityF(x, f);
	// cout << endl << endl << endl << endl << endl;
}

unsigned ExampleFunction::dimension()
{
    return 2 * _moduleNum; // num_blocks*2 
    // each two dimension represent the X and Y dimensions of each block
}
