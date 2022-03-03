// -*- mode:C++ ; compile-command: "g++ -I.. -I../include -g xcasctrl.cc -o xcasctrl" -*-

/*
 *  Copyright (C) 2005 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
using namespace std;
#include <string>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <time.h>
#include <sys/time.h>
//#include <unistd.h> // For reading arguments from file
#include <fcntl.h>
#include <cstdlib>
#include <signal.h>


volatile bool child_busy=false;
volatile bool data_ready=false;

void data_signal_handler(int signum){
  child_busy=false;
  data_ready=true;
}

// Test xcasce by calling factor(x^4-1)
int main(int ARGC, char *ARGV[]){    
  sigset_t mask, oldmask;
  sigemptyset (&mask);
  sigaddset (&mask, SIGUSR1);
  signal(SIGUSR1,data_signal_handler);
  pid_t child_id=fork();
  if (child_id<(pid_t) 0)
    cerr << "Make_child error: Unable to fork";
  if (!child_id){ // child process, call xcasce
    execvp("/home/parisse/src/xcasce",ARGV);
    return 1;
  }
  usleep(1000000); // Wait for child process to be loaded
  // Parent process, loop read entry, signal child_id, 
  // Get question
  string s("factor(x^4-1)");
  if (s=="quit"){
    kill(child_id,SIGKILL);
    return 0;
  }
  s += '\n';
  ofstream in("Cas.txt");
  in << getpid() << endl;
  in << s << endl;
  data_ready=false;
  child_busy=true;
  // Evaluate
  sigprocmask (SIG_BLOCK, &mask, &oldmask);
  kill(child_id,SIGUSR1);
  while (!data_ready)
    sigsuspend (&oldmask);
  sigprocmask (SIG_UNBLOCK, &mask, NULL);
  // Print result
  FILE * f =fopen("Out.txt","r");
  while (!feof(f)){
    char ch=fgetc(f);
    cout << ch ;
  }
  fclose(f);
  kill(child_id,SIGKILL);
  return 0;
}
