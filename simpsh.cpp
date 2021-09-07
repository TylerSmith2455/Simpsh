#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <bits/stdc++.h>
#include <cctype>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

const int MAX = 256;
char buf[MAX];				// Buffer for getcwd()
vector< pair <string,string> > vars;    // Vector pair which holds each variable along with its value
int sigintFlag = 0;

// Parse the tokens and insert them into a string vector
vector<string> makeTheTokens(string str) {
	istringstream iss(str);
	vector<string> myTokens;
	string s;

	while (iss >> quoted(s)) {
		myTokens.push_back(s);
	}
	return myTokens;
}

// Change directory
void changeDirectory(vector<string> myTokens) {
	if (myTokens.size() == 1)   // If cd is given with no other arguments, just return
		return;
	else {
		string temp = myTokens[1];
		// Variable substitution
		if (temp[0] == '$') {
			temp.erase(temp.begin());
			for (unsigned long i = 0; i < vars.size(); i++) {
				if (temp == vars[i].first) {
					temp = vars[i].second;
					break;
				}
			}
			// If the variable wasn't found
			if (('$' + temp) == myTokens[1]) {
				cout << "Not a variable name\n";
				return;
			}
		}
		else 
			temp = myTokens[1];
		if (chdir(temp.c_str()) == 0) { // If the directory given is valid then change directory
			getcwd(buf, sizeof(buf));
        		vars[1].second = string(buf);  // Update the CWD variable
		}
		else
			cout << "Not a valid directory" << endl;
	}
}

// List all variables
void listVars(vector<string> myTokens) {
	if (myTokens.size() == 3) {
		// If output redirection is needed
		if (myTokens[1] == "outto:") {
			string temp = myTokens[2];
                	// Variable substitution
                	if (temp[0] == '$') {
                        temp.erase(temp.begin());
                        for (unsigned long i = 0; i < vars.size(); i++) {
                                if (temp == vars[i].first) {
                                        temp = vars[i].second;
                                        break;
                                }
                        }
                        // If the variable wasn't found
                        if (('$' + temp) == myTokens[1]) {
                                cout << "Not a variable name\n";
                                return;
                        }
                }
                else
                        temp = myTokens[2];

		string myString;
		FILE *fp;                    // FILE descriptor          
               	fp = fopen(temp.c_str(), "w+"); // open file
                if (fp < 0) {
                      	cout << "Error opening file\n";
                       	return;
               	}
		// Print to the file
               	for (unsigned long i = 0; i < vars.size(); i++) {
               		fprintf(fp, "%s = %s\n",vars[i].first.c_str(),vars[i].second.c_str());
		}
               	fclose(fp); // Close the file
		}
	}
	else { // Print the variables to standard output
		for (unsigned long i = 0; i < vars.size(); i++)
                        cout << vars[i].first << " = " << vars[i].second << endl;
	}
}

// Make a variable
void makeVariable(vector<string> myTokens) {
	string temp = myTokens[0];
	string myVar = myTokens[2];
	// Make a valid variable
	if (temp != "CWD") {
		// First check to see if the variable already exists
		for (unsigned long i = 0; i < vars.size(); i++) {
			if (myTokens[0] == vars[i].first) { 
				vars[i].second = myTokens[2]; // If the variable exists, update the value
				return;
			}
		}
		if (isalpha(temp[0])) { // Checks for first character to be a letter
			int i = 1;
			while (temp[i]) {
				if (ispunct(temp[i])) { // Checks for special characters
					cout << "Invalid variable name" << endl;
					return;
				}	
				i++;
			}
			// If variable substitution is needed
			if (myVar[0] == '$') {
                        	myVar.erase(myVar.begin());
                        	for (unsigned long i = 0; i < vars.size(); i++) {
                                	if (myVar == vars[i].first) {
                                        	myVar = vars[i].second;
                                        	break;
                                	}
                        	}
                        	if (('$' + myVar) == myTokens[1]) {
                                	cout << "Not a variable name\n";
                                	return;
                        	}
                }
			vars.push_back(make_pair(temp, myVar)); // Add the new variable to vars
		}
		else 
			cout << "Invalid variable name" << endl;
	}
	else
		cout << "Cant use variable name CWD" << endl;
}

