#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cassert>
#include <vector>
#include <cmath>
#include <map>
#include "cell.h"
#include "net.h"
#include "partitioner.h"
using namespace std;


void Partitioner::parseInput(fstream& inFile)
{
    string str;
    // Set balance factor
    inFile >> str;
    _bFactor = stod(str);

    // Set up whole circuit
    while (inFile >> str) {
        if (str == "NET") {
            string netName, cellName, tmpCellName = "";
            inFile >> netName;
            int netId = _netNum;
            _netArray.push_back(new Net(netName)); // push new net
            _netName2Id[netName] = netId;
            while (inFile >> cellName) { // input cells
                if (cellName == ";") {
                    tmpCellName = "";
                    break;
                }
                else {
                    // a newly seen cell
                    if (_cellName2Id.count(cellName) == 0) {
                        int cellId = _cellNum;
                        _cellArray.push_back(new Cell(cellName, 0, cellId));
                        _cellName2Id[cellName] = cellId;
                        _cellArray[cellId]->addNet(netId); // list of nets the cell is connected to
                        _cellArray[cellId]->incPinNum(); // cell pin num += 1
                        _netArray[netId]->addCell(cellId); // list of cell that net connected to
                        ++_cellNum;
                        tmpCellName = cellName; // why tmp cell name???
                    }
                    // an existed cell
                    else {
                        if (cellName != tmpCellName) {
                            assert(_cellName2Id.count(cellName) == 1);
                            int cellId = _cellName2Id[cellName];
                            _cellArray[cellId]->addNet(netId); // list of nets the cell is connected to
                            _cellArray[cellId]->incPinNum(); // cell pin num += 1
                            _netArray[netId]->addCell(cellId); // list of cell that net connected to
                            tmpCellName = cellName;
                        }
                    }
                }
            }
            ++_netNum;
        }
    }
    return;
}

Cell* Partitioner::get_cell_from_node(Node* some_node){
	if ( some_node == NULL ) return NULL;
	return _cellArray[some_node -> getId()];
}

bool Partitioner::check_balance(Cell* moving_cell){
	// seems no use?
	int A_size = _partSize[0];
	int B_size = _partSize[1];
	if (moving_cell -> getPart()){ // B, so A add
		A_size += 1;
		B_size -= 1;
	}
	else{ // A
		B_size += 1;
		A_size -= 1;
	}
	if ( (A_size < _cellNum * (1-_bFactor)/2)  ){
		_select_part = 1;
	}
	if ( (A_size > _cellNum * (1+_bFactor)/2) ){
		_select_part = 0;
	}
	
	if ( (A_size < _cellNum * (1-_bFactor)/2) || (A_size > _cellNum * (1+_bFactor)/2) ){
		return false;
	}
	else{
		return true;
	}
}

Cell* Partitioner::find_next_cell(Cell* base_cell){
	// Cell* max_gain_cell = get_cell_from_node(_maxGainCell);
	// int current_gain = max_gain_cell -> getGain();
	// cerr << "find next cell function " << endl;
	int current_gain = base_cell->getGain();
	// cerr << "find next cell for: " << base_cell -> getName() << endl;
	// reportBucket();
	
	
	Node* base_cellNode = base_cell -> getNode();
	Node* node = _bList[0][current_gain];
	int next_flag = 0;
	while (node != NULL){ // next node is not NULL
			if (node != base_cellNode && get_cell_from_node(node)->getPart() == _select_part){
				base_cell = _cellArray[node -> getId()];
				// cerr << "pick next cell: " << base_cell -> getName() << " to be base cell" << endl;
				next_flag = 1;
				break;
			}
			node = node -> getNext();
			
	}
	if (next_flag == 0){ // next node is NULL, so we have to traverse map
		// base_cell = _cellArray[base_cell -> getNode() -> getId()];
		// cerr << "find cell with other gain" << endl;
		--current_gain;
		while (((!_bList[0].count(current_gain)) || _bList[0][current_gain] == NULL) && current_gain > (-_maxPinNum)){
			current_gain -= 1;
			// cerr << "current gain: " << current_gain << endl;
		}
		if (current_gain == (-_maxPinNum) ){
			//cout << "did not find valid cell" << endl;
			return NULL;
		}
		base_cell = get_cell_from_node(_bList[0][current_gain]);
	}
	// cerr << "function return" << endl;
	// cerr << "next cell founded: " << base_cell -> getName() << endl;
	return base_cell;
}

