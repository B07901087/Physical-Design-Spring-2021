#include "module.h"
#define REMOVE_DBG 0
#define INSERT_DBG 0
#define WLDEBUG    0

using namespace std;

//=====================================================================================
// Class Node

//=====================================================================================
#include "module.h"



Node* Node::_root;

void Node::checkParent(){
	if (_parent == NULL) cout << "node: " << this -> getBlock() -> getId() << " is a root! " << endl;
	else if (_parent -> getLeft() == this){
		return;
	}
	else if (_parent -> getRight() == this){
		return;
	}
	else{
		cout << "find a no parent node: " << this -> getBlock() -> getId() << "and its parent is: " << _parent -> getBlock() -> getId() << endl;
		exit(0);
	}
}
int Node::isLeft(){
	int isLeft;
	if (_parent == NULL && _root == this){ // root
		isLeft = 2;
	}
	else if (_parent -> getLeft() == this){
		isLeft = 1;
	}
	else if (_parent -> getRight() == this){
		isLeft = 0;
	}
	else{
		return -1;
	}
	return isLeft;
}

int Node::replace(Node* target){
	// connect to other
	this -> setParent(target->getParent());
	this -> setLeft(target->getLeft());
	this -> setRight(target -> getRight());
	 // other 3 connect to it
	 //parent
	int isLeft = target -> isLeft();
	if (isLeft == -1) {cerr << "replace: orphan!" << endl; }
	if (isLeft == 2) { cerr << "replace: didnnot handle root yet! " << endl; exit(0);}// ============================
	if (isLeft == 1) target->getParent()->setLeft(this);
	else if (isLeft == 2) { _root = this; this -> setParent(NULL); }
	else target->getParent()->setRight(this);
	if (target -> getLeft() != NULL) target -> getLeft() -> setParent(this);
	if (target -> getRight() != NULL) target -> getRight() -> setParent(this);
	
	// clean the target
	target -> setParent(NULL);
	target -> setLeft(NULL);
	target -> setRight(NULL);
	
	return 0;
	
}
	

void Node::insert(int lr, Node* child){
	if (child -> getRight() != NULL || child -> getLeft() != NULL || child-> getParent() != NULL) { cerr << "insert: dirty child!" << endl; exit(0); }
	int random_num = rand()%2;
	if (lr == 0){ // left
		if (_left != NULL){ // if parent have child
			if (random_num){ // put origin on the left
				child -> setLeft(_left);
				_left -> setParent(child);
				_left = child;
				child -> setParent(this);
				if (INSERT_DBG) cout << "insert: original child move to left" << endl;
			}
			else{ // put  origin on the right
				child -> setRight(_left);
				_left -> setParent(child);
				_left = child;
				child -> setParent(this);
				if (INSERT_DBG) cout << "insert: original child move to right" << endl;
			}
		}else{
			_left = child;
			child -> setParent(this);
			if (INSERT_DBG) cout << "insert: no original chind for parent" << endl;
		}
	}
	else{
		if (_right != NULL){ 
			if (random_num){ // put origin on the left
				child -> setLeft(_right);
				_right -> setParent(child);
				_right = child;
				child -> setParent(this);
				if (INSERT_DBG) cout << "insert: original child move to left" << endl;
			}
			else{ // put  origin on the right
				child -> setRight(_right);
				_right -> setParent(child);
				_right = child;
				child -> setParent(this);
				if (INSERT_DBG) cout << "insert: original child move to right" << endl;
			}
		}else{
			_right = child;
			child->setParent(this);
			if (INSERT_DBG) cout << "insert: no original chind for parent" << endl;
		}
		
	}
}