// Unset a variable
void unsetVar(vector<string> myTokens) {
	// Check that a variable was given
	if (myTokens.size() == 1) {
		cout << "No variable given" << endl;
		return;
	}

	string temp = myTokens[1];
	// Do not unset PATH, CWD, or PS
	if ((temp != "PATH") && (temp != "CWD") && (temp != "PS")) {
		for (unsigned long i = 2; i < vars.size(); i++) {
                        if (temp == vars[i].first) {
                                vars.erase(vars.begin() + i); // If the variable was found, erase it
                        }
                }
		cout << "Variable given not found" << endl;
	}
	else 
		cout << "You can't unset PATH, CWD, or PS" << endl;
}

// Execute a command given
void execute(vector<string> command) {
	// If no commands were given
	if (command.size() == 1) {
		cout << "No command entered\n";
		return;
	}
	int flag = 1; // Flag if variable not found
	// Check if variable substitution is needed
	for (unsigned long i = 1; i < command.size(); i++) {
		string equal = command[i];
		if (equal[0] == '$') {
			equal.erase(equal.begin());
			for (unsigned long j = 0; j < vars.size(); j++) {
				if (equal == vars[j].first) {
					command[i] = vars[j].second;
					flag = 0;
				}
			}
			if (flag == 1) {
				cout << "Unknown variable hs been entered\n";
				return;
			}
		}
	}

	// Check for ":" in the path
	string path = vars[0].second;
	int pos = path.find(':');
	int length = path.size() - pos;
	if ( pos > 0) { // If the ":" is found, make a new string
		string temp;
		for (int j = 1; j < length; j++) 
			temp += path[pos+j];
		path = temp;
	}
	int inputFlag = 0, outputFlag = 0; // Flags to tell if I/O redirection is needed
	// Create child process
	pid_t pid1 = fork();
	if (pid1 == 0) { // Child process
		char * argv[command.size()];
		string program;
		string temp = command[1];
		// Check if I/O redirection is needed
		if (command[command.size()-2] == "infrom:") {
			if (command[command.size()-4] == "outto:") {
				inputFlag = 2;
				outputFlag = 1;
			}
			else 
				inputFlag = 1;
		}
		else if (command[command.size()-2] == "outto:") {
			if (command[command.size()-4] == "infrom:") {
				inputFlag = 1;
				outputFlag = 2;
			}
			else 
				outputFlag = 1;
		}
		// If both input and output redirection is needed
		if ((inputFlag > 0) && (outputFlag > 0)) {
                	int j = 0;
                	for (unsigned long i = 1; i < command.size() - 4; i++) {
                        	argv[j] = const_cast<char*>(command[i].c_str());
                        	j++;
                	}
                	argv[j] = NULL;
                	string temp = command[1];
			if (inputFlag == 1) {
				// open input file
				int fd = open(command[command.size()-3].c_str(), O_RDONLY, 0);
				if (fd < 0) {
					cout << "Error opening file\n";
					return;
				}
				dup2(fd,0); // redirect input
				// open output file
				int wd = open(command[command.size()-1].c_str(), O_WRONLY|O_APPEND, 0);
				if (wd < 0) {
					cout << "Error opening file\n";
                                        return;
                                }
				dup2(wd,1); // redirect output
			}
			else { 
				// open input file
				int fd = open(command[command.size()-1].c_str(), O_RDONLY, 0);
                                if (fd < 0) {
                                        cout << "Error opening file\n";
                                        return;
                                } 
                                dup2(fd,0); // redirect input
				// open output file
                                int wd = open(command[command.size()-3].c_str(), O_WRONLY|O_APPEND, 0);
                                if (wd < 0) {
                                        cout << "Error opening file\n";
                                        return;
                                }
                                dup2(wd,1); // redirect output
			}
		}
		// Else if only one redirection is needed
		else if ((inputFlag > 0) || (outputFlag > 0)) {
                        int j = 0;
                        for (unsigned long i = 1; i < command.size() - 2; i++) {
                                argv[j] = const_cast<char*>(command[i].c_str());
                                j++;
                        }
                        argv[j] = NULL;
			if (inputFlag == 1) { // If input redirection is needed
				// open input file
                                int fd = open(command[command.size()-1].c_str(), O_RDONLY, 0);
                                if (fd < 0) {
                                        cout << "Error opening file\n";
                                        return;
                                }
                                dup2(fd,0); // redirect input
                        }
                        else { // Else output redirection is needed
				// open output file
                                int fd = open(command[command.size()-1].c_str(), O_WRONLY|O_APPEND, 0);
                                if (fd < 0) {
                                        cout << "Error opening file\n";
                                        return;
                                }
                                dup2(fd,1); // redirect output
                        }
		}
		else { // Else I/O redirection isn't needed
			int j = 0;
			for (unsigned long i = 1; i < command.size(); i++) {
				argv[j] = const_cast<char*>(command[i].c_str());
				j++;
			}
			argv[j] = NULL;
		}
		// If the path isn't given use PATH variable
                if ((temp[0] != '/') && (temp[0] != '.')) {
                	string program = path;
                        program += '/';
                        program += temp;
                        if (execve(const_cast<char*>(program.c_str()), argv, NULL) == -1) {
 	                       cout << "Could not execute program from PATH variable" << endl;
                               exit(0);
			}
                 }
                 else { // Else use the path given
                 	if (execve(const_cast<char*>(command[1].c_str()), argv, NULL) == -1) {
                        	cout << "Could not find and execute program" << endl;
                                exit(0);
                        }
                 } 
	}
	else {
		waitpid(pid1, 0, 0); // Wait for child process to terminate
	}

}

