#ifndef MODULE_H
#define MODULE_H

#include <vector>
#include <string>
#include <iostream>
using namespace std;

#ifndef NODE
#define NODE

class Block;

class Node
{
    friend class Block;

public:
    // Constructor and destructor
    Node(Block* b) : _block(b), _parent(NULL), _left(NULL), _right(NULL) {}
    ~Node(){}

    // Basic access methods
    
    Node* getParent() const   { return _parent; }
    Node* getLeft() const   { return _left; }
	Node* getRight() const   { return _right; }
	Block* getBlock() const { return _block; }
	void checkParent();

    // Set functions
    //void setId(const int& id) { _id = id; }
    void setParent(Node* parent)  { _parent = parent; }
    void setLeft(Node* left)  { _left = left; }
	void setRight(Node* right)  { _right = right; }
	void setBlock(Block* block) { _block = block; }
	static void setRoot(Node* node) { _root = node; }
	void setcurX(int y)  {_curX = y; }
	
	// operation
	void insert(int lr, Node* child); // may use to insert child(left or right)
	int remove(int& leafmostid, int& leafmostparentid, int& leafmostlr);
	void swap(Node* target) {Block* tmp = _block; _block = target-> getBlock(); target-> setBlock(tmp);}// swap with who
	int  isLeft();
	int  replace(Node* target);
	int getcurX() {return _curX; }
	
	
	
	void backup(){ _left_backup = _left; _right_backup = _right; _parent_backup = _parent; }
	void recovern() { _left = _left_backup; _right = _right_backup; _parent = _parent_backup; }
	
	
	static Node*	_root;
	

private:
    //int         _id;    // ? or block?
	//int 		_upperY; // for greedy soul
	int 		_curX;
	Block* 		_block;
    Node*       _parent;	
    Node*       _left;  
	Node* 		_right; 
	Node* 		_parent_backup;
	Node* 		_left_backup;
	Node*		_right_backup;
};


#endif

#ifndef TERMINAL
#define TERMINAL

class Terminal
{
public:
    // constructor and destructor
    Terminal(string& name, size_t x, size_t y) :
        _name(name), _x(x), _y(y) { }
    ~Terminal()  { }

    // basic access methods
    const string getName()  { return _name; }
    const size_t getX()    { return _x; }
    const size_t getY()    { return _y; }

    // set functions
    void setName(string& name) { _name = name; }
    // void setPos(size_t x1, size_t y1, size_t x2, size_t y2) {
        // _x1 = x1;   _y1 = y1;
        // _x2 = x2;   _y2 = y2;
    // }

private:
    string      _name;      // module name
    size_t      _x;        // min x coordinate of the terminal
    size_t      _y;        // min y coordinate of the terminal
    //size_t      _x2;        // max x coordinate of the terminal
    //size_t      _y2;        // max y coordinate of the terminal
};

#endif

#ifndef BLOCK
#define BLOCK

class Block  //public Terminal
{
public:
    // constructor and destructor
    Block(string& str, int i, int w, int h):_name(str), _id(i), _w(w), _h(h), _x1(0), _x2(0), _y1(0), _y2(0), _bestx1(0), _bestx2(0), _besty1(0), _besty2(0) { _node = new Node(this); }
    ~Block() { delete _node; }

    // basic access methods
    const size_t getWidth(bool rotate = false)  { return rotate? _h: _w; }
    const size_t getHeight(bool rotate = false) { return rotate? _w: _h; }
    const size_t getArea()  { return _h * _w; }
	int getId() const       { return _id; }
    //static size_t getMaxX() { return _maxX; }
    //static size_t getMaxY() { return _maxY; }
	
	

    // set functions
	void setX1(size_t w)         { _x1 = w; }
	void setX2(size_t w)         { _x2 = w; }
	void setY1(size_t w)         { _y1 = w; }
	void setY2(size_t w)         { _y2 = w; }
    void setWidth(size_t w)         { _w = w; }
    void setHeight(size_t h)        { _h = h; }
    //static void setMaxX(size_t x)   { _maxX = x; }
    //static void setMaxY(size_t y)   { _maxY = y; }
	
	//operation
	void rotate()					{ size_t tmp = _w; _w = _h; _h = tmp; }
	
	// term //
	// basic access methods
    const string getName()  { return _name; }
    const size_t getX1()    { return _x1; }
    const size_t getX2()    { return _x2; }
    const size_t getY1()    { return _y1; }
    const size_t getY2()    { return _y2; }
	const size_t getbestX1()    { return _bestx1; }
    const size_t getbestX2()    { return _bestx2; }
    const size_t getbestY1()    { return _besty1; }
    const size_t getbestY2()    { return _besty2; }

    // set functions
    void setName(string& name) { _name = name; }
    void setPos(size_t x1, size_t y1, size_t x2, size_t y2) {
        _x1 = x1;   _y1 = y1;
        _x2 = x2;   _y2 = y2;
    }
	void setBest(){
		_bestx1 = _x1;   _besty1 = _y1;
        _bestx2 = _x2;   _besty2 = _y2;
	}
	
	// node
	Node* getNode() { return _node; }
	void backupNode() { _node -> backup(); }
	void recoverNode() { _node -> recovern(); }
	
	
	// for sorting
	


private:
	int				_id; 
    size_t          _w;         // width of the block
    size_t          _h;         // height of the block
    //static size_t   _maxX;      // maximum x coordinate for all blocks
    //static size_t   _maxY;      // maximum y coordinate for all blocks
	// term
	string     		_name;      // module name
    size_t      	_x1;        // min x coordinate of the terminal
    size_t      	_y1;        // min y coordinate of the terminal
    size_t      	_x2;        // max x coordinate of the terminal
    size_t      	_y2;        // max y coordinate of the terminal
	size_t			_bestx1;	// the best result
	size_t			_bestx2;
	size_t			_besty1;
	size_t			_besty2;
	Node*			_node;
};

// class CompareBlock {
	// friend class Block;
    // public:
    ////after making CompareInformations a friend class to Information...
    // bool operator(const Block* b1, const Block* b2) {
        // return  (   (b1 -> getWidth() * b1 -> getHeight()) < (b2 -> getWidth() * b2 -> getHeight())   );
    // }
// };


#endif


#ifndef NET
#define NET

class Net
{
public:
    // constructor and destructor
    Net():   _first(1), _minX(2147483647.0), _minY(2147483647.0), _maxX(-1.0), _maxY(-1.0){ }
    ~Net()  { _first = 0;}

    // basic access methods
    //const vector<Terminal*> getTermList()   { return _termList; }
	const vector<Block*>& getBlockList()   { return _blockList; }
    // modify methods
    void addTerm(Terminal* term);
	void addBlock(Block* block)	 { _blockList.push_back(block); }
	

    // other member functions
    double calcHPWL();
	double getmaxX() {return _maxX; }
	double getminX() {return _minX; }
	double getmaxY() {return _maxY; }
	double getminY() {return _minY; }
	

private:
    vector<Terminal*>   _termList;  // list of terminals the net is connected to
	vector<Block*>		_blockList;
	
	// for terminal (min max)
	int 				_first;
	double 				_maxX;
	double 				_maxY;
	double 				_minX;
	double 				_minY;
	
};
#endif

#endif  // MODULE_H
