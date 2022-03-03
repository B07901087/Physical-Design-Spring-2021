#ifndef PARTITIONER_H
#define PARTITIONER_H

#include <fstream>
#include <vector>
#include <map>
#include "cell.h"
#include "net.h"
using namespace std;

class Partitioner
{
public:
    // constructor and destructor
    Partitioner(fstream& inFile) :
        _cutSize(0), _netNum(0), _cellNum(0), _maxPinNum(0), _bFactor(0),
        _accGain(0), _maxAccGain(0), _iterNum(0), _maxGain(0), _balanceSub(0) {
        parseInput(inFile);
        _partSize[0] = 0;
        _partSize[1] = 0;
    }
    ~Partitioner() {
        clear();
    }

    // basic access methods
    int getCutSize() const          { return _cutSize; }
    int getNetNum() const           { return _netNum; }
    int getCellNum() const          { return _cellNum; }
    double getBFactor() const       { return _bFactor; }
    int getPartSize(int part) const { return _partSize[part]; }

    // modify method
    void parseInput(fstream& inFile);
    void partition();

    // member functions about reporting
    void printSummary() const;
    void reportNet() const;
    void reportCell() const;
    void writeResult(fstream& outFile);
	
	// added functions
	void update_gain(Cell* base_cell);
	bool check_balance(Cell* moving_cell);
	Cell* find_next_cell(Cell* base_cell);
	Cell* pick_base_cell();
	void partition_init();
	void update_cut_size();
	void gain_init1();
	void move_node_in_bucket(Node* target_node, int src, int des);
	Cell* get_cell_from_node(Node* some_node);
	void reset_net_part();
	void reportNetPart() ;
	
	//utility
	void reportCellGain();
	void reportCellPart() ;
	void reportBucket() ;
	
	

private:
    int                 _cutSize;       // cut size
    int                 _partSize[2];   // size (cell number) of partition A(0) and B(1)
    int                 _netNum;        // number of nets
    int                 _cellNum;       // number of cells
    int                 _maxPinNum;     // Pmax for building bucket list
    double              _bFactor;       // the balance factor to be met
    Node*               _maxGainCell;   // pointer to max gain cell
    vector<Net*>        _netArray;      // net array of the circuit
    vector<Cell*>       _cellArray;     // cell array of the circuit
    map<int, Node*>     _bList[2];      // bucket list of partition A(0) and B(1)// int is gain!!!
    map<string, int>    _netName2Id;    // mapping from net name to id
    map<string, int>    _cellName2Id;   // mapping from cell name to id

    int                 _accGain;       // accumulative gain
    int                 _maxAccGain;    // maximum accumulative gain
    int                 _moveNum;       // number of cell movements
    int                 _iterNum;       // number of iterations
    int                 _bestMoveNum;   // store best number of movements
    int                 _unlockNum[2];  // number of unlocked cells
    vector<Cell *>         _moveStack;     // history of cell movement (partition sum k step)
	int 				_maxGain;		// max gain
	int 				_balanceSub;	// sub from the mid
	int 				_firsttime;		// firsttime
	int 				_select_part;

    // Clean up partitioner
    void clear();
};

#endif  // PARTITIONER_H
