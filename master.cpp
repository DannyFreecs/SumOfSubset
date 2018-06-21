#include <fstream> // File operations
#include "pvm3.h" // Parallel API
#include <vector> // Dynamic data storage
#include <stdlib.h>
#include <fstream>

//Main entry point
//sum -> the sum you want to check in the subsets
//N   -> the size of the given base set

int main(int argc, char** argv)
{
	int sum = atoi(argv[1]);
	std::ifstream input(argv[2]);
	
	if(input.fail())
		return -1;
	
	//reading input data
	int N;
	input >> N;
	std::vector<int> input_data(N);
	
	for(int i=0; i<N; i++)
		input >> input_data[i];
	
	input.close();
		
	//creating a new child, and sending the necessary data
	int tid;
	int started = pvm_spawn("child", (char**)0, 0, "", 1, &tid);
	if(started < 1)
	{
		pvm_kill(tid);
		pvm_exit();
		return -1;
	}
	
	pvm_initsend(PvmDataDefault);
	
	pvm_pkint(&sum, 1, 1);
	pvm_pkint(&N, 1, 1);
	pvm_pkint(input_data.data(), N, 1);
	
	pvm_send(tid, 1);
	
	//receiving the result from the child
	int end_result;
	
	pvm_recv(tid, 1);
	pvm_upkint(&end_result, 1, 1);
	
	//writing the result into given output file
	std::ofstream output(argv[3]);	
	
	(end_result > 0) ? output << "May the subset be with You!" : output << "I find your lack of subset disturbing!";
	output.close();
	
	pvm_exit();
	
	return 0;
}
