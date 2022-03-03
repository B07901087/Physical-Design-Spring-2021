#include "GlobalPlacer.h"
#include "ExampleFunction.h"
#include "NumericalOptimizer.h"
// #include <iostream>
// #include <vector>
// #include <numeric>      // std::iota
#include <algorithm>    // std::sort, std::stable_sort
#include<bits/stdc++.h>
// GlobalPlacer::GlobalPlacer(Placement &placement)
	// :_placement(placement)
// {

// }
#include <stdint.h>
float GlobalPlacer::fastExp2(register float x)  // cubic spline approximation
{
    union { float f; int32_t i; } reinterpreter;

    reinterpreter.i = (int32_t)(12102203.0f*x) + 127*(1 << 23);
    int32_t m = (reinterpreter.i >> 7) & 0xFFFF;  // copy mantissa
    // empirical values for small maximum relative error (8.34e-5):
    reinterpreter.i +=
         ((((((((1277*m) >> 14) + 14825)*m) >> 14) - 79749)*m) >> 11) - 626;
    return reinterpreter.f;
}

typedef std::pair<int,unsigned> mypair;

bool comparator ( const mypair& l, const mypair& r) { return l.second > r.second; }

double GlobalPlacer::rand_legal_pos_x(double x, double w){
	double right_bound = _placement.boundryRight();
	double left_bound = _placement.boundryLeft();
	double length = (right_bound - left_bound);
	if (( x + w ) > right_bound){
		cerr << "(rand_legal_pos_x)illegal outlier(higher)" << endl; 
		cerr << "x is: " << x << ", module width is: " << w << endl;
		return left_bound + length / 2 +  rand() % (int)(length/2 - w);
	}
	if (x < left_bound){
		cerr << "(rand_legal_pos_x)illegal outlier(lower)" << endl; 
		cerr << "x is: " << x << endl;
		return left_bound +  rand() % (int)(length/2 - w);
	}
	return x;
}

double GlobalPlacer::rand_legal_pos_y(double y, double h){
	double lower_bound = _placement.boundryBottom();
	double upper_bound = _placement.boundryTop();
	double length = (upper_bound - lower_bound) / 2;
	if (( y + h ) > upper_bound){
		cerr << "(rand_legal_pos_y)illegal outlier(higher)" << endl; 
		cerr << "y is: " << y << ", module high is: " << h << endl;
		return lower_bound + length / 2 +  rand() % (int)(length/2 - h);
	}
	if (y < lower_bound){
		cerr << "(rand_legal_pos_y)illegal outlier(lower)" << endl; 
		cerr << "y is: " << y << endl;
		return lower_bound +  rand() % (int)(length/2 - h);
	}
	return y;
}

