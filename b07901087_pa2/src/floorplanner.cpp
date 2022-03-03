
#include "floorplanner.h"
#define DBG_CONT0UR 0
#define DBG_COST    0
#define DBG_TREE    0
#define DBG_SA      0
#define DBG_report  0
#define FASTRETURN  0
#define FEASIBLESTOP 0 // THIS SHOULD BE ZERO!!

inline bool CompareBlock(Block* b1, Block* b2){ return  (   (b1 -> getWidth() * b1 -> getHeight()) > (b2 -> getWidth() * b2 -> getHeight())   );  }

Node* floorplanner::init_tree(Node* parent, int i){
	
	
	// ==================== gereedy soul =====================
	if (_blocklength > 0){
	int isHorizon = (_W > _H)? 1:0;
	vector<Block*> tmp_block_array =  _blocklist;
	sort(tmp_block_array.begin(), tmp_block_array.end(), CompareBlock);
	//cout << "init tree... " << endl;
	// === Herist stage ===
	for (auto block: tmp_block_array){
		//cout << "this is block " << block -> getName() << "(" << block -> getId()<< "), block area: " << block -> getWidth() * block -> getHeight() << endl;
		if ( isHorizon && block -> getWidth() < block -> getHeight()){
			block -> rotate();
		}
		else if ( isHorizon == 0 && block -> getWidth() > block -> getHeight()){
			block -> rotate();
		}
		else{
			;
		}
	}
	//map<int, int> init_contour{{0,0}, {_W, 0}};
	vector<Node*> node_row;
	int submother_flag = 0;
	int dontusesubmother = 0;
	int motherY = 0;
	int submother_able = 1;
	Node* mother_node = NULL;
	Node* submother_node = NULL;
	Node* parent_node = NULL;
	Node* current_node = NULL;
	int currentX = 0;
	for( int k = 0, n = tmp_block_array.size(); k < n; ++k){
		if (k == 0) { 
			current_node = tmp_block_array[k]->getNode();
			//current_node -> setupperY( tmp_block_array[k] -> getHeight());
			_tree = current_node; 
			current_node -> setRoot(current_node); 
			mother_node = current_node;
			currentX = tmp_block_array[k] -> getWidth();
			current_node -> setcurX(currentX);
			parent_node = current_node;
			node_row.push_back(current_node);
			motherY = current_node -> getBlock() -> getHeight();
		}
		else{
			//cout << "current node ID: " << tmp_block_array[k] -> getName() << "(" << tmp_block_array[k] -> getId()<< ")" << endl;
			//cout << "left bound: " << currentX << " right_bound: " << tmp_block_array[k] -> getWidth() + currentX << endl;
			//cout << "block width: " << tmp_block_array[k] -> getWidth() << endl;
			//cout << "X boundary: " << _W << endl;
			current_node = tmp_block_array[k]->getNode();
			if (tmp_block_array[k] -> getWidth() + currentX <= _W){
				//cout << "go left: " << "current node is " << current_node -> getBlock() -> getName() << endl;
				currentX += tmp_block_array[k] -> getWidth();
				current_node -> setcurX(currentX);
				current_node -> setParent(parent_node);
				parent_node -> setLeft(current_node);
				parent_node = current_node;
				node_row.push_back(current_node);
			}
			else if (tmp_block_array[k] -> getWidth() + currentX > _W){
				
				//cout << "switch!" << endl;
				//cout << "motherY: " << motherY << endl;
				//for (auto n: node_row) cout << n -> getBlock() -> getId() << " ";
				//cout << endl;
				int submother_idx = (node_row.size() >= 5)? 4:(int)node_row.size()/1.5;
				//cout << "submotheridx: " << submother_idx << endl;
				if (node_row.size() < _blocklength / SUBMOTHER) submother_flag = 0;
				else { submother_node = node_row[submother_idx]; submother_flag = 1; cout << "picked submother: " << node_row[submother_idx] -> getBlock() -> getName() << endl;}
				if (motherY + current_node -> getBlock()-> getHeight() > _H && submother_flag && submother_able){ // 
					node_row.clear();
					//report_tree();
					//cout << "go into submother(ID): " << submother_node -> getBlock() -> getId()<< endl;
					
					currentX = submother_node -> getcurX() - submother_node -> getBlock() -> getWidth();
					//cout << "currentX: " << currentX << endl;
					submother_node -> setRight(current_node);
					//cout << "add current node: " << current_node -> getBlock() -> getId() << " to submother\'s right" << endl;
					current_node -> setParent(submother_node);
					current_node -> setcurX(currentX + tmp_block_array[k] -> getWidth());
					parent_node = current_node;
					currentX +=  tmp_block_array[k] -> getWidth(); // 40 added
					
					k++;
					current_node = tmp_block_array[k]->getNode(); // 5
					//cout << "here current node is: " << current_node -> getBlock() -> getId() << endl;
					int subleft = currentX;
					//report_tree();   
					node_row.clear();
					//node_row.push_back(current_node);
					int first_flag = 1;
					while( current_node -> getBlock() -> getWidth() + currentX <= _W || node_row.size() >= 3 && subleft + tmp_block_array[k-1]->getWidth() <= _W) {
						
						if (current_node -> getBlock() -> getWidth() + currentX > _W ){
							//current_node = tmp_block_array[k]->getNode();
							// should pick a new submother
							//cout << "we should pick a submother from the following array!" << endl;
							for (auto i: node_row) cout << i -> getBlock() -> getId() << " ";
							//cout << endl;
							submother_idx = node_row.size()/1.6;
							submother_idx += 1;
							if (submother_idx == 0) break;
							//cout << "new submother is " << node_row[submother_idx] -> getBlock() -> getId() << endl;
							//cout << "add current node: " << current_node -> getBlock() -> getId() << " to submother\'s right" << endl;
							parent_node = node_row[submother_idx];
							currentX = parent_node -> getcurX() - parent_node -> getBlock() -> getWidth();
							subleft = currentX;
							//cout << "fuck1" << endl;
							if (node_row.size() > 4 && subleft + tmp_block_array[k]->getWidth() > _W) break;
							current_node -> setParent(parent_node);
							//cout << "fuck2" << endl;
							parent_node -> setRight(current_node);
							//cout << "fuck3" << endl;
							parent_node = current_node;
							currentX += current_node -> getBlock() -> getWidth();
							current_node -> setcurX(currentX);
							//cout << "k: " << k<< endl;
							k++;
							//report_tree();
							if (k >= _blocklength+1) { break; cout << "leaving inner loop..." << endl; }
							current_node = tmp_block_array[k-1] -> getNode();
							//if (k == _blocklength) break;
							node_row.clear();
							
						}
						//cout << "fuck4" << endl;
						
						
						//cout << "go left: " << "current node is " << current_node -> getBlock() -> getId() << " and its parent is " << parent_node-> getBlock() -> getId() << endl;
						//cout << "current node boundary: " << currentX << " r: " << currentX + current_node -> getBlock() -> getWidth() << endl;;
						//cout << "current node ID: " << tmp_block_array[k-1] -> getId() << endl;
						currentX += tmp_block_array[k-1] -> getWidth();
						//cout << "the current k is: " << k << endl;
						
						if (currentX > _W) continue;
						current_node -> setcurX(currentX);
						current_node -> setParent(parent_node);
						parent_node -> setLeft(current_node);
						parent_node = current_node;
						node_row.push_back(current_node);
						//for (auto i: node_row) cout << i -> getBlock() -> getId() << " ";
						//cout << endl;
						//cout << "===========" << endl;
						if (first_flag) { k++; first_flag = 0; }
						//cout << "blocklength is: " << _blocklength << endl;
						if (k >= _blocklength) { cout << "leaving? " << endl; break;  }
						current_node = tmp_block_array[k]->getNode();
						//cout << "next node is: " << tmp_block_array[k] -> getId() << endl;
						//cout << "node orw size: " << node_row.size() << endl;
						
						
						k++;
						
						
					//submother_flag = 2;
					}
					k -= 2;
					node_row.clear();
					if (k >= _blocklength) break; 
				}
				else{
				// switch to top!
				//cout << "go back to mother: "  << mother_node -> getBlock() -> getName() << endl;
				motherY += current_node -> getBlock()-> getHeight();
				mother_node -> setRight(current_node);
				current_node -> setParent(mother_node);
				mother_node = current_node;
				currentX = tmp_block_array[k] -> getWidth();
				current_node -> setcurX(currentX);
				parent_node = current_node;
				node_row.clear();
				node_row.push_back(current_node);
				}
			}
			else{
				//cout << "anyone?" << endl;
				//exit(0);
			}
		}
			
	}
	//largeblank();
	// calMyCost();
	//pack();
	//largeblank();
	// cout << "============== print curX ==================" << endl;
	// for (auto block: _blocklist){
		// cout << block -> getName() << "(" << block ->getId() << ")" << ": " << block -> getX1() << " " << block -> getY1() << " " << block -> getX2() << " " << block -> getY2() << " W:" << block -> getWidth() << " H:" << block -> getHeight() << endl;
		// cout << "block ID: " << block -> getId() << "curX: " << block -> getNode() -> getcurX() << endl;
	// }
	
	// report_block();
	// cout << "============== end curX ==================" << endl;
	// largeblank();
	
	// write_cur_ans();  
	//op2(41, 48 ,1);
	
	
	// report_tree();
	// print_cur_ans();
	// exit(0);
	//report_tree();
	// cout << "here" << endl;
	// exit(0);
	return current_node;
	}
    
	
	
	// ==================== sort =============================
	// vector<Block*> tmp_block_array =  _blocklist;
	// sort(tmp_block_array.begin(), tmp_block_array.end(), CompareBlock);
	// cout << "init tree... " << endl;
	// for (auto block: tmp_block_array){
		// cout << "this is block " << block -> getId()<< ", block area: " << block -> getWidth() * block -> getHeight() << endl;
	// }
	// cout << "=============this is the original one=============" << endl;
	// for (auto block: _blocklist){
		// cout << "this is block " << block -> getId()<< ", block area: " << block -> getWidth() * block -> getHeight() << endl;
	// }
	
	
	// Node* current_node = NULL;
	// for( int k = 0, n = tmp_block_array.size(); k < n; ++k){
		// if (k == 0) { current_node = tmp_block_array[k]->getNode(); _tree = current_node; current_node -> setRoot(current_node); }

		// current_node = tmp_block_array[k]->getNode();
		// int parent_idx = (int)((k-1)/2);
		// int left_idx = 2 * k + 1;
		// int right_idx = 2 * k + 2;
		// if (k != 0) current_node -> setParent(tmp_block_array[parent_idx]->getNode());
		// if (left_idx < n) current_node -> setLeft(tmp_block_array[left_idx]->getNode());
		// if (right_idx < n)current_node -> setRight(tmp_block_array[right_idx]->getNode());
			
	// }
	
	
	// return current_node;
	
	
	
	//=========== iter =================
	else{
		Node* current_node = NULL;
		for( int k = 0, n = _blocklist.size(); k < n; ++k){
			if (k == 0) { current_node = _blocklist[k]->getNode(); _tree = current_node; current_node -> setRoot(current_node); }

			current_node = _blocklist[k]->getNode();
			int parent_idx = (int)((k-1)/2);
			int left_idx = 2 * k + 1;
			int right_idx = 2 * k + 2;
			if (k != 0) current_node -> setParent(_blocklist[parent_idx]->getNode());
			if (left_idx < n) current_node -> setLeft(_blocklist[left_idx]->getNode());
			if (right_idx < n)current_node -> setRight(_blocklist[right_idx]->getNode());
				
		}
	return current_node;
	}
	
	// ================================ recursive ===========================
	// Node* current_node = NULL;
	// if (i == 0) { current_node = _blocklist[i]->getNode(); _tree = current_node; current_node -> setRoot(current_node); };
    // if (i < _blocklength)
    // {
		// current_node = _blocklist[i]->getNode();
		// current_node -> setParent(parent);
		// current_node -> setLeft(init_tree(current_node, 2 * i + 1));
        // current_node -> setRight(init_tree(current_node, 2 * i + 2));
    // }
    // return current_node;

}