int Node::remove(int& leafmostid, int& leafmostparentid, int& leafmostlr){
	int isLeft;
	int leftwarn = 0;
	int rightwarn = 0;
	int buffer1 = 0, buffer2 = 0, buffer3 = 0;
	leafmostid = -1;
	leafmostparentid = -1;
	leafmostlr = -1;
	isLeft = this -> isLeft();
	if (isLeft == -1){
		cout << "remove: no parent child!" << endl;
		cout << "parent ID: " << _parent -> getBlock() -> getId() << endl;
		exit(0);
	}
	if (isLeft == 2){
		cout << "remove root didnot handle yet!" << endl;
		exit(0);
	}
	
	
	if (_left == NULL && _right == NULL){ // leaf
		if (isLeft == 1) _parent -> setLeft(NULL);  
		else if (isLeft == 2) { cerr << "the tree only one node!" << endl; _root = NULL; }
		else  _parent -> setRight(NULL);
		_left = NULL;
		_right = NULL;
		_parent = NULL;
		return 1;
	}
	else if(_left != NULL && _right != NULL){ // both child --> get leafmost to insert here 
		if (REMOVE_DBG)cout << "insert: two child" << endl;
		int firstlr = rand()%2; //=========================================
		Node* leafmost = (firstlr)? _right: _left;
		Node* leafmostparent = this;
		while(leafmost -> getRight() != NULL || leafmost -> getLeft() != NULL){
			leafmostparent = leafmost;
			if (firstlr == 0){
				if (leafmost -> getLeft() != NULL){
					leafmost = leafmost -> getLeft();
					leafmostlr = 0;
				}
				else{
					leafmost = leafmost -> getRight();
					leafmostlr = 1;
				}
			}
			else{
				if (leafmost -> getRight() != NULL){
					leafmost = leafmost -> getRight();
					leafmostlr = 1;
				}
				else{
					leafmost = leafmost -> getLeft();
					leafmostlr = 0;
				}
			}
		}
		leafmostparentid = leafmostparent -> getBlock() -> getId(); // rocode replace node
		leafmostid = leafmost -> getBlock() -> getId();
		if (REMOVE_DBG)cout << "pick " << leafmostid << " to replace it!" << endl;
		if (leafmost == _right) rightwarn = 1;
		if (leafmost == _left) leftwarn = 1;
		int ret = leafmost -> remove(buffer1, buffer2, buffer3);
		if (ret != 1){
			cerr << "this is not leafmost node!" << endl;
			exit(0);
		}
		//parent -> insert(isLeft, leafmost);
		
		// parent
		if (isLeft == 1) _parent -> setLeft(leafmost); 
		else if (isLeft == 2) _root = leafmost;
		else _parent -> setRight(leafmost);
		
		// leafmost
		if (isLeft == 2) leafmost -> setParent(NULL);
		else leafmost -> setParent(_parent);
		leafmost -> setLeft( _left);
		leafmost -> setRight( _right);
		if (rightwarn) { if (_right != NULL ) { cerr << "error in rightwarn! " << endl; exit(0); } }
		if (leftwarn)  { if (_left != NULL ) { cerr << "error in leftwarn! " << endl; exit(0); } }
		
		// child
		if (!leftwarn) _left -> setParent(leafmost);
		if (!rightwarn) _right -> setParent(leafmost);
		_left = NULL;
		_right = NULL;
		_parent = NULL;
		return 2;
		
	}
	else if(_left != NULL){ // left child
		if (isLeft){ 
			if (isLeft == 2){ _root = _left; _left -> setParent(NULL); }
			else{
				_parent -> setLeft(_left); 
				_left -> setParent(_parent);
			}
			
		}
		else{ 
			_parent -> setRight(_left); 
			_left -> setParent(_parent);
		}
		leafmostparentid = this -> getBlock() -> getId(); // rocode replace node
		leafmostid = _left -> getBlock() -> getId();
		leafmostlr = 0;
		_left = NULL;
		_right = NULL;
		_parent = NULL;
		return 3;
	}
	else if(_right != NULL){ // right child
		if (isLeft){ 
			if (isLeft == 2){ _root = _left; _left -> setParent(NULL); }
			else{
				_parent -> setLeft(_right); 
				_right -> setParent(_parent);
			}
		}
		else{ 
			_parent -> setRight(_right); 
			_right -> setParent(_parent);
		}
		leafmostparentid = this -> getBlock() -> getId(); // rocode replace node
		leafmostid = _right -> getBlock() -> getId();
		leafmostlr = 1;
		_left = NULL;
		_right = NULL;
		_parent = NULL;
		return 4;
	}
	else{
		cout << "error in remove operation!" << endl;
		exit(0);
	}
	
	_left = NULL;
	_right = NULL;
	_parent = NULL;
	return 0;
}