Cell* Partitioner::pick_base_cell(){
	// may based on different heuristic
	// 1. find from the max pointer
	Cell* base_cell;
	Cell* maxGainCell = get_cell_from_node(_maxGainCell);
	base_cell = maxGainCell;
	//cerr << "maxGainCell is: " << maxGainCell->getName() << endl;
	int count = 0;
	int flag = 0;
	if (maxGainCell == NULL ) return NULL; // ***** not sure ***** 
	int current_gain = maxGainCell -> getGain();
	// 2. check if it is balanced and min weight
	while(!check_balance(base_cell)){
		//cerr << "unbalanced" << endl;
		flag = 1;
		base_cell = find_next_cell(base_cell);
		++ count;
		// cerr << base_cell->getName();
		if (count > 5) { reportBucket();exit(0);}
		if (base_cell == NULL) {cerr << "no base cell?" << endl;return NULL;}
		// cerr << "consider cell: " << base_cell -> getName() << " with gain: " << base_cell -> getGain() << endl;
		
	}
	// 3. return the corresponding node and update max gain cell
	//if (!flag) {maxGainCell = find_next_cell(base_cell);}
	//cerr << "final base cell: " << base_cell -> getName() << endl;
	return base_cell;
}

void Partitioner::partition_init(){ // initial partition
	for( int i = 0; i <  (int)(_cellNum / 2); ++i ){ // partition to A
		_partSize[0] += 1;
		_unlockNum[0] += 1;
		_cellArray[i] -> setPart(0);
		for( auto netofcell : _cellArray[i] -> getNetList()){
			_netArray[netofcell] -> incPartCount(0);
		}
		if ( _maxPinNum < _cellArray[i] -> getPinNum() ) { _maxPinNum = _cellArray[i] -> getPinNum(); }
	}
	for( int i = (int)(_cellNum / 2); i <  _cellNum; ++i ){ // partition to B
		_partSize[1] += 1;
		_unlockNum[1] += 1;
		_cellArray[i] -> setPart(1);
		for( auto netofcell : _cellArray[i] -> getNetList()){
			_netArray[netofcell] -> incPartCount(1);
		}
		if ( _maxPinNum < _cellArray[i] -> getPinNum() ) { _maxPinNum = _cellArray[i] -> getPinNum(); }
	}
	_balanceSub = 0;
}

void Partitioner::update_cut_size(){
	_cutSize = 0;
	for ( auto net_pt : _netArray){ // cut size initialize
		if ( net_pt -> getPartCount(0) && net_pt -> getPartCount(1) ){
			_cutSize += 1;
		}
	}
}

void Partitioner::gain_init1(){ 
	// 1.calculate gain of each node
	//cerr << "#### gain init part ####" << endl;
	bool in_A; // for cut size
	bool in_B;
	
	for( auto net : _netArray ){ // use net to get cut size
		in_A = false;
		in_B = false;
		for (auto cell_id : net -> getCellList() ){
			Cell* net_cell = _cellArray[cell_id];
			int cell_side = net_cell -> getPart();
			if( (net -> getPartCount(cell_side)) == 1) net_cell -> incGain(); // F = 1
			if( (net -> getPartCount(!cell_side)) == 0) net_cell -> decGain();// T = 0
		}
	}
	// 2.put nodes into corresponding position ( blist )
	// 3.update the max_pointer
	for( int i = 0; i <  _cellNum ; ++i ){
		int gain = _cellArray[i] -> getGain();
		move_node_in_bucket(_cellArray[i] -> getNode(), _maxPinNum+1, gain);
		if (gain > _maxGain) { _maxGain = gain; _maxGainCell = _bList[0][gain]; } 
	}
	//reportCellGain();
	//cerr << "#### end of gain init part ####" << endl;
}