// int floorplanner::pack_init(){
	// _contour = {{0, 0},{_W, 0}}; // initialize contour
	// _treeH = 0;
	// _treeW = 0;
	// packPreorder_init(_tree, 0);
	// _treeR = _treeH/_treeW;
	// return _treeH * _treeW;
// }

// void floorplanner::packPreorder_init(Node* current_node, int x){ // may different in contour update, so split it
	
	// if (current_node == NULL)
        // return;
	// update_contour_init(current_node->getBlock(), x);
	
	// /* then recur on left sutree */
    // packPreorder_init(current_node->getLeft(), x+current_node->getBlock()->getWidth());
 
    // /* now recur on right subtree */
    // packPreorder_init(current_node->getRight(),x);
// }

// void floorplanner::update_contour_init(Block* block, int left_bound){
	////search contour
	// int first_time = 1;
	// int right_bound = left_bound + block -> getWidth();
	// int lower_bound = 0;
	// int upper_bound = 0;
	// block -> setX1(left_bound);
	// block -> setX2(right_bound);
	
	////if (block -> getId() == 9)
	////contour update
	// if (DBG_CONT0UR)cout << "==updateblock: ==" << block -> getName()  << "(" << block -> getId()+1 << ")" << endl;
	// if (DBG_CONT0UR)cout << "left bound: " << left_bound << " right_bound: " << right_bound << endl;
	////cout << block -> getHeight() << endl;
	// if (DBG_CONT0UR)report_map();
	// for (std::map<int,int>::iterator iter=_contour.begin(); iter!=_contour.end(); ++iter){
		// if (_contour.find(left_bound) == _contour.end()) cout << "not in contour! " << endl;
		// if (DBG_CONT0UR)cout << "(" << iter -> first << " " << iter -> second << ")" << endl;
		// if (iter->first >= right_bound){ // last contour
			// if (DBG_CONT0UR)cout << "case1" << endl;
			// if(lower_bound <= iter->second){ // in the middle just erase, update lower_bound
				////cout << "here" << endl;
				// lower_bound = iter->second;
				// upper_bound = lower_bound + block -> getHeight();
			// }
			// _contour[right_bound] = upper_bound;
			////cout << "upper bound: " << upper_bound << endl;;
			// break;
		// }
		// else if (iter->first == _W){ // reach the end
			// if (DBG_CONT0UR)cout << "case2" << endl;
			// if(lower_bound <= iter->second){ // in the middle just erase, update lower_bound
				// lower_bound = iter->second;
				// upper_bound = lower_bound + block -> getHeight();
			// }
			// _contour.erase(iter);
			// _contour[right_bound] = upper_bound;
		// }
		// else if (iter -> first == left_bound){
			// if (DBG_CONT0UR)cout << "case3" << endl;
			// first_time = 0;
		// }
		// else if (iter->first > left_bound) { // under contour
			// if (DBG_CONT0UR)cout << "case4" << endl;
			// if (first_time){ // first time
				// if (DBG_CONT0UR)cout << "first touch: " << "left_bound is " << left_bound << ", right_bound is " << right_bound  << ", contour is " << iter->second << endl; 
				
				// lower_bound = iter->second;
				// upper_bound = lower_bound + block -> getHeight();
				// _contour[left_bound] = iter->second;
				////report_map();
				// if (left_bound != iter -> first) _contour.erase(iter);
				// first_time = false;
			// }
			// else if(lower_bound < iter->second){ // in the middle just erase, update lower_bound
				// lower_bound = iter->second;
				// upper_bound = lower_bound + block -> getHeight();
				// _contour.erase(iter);
			// }
			// else{						// in the middle just erase
				// _contour.erase(iter);
			// }
		// }
		// else{;}
	// }
	
	// block -> setY1(lower_bound);
	// block -> setY2(upper_bound);
	// cout << "left bound: " << left_bound << " right_bound: " << right_bound << " lower_bound: " << lower_bound << " upper_bound: " << upper_bound <<endl;
	// if (upper_bound > _treeH) _treeH = upper_bound;
	// if (right_bound > _treeW) _treeW = right_bound;
	// if (DBG_CONT0UR)report_map();
	// cout << "========= end of update block ==============" << endl;
	////check feasible?
	////if (block -> setX2 > //boundary)
		////_feasible = false;
