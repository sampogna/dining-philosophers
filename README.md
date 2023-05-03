# Dining Philosophers
Program written in C to solve the classic Dining Philosophers problem. Pthread library resources were used to solve this problem. Such as semaphores and mutexes. 

Outputs in terminal:
    1. When an philosopher eats, the program will print in screen who's eating and with who he's eating with. Identified by Id;
    2. At the end of execution the program will print in screen the total number eaten by every philosopher sorted by this number upwards.

Recommended use:
    1. Compile the program in cygwin command line with the line above (remember to locate where your cygwin terminal is pointing to and the .c file in there):
    
          gcc -Wall -pedantic -o filosofos.exe filosofos.c -lpthread
          
    2. Run the program with:
          ./filosofos <number_of_philosophers> <max_iterations>
            
            *With <number_of_philosophers> being the number of philosophers 
            *With <max_iterations> being the stop condition, or the max number of times a single philosopher can eat.
            
   
