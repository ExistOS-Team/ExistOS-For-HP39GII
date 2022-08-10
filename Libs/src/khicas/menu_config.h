#ifndef CONFIG_H

#define CONFIG_H

#define CONFIG_FILE "\\\\fls0\\EIGEN.cfg"

typedef enum {
	CHECK_BOX, FUNCTION_CALL,
} Menu7_Item_Type;

typedef struct {
	char* str;
	Menu7_Item_Type type;
	union { //The item can act on a variable or call a function
		struct {
			int* value;
			void (*save_function)(void);
		} val;
		void (*function)(void);
	} action;
} Menu7_Item;

typedef struct {
	Menu7_Item** items;
	int items_number;
} Menu7;


void draw_menu(Menu7* menu);
void menu_clear();
void menu_setup();
void menu_about();
void menu_help_console();
void menu_help_eqw();
void menu_help_script();
void menu_help_graph();
Menu7_Item* menu_create_item(const char* str, Menu7_Item_Type type, void* other, void* save_func);

void save_config();
void load_config();


#endif // CONFIG_H