// void Partitioner::recover_blist(){
	// Node* list_node = _bList[0][gain];
	// while (list_node != NULL){
		// Cell* list_cell = get_cell_from_node(list_node);
		// move_node_in_bucket(list_node, _maxPinNum+1, list_cell -> getGain());
		// list_cell -> unlock();
		// list_node = list_node -> getNext();
	// }
// }

// void gain_init2(){ // smaller init (maybe not use)
	
// }

void Partitioner::reset_net_part(){
	Net* net1;
	_partSize[0] = 0;
	_partSize[1] = 0;
	
	for (auto net :  _netArray){
		net -> setPartCount(0, 0);
		net -> setPartCount(1, 0);
	}
	
	for ( auto celll : _cellArray){ 
		celll -> setGain(0);
		celll -> unlock();
		int cellpart = celll -> getPart();
		++_partSize[cellpart];
		for (auto net1_idx : celll -> getNetList()){
			net1 = _netArray[net1_idx];
			net1 -> incPartCount(cellpart);
		
		}
	}
}

void Partitioner::move_node_in_bucket(Node* target_node, int src, int des){
	// 1. remove the cell by its id 
	//Node* target_node = _cellArray[id] -> getNode();
	Node* next_node = target_node -> getNext();
	Node* prev_node = target_node -> getPrev();
	// //cerr << "for cell " << get_cell_from_node(target_node) -> getName() <<":"<< endl;
	// if (prev_node != NULL) //cerr << "prev node: " << get_cell_from_node(prev_node) -> getName() << endl;
	// else //cerr << "prev node: NULL" << endl;
	// if (next_node != NULL) //cerr << "next node: " << get_cell_from_node(next_node) -> getName() << endl;
	// else //cerr << "next node: NULL" << endl;
	// reportBucket();
	
	// 1-1. if it is the start
	if ( _firsttime){ // first time skip
		if ( _bList[0][src] == target_node ){
			
			_bList[0][src] = next_node;
			// reportBucket();
			if (next_node != NULL) next_node -> setPrev(NULL);
		}
		// 1-2 if it is at the end
		else if ( next_node == NULL ){
			if (prev_node!= NULL){
				prev_node -> setNext(NULL);
				//if (get_cell_from_node(prev_node) -> getName() == "c3" ) //cerr << "c3 is setted" << endl;
			}
		} 
		// 1-3 if it is in the middle
		else{
			if (prev_node != NULL) prev_node -> setNext(next_node);
			if (next_node != NULL) next_node -> setPrev(prev_node);
		}
	}
	_firsttime = 1;
	// 2. add it to the beginning of des gain
	// //cerr << "hey" << endl;
	// if (des == _maxPinNum+1){
		// target_node
	// }
	if (_bList[0].count(des) && _bList[0][des] != NULL){
		target_node -> setNext(_bList[0][des]);
		_bList[0][des] -> setPrev(target_node);
	}
	else{
		target_node -> setNext(NULL);
	}
	// //cerr << "yo" << endl;
	target_node -> setPrev(NULL);
	// //cerr << "wryyyyy" << endl;
	_bList[0][des] = target_node;
	
}