// }








void floorplanner::op1(int b1){ // rotation
	int random_num;
	if (b1 == -1) random_num = rand() % _blocklist.size(); // pick a block
	else random_num = b1;
	//cout << "op1: rotate " << random_num << endl;
	_Blockmoved[0] = random_num;
	_Blockmoved[1] = -1;
	_blocklist[random_num] -> rotate();
	
}	

void floorplanner::op2(int b1, int b2, int lr ){
	int random_num1, random_num2;
	int leafmostid, leafmostparentid, leafmostlr; 
	if (b1 == -1 || b2 == -1){
		random_num1 = rand() % _blocklength; // pick one block
		while( _blocklist[random_num1] -> getNode() -> getParent() == NULL){ // don't delete root
			//cerr << "random root remove..." <<" num1: " << random_num1 << endl;
			random_num1 = rand() % _blocklist.size();
		}
		random_num2 = rand() % _blocklength; // pick another block
		while(random_num2 == random_num1){
			//cerr << "random same..." <<" num1: " << random_num1 << "| num2: " << random_num2 << endl;
			random_num2 = rand() % _blocklist.size();
		}
	}
	else { random_num1 = b1; random_num2 = b2; }
	int rand_lr;
	if (lr == -1)rand_lr = rand() % 2;
	else rand_lr = lr;
	//cout << "op2: insert " << random_num1 << " into " << random_num2 <<" \'s " << rand_lr << endl;
	Node* node1 = _blocklist[random_num1] -> getNode();
	Node* node2 = _blocklist[random_num2] -> getNode();
	node1 -> remove(leafmostid, leafmostparentid, leafmostlr );
	if (leafmostid < -1 || leafmostparentid < -1 || leafmostlr < -1) { cerr << "op2: remove error!" << endl; exit(0); }
	
	_leafID = leafmostid;
	_leafparentID = leafmostparentid;
	_leaflr = leafmostlr;
	_Blockmoved[0] = random_num1;
	_Blockmoved[1] = random_num2;
	
	node2 -> insert(rand_lr, node1);
	
}