double GlobalPlacer::calinitDensityFG(const vector<double> &x, double& density_grad){ // use sigmoid function
	// cout << "================= (Global) calDensityFG ===================" << endl;
	density_grad = 0;
	double lboundary = _placement.boundryLeft();
	double bboundary = _placement.boundryBottom();
	double total_bin_density = 0;
	double Mb = _placement.numModules() /(_binCol*_binRow);
	for(int i = 0; i < _binCol; ++i){
		double gridL = _binW * i + lboundary;
		double gridR = gridL + _binW;
		for (int j = 0; j < _binRow; ++j){
			double gridB = _binH * j+  bboundary;
			double gridT = gridB + _binH;
			double binDensity = 0;
			vector<double> module_densex_in_bin(_placement.numModules(), 0.0);
			vector<double> module_densey_in_bin(_placement.numModules(), 0.0);
			for(int k = 0; k < _placement.numModules(); ++k){ // f
				double mleft = x[2*k] - lboundary;
				double mlower = x[2*k+1] - bboundary;
				int lHidx = (int)(mleft / _binW);
				int bVidx = (int)(mlower / _binH);
			
				if (i > lHidx-3 && i < lHidx+4 && j > bVidx-3 && j < bVidx+4){ //the module fall in the bin
					module_densex_in_bin[k] = sigmoid(x[2*k]-gridL)*sigmoid(gridR-x[2*k]);
					module_densey_in_bin[k] = sigmoid(x[2*k+1]-gridB)*sigmoid(gridT-x[2*k+1]);
					binDensity += module_densex_in_bin[k] * module_densey_in_bin[k];
				}
			}
			total_bin_density += (binDensity-Mb)*(binDensity-Mb);
			if (module_densex_in_bin.size() != _placement.numModules()){cout << "num is not correct" << endl; exit(0);}
			for(int k = 0; k < _placement.numModules(); ++k){ // cal gradient
				double mleft = x[2*k] - lboundary;
				double mlower = x[2*k+1] - bboundary;
				int lHidx = (int)(mleft / _binW);
				int bVidx = (int)(mlower / _binH);
				if (i > lHidx-3 && i < lHidx+4 && j > bVidx-3 && j < bVidx+4){ //the module fall in the bin
					if (module_densex_in_bin[k] == 0 || module_densey_in_bin[k] == 0){
						cerr << "density is strange!" << endl;
						exit(0);
					}
					double constant = _gDenseReviseGlobal* 2.0*(binDensity-Mb)*_currlambda/(Mb*_binNum); //normalize
					double tmp1 = constant* _alpha * ( fastExp2(-_alpha*(x[2*k]-gridL)) - fastExp2(-_alpha*(gridR-x[2*k])) ) * module_densex_in_bin[k]*module_densex_in_bin[k]*module_densey_in_bin[k];
					double tmp2 = constant* _alpha * ( fastExp2(-_alpha*(x[2*k+1]-gridB)) - fastExp2(-_alpha*(gridT-x[2*k+1])) ) * module_densey_in_bin[k]*module_densey_in_bin[k]*module_densex_in_bin[k];
					density_grad += abs(tmp1);
					density_grad += abs(tmp2);
				}	
			}
		}
	}
	// cout << "end of loop" << endl;
	double DenseF = _fDenseReviseGlobal* _currlambda*total_bin_density/(Mb*_binNum);  //normalize
	//_fDensity = _lambda*tmp/(Mb*_binNum);
	// for(int i = 0; i < g.size(); ++i){
		// if (i % 3000 == 0) cout << "g[" << i << "] = " << g[i] << endl;
	// }
	// for (unsigned i = 0; i < 2*_placement.numModules(); i++) {
		// if (i % 3000 == 0) cout << "x[" << i << "] = " << x[i] << endl;
	// }
	
	return DenseF;
}

double GlobalPlacer::calinitWLFG(const vector<double> &x, double& wire_grad){ // use LSE function
	vector<double> tempG(_placement.numModules() * 2, 0.0);
	wire_grad = 0;
	double wire_f = 0;
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
		wire_f += log(part1) + log(part2) + log(part3) + log(part4);
		
		// compute g(gradient)
		for(int j = 0; j <  thisnet.numPins(); ++j){
		// take log of each part and sum them all
			int midx = thisnet.pin(j).moduleId();
			tempG[2*midx] += ( exp(x[2*midx] / _gamma) / part1 + (-1.0) * exp(-1.0*x[2*midx] / _gamma) / part2 ) / _initWL;
			tempG[2*midx+1] += ( exp(x[2*midx+1] / _gamma) / part3 + (-1.0) * exp(-1.0*x[2*midx+1] / _gamma) / part4 ) / _initWL;
		}
		
		part1 = part2 = part3 = part4 =0;
	}
	
	wire_f = _fWLReviseGlobal* wire_f * _gamma / _initWL;
	// for (unsigned i = 0; i < 2*_placement.numModules(); i++) {
		// if (i % 3000 == 0) cout << "x[" << i << "] = " << x[i] << endl;
	// }
	for(int i = 0; i < tempG.size(); ++i){
		// if (i % 3000 == 0) cout << "g[" << i << "] = " << tempG[i] << endl;
		wire_grad += abs(tempG[i]);
	}		
	wire_grad *= _gWLReviseGlobal;
	return wire_f;
}

