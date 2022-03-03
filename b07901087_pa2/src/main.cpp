#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>
#include <chrono>
#include "floorplanner.h"
using namespace std;

int main(int argc, char** argv)
{
	clock_t  end;
    fstream input_blk, input_net, output;
    double alpha;
	
    if (argc == 5) {
        alpha = stod(argv[1]);
        input_blk.open(argv[2], ios::in);
        input_net.open(argv[3], ios::in);
        output.open(argv[4], ios::out);
        if (!input_blk) {
            cerr << "Cannot open the input file \"" << argv[2]
                 << "\". The program will be terminated..." << endl;
            exit(1);
        }
        if (!input_net) {
            cerr << "Cannot open the input file \"" << argv[3]
                 << "\". The program will be terminated..." << endl;
            exit(1);
        }
        if (!output) {
            cerr << "Cannot open the output file \"" << argv[4]
                 << "\". The program will be terminated..." << endl;
            exit(1);
        }
    }
    else {
        cerr << "Usage: ./fp <alpha> <input block file> " <<
                "<input net file> <output file>" << endl;
        exit(1);
    }
	//auto start = chrono::high_resolution_clock::now();
    floorplanner* fp = new floorplanner(alpha, input_blk, input_net);
    fp->floorplan();
	
	//auto stop = chrono::high_resolution_clock::now();
	//auto duration = chrono::duration_cast<seconds>(stop - start);
	end = clock();
	double elapse_time = (double)end / CLOCKS_PER_SEC;
	//fp -> report_ans(output, (double)duration.count());
	fp -> report_ans(output, elapse_time);

    return 0;
}