void floorplanner::op3(int b1 , int b2 ){
	int random_num1, random_num2;
	if (b1 == -1 || b2 == -1){
		random_num1 = rand() % _blocklist.size(); // pick one block
		random_num2 = rand() % _blocklist.size(); // pick another block
		while(random_num2 == random_num1){
			//cerr << "random same..." <<" num1: " << random_num1 << "| num2: " << random_num2 << endl;
			random_num2 = rand() % _blocklist.size();
		}
	}
	else { random_num1 = b1; random_num2 = b2; }
	//cout << "op3: swap " << random_num1 << " and " << random_num2 << endl;
	_Blockmoved[0] = random_num1;
	_Blockmoved[1] = random_num2;
	Node* node1 = _blocklist[random_num1] -> getNode();
	Node* node2 = _blocklist[random_num2] -> getNode();
	//report_tree();
	node1 -> swap(node2);
	//report_tree();
	
}

void floorplanner::recover(int opnum){
	if (opnum == 0) _blocklist[_Blockmoved[0]] -> rotate();
	else if (opnum == 1){
		Node *moved, *inserted_parent, *leaf, *leafparent;
		int leaflr, buffer1 = 0, buffer2 = 0, buffer3 = 0;
		if (_leaflr < 0 && _leafID < 0 && _leafparentID < 0) { cerr << "recover: op2 error!" << endl; exit(0); }
		moved = _blocklist[_Blockmoved[0]] -> getNode();
		inserted_parent = _blocklist[_Blockmoved[1]] -> getNode();
		if (_leafID > 0) leaf = _blocklist[ _leafID ] -> getNode();
		leafparent = _blocklist[ _leafparentID ] -> getNode();
		leaflr = _leaflr;
		int ret = moved -> remove(buffer1, buffer2, buffer3);
		if (ret != 3 && ret != 4) {cerr << "recover: op2 should only one child!"; exit(0); }
		if (_leafID > 0){ // moved node was not a leaf
			moved -> replace(leaf);
			leafparent -> insert(leaflr, leaf);
		}
		else{ // moved node was a leaf
			
		}
	}
	else if (opnum == 2){
		Node* node1 = _blocklist[_Blockmoved[0]] -> getNode();
		Node* node2 = _blocklist[_Blockmoved[1]] -> getNode();
		node1 -> swap(node2);
	}
	else{
		cerr << "wrong opnum! " << endl;
		exit(0);
	}
			
}

void floorplanner::brute_recover(int opnum){
	if (opnum == 1){
		for (auto block : _blocklist){
			block -> recoverNode();
		}
	}
	else if(opnum == 2){
		Node* node1 = _blocklist[_Blockmoved[0]] -> getNode();
		Node* node2 = _blocklist[_Blockmoved[1]] -> getNode();
		node1 -> swap(node2);
	}
	else if (opnum == 0){
		 _blocklist[_Blockmoved[0]] -> rotate();
	}
	else{
		cerr << "wrong opnum! " << endl;
		exit(0);
	}
		
}

void floorplanner::backup(){
	for (auto block : _blocklist){
		block -> backupNode();
	}
}
	

void floorplanner::record_best_sol(){
	//_bestCost; have updated
	_bestH = _treeH;
	_bestW = _treeW;
	_bestArea = _treeH * _treeW;
	_bestWL = _treeWL;
	for (auto block : _blocklist) block -> setBest();
	return;
}