double GlobalPlacer::initplace1(vector<double> &x){ // initial placement distribute among bins
	int mxidx;
	int myidx;
	//int module_num = _placement.numModules();
	// int row_num = (int)sqrt(module_num) + 1;
	// int col_num = (int)sqrt(module_num) + 1;
	int bidx;
	int ridx; // row idxx
	int cidx;
	double bx;
	double by;
	double HPWL;
	vector<double> bestX(_placement.numModules()*2, 0.0);
	for (int iter = 0; iter < 3; ++iter){ // see whether wire length can be improved
		
		for (int i = 0; i < _placement.numModules(); ++i){
			mxidx = 2*i;
			myidx = mxidx + 1;
			bidx = i % _binNum;
			ridx = (int)(bidx / _binCol);
			cidx = bidx % _binCol;
			bx = _binW * cidx + _placement.boundryLeft();
			by = _binH * ridx + _placement.boundryBottom();
			if (_binW - _placement.module(i).width() < 0 || _binH - _placement.module(i).height() < 0 ){
				cerr << "bin number too large!" << endl;
				exit(0);
			}
			x[mxidx] = bx + rand() % (int)(_binW - _placement.module(i).width());
			x[myidx] = by + rand() % (int)(_binH - _placement.module(i).height());
			
			_placement.module(i).setPosition(x[mxidx], x[myidx]);
		
		}
		
		if (iter == 0){
			HPWL = _placement.computeHpwl();
			for (int n = 0; n < x.size(); n++)
			{
				bestX[n] = x[n];
			} 
		}
		else{
			if (_placement.computeHpwl() < HPWL){
				cout << "new answer founded!" << endl;
				HPWL = _placement.computeHpwl();
				for (int n = 0; n < x.size(); n++)
				{
					bestX[n] = x[n];
				} 
			}	
		}
		// cout << "round " << iter << ": " << "HPWL = " << _placement.computeHpwl() << endl;
	}
	int p;
	int q;
	for (int i = 0; i < _placement.numModules(); ++i){
		p = 2 * i;
		q = p + 1;
		x[p] = bestX[p];
		x[q] = bestX[q];
		_placement.module(i).setPosition(x[p], x[q]);
	}
	return HPWL;
}

double GlobalPlacer::initplace2(vector<double> &x){ 
	// vector<int> module_degree(_placement.numModules(), 0);
	vector<int> module_used(_placement.numModules(), 0);
	vector<mypair> netordered;
	for(int i = 0; i < _placement.numNets(); ++i){
		Net thisnet = _placement.net(i);
		netordered.push_back(std::make_pair(i,thisnet.numPins()));
	}
	// for(int i = 0; i < netordered.size(); ++i){
		// cout << netordered[i].first << ", " << netordered[i].second << endl;
	// }
	sort(netordered.begin(), netordered.end(), comparator);
	
	double bboundary = _placement.boundryBottom();
	double lboundary = _placement.boundryLeft();
	cout << "num of modules: " << _placement.numModules() << endl;
	double avg = (double)_placement.numModules()/225;
	cout << "average modules in bin: " << avg <<  endl;
	int max_net = 0;
	double bx;
	double by;
	
	int count = 0;
	int row = 15;
	int col = 7;
	int cur_module_num = 0;
	int unused_modules = 0;
	for (int i = 0; i < _placement.numNets(); ++i){
		Net thisnet = _placement.net(netordered[i].first);
		for (int j = 0; j < thisnet.numPins(); ++j){
			int midx = thisnet.pin(j).moduleId();
			if (module_used[midx] == 0){
				Module thismodule = _placement.module(midx);
				module_used[midx] = 1;
				cur_module_num += 1;
				////////////// this is bin boundary ////////////////
				bx = _binW * col + _placement.boundryLeft();
				by = _binH * row + _placement.boundryBottom();
				x[midx*2] = bx + rand() % (int)(_binW - thismodule.width());
				x[midx*2 + 1] = by + rand() % (int)(_binH - thismodule.height());
				_placement.module(midx).setPosition(x[midx*2], x[midx*2 + 1]);
				if (cur_module_num > avg){ // change bin
					cout << "current bin: " << "row = " << row << "col = " << col << endl;
					row += 1;
					if (row >= 15){ // change col
						row = 0;
						if (count % 2 == 0){
							col -= count;
						}
						else{
							col += count;
						}
						count += 1;
						if (count == 16){ // need to clean all module
							for(int t = 0; t < _placement.numModules(); ++t){
								if (module_used[t] == 0) unused_modules += 1;
							}
							cout << "unused modules: " << unused_modules << endl;
							exit(0);
						}
					} // change column
					cur_module_num = 0;
				} // change row or column
			} // module is used
			else{
				//cout << "module is used" << endl;
			}
		} // traverse pin
		
	}
	// for(int i = 0; i < _placement.numModules(); ++i){
		// cout << "module(" << i << "): " << x[2*i] << ", " << x[2*i + 1] << endl; 
	// }
	cout << "max net degree: " << max_net << endl;
	return -1;
	
}



