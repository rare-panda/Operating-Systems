README FILE FOR PHASE 1 AND 2 OF PROJECT3.


Files Changed:       term.c and process.c
Variables and semaphores declared:
term.c : term_mutex is a semaphore that gurantees mutual exclusion.

		 term_empty is a semaphore that waits on empty termIO request.
		 
process.c:  pmutex is a semaphore that guarantees mutual exclusion between insert_endIO_list() and endIO_move_to_ready()

            psyncmutex is a semaphore that gurantees there is no insert_endIO request before accessing move_to_ready
			
            pipmutex is a semaphore that increments insertcount exclusively.
			
            insertcount keep tracks of number of insertion requests waiting on pmutex. If count is 0, then only allow
            move_to_ready.	
			
Execution Process:   Same execution process as the original project. (./simos.exe)
Test Files Provided: TEST_AddMultiply, TEST_OPIfgo, TEST_Calculator

Description for what Test Files Does:

TEST_AddMultiply: 
1-> Adds Line 14(7) and Line 15(15) and Stores in Line 18(22) and Prints it.
2-> Multiply Line 16(5) and Line 17(9) and Stores in Line 19.
3-> Loads Result from 1(Line 18(22)) and multiplies with Line 19(45) and stores in Line 20(990).

TEST_OPIfgo:
1-> Adds Line 18(6) and Line 19(8) and stores in Line 20 and print it(14).
2-> If Line 20 is greater than 0, multiply Line 20(14) with Line 22(25) else exit.
3-> Store Line 20(350) and print it.

TEST_Calculator:
Implement Add, Subtract and Multiply.

Line 35 is the menu: 1.Add 2.Subtract 3.Multiply

Executes the operation on values present in Line 38 and Line 39.
Stores the output in Line 40 and prints it.