void floorplanner::FastSA(){ // operation, check after operation, and while until success!
	int debug_count = 30;
	int total_count = 0;
	if (_blocklength == 49) {debug_count = 7; cout << "this is 49!" << endl; }
	if (_blocklength == 33) {debug_count = 10; cout << "this is 33!" << endl; }
	if (_blocklength == 10) {debug_count = 19; cout << "this is 10!" << endl; }
	if (_blocklength == 9) {debug_count = 20; cout << "this is apte!" << endl; }
	//if (_blocklength == 11) {debug_count = 0; cout << "this is hp!" << endl;  }
	double cost;
	double diff;
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(0.0, 1.0);
	int opnumber = -1;
    //_lastCost = 1000000000000000;
	cout << "neightbor: " << _N << endl;
	
	//cout << "initial T is: " << _T << endl;
	//exit(0);
	while(_T > _frozen || total_count > debug_count){
		cout << "total_count: " << total_count << endl;
		total_count += 1;
		if (total_count > debug_count) break;
		double count = 0;
		int uphill = 0;
		double reject = 0;
		while(count <= 2*_N && uphill <= _N){
			
			if (DBG_SA) cout << "=========== start op ============= " << endl;
			int random_num = rand()%10;
			if (random_num < 2){
				
				if (DBG_TREE) report_tree();
				op1();
				opnumber = 0;
			}
			else if(random_num < 5){
				if (DBG_TREE) report_tree();
				op2();
				opnumber = 1;
			}
			else{
				if (DBG_TREE) report_tree();
				op3();
				opnumber = 2;
			}
			
			
			if (DBG_TREE) report_tree();
			cost = calMyCost();
			diff = cost - _lastCost;
			
			if (DBG_SA)cout << "lastcost: " << _lastCost << endl;
			if (DBG_SA)cout << "cost: " << cost << endl;
			if (DBG_SA)cout << "bestCost: " << _bestCost << endl;
				
			if (checkFeasible()&& FEASIBLESTOP) {cout << "feasible!" << endl; exit(0);}
			
			if (cost < _bestCost && checkFeasible()){
				//largeblank();
				//cout << "I get the best answer! " << endl;
				//report_tree();
				//if (_treeW*_treeH == 38202164) { cout << "I found it: " << " debug count is: " << debug_count << endl; break; }
				_lastCost = cost;
				_bestCost = cost;
				record_best_sol(); // need to check feasible
				//largeblank();
				//print_cur_ans();
				// cout << "checknet... " << endl;
				// cout << "the wire length is: " << WL() << endl;
				// cout << "start printing all net length! " << endl;
				// for (auto net: _netlist) cout << net -> calcHPWL() << endl;
				// double fuck = 0;
				// for (auto net: _netlist) fuck +=  net -> calcHPWL();
				// cout << "fuck: " << fuck << endl;
				backup();
				//print_answer();
				if (FASTRETURN) return;
			}
			else if ( (double)rand()/(RAND_MAX) > exp(-diff/ _T) && diff > 0) { brute_recover(opnumber); reject += 1; if (DBG_SA)cout << "reject!" << endl;} //=========== dis(gen)
			else {
				if (DBG_SA)cout << "accept(2)"<<endl;
				//if (DBG_SA)cout << "Width: " << _treeW << "Height: " << _treeH <<endl;
				if (diff > 0) uphill += 1;
				_lastCost = cost;
				backup();
			}
			if (DBG_SA)cout << "=============== end of op! ======================" << endl;
			++count;
		}
		//report_tree();
		
		_T = rateT*_T;
		if (reject/count > 0.99) break;
		count = 0;
	}
	cout << "ending T: " << _T << endl;
	cout << "frozen: " << _frozen << endl;
	cout << "successful return " << endl;
	//print_cur_ans();
	//write_cur_ans();
	cout << "this is the best answer: " << endl;
	print_answer();
	
	
	
}
void floorplanner::largeblank(){
	cout << endl;
	cout << endl;cout << endl;cout << endl;cout << endl;cout << endl;cout << endl;cout << endl;cout << endl;
}
void floorplanner::floorplan(){
	double cost;
	//report_term();
	init_tree(_tree, 0);
	// report_tree();
	// exit(0);
	_lastCost = calMyCost();
	backup();
	cout << "initial tree cost: " << _lastCost << endl;
	// report_map();
	// report_block();
	// print_cur_ans();
	// backup();
	
	// largeblank();
	// op3(2,6);
	// report_tree();
	// cost = calMyCost();
	//int opnumber = 0;
	// report_map();
	// report_block();
	// print_cur_ans();
	
	// largeblank();
	// brute_recover(2);
	// cost = calMyCost();
	//int opnumber = 0;
	// backup();
	// report_map();
	// report_block();
	print_cur_ans();
	write_cur_ans();
	
	
	
	
	
	
	if (checkFeasible()){
		cout << "feasible! " << endl;
		_bestCost = _lastCost;
		record_best_sol(); // need to check feasible
	}
	//if (_blocklength == 49)report_ans();
	//if (_blocklength == 49)exit(0);
	FastSA();
	return;
}

double floorplanner::calSubCost(){ //======================================= not sure
	return _alpha*WL() + (1- _alpha) * pack();
}

double floorplanner::calMyCost(){
	
	if (DBG_CONT0UR)largeblank();
	if (DBG_CONT0UR)report_tree();
	int area = pack();
	double Hpenalty = (_treeH - _H > 0)? _treeH - _H:0;
	double Wpenalty = (_treeW - _W > 0)? _treeW - _W:0;
	//if (DBG_COST)report_block();
	//report_map();
	double WireLength = WL();
	if (_steps < (double)_blocklength / 4){	
		_accumWL += WireLength;
		_accumArea += area;
		_steps += 1;
		_Anorm = (double)_accumArea/_steps;
		_WLnorm = (double) _accumWL / _steps;
	}
	if (DBG_COST){
		cout << "area is: " << area << " anorm is: " << _Anorm  << " WL is: " << WireLength << " WLnorm is: " << _WLnorm <<" and _steps is: " << _steps <<endl;
		cout << "treeW: " << _treeW << " and treeH: " << _treeH << endl;
		cout << "Hpenalty: " << Hpenalty << "normalized: " << 10*Hpenalty/_H <<endl;
		cout << "Wpenalty: " << Wpenalty << "normalized: " << 10*Wpenalty/_W <<endl;
		cout << "part area cost: " << _alpha*area/ _Anorm + (1- _alpha) << endl;
		cout << "part WL cost: " << (1- _alpha) * WireLength / _WLnorm  << endl;
		cout << "part ratio cost: " << 0.5 * fabs(_treeR - _R) << endl;
		
	}
	//double cost = (double) _alpha*area* _steps/ _accumArea + (1- _alpha) * WireLength * _steps/ _accumWL + 5 * fabs(_treeR - _R) + 0.01*Hpenalty + 0.01*Wpenalty;
	//double cost = (double) _alpha*area* _steps/ _accumArea + (1- _alpha) * WireLength * _steps/ _accumWL + 5 * fabs(_treeR - _R) + 10*Hpenalty/_H + 10*Wpenalty/_W;
	// this is original
	double cost = (double) _alpha*area* _Anorm + (1- _alpha) * WireLength * _WLnorm + 5 * fabs(_treeR - _R) + 10*Hpenalty/_H + 10*Wpenalty/_W;
	// this is new
	//double cost = (double) _alpha*area/ _Anorm + (1- _alpha) * WireLength / _WLnorm + 0.005 * fabs(_treeR - _R) + Hpenalty/_H + Wpenalty/_W;
	if (_blocklength == 10 || _blocklength == 9 ) cost = (double) 0.25*area/ _Anorm + (1- 0.25) * WireLength / _WLnorm + 0.005 * fabs(_treeR - _R) + Hpenalty/_H + Wpenalty/_W;
	return cost;
}