void Partitioner::update_gain(Cell* base_cell){
	int check_sum = 0; // ****** this is for debug ********
	bool F = base_cell -> getPart();
	bool T = !F;
	int F_part = (F)? 1:0;
	int T_part = (T)? 1:0;
	int j_gain;
	
	base_cell -> move();
	base_cell -> lock();
	Cell* maxupdatetmp;
	if (base_cell->getNode() == _maxGainCell){
		// Cell* tmpcell = NULL;
		Cell* tmpcell = find_next_cell(get_cell_from_node(_maxGainCell));
		if (tmpcell == NULL){
			move_node_in_bucket(base_cell -> getNode(), base_cell -> getGain(), _maxPinNum+1);
			return;
		}
		_maxGainCell = tmpcell-> getNode();
		//cerr << "here" << endl;
		
		_maxGain = tmpcell->getGain();
	}
	move_node_in_bucket(base_cell -> getNode(), base_cell -> getGain(), _maxPinNum+1); // move to locked place: _maxPinNum+1
	
	for( auto i : base_cell -> getNetList()){ // for each net n on the base cell 
		check_sum = 0;
		// cout << "####### update ########" << endl;
		// cout << "traverse net: " << _netArray[i] -> getName() << endl;
		// //cerr << "current max gain cell: " << get_cell_from_node(_maxGainCell)->getName() << endl;
		// //cerr << "current max gain: " << _maxGain << endl;
		////cerr << "true max gain: " << get_cell_from_node(_maxGainCell)->getGain() << endl;
		int T_count = _netArray[i] -> getPartCount(T); 
		// cout << "T part number: " << T_count << endl;
		if ( T_count == 0 ){ // if T(n) = 0
			for( auto j: _netArray[i] -> getCellList() ){
				if ( !_cellArray[j] -> getLock() ){ // increment gains of all free cells on n
					//cerr << "therre" << endl;
					if ( (_cellArray[j] -> getGain() == _maxGain) && (!_cellArray[j] -> getLock()) ){ // if not lock then update maxgain
						_maxGainCell = _cellArray[j] -> getNode();
						_maxGain += 1; 
						// //cerr << "max gain cell changed: " << _cellArray[j]->getName() << endl;
						
					}
					 _cellArray[j] -> incGain();	// from -1 to 0	
					j_gain = _cellArray[j] -> getGain();
					// cout << "---" << endl;
					// reportBucket();
					// cout << "case1: increase gain of cell -> " << _cellArray[j]->getName() << endl;
					move_node_in_bucket(_cellArray[j] -> getNode(), j_gain-1, j_gain);	
					//reportBucket();		
					// cout << "---" << endl;					
				}
			}
			//base_cell -> incGain(); // from -1 to 1 
		}
		else if ( T_count == 1 ){ // else if T(n) = 1
			for( auto j: _netArray[i] -> getCellList() ){
				if (  (_cellArray[j] -> getPart() == T)  && (!_cellArray[j] -> getLock()) ){ // increment gains of all free cells on n (!_cellArray[j] -> getLock()) &&
					if ( _cellArray[j]->getNode() == _maxGainCell ){ // if not lock then update maxgain
						//cerr << "therre1" << endl;
						maxupdatetmp = find_next_cell(_cellArray[j]);
						if(maxupdatetmp == NULL){
							_maxGainCell = NULL;
							_maxGain = -_maxPinNum;
						}
						else{
							_maxGainCell = maxupdatetmp -> getNode();
							_maxGain = maxupdatetmp -> getGain();
							// //cerr << "max gain cell changed: " << get_cell_from_node(_maxGainCell) -> getName() << endl;
							
						}
					}
					 _cellArray[j] -> decGain();
					 j_gain = _cellArray[j] -> getGain();
					 // cout << "---" << endl;
					 // reportBucket();
					 // cout << "case2: decrease gain of cell -> " << _cellArray[j]->getName() << endl;
					move_node_in_bucket(_cellArray[j] -> getNode(), j_gain+1, j_gain);
					//reportBucket();
					//cout << "---" << endl;
					check_sum += 1;
				}
				
			}
			if (check_sum > 1) cout << "something wrong in T_count" << endl;
		}
		check_sum = 0;
		// base_cell -> move();
		// cout << "---" << endl;
		// reportBucket();
		// //cerr << "move base cell: " << 
		_netArray[i] -> incPartCount(T); // use bool may failed
		_netArray[i] -> decPartCount(F);
		int F_count = _netArray[i] -> getPartCount(F); // ***when a cell is locked, we may remove it from the bucket list and store it?***
		// cout << "F part number: " << F_count << endl;
		if ( F_count == 0 ){ //  if F(n) = 0
			for( auto j: _netArray[i] -> getCellList() ){ 
				if ( !_cellArray[j] -> getLock() ){ // decrement gains of all free cells on n
					if ( _cellArray[j]->getNode() == _maxGainCell ){
						//cerr << "therre11" << endl;
						maxupdatetmp = find_next_cell(_cellArray[j]);
						if(maxupdatetmp == NULL){
							_maxGainCell = NULL;
							_maxGain = -_maxPinNum;
						}
						else{
							_maxGainCell = maxupdatetmp -> getNode();
							_maxGain = maxupdatetmp -> getGain();
							// //cerr << "max gain cell changed: " << get_cell_from_node(_maxGainCell) -> getName() << endl;
							
						}
						
					}
					_cellArray[j] -> decGain();	// all from 0 -> -1 
					j_gain = _cellArray[j] -> getGain();
					// cout << "---" << endl;
					// reportBucket();
					// cout << "case3: decrease gain of cell -> " << _cellArray[j]->getName() << endl;
					move_node_in_bucket(_cellArray[j] -> getNode(), j_gain+1, j_gain);
					//reportBucket();
					// cout << "---" << endl;
				}
			}
			//base_cell -> decGain(); // from 1 to -1 
		}
		else if ( F_count == 1 ){ // if F(n) = 1
			for( auto j: _netArray[i] -> getCellList() ){
				if (  (_cellArray[j] -> getPart() == F) && (!_cellArray[j] -> getLock()) ){ // increment gain of the only F cell on n (!_cellArray[j] -> getLock()) &&
					if ( _cellArray[j] -> getGain() == _maxGain ){
						_maxGainCell = _cellArray[j] -> getNode();
						_maxGain += 1; 	
						// //cerr << "max gain cell changed: " << get_cell_from_node(_maxGainCell) -> getName() << endl;
						
					}
					 _cellArray[j] -> incGain();
					j_gain = _cellArray[j] -> getGain();
					// cout << "---" << endl;
					// reportBucket();
					// cout << "case4: increase gain of cell -> " << _cellArray[j]->getName() << endl;
					move_node_in_bucket(_cellArray[j] -> getNode(), j_gain-1, j_gain);	
					//reportBucket();
					// cout << "---" << endl;
					check_sum += 1;					
				}
				
			}
			if (check_sum > 1) cout << "something wrong in T_count" << endl;
		}
		// cout << "###### end of update #########" << endl;
	}
	
	//if ((max_gain_cell_flag == 0) && (base_cell->getNode() == _maxGainCell)) _maxGainCell = find_next_cell(get_cell_from_node(_maxGainCell))-> getNode();
	
	////cerr << "max gain cell after updated: " << get_cell_from_node(_maxGainCell) -> getName() << endl;
}

