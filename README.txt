Authors: Tyler Smith
	 Taylor Smith

Files: 
	- Makefile: compiles c++ program (type make to compile)
	- simpsh.cpp: cpp file that creates and runs a simple shell

Description:
	
	This simple shell handles the following actions; make and store variables, handles comments, runs programs, changes
directories, unsets variables, lists variables, and does I/O redirection. The shell reads input from the user and parses the message
by using c++ istringstream, invalid commands are ignored and reported. In order to change the directory the function chdir() was
used and to hold the CWD variable getcwd() was used. To run programs, the shell uses fork() and then uses execve(). If input or 
output redirection is needed, open() is used to open the file, and dup2() is used to redirect the input or output.

Who did what:

	Tyler: Handled comment function, lv function, cd function, variable substitution, unset variable, I/O redirection, and
helped with the ! function.

	Taylor: Handled make variable function, parsing, quit function, main function, and did almost all of the ! function.

How to Compile: make

Commands that can be taken by simpsh:
	- lv (list variables)
	- # (acts as a comment and ignores everything after)
	- cd (change directories)
	- quit (exits the program)
	- unset (unsets a variable)
	- = (given as second tos et variable equal to something) ex. var1 = 10
	- ! (runs specified program when given the path or runs program in current working directory if not given path)
	Examples of ! command:
		- ! ls
		- ! /usr/bin/ls
		- ! echo hello world
		- ! /usr/bin/echo hello world
		- !  echo hello world outto: helloout.txt