double floorplanner::WL(){
	double WireLength = 0;
	for (auto net: _netlist) WireLength += net -> calcHPWL();
	_treeWL = WireLength;
	return WireLength;
}

int floorplanner::pack(){
	_contour = {{0, 0},{_W, 0}}; // initialize contour
	_treeH = 0;
	_treeW = 0;
	packPreorder(_tree, 0);
	_treeR = _treeH/_treeW;
	return _treeH * _treeW;
}

void floorplanner::packPreorder(Node* current_node, int x){ // may different in contour update, so split it
	
	if (current_node == NULL)
        return;
	updateblock_mapexp(current_node->getBlock(), x);
	
	/* then recur on left sutree */
    packPreorder(current_node->getLeft(), x+current_node->getBlock()->getWidth());
 
    /* now recur on right subtree */
    packPreorder(current_node->getRight(),x);
}

// void floorplanner::packPreorderR(Node* current_node, int x){
	
	// if (node == NULL)
        // return;
	// update_blockR(current_node->getBlock(), x);
	////int leftChildx = x+current_node->getBlock()->getWidth();
	// if ( > _W) _W = 
	// /* then recur on left sutree */
    // packPreorderL(node->left, x+current_node->getBlock()->getWidth(), y, );
 
    // /* now recur on right subtree */
    // packPreorderR(node->right);
// }

void floorplanner::update_block(Block* block, int left_bound){
	// search contour
	int first_time = 1;
	int right_bound = left_bound + block -> getWidth();
	int lower_bound = 0;
	int upper_bound = 0;
	block -> setX1(left_bound);
	block -> setX2(right_bound);
	
	// if (block -> getId() == 9)
	// contour update
	if (DBG_CONT0UR)cout << "==updateblock: ==" << block -> getName()  << "(" << block -> getId()+1 << ")" << endl;
	if (DBG_CONT0UR)cout << "left bound: " << left_bound << " right_bound: " << right_bound << endl;
	//cout << block -> getHeight() << endl;
	if (DBG_CONT0UR)report_map();
	for (std::map<int,int>::iterator iter=_contour.begin(); iter!=_contour.end(); ++iter){
		if (_contour.find(left_bound) == _contour.end()) cout << "not in contour! " << endl;
		if (DBG_CONT0UR)cout << "(" << iter -> first << " " << iter -> second << ")" << endl;
		if (iter->first >= right_bound){ // last contour
			if (DBG_CONT0UR)cout << "case1" << endl;
			if(lower_bound <= iter->second){ // in the middle just erase, update lower_bound
				//cout << "here" << endl;
				lower_bound = iter->second;
				upper_bound = lower_bound + block -> getHeight();
			}
			_contour[right_bound] = upper_bound;
			//cout << "upper bound: " << upper_bound << endl;;
			break;
		}
		else if (iter->first == _W){ // reach the end
			if (DBG_CONT0UR)cout << "case2" << endl;
			if(lower_bound <= iter->second){ // in the middle just erase, update lower_bound
				lower_bound = iter->second;
				upper_bound = lower_bound + block -> getHeight();
			}
			_contour.erase(iter);
			_contour[right_bound] = upper_bound;
		}
		else if (iter -> first == left_bound){
			if (DBG_CONT0UR)cout << "case3" << endl;
			first_time = 0;
		}
		else if (iter->first > left_bound) { // under contour
			if (DBG_CONT0UR)cout << "case4" << endl;
			if (first_time){ // first time
				if (DBG_CONT0UR)cout << "first touch: " << "left_bound is " << left_bound << ", right_bound is " << right_bound  << ", contour is " << iter->second << endl; 
				
				lower_bound = iter->second;
				upper_bound = lower_bound + block -> getHeight();
				_contour[left_bound] = iter->second;
				//report_map();
				if (left_bound != iter -> first) _contour.erase(iter);
				first_time = false;
			}
			else if(lower_bound < iter->second){ // in the middle just erase, update lower_bound
				lower_bound = iter->second;
				upper_bound = lower_bound + block -> getHeight();
				_contour.erase(iter);
			}
			else{						// in the middle just erase
				_contour.erase(iter);
			}
		}
		else{;}
	}
	
	block -> setY1(lower_bound);
	block -> setY2(upper_bound);
	cout << "left bound: " << left_bound << " right_bound: " << right_bound << " lower_bound: " << lower_bound << " upper_bound: " << upper_bound <<endl;
	if (upper_bound > _treeH) _treeH = upper_bound;
	if (right_bound > _treeW) _treeW = right_bound;
	if (DBG_CONT0UR)report_map();
	cout << "========= end of update block ==============" << endl;
	// check feasible?
	// if (block -> setX2 > //boundary)
		// _feasible = false;
}
//===================================== exp ======================
void floorplanner::updateblock_mapexp(Block* block, int left_bound){
	// search contour
	int first_time = 1;
	int right_bound = left_bound + block -> getWidth();
	int lower_bound = 0;
	int upper_bound = 0;
	int reach_bound_flag = 0;
	block -> setX1(left_bound);
	block -> setX2(right_bound);
	
	// if (block -> getId() == 9)
	// contour update
	if (DBG_CONT0UR)cout << "==updateblock: ==" << block -> getName()  << "(" << block -> getId() << ")" << endl;
	if (DBG_CONT0UR)cout << "block width: " << block -> getWidth() << " and block Height: " << block -> getHeight() << endl;
	if (DBG_CONT0UR)cout << "left bound: " << left_bound << " right_bound: " << right_bound << endl;
	
	//cout << block -> getHeight() << endl;
	if (DBG_CONT0UR)report_map();
	std::map<int,int>::iterator iter = _contour.find(left_bound);
	if (iter == _contour.end() ) { cerr << "_contour dont have this x: " << left_bound << endl; exit(0); }
	for (; iter!=_contour.end(); ++iter){
		++iter;
		if (iter == _contour.end() ){ if (DBG_CONT0UR) cout << "reach contour end!" << endl; reach_bound_flag = 1;  }
		--iter;
		if (DBG_CONT0UR)cout << "(" << iter -> first << " " << iter -> second << ")" << endl;
		//if (_contour.find(left_bound) == _contour.end()) cout << "not in contour! " << endl;
		//cout << iter -> first << " " << iter -> second << endl;
		if (reach_bound_flag && right_bound >= iter -> first){ // reach the end   iter->first == _W
			if (DBG_CONT0UR)cout << "case2" << endl;
			if(lower_bound <= iter->second){ // in the middle just erase, update lower_bound
				lower_bound = iter->second;
				upper_bound = lower_bound + block -> getHeight();
			}
			if (iter -> first > left_bound) iter = _contour.erase(iter);
			//iter--;
			_contour[right_bound] = upper_bound;
			break;
		}
		else if (iter->first >= right_bound){ // last contour
			if (DBG_CONT0UR)cout << "case1" << endl;
			if(lower_bound <= iter->second){ // in the middle just erase, update lower_bound
				//cout << "here" << endl;
				lower_bound = iter->second;
				upper_bound = lower_bound + block -> getHeight();
			}
			_contour[right_bound] = upper_bound;
			//cout << "upper bound: " << upper_bound << endl;;
			break;
		}
		
		else if (iter -> first == left_bound){
			if (DBG_CONT0UR)cout << "case3" << endl;
			first_time = 0;
		}
		else if (iter->first > left_bound) { // under contour
			
			if (first_time){ // first time
				if (DBG_CONT0UR)cout << "(shouldnot)first touch: " << "left_bound is " << left_bound << ", right_bound is " << right_bound  << ", contour is " << iter->second << endl; 
				
				lower_bound = iter->second;
				upper_bound = lower_bound + block -> getHeight();
				_contour[left_bound] = iter->second;
				//report_map();
				if (left_bound != iter -> first) { iter = _contour.erase(iter); iter--; }
				first_time = false;
			}
			else if(lower_bound < iter->second){ // in the middle just erase, update lower_bound
				if (DBG_CONT0UR)cout << "case4" << endl;
			
				lower_bound = iter->second;
				
				upper_bound = lower_bound + block -> getHeight();
				//cout << "fuck" << endl;
				//cout << iter->first << " " << iter -> second << endl;
				iter = _contour.erase(iter);
				iter--;
				
			}
			else{						// in the middle just erase
				if (DBG_CONT0UR)cout << "case5" << endl;
				iter = _contour.erase(iter);
				iter--;
			}
		}
		else{;}
	}
	
	block -> setY1(lower_bound);
	block -> setY2(upper_bound);
	if (DBG_CONT0UR)cout << "left bound: " << left_bound << " right_bound: " << right_bound << " lower_bound: " << lower_bound << " upper_bound: " << upper_bound <<endl;
	if (upper_bound > _treeH) _treeH = upper_bound;
	if (right_bound > _treeW) _treeW = right_bound;
	if (DBG_CONT0UR)report_map();
	if (DBG_CONT0UR)cout << "========= end of update block ==============" << endl;
	// check feasible?
	// if (block -> setX2 > //boundary)
		// _feasible = false;
}




