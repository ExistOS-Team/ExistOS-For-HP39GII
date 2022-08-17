#ifndef DCONSOLE_H
#define DCONSOLE_H
#define INPUTBUFLEN 500
#define LINE_ROW_MAX    200 // virtual console size (for console scrollback)
#define LINE_COL_MAX    32
//command history numbers:
#define N 41
#define HISTORYHEAP_N N+4
typedef char line_row[LINE_COL_MAX+1];

void delete_clipboard();
const char * paste_clipboard();
const char * input_matrix(bool list);
extern char xcas_status[256];
void set_xcas_status();

extern int xthetat;
const char * keytostring(int key,int keyflag); // key to char *, 0 if not handled
int handle_f5();
int get_filename(char * filename);

bool do_confirm(const char * s);
int confirm(const char * msg1,const char * msg2,bool acexit=false);
bool confirm_overwrite();
void invalid_varname();

void warn_python(int python,bool autochange=false);

void initializeConsoleMemory(line_row* area);

int dGetLineBox (char * s,int max,int width,int x,int y);

void draw_menu(int editor); // 0 console, 1 editor
void dConsoleRedraw ();

void dConsolePut(const char * str);
void dConsolePutChar (char c);

int dGetLine (char * s,int max, int isRecording=0, int ml=0);
int get_line_number(const char * msg1,const char * msg2);

//int dPrintf (const char * format,...);

void dConsoleCls ();
void dPuts(const char *);

//#define printf          dPrintf
#define puts            dPuts
#define putchar         dConsolePutChar
#define gets            dGetLine

void save_console_state_smem();
void load_console_state_smem();
int get_set_session_setting(int value);

#endif
