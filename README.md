# ECSE 427 Assignments
Each assignment progressively builds a more complex and functional simulated OS, starting from basic shell operations to advanced concepts like concurrent process execution and memory management.

### Note: 
The code in this repository is designed to compile and run on the DISCS server, as per the course requirements. It's tailored for the specific computing environment used by McGill University for the Operating Systems course, ensuring compatibility and functionality in line with the academic standards set for the assignments.
## Assignment 1: Building an OS Shell
Overview: The goal is to enhance a simple shell provided in C, simulating basic functionalities of an Operating System (OS) shell.
### Tasks:
- Enhance 'set' Command: Expand the command to support values with up to 5 alphanumeric tokens.
- Implement 'echo' Command: Add functionality to display strings or variables.
- Batch Mode Execution: Ensure shell does not enter an infinite loop in batch mode and suppresses prompt display for each line in batch mode.
- New Commands: Introduce my_ls, my_mkdir, my_touch, and my_cd commands for basic file and directory operations.
- Implement One-liners: Allow chaining of commands separated by semicolons.

## Assignment 2: Multi-process Scheduling
Overview: This assignment extends the shell from Assignment 1 to support running concurrent processes and explore different scheduling strategies.
### Tasks:

- Implement Scheduling Infrastructure: Modify the existing shell to run scripts as processes using a scheduler.
- Introduce 'exec' Command: Enable execution of up to three scripts concurrently, handling different scheduling policies (FCFS, SJF, RR).
- Scheduling Policies: Implement First-Come-First-Served (FCFS), Shortest Job First (SJF), Round Robin (RR), and SJF with job aging.

## Assignment 3: Memory Management
Overview: This final assignment involves implementing a memory manager with demand paging, extending the simulated OS developed in previous assignments.
### Tasks:
- Paging Infrastructure: Modify run and exec commands to incorporate paging, partition shell memory for frame store and variable store.
- Implement Demand Paging: Enable dynamic loading of script pages into shell memory based on necessity.
- LRU Page Replacement Policy: Implement Least Recently Used (LRU) policy for page replacement during page faults.
- Execution Parameters: Adapt shell memory sizes at compile time and handle scripts larger than shell memory.