void Partitioner::partition(){
	//_maxAccGain = 0;
	vector<int> record_of_gain;// ***** need to be deleted *****
	int max_iter = 2;
	int new_gain = 0;
	int accBalance = 0; // acc balanceSub
	int last_balance_sub = 0;
	int fast_stop = 100; // unused for now
	_firsttime = 0;
	partition_init();
	_iterNum = 0;
	_moveStack.resize(_cellNum,0);
	int final_move;
	update_cut_size();
	int total_cut = _cutSize;
	int break_flag = 0;
	while( _iterNum < max_iter ){
		//cerr << "iter: " << _iterNum << endl;
		_bestMoveNum = -1;
		_maxAccGain = 0;
		int accgain = 0;
		last_balance_sub = 0;
		gain_init1();
		//reportBucket();
		final_move = -1;
		for( int move = 0; move < _cellNum; ++move){
			//cerr << "--- step: ---" << move << endl;
			if (_maxGainCell == NULL) { break_flag = 1;break;}
			// cerr << "max gain cell: " <<get_cell_from_node(_maxGainCell) -> getName() << endl;
			// //cerr << "gain of max gain: " << get_cell_from_node(_maxGainCell) -> getGain() << endl;
			
			Cell *base_cell = pick_base_cell();
			
			// if ( base_cell->getPart() == 0 ) { _balanceSub -= 1;  } else { _balanceSub += 1;} // part bigger then positive
			if (base_cell == NULL) { cout << "no appropriate cell at " << move << " th move" << endl;break_flag = 1;break; }
			
			if ( base_cell->getPart() == 0 ) { _balanceSub -= 1; _partSize[0]-=1; _partSize[1]+=1 ;} else { _balanceSub += 1; _partSize[0]+=1; _partSize[1]-=1; } // part bigger then positive
			
			// //cerr << "picked cell: " << base_cell -> getName() << endl;
			new_gain = base_cell -> getGain();
			// cout << "gain: " << new_gain << endl;
			record_of_gain.push_back(new_gain);
			accgain += new_gain;
			// cout << "acc Gain until step  " << move << " : " << accgain << endl;
			if ( (accgain > _maxAccGain) || ( (accgain == _maxAccGain) && (abs(last_balance_sub) > abs(_balanceSub)) ) ){
				_maxAccGain = accgain; 
				last_balance_sub = _balanceSub;
				_bestMoveNum = move;
				 ///////////////TODO
			}
			_moveStack[move] = base_cell;
			final_move = move;
			// cout << "stack move" << endl;
			update_gain(base_cell);
			
			// cout << "gain updated for step " << move <<endl;
			// reportCellGain() ;
			// reportCellPart() ;
			// reportBucket();
			//cerr << "--- end of step ---" << move << endl;
		}
		if (break_flag) final_move -= 1;
		//cerr << "here" << endl;
		for( int k = 0; k <= final_move; ++k){ // recover
		// //cerr << "k: " << k << endl;
			_moveStack[k] -> move();
		}
		// //cerr << "y0000000000000000" << endl;
		for( int k = 0; k <= _bestMoveNum; ++k){
			
			_moveStack[k] -> move();
		}
		reset_net_part();
		
		if (_maxAccGain == 0) {
			//cerr << "stop" << endl;
			//cerr << "=========== final result of iter " << _iterNum << ":" << endl;
			//cerr << "best move: " << _bestMoveNum << endl;
			//cerr << "max accgain: " << _maxAccGain << endl;
			total_cut -= _maxAccGain;
			//cerr << "current cutsize: " << total_cut << endl;
			update_cut_size();
			// //cerr << "record of gains: " ;
			// for (int i = 0; i < record_of_gain.size(); ++i) //cerr << record_of_gain[i] << " ";
			// //cerr << endl;
			//reportNetPart() ;
			if (_cutSize != total_cut) {cerr << "no!!! cutsize is " << _cutSize << endl; exit(0);}
			//cerr << "move step: " << _bestMoveNum << endl;
			
			record_of_gain.clear();
			//reportCellGain() ;
			//reportCellPart() ;
			//cerr << "==================================" << endl;
			break;
		}
		
		
		//////////////recover_blist();
		//cerr << "=========== final result of iter " << _iterNum << ":" << endl;
		//cerr << "max accgain: " << _maxAccGain << endl;
		total_cut -= _maxAccGain;
		//cerr << "current cutsize: " << total_cut << endl;
		update_cut_size();
		//reportNetPart() ;
		if (_cutSize != total_cut) {cerr << "no!!! cutsize is " << _cutSize << endl; exit(0);}
		//cerr << "move step: " << _bestMoveNum << endl;
		//cerr << "record of gains: " ;
		// for (int i = 0; i < record_of_gain.size(); ++i) cerr << record_of_gain[i] << " ";
		// cerr << endl;
		record_of_gain.clear();
		//reportCellGain() ;
		//reportCellPart() ;
		//cerr << "==================================" << endl;
		++_iterNum;
		
	}
	
	
	update_cut_size();
}