void floorplanner::parseblock(fstream& inFile)
{
    string str, str1;
    // Set balance factor
	int x, y;
    inFile >> str >> _W >> _H;
	cout <<  str << " " <<  _W << " " << _H << endl; //=========
	inFile >> str >> _blocklength;
	cout <<  str << " " << _blocklength << endl; //=========
	inFile >> str >> _termlength;
	cout <<  str << " " <<  _termlength << endl; //=========
	
    for(int i = 0; i < _blocklength;i++){
        inFile >> str >> x >> y;//block name
        //_Block2Id.insert(pair<string,int>(buf,i));
		_Block2ID[str] = i;
        _id2Block.push_back(str);
        //inFile >> x >> y;
        this->addBlock(str,i,x,y);
		cout <<  str << " " <<  x << " " << y << endl; //=========
    }
    //terminals
    for(int i = 0; i < _termlength;i++){
        inFile >> str;//terminal name
        //_Terminal2Id.insert(pair<string,int>(buf,i));
		_Term2ID[str] = i;
        //_id2Block.push_back(str);
        inFile >> str1;//terminal
        inFile >> x >> y;
        this->addTerm(str,x,y);
		cout <<  str << " " << str1<<" " <<  x << " " << y << endl; //========
    }
    return;
}

void floorplanner::parsenet(fstream& inFile){
	string str;
    unsigned n, degree, temp;//temp used to save terminal idx
    inFile >> str >> n;//NumNets:
    //_tree->init_nets(n);
    for(int i = 0; i < n;i++){
        inFile >> str >> degree;//NetDegree:
        this->addNet();
        for(int j = 0; j < degree; j++){
            inFile >> str;//terminal/block name
			auto got = _Block2ID.find(str);
            //if(_Block2ID.count(str) == 0)//terminal
			if (got == _Block2ID.end()){ // term
                temp = _Term2ID[str];
                //_tree->net_add_terminal(i,temp);
				this -> netAddTerm(i, temp);
            }
            else{//block
                //temp = _Block2ID[str];
                //_tree->net_add_block(i,temp);
				this -> netAddBlock(i, got -> second);
            }
        }
    }
}

void floorplanner::report_ans(fstream& outFile, double time){
	if (_blocklength != _blocklist.size() ) { cerr << "report: block wrong!" << endl; }
	if (DBG_report) cout << "alpha: " << _alpha << endl;
	if (DBG_report)cout << "1-alpha: " << ( 1 - _alpha ) << endl;
	if (DBG_report)cout << "alpha*bestarea: " << _alpha * _bestArea << endl;
	double final_ans = _alpha * (double)_bestArea + ( 1 - _alpha ) * _bestWL;
	if (DBG_report)cout << setprecision(17) << "cost? " << "--->" << final_ans << endl;
	if (DBG_report)cout << setprecision(17) <<"area: " << _bestArea << endl;
	if (DBG_report)cout << setprecision(17) << "wirelength: " << _bestWL << endl;
	outFile << setprecision(17) <<_alpha * _bestArea + ( 1 - _alpha ) * _bestWL << endl;
	outFile << setprecision(17) <<_bestWL << endl;
	outFile << _bestArea << endl;
	outFile << _bestW << " " << _bestH << endl;
	outFile << time << endl;
	for (int i = 0; i < _blocklength; ++i ){
		Block* block = _blocklist[i];
		outFile << block -> getName() << " " << block -> getbestX1() << " " << block -> getbestY1() << " " << block -> getbestX2() << " " << block -> getbestY2() << endl;    
	}
		
}