// void Node::swap(Node* target){
	// Node *source_left, *source_right, *source_parent, *target_left, *target_right, *target_parent;
	// source_left = _left; source_right = _right; source_parent = _parent; target_left = target-> getLeft(); target_right -> getRight(); target_parent = target -> getParent();
	// int isLeft_source = this -> isLeft();
	// int isLeft_target = target -> isLeft();
	// if (isLeft_source == -1 || isLeft_target == -1){
		// cout << "swap: no parent child!" << endl;
		// exit(0);
	// }
	////change parent pointer
	// if (isLeft_target) target_parent -> setLeft(this);
	// else target_parent -> setRight(this);
	// if (isLeft_source) source_parent -> setLeft(target);
	// else source_parent -> setRight(target);
	////change child pointer
	// if (source_right != NULL) source_right -> setParent(target);
	// if (source_left != NULL) source_left -> setParent(target);
	// if (target_right != NULL) target_right -> setParent(target);
	// if (target_left != NULL) target_left -> setParent(target);
	////change target and source pointer
	// this -> setParent(target_parent); 
	// this -> setLeft(target_left); 
	// this -> setRight(target_right);
	// target -> setParent(source_parent);
	// target -> setLeft(source_left);
	// target -> setRight(source_right);
	
	// only need to change ID or BLOCK orz
	
	
	
	
// }

//
//=====================================================================================
// Class Net

//=====================================================================================

void Net::addTerm(Terminal* t){
	if ( _first ) { _minX = t -> getX(); _maxX = t -> getX(); _minY = t -> getY();  _maxY = t -> getY(); _first = 0;}
	if (t -> getX() < _minX) _minX = t -> getX();
	if (t -> getX() > _maxX) _maxX = t -> getX();
	if (t -> getY() < _minY) _minY = t -> getY();
	if (t -> getY() > _maxY) _maxY = t -> getY();
	_termList.push_back(t);
}

double Net::calcHPWL(){
	// double min_x = 1000000000000.0 , min_y = 10000000000.0, max_x = -1.0, max_y = -1.0;
	// int first = 1;
	double x, y;
	
	double min_x = _minX, min_y = _minY, max_x = _maxX, max_y = _maxY; // load Term max min
	for (auto block: _blockList){
		
		x = (double)block -> getX1() + (double)block -> getWidth()/2.0;//(double)(block -> getX1() + block -> getX2()) / 2;
		y = (double)block -> getY1() + (double)block -> getHeight()/2.0;//(double)(block -> getY1() + block -> getY2()) / 2;
		if (WLDEBUG)cout << "blockX: " << x << " and blockY: " << y<< endl;
		//if (first){ min_x = x; min_y = y; max_x = x; max_y = y; first = 0;}
		if (min_x > x) min_x = x;
		if (min_y > y) min_y = y;
		if (max_x < x) max_x = x;
		if (max_y < y) max_y = y;
	}
	
	// for (auto term: _termList){ //************************************************
		
		// x = (double)term -> getX();
		// y = (double)term -> getY();
		// if (WLDEBUG)cout << "termX: " << x << " and termY: " << y<< endl;
		// if (min_x > x) min_x = x;
		// if (min_y > y) min_y = y;
		// if (max_x < x) max_x = x;
		// if (max_y < y) max_y = y;
	// }
	if (WLDEBUG)cout << max_x << " " << min_x << " " << max_y << " " << min_y << endl;
	if (max_x < min_x || max_y < min_y) { 
		cerr << "HPWL: wrong max min" << endl; 
		cout << " ================ net block report ================= " << endl;
		for (int i = 0; i < _blockList.size(); ++i ){
			Block* block = _blockList[i];
			cout << block -> getName() << "(" << block ->getId() << ")" << ": " << block -> getX1() << " " << block -> getY1() << " " << block -> getX2() << " " << block -> getY2() << " W:" << block -> getWidth() << " H:" << block -> getHeight() << endl;           
		}
		cout << " ================ end ================= " << endl;
		cout << " ================ term report ================= " << endl;
		for (int i = 0; i < _termList.size(); ++i){
			Terminal* t = _termList[i];
			cout << t -> getName() << ": "  << t -> getX() << " " << t -> getY() << endl;           
		}
		cout << " ================ end term report ================= " << endl;
		if (WLDEBUG)cout << _minX << " " << _maxX << " " << _minY << " " <<  _maxY << " " << endl;
		exit(0); 
	}
	return max_x - min_x + max_y - min_y;
}