// do { // r pass if gain > 0
	// _iterNum = 0;
	// while(_iterNum < _cellNum){ // **** cut size ****
		// gain_init();
		// Cell *base_cell = pick_base_cell();
		// update_gain(base_cell);
		// ++_iterNum;
	// }
	// ++count;
	// } while ((_maxAccGain > 0) && count < max_iter);

void Partitioner::printSummary() const
{
    cout << endl;
    cout << "==================== Summary ====================" << endl;
    cout << " Cutsize: " << _cutSize << endl;
    cout << " Total cell number: " << _cellNum << endl;
    cout << " Total net number:  " << _netNum << endl;
    cout << " Cell Number of partition A: " << _partSize[0] << endl;
    cout << " Cell Number of partition B: " << _partSize[1] << endl;
    cout << "=================================================" << endl;
    cout << endl;
    return;
}

void Partitioner::reportNet() const
{
    cout << "Number of nets: " << _netNum << endl;
    for (size_t i = 0, end_i = _netArray.size(); i < end_i; ++i) {
        cout << setw(8) << _netArray[i]->getName() << ": ";
        vector<int> cellList = _netArray[i]->getCellList();
        for (size_t j = 0, end_j = cellList.size(); j < end_j; ++j) {
            cout << setw(8) << _cellArray[cellList[j]]->getName() << " ";
        }
        cout << endl;
    }
    return;
}