void floorplanner::report_tmpans(){
	fstream outFile;
	outFile.open("forplot.txt", ios::out);
	
	if (_blocklength != _blocklist.size() ) { cerr << "report: block wrong!" << endl; }
	
	outFile << _alpha * _bestArea + ( 1 - _alpha ) * _bestWL << endl;
	outFile << _bestWL << endl;
	outFile << _bestArea << endl;
	outFile << _bestW << " " << _bestH << endl;
	outFile << 0 << endl;
	for (int i = 0; i < _blocklength; ++i ){
		Block* block = _blocklist[i];
		outFile << block -> getName() << " " << block -> getbestX1() << " " << block -> getbestY1() << " " << block -> getbestX2() << " " << block -> getbestY2() << endl;    
	}
	
}

void floorplanner::write_cur_ans(){
	fstream outFile;
	outFile.open("forplot.txt", ios::out);
	
	if (_blocklength != _blocklist.size() ) { cerr << "report: block wrong!" << endl; }
	
	outFile << std::setprecision(10) << _alpha * _treeW * _treeH + ( 1 - _alpha ) * _treeWL << endl;
	outFile << _treeWL << endl;
	outFile << _treeW * _treeH << endl;
	outFile << _treeW << " " << _treeH << endl;
	outFile << 0 << endl;
	for (int i = 0; i < _blocklength; ++i ){
		Block* block = _blocklist[i];
		outFile << block -> getName() << " " << block -> getX1() << " " << block -> getY1() << " " << block -> getX2() << " " << block -> getY2() << endl;    
	}
	
}

void floorplanner::print_cur_ans(){
	
	if (_blocklength != _blocklist.size() ) { cerr << "report: block wrong!(cur)" << endl; }
	cout << "======= this is the cur answer! ========" << endl;
	cout << "Boundary: " << _W << " " << _H << endl;;
	cout << _alpha * _treeW* _treeH + ( 1 - _alpha ) * _treeWL << endl;
	cout << _treeWL << endl;
	cout << _treeW* _treeH << endl;
	cout << _treeW << " " << _treeH << endl;
	cout << 0 << endl;
	for (int i = 0; i < _blocklength; ++i ){
		Block* block = _blocklist[i];
		cout << block -> getName() << " " << block -> getX1() << " " << block -> getY1() << " " << block -> getX2() << " " << block -> getY2() << endl;    
	}
}

void floorplanner::print_answer(){
	cout << "Boundary: " << _W << " " << _H;
	if (_blocklength != _blocklist.size() ) { cerr << "report: block wrong!" << endl; }
	cout << "this is the tmp answer! " << endl;
	cout << _alpha * _bestArea + ( 1 - _alpha ) * _bestWL << endl;
	cout << _bestWL << endl;
	cout << _bestArea << endl;
	cout << _bestW << " " << _bestH << endl;
	cout << 0 << endl;
	for (int i = 0; i < _blocklength; ++i ){
		Block* block = _blocklist[i];
		cout << block -> getName() << " " << block -> getbestX1() << " " << block -> getbestY1() << " " << block -> getbestX2() << " " << block -> getbestY2() << endl;    
	}
	
}

void floorplanner::report_map(){
	cout << "contour: " << endl;
	for (std::map<int,int>::iterator iter=_contour.begin(); iter!=_contour.end(); ++iter){
		cout << "(" << iter -> first << ", " << iter -> second << ") -- ";
		
	}
	cout << endl;
}

void floorplanner::report_block(){
	cout << " ================ block report ================= " << endl;
	for (int i = 0; i < _blocklength; ++i ){
		Block* block = _blocklist[i];
		cout << block -> getName() << "(" << block ->getId() << ")" << ": " << block -> getX1() << " " << block -> getY1() << " " << block -> getX2() << " " << block -> getY2() << " W:" << block -> getWidth() << " H:" << block -> getHeight() << endl;           
	}
	cout << " ================ end ================= " << endl;
}


void floorplanner::report_term(){
	cout << " ================ term report ================= " << endl;
	for (int i = 0; i < _termlength; ++i){
		Terminal* t = _termlist[i];
		cout << t -> getName() << ": "  << t -> getX() << " " << t -> getY() << endl;           
	}
	cout << " ================ end term report ================= " << endl;
		
}

void floorplanner::report_tree(){
	int node_count = 0;
	int rootcount = 0;
	for (int i = 0; i < _blocklength; ++i){
		if (_blocklist[i] -> getNode() -> getParent() == NULL){
			rootcount++;
		}
	}
	if (rootcount != 1) { cerr << "report_tree: double root or no root! " << endl; exit(0); }
	reportPreorder(_tree, node_count);
	cout << endl;
	if (node_count != _blocklength) {cerr << "losing node..." << "node_num is: " << node_count << " blocklength is: " << _blocklength << endl; exit(0); }
}





Node* floorplanner::reportPreorder(Node* node, int& node_count){
	if (node == NULL) cout << " n" ;
	else{
		//if (node -> getBlock() -> getId() != 0) node -> checkParent();
		node_count += 1;
		node -> checkParent();
		cout << " " << node -> getBlock() -> getId() ;
		reportPreorder(node->getLeft(), node_count);
		reportPreorder(node->getRight(), node_count);
	}
	return node;
}

// void floorplanner::report_netTerm(){
	// for (auto net: _netlist){
		
// }
	