void GlobalPlacer::place()
{
	
	double density_g = 0;
	double density_f = 0;
	double WL_g = 0;
	double WL_f = 0;
	
    vector<double> x(_placement.numModules()*2, 0); 
	int outeritter = 3;
	if (_placement.numModules() == 12028){outeritter = 4;}
	if (_placement.numModules() < 30000) { _currlambda = 0.0001; }
	
	double chipwidth = _placement.boundryRight() - _placement.boundryLeft();
	double chipheight = _placement.boundryTop() - _placement.boundryBottom();
	int num_outlier = 0;
	double init_HPWL = initplace1(x);
	_initWL = init_HPWL;
	ExampleFunction ef(_placement, _binCol, _binRow, init_HPWL); // require to define the object function and gradient function
    NumericalOptimizer no(ef);
	cout << "WL: " << _placement.computeHpwl() << endl;
	density_f = calinitDensityFG(x, density_g);
	WL_f = calinitWLFG(x, WL_g);
	_fWLReviseGlobal = 1;
	_gWLReviseGlobal = 3;
	_fDenseReviseGlobal = 1;
	_gDenseReviseGlobal = 1;
	_currlambda = 0.0001;
	ef.setLambda(_currlambda);
	ef.setfDenseRevise(_fDenseReviseGlobal);
	ef.setgDenseRevise(_gDenseReviseGlobal);
	ef.setfWLRevise(_fWLReviseGlobal);
	ef.setgWLRevise(_gWLReviseGlobal);
	
	cout << "density_f: " << density_f <<endl;
	cout << "density_g: " << density_g <<endl;
	cout << "WL_f: " << WL_f << endl;
	cout << "WL_g: " << WL_g << endl;
	
	no.setX(x);
	cout << "initial solution:" << endl;
	for (unsigned i = 0; i < no.dimension(); i++) {
		if (i % 3000 == 0) cout << "x[" << i << "] = " << no.x(i) << endl;
	}
	// cout << "Objective: " << no.objective() << endl;
	// cout << "initial wirelength: " << init_HPWL << endl;
	// cout << "outline(H): " << _placement.boundryLeft() << " " << _placement.boundryRight() << endl;
	// cout << "outline(V): " << _placement.boundryBottom() << " " << _placement.boundryTop() << endl;
	
	// exit(0);
	///////////////////////////////
	int inneriter = 5;
	for( int iter = 0; iter < outeritter; ++iter){
		if (iter == 0 && _placement.numModules() < 30000) {inneriter = 7;}
		else if (_placement.numModules() < 30000){inneriter = 4;}
		no.setX(x);
		no.setNumIteration(inneriter); // user-specified parameter
		no.setStepSizeBound(max(chipheight,chipwidth)/1.0*7); // user-specified parameter
		no.solve(); // Conjugate Gradient solver
		// cout << "print no" << endl;
		// for (unsigned j = 0; j < no.dimension(); j++) {
			// if (j % 3000 == 0) cout << "x[" << j << "] = " << no.x(j) << endl;
		// }
		// cout << "=====================" << endl;
		for( int m = 0; m < _placement.numModules(); ++m){
			// if (_placement.module(m).x() != no.x(2*m) || _placement.module(m).y() != no.x(2*m+1)) { cerr << "something wrong in x array" << endl; exit(0); }
			x[2*m] = rand_legal_pos_x(no.x(2*m),_placement.module(m).width());
			x[2*m+1] = rand_legal_pos_y(no.x(2*m+1),_placement.module(m).height());
			if (no.x(2*m) != x[2*m] || no.x(2*m+1) != x[2*m+1]) { num_outlier += 1; cout << "outlier!" << endl;  }
			_placement.module(m).setPosition(x[2*m], x[2*m+1]); // here!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//if (_placement.module(m).x() != no.x(2*m) || _placement.module(m).y() != no.x(2*m+1)) { cerr << "something wrong in x array" << endl; exit(0); }
		}
		no.setX(x);
		
		cout << "==============================================" << endl;
		cout << "step: " << iter << " solution:" << endl;
		for (unsigned i = 0; i < no.dimension(); i++) {
			if (i % 3000 == 0) cout << "x[" << i << "] = " << x[i] << endl;
		}
		cout << "Objective: " << no.objective() << endl;
		cout << "fWL: " << ef.getfWL() << endl;
		cout << "fDensity: " << ef.getfDensity() << endl;
		cout << "gWL: " << ef.getgWL() << endl;
		cout << "getgDensity: " << ef.getgDensity() << endl;
		cout << endl;
		cout << "current WL: " << _placement.computeHpwl() << endl;
		cout << "number of outlier: " << num_outlier << endl;
		cout << "=============================================" << endl;
		// exit(0);
		num_outlier = 0;
		////////////////////////////////////////////////////////////////
		_currlambda += 0.01;
		ef.setLambda(_currlambda);
		
		
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
