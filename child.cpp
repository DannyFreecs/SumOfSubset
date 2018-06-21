#include "pvm3.h" // Parallel API
#include <vector> // Dynamic data storage

// Main entry point of child
int main()
{	
	//receiving message (sum, N, set)
	
	int parent_id = pvm_parent();
	pvm_recv(parent_id, 1);
	
	int sum;
	pvm_upkint(&sum, 1, 1);
	
	int N;
	pvm_upkint(&N, 1, 1);
	
	std::vector<int> set(N);
	pvm_upkint(set.data(), N, 1); 
	
	//result -> the result of one of the base cases
	int result;
	pvm_initsend(PvmDataDefault);
	
	if(sum == 0) //found subset
	{
		result = 1;
		
		pvm_pkint(&result, 1, 1);
		pvm_send(parent_id, 1);
	}
	else if(N == 0 && sum != 0) //did not find subset
	{
		result = 0;
		
		pvm_pkint(&result, 1, 1);
		pvm_send(parent_id, 1);
	}else
	{	
		//starting 2 other tasks to check the possibilities
		std::vector<int> child_tasks(2);
		int started = pvm_spawn((char*)"child", 0, PvmTaskDefault, 0, 2, child_tasks.data());
		if (started < 2)
		{
			pvm_kill(child_tasks[0]);
			pvm_kill(child_tasks[1]);
			
			pvm_exit();
			return -1;
		}
		
		//include actual element:
		//reducing the size of the base set, and subtracting the element from the given sum
		//sending the new values and data to the first child process
		N -= 1;
		int newsum = sum - set[N-1];
		
		pvm_pkint(&newsum, 1, 1);
		pvm_pkint(&N, 1, 1);
		pvm_pkint(set.data(), N, 1);
		
		pvm_send(child_tasks[0], 1);
			
		//exclude actual element (similar as above, only the sum is unchanged and sending to the other child)
		
		pvm_initsend(PvmDataDefault);
		
		pvm_pkint(&sum, 1, 1);
		pvm_pkint(&N, 1, 1);
		pvm_pkint(set.data(), N, 1);
		
		pvm_send(child_tasks[1], 1);
		
		//receiving the results of the child processes
		int result1;
		pvm_recv(child_tasks[0], 1);
		pvm_upkint(&result1, 1, 1);
		
		int result2;
		pvm_recv(child_tasks[1], 1);
		pvm_upkint(&result2, 1, 1);
		
		//summing the results, then sending onwards
		int parentresult = result1 + result2;
		
		pvm_initsend(PvmDataDefault);
		
		pvm_pkint(&parentresult, 1, 1);
		pvm_send(parent_id, 1);		
	}

	pvm_exit();
	
	return 0;
}