void Partitioner::reportCell() const
{
    cout << "Number of cells: " << _cellNum << endl;
    for (size_t i = 0, end_i = _cellArray.size(); i < end_i; ++i) {
        cout << setw(8) << _cellArray[i]->getName() << ": ";
        vector<int> netList = _cellArray[i]->getNetList();
        for (size_t j = 0, end_j = netList.size(); j < end_j; ++j) {
            cout << setw(8) << _netArray[netList[j]]->getName() << " ";
        }
        cout << endl;
    }
    return;
}

void Partitioner::writeResult(fstream& outFile)
{
    stringstream buff;
    buff << _cutSize;
    outFile << "Cutsize = " << buff.str() << '\n';
    buff.str("");
    buff << _partSize[0];
    outFile << "G1 " << buff.str() << '\n';
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        if (_cellArray[i]->getPart() == 0) {
            outFile << _cellArray[i]->getName() << " ";
        }
    }
    outFile << ";\n";
    buff.str("");
    buff << _partSize[1];
    outFile << "G2 " << buff.str() << '\n';
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        if (_cellArray[i]->getPart() == 1) {
            outFile << _cellArray[i]->getName() << " ";
        }
    }
    outFile << ";\n";
    return;
}

void Partitioner::reportCellGain() 
{
    cout << "Gain: "  << endl;
    for (size_t i = 0, end_i = _cellArray.size(); i < end_i; ++i) {
        cout << setw(8) << _cellArray[i]->getName() << ": ";
        cout << _cellArray[i]->getGain();
        cout << endl;
    }
    return;
}

void Partitioner::reportCellPart() 
{
    cout << "Part: " << endl;
    for (size_t i = 0, end_i = _cellArray.size(); i < end_i; ++i) {
        cout << setw(8) << _cellArray[i]->getName() << ": ";
        cout << _cellArray[i]->getPart();
        cout << endl;
    }
    return;
}

void Partitioner::reportBucket() 
{
    map<int, Node*>::const_iterator  iter;
	cout << "BucketA: " << endl;
    for(iter = _bList[0].begin(); iter != _bList[0].end(); iter++){
		if (iter->second != NULL) {
			cout<<"gain: " << iter->first<<" cell: "<<_cellArray[iter->second->getId()]->getName();	
			Node* temp = iter->second;
			while(temp->getNext() != NULL){
				cout<<" "<<_cellArray[temp->getNext()->getId()]->getName();
				temp = temp->getNext();
			}
			cout << endl;
			}
		else cerr << "gain: "<< iter->first<<" is NULL" << endl;
	}
	cout << "BucketB: " << endl;
	// for(iter = _bList[1].begin(); iter != _bList[1].end(); iter++){
		// cout<<"gain: " << iter->first<<" cell: "<<_cellArray[iter->second->getId()]->getName();
		// Node* temp = iter->second;
		// while(temp->getNext() != NULL){
			// cout<<" "<<_cellArray[temp->getNext()->getId()]->getName();
			// temp = temp->getNext();
		// }
		// cout << endl;
	// }
    return;
}

void Partitioner::clear()
{
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        delete _cellArray[i];
    }
    for (size_t i = 0, end = _netArray.size(); i < end; ++i) {
        delete _netArray[i];
    }
    return;
}

void Partitioner::reportNetPart() 
{
    cout << "NetPartCount: " << endl;
    for (auto net: _netArray) {
		cout << net -> getName() << ": " << endl;
        cout << setw(8) << net->getPartCount(0) << " ";
        cout << setw(8) << net->getPartCount(1) << " ";
        cout << endl;
    }
    return;
}