// SIGINT handler function
void sigint_handler(int sig) {
        sigset_t mask, prev_mask;

        // Block all signals
        sigfillset(&mask);
        sigprocmask(SIG_BLOCK, &mask, &prev_mask);
	
	// Set flag
	sigintFlag = 1;

        // Restore the original set of blocked signals
        sigprocmask(SIG_SETMASK, &prev_mask, NULL);
}

typedef void handler_t(int);

// sigaction wrapper function
handler_t *Signal(int signum, handler_t *handler) {
        struct sigaction action, old_action;
        action.sa_handler = handler;

        sigemptyset(&action.sa_mask); /* Block sigs of type being handled */
        action.sa_flags = SA_RESTART; /* Restart syscalls if possible */



        if (sigaction(signum, &action, &old_action) < 0)
                printf("Signal error");

        return (old_action.sa_handler);
}


int main() {
	vars.push_back(make_pair("PATH","/bin:/usr/bin"));  // Add the variable PATH
	getcwd(buf, sizeof(buf));                           // Get the current working directory
	vars.push_back(make_pair("CWD", string(buf)));      // Add the varaible CWD
	string PS = "simpsh:";
	vars.push_back(make_pair("PS", PS));      // Add the varaible PS
	string input;
	Signal(SIGINT, sigint_handler);
	for(;;) {
		cout << vars[2].second << " ";
		getline(cin, input);        // Get a line of input
		if (feof(stdin)) {
			cout << endl;
			return 0;
		}
		if (sigintFlag == 0) {
			vector<string> myTokens = makeTheTokens(input); // parse the input
			if (myTokens[0] == "#") {   // Comment indicator
				cout << "This is a comment" << endl;
			}
			else if (myTokens[0] == "lv") {   // List variables indicator
				listVars(myTokens);
			}
			else if (myTokens[0] == "cd") {   // Change directories indicator
				changeDirectory(myTokens);
			}
			else if (myTokens[0] == "quit") {  // Quit indicator
				return 0;
			}
			else if (myTokens[0] == "unset") {
				unsetVar(myTokens);
			}
			else if (myTokens[0] == "!") {
				execute(myTokens);
			}
			else if (myTokens.size() >= 3) {  // Make variable indicator
				if (myTokens[1] == "=")
					makeVariable(myTokens);
			}
			else 
				cout << "Invalid command\n";
		
		}
		else 
			sigintFlag = 0;

	}
	return 0;
}


