// -*- mode:C++ ; compile-command: "g++ -I.. -I../include -g casctrl.cc -DHAVE_LIBREADLINE -o casctrl -lreadline -lhistory -lncurses" -*-

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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#ifdef HAVE_LIBREADLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif // HAVE_LIBREADLINE
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

#ifdef HAVE_LIBREADLINE
static char *line_read = (char *)NULL;

/* Read a string, and return a pointer to it.  Returns NULL on EOF. */
char *
rl_gets (int count)
{
  /* If the buffer has already been allocated, return the memory
     to the free pool. */
  if (line_read)
    {
      free (line_read);
      line_read = (char *)NULL;
    }
  
  /* Get a line from the user. */
  string prompt(">> ");
  line_read = readline ((char *)prompt.c_str());
  
  /* If the line has any text in it, save it on the history. */
  if (line_read && *line_read)
    add_history (line_read);
  
  return (line_read);
}

#endif // HAVE_LIBREADLINE

volatile bool child_busy=false;
volatile bool data_ready=false;

void data_signal_handler(int signum){
  child_busy=false;
  data_ready=true;
}

int main(int ARGC, char *ARGV[]){    
  using_history();
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
  // Parent process, loop read entry, signal child_id, 
  usleep(1000);
  for (int count=0;;++count) {
    // Get question
    char * res=rl_gets(count);
    if (!res)
      break;
    string s(res);
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
  }
  return 0;
}
