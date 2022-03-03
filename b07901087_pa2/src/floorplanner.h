#ifndef FLOORPLANNER_H
#define FLOORPLANNER_H

#include <fstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <random>
#include <cmath>
#include <chrono> // for clock
#include <algorithm>
#include "module.h"
#include <iomanip>      // std::setprecision
#include <limits>


// some param for SA
#define 	rateT 		0.85
#define		kn_rate		10             //10
#define		P 			0.99
#define		FROZEN		0.01
#define		DELTA		0.01
#define		TINTMAX		2147483647
#define 	SUBMOTHER	8

typedef std::numeric_limits< double > dbl;

using namespace std;



class floorplanner
{
public:
    // constructor and destructor
    floorplanner(double a, fstream& blockfile, fstream& netfile) :
        _bestCost(2147483647), _bestW(2147483647),_bestH(2147483647), _bestArea(2147483647),_bestWL(2147483647), _lastCost(0),
        _accumArea(0), _accumWL(0) ,_first_feas(1) {
		_alpha = a;
		
		_T = DELTA / fabs(log(P));
		_frozen = FROZEN;
        parseblock(blockfile);
		parsenet(netfile);
		_N = kn_rate * _blocklength;
        _R = _H / _W;
		if(_blocklength == 49){
			//_alpha = 75
			_steps = 100;
			_Anorm = 40200000;
			_WLnorm = 1560000;
		}
		if(_blocklength == 33){
			_steps = 100;
			_Anorm = 1440000;
			_WLnorm = 122000;
		}
		if (_blocklength == 9){ // apte
			_steps = 100;
			_Anorm = 54000000;
			_WLnorm = 830000;
		}
		if(_blocklength == 10){ // xerox
			_steps = 100;
			_Anorm = 24350000;
			_WLnorm = 480000;
		}
		if (_blocklength == 11){ // hp
			_steps = 100;
			_Anorm = 12290000;
			_WLnorm = 300000;
		}
			
    }
    ~floorplanner() {
		for(int i = 0; i < _blocklist.size(); ++i) delete _blocklist[i];
        for(int i = 0; i < _termlist.size(); ++i) delete _termlist[i];
        for(int i = 0; i < _netlist.size(); ++i) delete _netlist[i];
    }

    // Input
    void addBlock(string str, int i, int x, int y) { Block* b = new Block(str, i, x, y); _blocklist.push_back(b); }
    void addTerm(string str, int x, int y) { Terminal* t = new Terminal(str, x, y); _termlist.push_back(t); }
	void addNet() { Net* n = new Net(); _netlist.push_back(n); }
	void netAddBlock(int idx, int b){ _netlist[idx] -> addBlock(_blocklist[b]); }
	void netAddTerm(int idx, int t){ _netlist[idx] -> addTerm(_termlist[t]); }

    // parsing
    void parseblock(fstream& inFile);
    void parsenet(fstream& inFile);
	

    // member functions about reporting
    
	
	// B* tree operations
	Node* init_tree(Node* parent, int i);
	int  pack();
	void packPreorder(Node* current_node, int x);
	void update_block(Block* block, int left_bound);
	void op1(int b1 = -1);
	void op2(int b1 = -1, int b2 = -1, int lr = -1);
	void op3(int b1 = -1, int b2 = -1);
	void updateblock_mapexp(Block* block, int left_bound);
	
	
	
	//SA fun
	void 		FastSA();
	int 		checkFeasible(){ int ret = (_treeH <= _H && _treeW <= _W)? 1:0; return ret;}
	void 		recover(int opnum);
	void 		brute_recover(int opnum);
	void 		backup();
	void 		record_best_sol();
	double		calSubCost();
	double		calMyCost();
	void		floorplan();
	
	// cost fun
	double 		WL();
	
	//utility
	void report_map();
	void report_block();
	void report_tree();
	Node* reportPreorder(Node* node,int& node_count);
	void report_tmpans();
	void print_answer();
	void print_cur_ans();
	void largeblank();
	void report_term();
	void write_cur_ans();
	
	void report_ans(fstream& outFile, double time); 
	
	
	
	
	
	
	
	

private:
	
	// for SA
	double 				_alpha;
	int 				_N;			// pick N neighber structure
	double				_frozen;
	
	
	double				_T;			// temperature
	int 				_accumArea;	// used for cost fun
	double 				_accumWL;
	int 				_steps;		// for accum calc
	int 				_first_feas;
	
	// for SA recover
	int					_Blockmoved[2];	// index of moved block
	int 				_leafID;
	int 				_leafparentID;
	int 				_leaflr;
	
	double				_Anorm;
	double				_WLnorm;
	
	
	
	
	// B* tree	(current answer)
	double 				_treeW;		// tree boundary
	double				_treeH;
	double				_treeR;		// ratio
	double				_treeWL;
	//double				_treeCost;
	
	// best answer
	double				_bestCost;
	int					_bestH;
	int 				_bestW;
	int 				_bestArea;
	double				_bestWL;
	double				_lastCost;
	
	// outline
	int 				_blocklength;
	int					_termlength;
	double 				_W;			// outline
	double 				_H;			// outline
	double				_R;			// ratio	

	//	input
	unordered_map<string, int>	_Block2ID;
	unordered_map<string, int>	_Term2ID;
	// output
	vector<string>		_id2Block;
	//vector<string>		_id2Term;
	
	
	
	
	
	//double 				_cost;
	// tree struct
	Node* 				_tree; 		// root of the binary tree
	vector<Block*> 		_blocklist;
	vector<Terminal*>	_termlist;
	vector<Net*>		_netlist;
	map<int, int> 		_contour; // contour 
	
    
    
};

#endif  // FLOORPLANNER_H
