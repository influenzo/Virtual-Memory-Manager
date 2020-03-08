Matthew Selva & Lorenzo DeSimone - Virtual Memory Manager

|=|=|=|=|=|=| Overview |=|=|=|=|=|=|

Compiles the MemoryManager.c file into an executable called MemMang, which can be run with ./MemMang in terminal.

This project is centered around the creation of a simulated memory manager that takes instructions from the user to modify physical memory represented by an array. This includes basic paging through a per-process page table, virtual to physical address translation, and support for up to four processes running concurrently. The manager outputs the results of each action, printant an error message if an illegal action has been instructe by the user.


|=|=|=|=|=|=| Testing |=|=|=|=|=|=|

Testing is done line by line in the terminal, going through a series of tasks for the memory manager to handle. We began by mapping a space in memory for a process, storing the process, and loading it to ensure a proper storing technique is used, and the process is retained in memory correctly. Some "incorrect" inputs are fed to the manager as well, such as trying to write to a space that only has read permissions, to ensure the user maintains correct permissions per process.


Ultimately, we compiled our tests into four testing files, which can be run directly through terminal by typing ./MemMang followed by < 1Test, < 2Test, etc.




