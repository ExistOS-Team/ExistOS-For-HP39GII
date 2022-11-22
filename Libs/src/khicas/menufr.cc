#include "giacPCH.h"
#include "input_lexer.h"
#include "input_parser.h"
#include "libfx.h"
#include "syscalls.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#include "menuGUI.h"
#include "textGUI.h"
#include "main.h"
#include "input_lexer.h"
#include "input_parser.h"
#include "console.h"

#ifdef SCROLLBAR
typedef scrollbar TScrollbar;
#endif

#define C24 8 // 24 on 90
#define C18 8 // 18
#define C10 7 // 18
#define C6 2 // 6

int doMenu(Menu* menu, MenuItemIcon* icontable) { // returns code telling what user did. selection is on menu->selection. menu->selection starts at 1!
  int itemsStartY=menu->startY; // char Y where to start drawing the menu items. Having a title increases this by one
  int itemsHeight=menu->height;
  int showtitle = menu->title != NULL;
  if (showtitle) {
    itemsStartY++;
    itemsHeight--;
  }
  char keyword[5];
  keyword[0]=0;
  if(menu->selection > menu->scroll+(menu->numitems>itemsHeight ? itemsHeight : menu->numitems))
    menu->scroll = menu->selection -(menu->numitems>itemsHeight ? itemsHeight : menu->numitems);
  if(menu->selection-1 < menu->scroll)
    menu->scroll = menu->selection -1;
  
  while(1) {
    Cursor_SetFlashOff();
    if (menu->selection <=1)
      menu->selection=1;
    if (menu->selection > menu->scroll+(menu->numitems>itemsHeight ? itemsHeight : menu->numitems))
      menu->scroll = menu->selection -(menu->numitems>itemsHeight ? itemsHeight : menu->numitems);
    if (menu->selection-1 < menu->scroll)
      menu->scroll = menu->selection -1;
    //if(menu->statusText != NULL) DefineStatusMessage(menu->statusText, 1, 0, 0);
    // Clear the area of the screen we are going to draw on
    if(0 == menu->pBaRtR) giac::drawRectangle(C18*(menu->startX-1), C24*(menu->miniMiniTitle ? itemsStartY:menu->startY), C18*menu->width*C6+((menu->scrollbar && menu->scrollout)?C6:0), C24*menu->height-(menu->miniMiniTitle ? C24:0), COLOR_WHITE);
    if (menu->numitems>0) {
      for(int curitem=0; curitem < menu->numitems; curitem++) {
        // print the menu item only when appropriate
        if(menu->scroll < curitem+1 && menu->scroll > curitem-itemsHeight) {
          char menuitem[256] = "";
          if(menu->numitems>=100 || menu->type == MENUTYPE_MULTISELECT){
	    strcpy(menuitem, "  "); //allow for the folder and selection icons on MULTISELECT menus (e.g. file browser)
	    strcpy(menuitem+2,menu->items[curitem].text);
	  }
	  else {
	    int cur=curitem+1;
	    if (menu->numitems<10){
	      menuitem[0]='0'+cur;
	      menuitem[1]=' ';
	      menuitem[2]=0;
	      strcpy(menuitem+2,menu->items[curitem].text);
	    }
	    else {
	      menuitem[0]=cur>=10?('0'+(cur/10)):' ';
	      menuitem[1]='0'+(cur%10);
	      menuitem[2]=' ';
	      menuitem[3]=0;
	      strcpy(menuitem+3,menu->items[curitem].text);
	    }
	  }
          //strncat(menuitem, menu->items[curitem].text, 68);
          if(menu->items[curitem].type != MENUITEM_SEPARATOR) {
            //make sure we have a string big enough to have background when item is selected:          
            // MB_ElementCount is used instead of strlen because multibyte chars count as two with strlen, while graphically they are just one char, making fillerRequired become wrong
            int fillerRequired = menu->width - MB_ElementCount(menu->items[curitem].text) - (menu->type == MENUTYPE_MULTISELECT ? 2 : 3);
            for(int i = 0; i < fillerRequired; i++) strcat(menuitem, " ");
            PrintXY(6*menu->startX,8*(curitem+itemsStartY-menu->scroll),(const unsigned char*)menuitem, (menu->selection == curitem+1 ? TEXT_MODE_INVERT : TEXT_MODE_NORMAL));
          } else { 
            /*int textX = (menu->startX-1) * C18;
            int textY = curitem*C24+itemsStartY*C24-menu->scroll*C24-C24+C6;
            clearLine(menu->startX, curitem+itemsStartY-menu->scroll, (menu->selection == curitem+1 ? textColorToFullColor(menu->items[curitem].color) : COLOR_WHITE));
            drawLine(textX, textY+C24-4, LCD_WIDTH_PX-2, textY+C24-4, COLOR_GRAY);
            PrintMini(&textX, &textY, (unsigned char*)menuitem, 0, 0xFFFFFFFF, 0, 0, (menu->selection == curitem+1 ? COLOR_WHITE : textColorToFullColor(menu->items[curitem].color)), (menu->selection == curitem+1 ? textColorToFullColor(menu->items[curitem].color) : COLOR_WHITE), 1, 0);*/
          }
          // deal with menu items of type MENUITEM_CHECKBOX
          if(menu->items[curitem].type == MENUITEM_CHECKBOX) {
            PrintXY(6*(menu->startX+menu->width-1),8*(curitem+itemsStartY-menu->scroll),
              (menu->items[curitem].value == MENUITEM_VALUE_CHECKED ? (const unsigned char*)"\xe6\xa9" : (const unsigned char*)"\xe6\xa5"),
              (menu->selection == curitem+1 ? TEXT_MODE_INVERT : (menu->pBaRtR == 1? TEXT_MODE_NORMAL : TEXT_MODE_NORMAL)));
          }
          // deal with multiselect menus
          if(menu->type == MENUTYPE_MULTISELECT) {
            if((curitem+itemsStartY-menu->scroll)>=itemsStartY &&
              (curitem+itemsStartY-menu->scroll)<=(itemsStartY+itemsHeight) &&
              icontable != NULL
            ) {
#if 0
              if (menu->items[curitem].isfolder == 1) {
                // assumes first icon in icontable is the folder icon
                CopySpriteMasked(icontable[0].data, (menu->startX)*C18, (curitem+itemsStartY-menu->scroll)*C24, 0x12, 0x18, 0xf81f  );
              } else {
                if(menu->items[curitem].icon >= 0) CopySpriteMasked(icontable[menu->items[curitem].icon].data, (menu->startX)*C18, (curitem+itemsStartY-menu->scroll)*C24, 0x12, 0x18, 0xf81f  );
              }
#endif
            }
            if (menu->items[curitem].isselected) {
              if (menu->selection == curitem+1) {
                PrintXY(6*menu->startX,8*(curitem+itemsStartY-menu->scroll),(const unsigned char*)"\xe6\x9b", TEXT_MODE_NORMAL);
              } else {
                PrintXY(6*menu->startX,8*(curitem+itemsStartY-menu->scroll),(const unsigned char*)"\xe6\x9b", TEXT_MODE_NORMAL);
              }
            }
          }
        }
      }
      if (menu->scrollbar) {
#ifdef SCROLLBAR
        TScrollbar sb;
        sb.I1 = 0;
        sb.I5 = 0;
        sb.indicatormaximum = menu->numitems;
        sb.indicatorheight = itemsHeight;
        sb.indicatorpos = menu->scroll;
        sb.barheight = itemsHeight*C24;
        sb.bartop = (itemsStartY-1)*C24;
        sb.barleft = menu->startX*C18+menu->width*C18 - C18 - (menu->scrollout ? 0 : 5);
        sb.barwidth = C6;
        Scrollbar(&sb);
#endif
      }
      //if(menu->type==MENUTYPE_MULTISELECT && menu->fkeypage == 0) drawFkeyLabels(0x0037); // SELECT (white)
    } else {
      giac::printCentered(menu->nodatamsg, (itemsStartY*C24)+(itemsHeight*C24)/2-12);
    }
    if(showtitle) {
      if(menu->miniMiniTitle) {
        int textX = 0, textY=(menu->startY-1)*C24;
        PrintMini( textX, textY, (const unsigned char *)menu->title, 0 );
      } else PrintXY(6*menu->startX, 8*menu->startY, (const unsigned char *)menu->title, TEXT_MODE_NORMAL);
      if(menu->subtitle != NULL) {
        int textX=(MB_ElementCount(menu->title)+menu->startX-1)*C18+C10, textY=C6;
        PrintMini(textX, textY, (const unsigned char *)menu->subtitle, 0);
      }
      PrintXY(104, 1, (const unsigned char *)"____", 0);
      PrintXY(104, 1, (const unsigned char *)keyword, 0);
    }
    /*if(menu->darken) {
      DrawFrame(COLOR_BLACK);
      VRAMInvertArea(menu->startX*C18-C18, menu->startY*C24, menu->width*C18-(menu->scrollout || !menu->scrollbar ? 0 : 5), menu->height*C24);
    }*/
    if(menu->type == MENUTYPE_NO_KEY_HANDLING) return MENU_RETURN_INSTANT; // we don't want to handle keys
    unsigned int key;
    ck_getkey(&key);
    if (isalpha(key)){
      key=tolower(key);
      int pos=strlen(keyword);
      if (pos>=4)
	pos=0;
      keyword[pos]=key;
      keyword[pos+1]=0;
      int cur=0;
      for (;cur<menu->numitems;++cur){
#if 1
	if (strcmp(menu->items[cur].text,keyword)>=0)
	  break;
#else
	char c=menu->items[cur].text[0];
	if (key<=c)
	  break;
#endif
      }
      if (cur<menu->numitems){
	menu->selection=cur+1;
	if(menu->selection > menu->scroll+(menu->numitems>itemsHeight ? itemsHeight : menu->numitems))
	  menu->scroll = menu->selection -(menu->numitems>itemsHeight ? itemsHeight : menu->numitems);
	if(menu->selection-1 < menu->scroll)
	  menu->scroll = menu->selection -1;
      }
      continue;
    }
    switch(key) {
    case KEY_CTRL_PAGEDOWN:
      menu->selection+=6;
      if (menu->selection >= menu->numitems)
	menu->selection=menu->numitems;
      if(menu->selection > menu->scroll+(menu->numitems>itemsHeight ? itemsHeight : menu->numitems))
	menu->scroll = menu->selection -(menu->numitems>itemsHeight ? itemsHeight : menu->numitems);
      break;
    case KEY_CTRL_DOWN:
      if(menu->selection == menu->numitems)
	{
          if(menu->returnOnInfiniteScrolling) {
            return MENU_RETURN_SCROLLING;
          } else {
            menu->selection = 1;
            menu->scroll = 0;
          }
        }
      else
        {
	  menu->selection++;
          if(menu->selection > menu->scroll+(menu->numitems>itemsHeight ? itemsHeight : menu->numitems))
            menu->scroll = menu->selection -(menu->numitems>itemsHeight ? itemsHeight : menu->numitems);
        }
      if(menu->pBaRtR==1) return MENU_RETURN_INSTANT;
      break;
    case KEY_CTRL_PAGEUP:
      menu->selection-=6;
      if (menu->selection <=1)
	menu->selection=1;
      if(menu->selection-1 < menu->scroll)
	menu->scroll = menu->selection -1;
      break;
    case KEY_CTRL_UP:
      if(menu->selection == 1)
	{
	  if(menu->returnOnInfiniteScrolling) {
	    return MENU_RETURN_SCROLLING;
	  } else {
	    menu->selection = menu->numitems;
	    menu->scroll = menu->selection-(menu->numitems>itemsHeight ? itemsHeight : menu->numitems);
	  }
	}
      else
	{
	  menu->selection--;
	  if(menu->selection-1 < menu->scroll)
	    menu->scroll = menu->selection -1;
	}
      if(menu->pBaRtR==1) return MENU_RETURN_INSTANT;
      break;
    case KEY_CTRL_F1:
      if(menu->type==MENUTYPE_MULTISELECT && menu->fkeypage == 0 && menu->numitems > 0) {
          /*if(menu->items[menu->selection-1].isselected) {
            menu->items[menu->selection-1].isselected=0;
            menu->numselitems = menu->numselitems-1;
	    } else {
            menu->items[menu->selection-1].isselected=1;
            menu->numselitems = menu->numselitems+1;
	    }
	    return key; //return on F1 too so that parent subroutines have a chance to e.g. redraw fkeys*/
      } else if (menu->type == MENUTYPE_FKEYS) {
	return key;
      }
      break;
    case KEY_CTRL_F2:
    case KEY_CTRL_F3:
    case KEY_CTRL_F4:
    case KEY_CTRL_F5:
    case KEY_CTRL_F6:
      if (menu->type == MENUTYPE_FKEYS || menu->type==MENUTYPE_MULTISELECT) return key; // MULTISELECT also returns on Fkeys
      break;
    case KEY_CTRL_PASTE:
      if (menu->type==MENUTYPE_MULTISELECT) return key; // MULTISELECT also returns on paste
    case KEY_CTRL_OPTN:
      if (menu->type==MENUTYPE_FKEYS || menu->type==MENUTYPE_MULTISELECT) return key;
      break;
    case KEY_CTRL_FORMAT:
      if (menu->type==MENUTYPE_FKEYS) return key; // return on the Format key so that event lists can prompt to change event category
      break;
    case KEY_CTRL_RIGHT:
      if(menu->type != MENUTYPE_MULTISELECT) break;
      // else fallthrough
    case KEY_CTRL_EXE:
      if(menu->numitems>0) return MENU_RETURN_SELECTION;
      break;
    case KEY_CTRL_LEFT:
      if(menu->type != MENUTYPE_MULTISELECT) break;
      // else fallthrough
    case KEY_CTRL_DEL:
      if (strlen(keyword))
	keyword[strlen(keyword)-1]=0;
      else {
	if (strcmp(menu->title,"Variables")==0)
	  return key;
      }
      break;
    case KEY_CTRL_AC:
      if (strlen(keyword)){
	keyword[0]=0;
	SetSetupSetting( (unsigned int)0x14, 0x88);	
	//DisplayStatusArea();
	break;
      }
    case KEY_CTRL_EXIT: 
      return MENU_RETURN_EXIT;
      break;
    case KEY_CHAR_1:
    case KEY_CHAR_2:
    case KEY_CHAR_3:
    case KEY_CHAR_4:
    case KEY_CHAR_5:
    case KEY_CHAR_6:
    case KEY_CHAR_7:
    case KEY_CHAR_8:
    case KEY_CHAR_9:
      if(menu->numitems>=(key-0x30)) {
	menu->selection = (key-0x30);
	if (menu->type != MENUTYPE_FKEYS) return MENU_RETURN_SELECTION;
      }
      break;
    case KEY_CHAR_0:
      if(menu->numitems>=10) {
	menu->selection = 10;
	if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
      }
      break;
    case KEY_CTRL_XTT:
      if(menu->numitems>=11) {
	menu->selection = 11;
	if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
      }
      break;
    case KEY_CHAR_LOG:
      if(menu->numitems>=12) {
	menu->selection = 12;
	if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
      }
      break;
    case KEY_CHAR_LN:
      if(menu->numitems>=13) {
	menu->selection = 13;
	if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
      }
      break;
    case KEY_CHAR_SIN:
    case KEY_CHAR_COS:
    case KEY_CHAR_TAN:
      if(menu->numitems>=(key-115)) {
	menu->selection = (key-115);
	if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
      }
      break;
    case KEY_CHAR_FRAC:
      if(menu->numitems>=17) {
	menu->selection = 17;
	if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
      }
      break;
    case KEY_CTRL_FD:
      if(menu->numitems>=18) {
	menu->selection = 18;
	if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
      }
      break;
    case KEY_CHAR_LPAR:
    case KEY_CHAR_RPAR:
      if(menu->numitems>=(key-21)) {
	menu->selection = (key-21);
	if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
      }
      break;
    case KEY_CHAR_COMMA:
      if(menu->numitems>=21) {
	menu->selection = 21;
	if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
      }
      break;
    case KEY_CHAR_STORE:
      if(menu->numitems>=22) {
	menu->selection = 22;
	if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
      }
      break;
    }
  }
  return MENU_RETURN_EXIT;
}

void reset_alpha(){
  SetSetupSetting( (unsigned int)0x14, 0);	
  //DisplayStatusArea();
}

#define CAT_CATEGORY_ALL 0
#define CAT_CATEGORY_ALGEBRA 1
#define CAT_CATEGORY_LINALG 2
#define CAT_CATEGORY_CALCULUS 3
#define CAT_CATEGORY_ARIT 4
#define CAT_CATEGORY_COMPLEXNUM 5
#define CAT_CATEGORY_PLOT 6
#define CAT_CATEGORY_POLYNOMIAL 7
#define CAT_CATEGORY_PROBA 8
#define CAT_CATEGORY_PROGCMD 9
#define CAT_CATEGORY_REAL 10
#define CAT_CATEGORY_SOLVE 11
#define CAT_CATEGORY_STATS 12
#define CAT_CATEGORY_TRIG 13
#define CAT_CATEGORY_OPTIONS 14
#define CAT_CATEGORY_LIST 15
#define CAT_CATEGORY_MATRIX 16
#define CAT_CATEGORY_PROG 17
#define CAT_CATEGORY_SOFUS 18
#define CAT_CATEGORY_LOGO 19 // should be the last one

void init_locale(){
  giac::lang=1;
}

const catalogFunc completeCat[] = { // list of all functions (including some not in any category)
  // {"cosh(x)", 0, "Hyperbolic cosine of x.", 0, 0, CAT_CATEGORY_TRIG},
  // {"exp(x)", 0, "Renvoie e^x.", "1.2", 0, CAT_CATEGORY_REAL},
  // {"exponential_regression_plot(Xlist,Ylist)", 0, "Graphe d'une regression exponentielle.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];exponential_regression_plot(X,Y);scatterplot(X,Y)", 0, CAT_CATEGORY_STATS},
  // {"log(x)", 0, "Logarithme naturel de x.", 0, 0, CAT_CATEGORY_REAL},
  // {"logarithmic_regression_plot(Xlist,Ylist)", 0, "Graphe d'une regression logarithmique.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];logarithmic_regression_plot(X,Y);scatterplot(X,Y)", 0, CAT_CATEGORY_STATS},
  // {"polynomial_regression_plot(Xlist,Ylist,n)", 0, "Graphe d'une regression polynomiale de degre <= n.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];polynomial_regression_plot(X,Y,2);scatterplot(X,Y)", 0, CAT_CATEGORY_STATS},
  // {"power_regression_plot(Xlist,Ylist,n)", 0, "Graphe d'une regression puissance.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];power_regression_plot(X,Y);scatterplot(X,Y)", 0, CAT_CATEGORY_STATS},
  // {"sinh(x)", 0, "Hyperbolic sine of x.", 0, 0, CAT_CATEGORY_TRIG},
  // {"tanh(x)", 0, "Hyperbolic tangent of x.", 0, 0, CAT_CATEGORY_TRIG},
  {" boucle for (pour)", "for ", "Boucle definie pour un indice variant entre 2 valeurs fixees", "#\nfor ", 0, CAT_CATEGORY_PROG},
  {" boucle liste", "for in", "Boucle sur tous les elements d'une liste.", "#\nfor in", 0, CAT_CATEGORY_PROG},
  {" boucle while (tantque)", "while ", "Boucle indefinie tantque.", "#\nwhile ", 0, CAT_CATEGORY_PROG},
  {" test si alors", "if ", "Test", "#\nif ", 0, CAT_CATEGORY_PROG},
  {" test sinon", "else ", "Clause fausse du test", 0, 0, CAT_CATEGORY_PROG},
  {" fonction def.", "f(x):=", "Definition de fonction.", "#\nf(x):=", 0, CAT_CATEGORY_PROG},
  {" local j,k;", "local ", "Declaration de variables locales Xcas", 0, 0, CAT_CATEGORY_PROG},
  {" range(a,b)", "in range(", "Dans l'intervalle [a,b[ (a inclus, b exclus)", "# in range(1,10)", 0, CAT_CATEGORY_PROG},
  {" return res;", "return ", "return ou retourne quitte la fonction et renvoie le resultat res", 0, 0, CAT_CATEGORY_PROG},
  {" edit list ", "list ", "Assistant creation de liste.", 0, 0, CAT_CATEGORY_LIST},
  {" edit matrix ", "matrix ", "Assistant creation de matrice.", 0, 0, CAT_CATEGORY_MATRIX},
  //{"fonction def Xcas", "fonction f(x) local y;   ffonction:;", "Definition de fonction.", "#fonction f(x) local y; y:=x^2; return y; ffonction:;", 0, CAT_CATEGORY_PROG},
  {"!", "!", "Non logique (prefixe) ou factorielle de n (suffixe). Raccourci shift F1", "#7!", "#!b", CAT_CATEGORY_PROGCMD},
  {"#", "#", "Commentaire Python, en Xcas taper //. Raccourci ALPHA F2", 0, 0, CAT_CATEGORY_PROG},
  {"%", "%", "a % b signifie a modulo b", 0, 0, CAT_CATEGORY_ARIT | (CAT_CATEGORY_PROGCMD << 8)},
  {"&", "&", "Et logique ou +", "#1&2", 0, CAT_CATEGORY_PROGCMD},
  {":=", ":=", "Affectation vers la gauche (inverse de =>). Raccourci SHIFT F1", "#a:=3", 0, CAT_CATEGORY_PROGCMD|(CAT_CATEGORY_SOFUS<<8)},
  {"<", "<", "Inferieur strict. Raccourci SHIFT F2", 0, 0, CAT_CATEGORY_PROGCMD},
  {"=>", "=>", "Affectation vers la droite ou conversion en (touche ->). Par exemple 5=>a ou x^4-1=>* ou (x+1)^2=>+ ou sin(x)^2=>cos.", "#5=>a", 0, CAT_CATEGORY_PROGCMD},
  {">", ">", "Superieur strict. Raccourci F2.", 0, 0, CAT_CATEGORY_PROGCMD},
  {"\\", "\\", "Caractere \\", 0, 0, CAT_CATEGORY_PROGCMD},
  {"_", "_", "Caractere _. Raccourci (-).", 0, 0, CAT_CATEGORY_PROGCMD},
  {"_cdf", "_cdf", "Suffixe pour obtenir une distribution cumulee. Taper F2 pour la distribution cumulee inverse.", "#_icdf", 0, CAT_CATEGORY_PROBA},
  {"_plot", "_plot", "Suffixe pour obtenir le graphe d'une regression.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];polynomial_regression_plot(X,Y,2);scatterplot(X,Y)", 0, CAT_CATEGORY_STATS},
  {"a and b", " and ", "Et logique", 0, 0, CAT_CATEGORY_PROGCMD},
  {"a or b", " or ", "Ou logique", 0, 0, CAT_CATEGORY_PROGCMD},
  {"a2q(A,[vars])", 0, "Matrix to quadratic form", "[[1,2],[2,3]]","[[1,2],[2,3]],[x,y]", CAT_CATEGORY_LINALG},
  {"abcuv(a,b,c)", 0, "Cherche 2 polynomes u,v tels que a*u+b*v=c","x+1,x^2-2,x", 0, CAT_CATEGORY_POLYNOMIAL},
  {"abs(x)", 0, "Valeur absolue, module ou norme de x", "-3", "[1,2,3]", CAT_CATEGORY_COMPLEXNUM | (CAT_CATEGORY_REAL<<8)},
  {"append", 0, "Ajoute un element en fin de liste l","#l.append(x)", 0, CAT_CATEGORY_LIST},
  {"approx(x)", 0, "Valeur approchee de x. Raccourci S-D", "pi", 0, CAT_CATEGORY_REAL},
  {"arg(z)", 0, "Argument du complexe z.", "1+i", 0, CAT_CATEGORY_COMPLEXNUM},
  {"asc(string)", 0, "Liste des codes ASCII d'une chaine", "\"Bonjour\"", 0, CAT_CATEGORY_ARIT},
  {"assume(hyp)", 0, "Hypothese sur une variable.", "x>1", "x>-1 and x<1", CAT_CATEGORY_PROGCMD | (CAT_CATEGORY_SOFUS<<8)},
  {"avance n", "avance ", "La tortue avance de n pas, par defaut n=10", "#avance 40", 0, CAT_CATEGORY_LOGO},
  {"axes", "axes", "Axes visibles ou non axes=1 ou 0", "#axes=0", "#axes=1", CAT_CATEGORY_PROGCMD << 8},
  {"baisse_crayon ", "baisse_crayon ", "La tortue se deplace en marquant son passage.", 0, 0, CAT_CATEGORY_LOGO},
  {"barplot(list)", 0, "Diagramme en batons d'une serie statistique 1d.", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS},
  {"binomial(n,p,k)", 0, "binomial(n,p,k) probabilite de k succes avec n essais ou p est la proba de succes d'un essai. binomial_cdf(n,p,k) est la probabilite d'obtenir au plus k succes avec n essais. binomial_icdf(n,p,t) renvoie le plus petit k tel que binomial_cdf(n,p,k)>=t", "10,.5,4", 0, CAT_CATEGORY_PROBA},
  {"bitxor", "bitxor", "Ou exclusif", "#bitxor(1,2)", 0, CAT_CATEGORY_PROGCMD},
  {"black", "black", "Option d'affichage", "#display=black", 0, CAT_CATEGORY_PROGCMD},
  {"blue", "blue", "Option d'affichage", "#display=blue", 0, CAT_CATEGORY_PROGCMD},
  {"cache_tortue ", "cache_tortue ", "Cache la tortue apres avoir trace le dessin.", 0, 0, CAT_CATEGORY_LOGO},
  {"camembert(list)", 0, "Diagramme en camembert d'une serie statistique 1d.", "[[\"France\",6],[\"Allemagne\",12],[\"Suisse\",5]]", 0, CAT_CATEGORY_STATS},
  {"ceiling(x)", 0, "Partie entiere superieure", "1.2", 0, CAT_CATEGORY_REAL},
  {"cercle(centre,rayon)", 0, "Cercle donne par centre et rayon ou par un diametre", "2+i,3", "1-i,1+i", CAT_CATEGORY_PROGCMD},
  {"cfactor(p)", 0, "Factorisation sur C.", "x^4-1", 0, CAT_CATEGORY_ALGEBRA | (CAT_CATEGORY_COMPLEXNUM << 8)},
  {"char(liste)", 0, "Chaine donnee par une liste de code ASCII", "[97,98,99]", 0, CAT_CATEGORY_ARIT},
  {"charpoly(M,x)", 0, "Polynome caracteristique de la matrice M en la variable x.", "[[1,2],[3,4]],x", 0, CAT_CATEGORY_MATRIX},
  {"clearscreen()", "clearscreen()", "Efface l'ecran.", 0, 0, CAT_CATEGORY_PROGCMD},
  {"coeff(p,x,n)", 0, "Coefficient de x^n dans le polynome p.", "(1+x)^6,x,3", 0, CAT_CATEGORY_POLYNOMIAL},
  {"comb(n,k)", 0, "Renvoie k parmi n.", "10,4", 0, CAT_CATEGORY_PROBA},
  {"cond(A,[1,2,inf])", 0, "Nombre de condition d'une matrice par rapport a la norme specifiee (par defaut 1)", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
  {"conj(z)", 0, "Conjugue complexe de z.", "1+i", 0, CAT_CATEGORY_COMPLEXNUM},
  {"correlation(l1,l2)", 0, "Correlation listes l1 et l2", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"covariance(l1,l2)", 0, "Covariance listes l1 et l2", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"cpartfrac(p,x)", 0, "Decomposition en elements simples sur C.", "1/(x^4-1)", 0, CAT_CATEGORY_ALGEBRA | (CAT_CATEGORY_COMPLEXNUM << 8)},
  {"crayon ", "crayon ", "Couleur de trace de la tortue", "#crayon rouge", 0, CAT_CATEGORY_LOGO},
  {"cross(u,v)", 0, "Produit vectoriel de u et v.","[1,2,3],[0,1,3]", 0, CAT_CATEGORY_LINALG},
  {"csolve(equation,x)", 0, "Resolution exacte dans C d'une equation en x (ou d'un systeme polynomial).","x^2+x+1=0", 0, CAT_CATEGORY_SOLVE | (CAT_CATEGORY_COMPLEXNUM << 8)},
  {"curl(u,vars)", 0, "Rotationnel du vecteur u.", "[2*x*y,x*z,y*z],[x,y,z]", 0, CAT_CATEGORY_LINALG},
  {"cyan", "cyan", "Option d'affichage", "#display=cyan", 0, CAT_CATEGORY_PROGCMD},
  {"debug(f(args))", 0, "Execute la fonction f en mode pas a pas.", 0, 0, CAT_CATEGORY_PROG},
  {"degree(p,x)", 0, "Degre du polynome p en x.", "x^4-1", 0, CAT_CATEGORY_POLYNOMIAL},
  {"denom(x)", 0, "Denominateur de l'expression x.", "3/4", 0, CAT_CATEGORY_POLYNOMIAL},
  {"desolve(equation,t,y)", 0, "Resolution exacte d'equation differentielle ou de systeme differentiel lineaire a coefficients constants.", "[y'+y=exp(x),y(0)=1]", "[y'=[[1,2],[2,1]]*y+[x,x+1],y(0)=[1,2]]", CAT_CATEGORY_SOLVE | (CAT_CATEGORY_CALCULUS << 8)},
  {"det(A)", 0, "Determinant de la matrice A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX |  (CAT_CATEGORY_LINALG<<8)},
  {"diff(f,var,[n])", 0, "Derivee de l'expression f par rapport a var (a l'ordre n, n=1 par defaut), par exemple diff(sin(x),x) ou diff(x^3,x,2). Pour deriver f par rapport a x, utiliser f' (raccourci F3). Pour le gradient de f, var est la liste des variables.", "sin(x),x", "sin(x^2),x,3", CAT_CATEGORY_CALCULUS},
  {"display", "display", "Option d'affichage", "#display=red", 0, CAT_CATEGORY_PROGCMD},
  {"disque n", "disque ", "Cercle rempli tangent a la tortue, de rayon n. Utiliser disque n,theta pour remplir un morceau de camembert ou disque n,theta,segment pour remplir un segment de disque", "#disque 30", "#disque(30,90)", CAT_CATEGORY_LOGO},
  {"dot(a,b)", 0, "Produit scalaire de 2 vecteurs. Raccourci: *", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_LINALG},
  {"draw_arc(x1,y1,rx,ry,theta1,theta2,c)", 0, "Arc d'ellipse pixelise.", "100,100,60,80,0,pi,magenta", 0, CAT_CATEGORY_PROGCMD},
  {"draw_circle(x1,y1,r,c)", 0, "Cercle pixelise. Option filled pour le remplir.", "100,100,60,cyan+filled", 0, CAT_CATEGORY_PROGCMD},
  {"draw_line(x1,y1,x2,y2,c)", 0, "Droite pixelisee.", "100,50,300,200,blue", 0, CAT_CATEGORY_PROGCMD},
  {"draw_pixel(x,y,color)", 0, "Colorie le pixel x,y. Faire draw_pixel() pour synchroniser l'ecran.", 0, 0, CAT_CATEGORY_PROGCMD},
  {"draw_polygon([[x1,y1],...],c)", 0, "Polygone pixelise.", "[[100,50],[30,20],[60,70]],red+filled", 0, CAT_CATEGORY_PROGCMD},
  {"draw_rectangle(x,y,w,h,c)", 0, "Rectangle pixelise.", "100,50,30,20,red+filled", 0, CAT_CATEGORY_PROGCMD},
  {"draw_string(s,x,y,c)", 0, "Affiche la chaine s en x,y", "\"Bonjour\",80,60", 0, CAT_CATEGORY_PROGCMD},
  {"droite(equation)", 0, "Droite donnee par une equation ou 2 points", "y=2x+1", "1+i,2-i", CAT_CATEGORY_PROGCMD},
  {"ecris ", "ecris ", "Ecrire a la position de la tortue", "#ecris \"coucou\"", 0, CAT_CATEGORY_LOGO},
  {"efface", "efface", "Remise a zero de la tortue", 0, 0, CAT_CATEGORY_LOGO},
  {"egcd(A,B)", 0, "Cherche des polynomes U,V,D tels que A*U+B*V=D=gcd(A,B)","x^2+3x+1,x^2-5x-1", 0, CAT_CATEGORY_POLYNOMIAL},
  {"eigenvals(A)", 0, "Valeurs propres de la matrice A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX |  (CAT_CATEGORY_LINALG<<8)},
  {"eigenvects(A)", 0, "Vecteurs propres de la matrice A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX |  (CAT_CATEGORY_LINALG<<8)},
  {"elif (test)", "elif", "Tests en cascade", 0, 0, CAT_CATEGORY_PROG},
  //{"end", "end", "Fin de bloc", 0, 0, CAT_CATEGORY_PROG},
  {"erf(x)", 0, "Fonction erreur en x.", "1.2", 0, CAT_CATEGORY_PROBA},
  {"erfc(x)", 0, "Fonction erreur complementaire en x.", "1.2", 0, CAT_CATEGORY_PROBA},
  {"euler(n)",0,"Indicatrice d'Euler: nombre d'entiers < n premiers avec n","25",0,CAT_CATEGORY_ARIT},
  {"eval(f)", 0, "Evalue f.", 0, 0, CAT_CATEGORY_PROGCMD},
  {"evalc(z)", 0, "Ecrit z=x+i*y.", "1/(1+i*sqrt(3))", 0, CAT_CATEGORY_COMPLEXNUM},
  {"exact(x)", 0, "Convertit x en rationnel. Raccourci shift S-D", "1.2", 0, CAT_CATEGORY_REAL},
  {"exp2trig(expr)", 0, "Conversion d'exponentielles complexes en sin/cos", "exp(i*x)", 0, CAT_CATEGORY_TRIG},
  {"exponential_regression(Xlist,Ylist)", 0, "Regression exponentielle.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"exponentiald(lambda,x)", 0, "Loi exponentielle de parametre lambda. exponentiald_cdf(lambda,x) probabilite que \"loi exponentielle <=x\" par ex. exponentiald_cdf(2,3). exponentiald_icdf(lambda,t) renvoie x tel que \"loi exponentielle <=x\" vaut t, par ex. exponentiald_icdf(2,0.95) ", "5.1,3.4", 0, CAT_CATEGORY_PROBA},
  {"extend", 0, "Concatene 2 listes. Attention a ne pas utiliser + qui effectue l'addition de 2 vecteurs.","#l1.extend(l2)", 0, CAT_CATEGORY_LIST},
  {"factor(p,[x])", 0, "Factorisation du polynome p (utiliser ifactor pour un entier). Raccourci: p=>*", "x^4-1", "x^6+1,sqrt(3)", CAT_CATEGORY_ALGEBRA | (CAT_CATEGORY_POLYNOMIAL << 8)},
  {"filled", "filled", "Option d'affichage", 0, 0, CAT_CATEGORY_PROGCMD},
  {"float(x)", 0, "Convertit x en nombre approche (flottant).", "pi", 0, CAT_CATEGORY_REAL},
  {"floor(x)", 0, "Partie entiere de x", "pi", 0, CAT_CATEGORY_REAL},
  {"fonction f(x)", "fonction", "Definition de fonction (Xcas). Par exemple\nfonction f(x)\n local y;\ny:=x*x;\nreturn y;\nffonction", 0, 0, CAT_CATEGORY_PROG},
  {"from math/... import *", "from math import *", "Instruction pour utiliser les fonctions de maths ou des fonctions aleatoires [random] ou la tortue en anglais [turtle]. Importer math n'est pas necessaire dans KhiCAS", "#from random import *", "#from turtle import *", CAT_CATEGORY_PROG},
  {"fsolve(equation,x=a[..b])", 0, "Resolution approchee de equation pour x dans l'intervalle a..b ou en partant de x=a.","cos(x)=x,x=0..1", "cos(x)-x,x=0.0", CAT_CATEGORY_SOLVE},
  {"gauss(q)", 0, "Reduction d'une forme quadratique", "x^2+x*y+x*z+y^2+z^2,[x,y,z]", 0, CAT_CATEGORY_LINALG},
  {"gcd(a,b,...)", 0, "Plus grand commun diviseur. Voir iegcd ou egcd pour Bezout.", "23,13", "x^2-1,x^3-1", CAT_CATEGORY_ARIT | (CAT_CATEGORY_POLYNOMIAL << 8)},
  {"gl_x", "gl_x", "Reglage graphique X gl_x=xmin..xmax", "#gl_x=0..2", 0, CAT_CATEGORY_PROGCMD << 8},
  {"gl_y", "gl_y", "Reglage graphique Y gl_y=ymin..ymax", "#gl_y=-1..1", 0, CAT_CATEGORY_PROGCMD << 8},
  {"gramschmidt(M)", 0, "Orthonormalisation de Gram-Schmidt (vecteurs lignes ou famille libre)", "[[1,2,3],[4,5,6]]", "[1,1+x],(p,q)->integrate(p*q,x,-1,1)", CAT_CATEGORY_LINALG},
  {"green", "green", "Option d'affichage", "#display=green", 0, CAT_CATEGORY_PROGCMD},
  {"halftan(expr)", 0, "Exprime cos, sin, tan avec tan(angle/2).","cos(x)", 0, CAT_CATEGORY_TRIG},
  {"hermite(n)", 0, "n-ieme polynome de Hermite", "10", "10,t", CAT_CATEGORY_POLYNOMIAL},
  {"hilbert(n)", 0, "Matrice de Hilbert de taille n.", "4", 0, CAT_CATEGORY_MATRIX},
  {"histogram(list,min,size)", 0, "Histogramme d'une liste de donneees, classes commencant a min de taille size.","ranv(100,uniformd,0,1),0,0.1", 0, CAT_CATEGORY_STATS},
  {"iabcuv(a,b,c)", 0, "Cherche 2 entiers u,v tels que a*u+b*v=c","23,13,15", 0, CAT_CATEGORY_ARIT},
  {"ichinrem([a,m],[b,n])", 0,"Restes chinois entiers de a mod m et b mod n.", "[3,13],[2,7]", 0, CAT_CATEGORY_ARIT},
  {"idivis(n)", 0, "Liste des diviseurs d'un entier n.", "10", 0, CAT_CATEGORY_ARIT},
  {"idn(n)", 0, "matrice identite n * n", "4", 0, CAT_CATEGORY_MATRIX},
  {"iegcd(a,b)", 0, "Determine les entiers u,v,d tels que a*u+b*v=d=gcd(a,b)","23,13", 0, CAT_CATEGORY_ARIT},
  {"ifactor(n)", 0, "Factorisation d'un entier (pas trop grand!). Raccourci n=>*", "1234", 0, CAT_CATEGORY_ARIT},
  {"ilaplace(f,s,x)", 0, "Transformee inverse de Laplace de f", "s/(s^2+1),s,x", 0, CAT_CATEGORY_CALCULUS},
  {"im(z)", 0, "Partie imaginaire.", "1+i", 0, CAT_CATEGORY_COMPLEXNUM},
  {"inf", "inf", "Plus l'infini. Utiliser -inf pour moins l'infini ou infinity pour l'infini complexe. Raccourci shift INS.", "-inf", "infinity", CAT_CATEGORY_CALCULUS},
  {"input()", "input()", "Lire une chaine au clavier", "\"Valeur ?\"", 0, CAT_CATEGORY_PROG},
  {"integrate(f,x,[,a,b])", 0, "Primitive de f par rapport a la variable x, par ex. integrate(x*sin(x),x). Pour calculer une integrale definie, entrer les arguments optionnels a et b, par ex. integrate(x*sin(x),x,0,pi). Raccourci SHIFT F3.", "x*sin(x),x", "cos(x)/(1+x^4),x,0,inf", CAT_CATEGORY_CALCULUS},
  {"interp(X,Y[,interp])", 0, "Interpolation de Lagrange aux points (xi,yi) avec X la liste des xi et Y des yi. Renvoie la liste des differences divisees si interp est passe en parametre.", "[1,2,3,4,5],[0,1,3,4,4]", "[1,2,3,4,5],[0,1,3,4,4],interp", CAT_CATEGORY_POLYNOMIAL},
  {"inv(A)", 0, "Inverse de A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX |  (CAT_CATEGORY_LINALG<<8)},
  {"inverser(v)", "inverser ", "La variable v est remplacee par son inverse", "#v:=3; inverser v", 0, CAT_CATEGORY_SOFUS},
  {"iquo(a,b)", 0, "Quotient euclidien de deux entiers.", "23,13", 0, CAT_CATEGORY_ARIT},
  {"irem(a,b)", 0,"Reste euclidien de deux entiers", "23,13", 0, CAT_CATEGORY_ARIT},
  {"isprime(n)", 0, "Renvoie 1 si n est premier, 0 sinon.", "11", "10", CAT_CATEGORY_ARIT},
  {"jordan(A)", 0, "Forme normale de Jordan de la matrice A, renvoie P et D tels que P^-1*A*P=D", "[[1,2],[3,4]]", "[[1,1,-1,2,-1],[2,0,1,-4,-1],[0,1,1,1,1],[0,1,2,0,1],[0,0,-3,3,-1]]", CAT_CATEGORY_MATRIX},
  {"laguerre(n,a,x)", 0, "n-ieme polynome de Laguerre (a=0 par defaut).", "10", 0, CAT_CATEGORY_POLYNOMIAL},
  {"laplace(f,x,s)", 0, "Transformee de Laplace de f","sin(x),x,s", 0, CAT_CATEGORY_CALCULUS},
  {"lcm(a,b,...)", 0, "Plus petit commun multiple.", "23,13", "x^2-1,x^3-1", CAT_CATEGORY_ARIT | (CAT_CATEGORY_POLYNOMIAL << 8)},
  {"lcoeff(p,x)", 0, "Coefficient dominant du polynome p.", "x^4-1", 0, CAT_CATEGORY_POLYNOMIAL},
  {"legendre(n)", 0, "n-ieme polynome de Legendre.", "10", "10,t", CAT_CATEGORY_POLYNOMIAL},
#ifdef RELEASE
  {"len(l)", 0, "Taille d'une liste.", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_LIST},
#endif
  {"leve_crayon ", "leve_crayon ", "La tortue se deplace sans marquer son passage", 0, 0, CAT_CATEGORY_LOGO},
  {"limit(f,x=a)", 0, "Limite de f en x = a. Ajouter 1 ou -1 pour une limite a droite ou a gauche, limit(sin(x)/x,x=0) ou limit(abs(x)/x,x=0,1). Raccourci: SHIFT MIXEDFRAC", "sin(x)/x,x=0", "exp(-1/x),x=0,1", CAT_CATEGORY_CALCULUS},
  {"line_width_", "line_width_", "Prefixe d'epaisseur (2 a 8)", 0, 0, CAT_CATEGORY_PROGCMD},
  {"linear_regression(Xlist,Ylist)", 0, "Regression lineaire.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"linear_regression_plot(Xlist,Ylist)", 0, "Graphe d'une regression lineaire.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];linear_regression_plot(X,Y);scatterplot(X,Y)", 0, CAT_CATEGORY_STATS },
  {"linetan(expr,x,x0)", 0, "Tangente au graphe en x=x0.", "sin(x),x,pi/2", 0, CAT_CATEGORY_PLOT},
  {"linsolve([eq1,eq2,..],[x,y,..])", 0, "Resolution de systeme lineaire. Peut utiliser le resultat de lu pour resolution en O(n^2).","[x+y=1,x-y=2],[x,y]", "#p,l,u:=lu([[1,2],[3,4]]); linsolve(p,l,u,[5,6])", CAT_CATEGORY_SOLVE | (CAT_CATEGORY_LINALG <<8) | (CAT_CATEGORY_MATRIX << 16)},
  {"logarithmic_regression(Xlist,Ylist)", 0, "Regression logarithmique.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"lu(A)", 0, "decomposition LU de la matrice A, P*A=L*U, renvoie P permutation, L et U triangulaires inferieure et superieure", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
  {"magenta", "magenta", "Option d'affichage", "#display=magenta", 0, CAT_CATEGORY_PROGCMD},
  {"map(l,f)", 0, "Applique f aux elements de la liste l.","[1,2,3],x->x^2", 0, CAT_CATEGORY_LIST},
  {"matpow(A,n)", 0, "Renvoie A^n, la matrice A la puissance n", "[[1,2],[3,4]],n","#assume(n>=1);matpow([[0,2],[0,4]],n)", CAT_CATEGORY_MATRIX},
  {"matrix(l,c,func)", 0, "Matrice de terme general donne.", "2,3,(j,k)->j^k", 0, CAT_CATEGORY_MATRIX},
  {"mean(l)", 0, "Moyenne arithmetique liste l", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS},
  {"median(l)", 0, "Mediane", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS},
  {"montre_tortue ", "montre_tortue ", "Affiche la tortue", 0, 0, CAT_CATEGORY_LOGO},
  {"mult_c_conjugate", 0, "Multiplier par le conjugue complexe.", "1+2*i", 0,  (CAT_CATEGORY_COMPLEXNUM << 8)},
  {"mult_conjugate", 0, "Multiplier par le conjugue (sqrt).", "sqrt(2)-sqrt(3)", 0, CAT_CATEGORY_ALGEBRA},
  {"normald([mu,sigma],x)", 0, "Loi normale, par defaut mu=0 et sigma=1. normald_cdf([mu,sigma],x) probabilite que \"loi normale <=x\" par ex. normald_cdf(1.96). normald_icdf([mu,sigma],t) renvoie x tel que \"loi normale <=x\" vaut t, par ex. normald_icdf(0.975) ", "1.2", 0, CAT_CATEGORY_PROBA},
  {"not(x)", 0, "Non logique.", 0, 0, CAT_CATEGORY_PROGCMD},
  {"numer(x)", 0, "Numerateur de x.", "3/4", 0, CAT_CATEGORY_POLYNOMIAL},
  {"odesolve(f(t,y),[t,y],[t0,y0],t1)", 0, "Solution approchee d'equation differentielle y'=f(t,y) et y(t0)=y0, valeur en t1 (ajouter curve pour les valeurs intermediaires de y)", "sin(t*y),[t,y],[0,1],2", "0..pi,(t,v)->{[-v[1],v[0]]},[0,1]", CAT_CATEGORY_SOLVE},
  {"partfrac(p,x)", 0, "Decomposition en elements simples. Raccourci p=>+", "1/(x^4-1)", 0, CAT_CATEGORY_ALGEBRA},
  {"pas_de_cote n", "pas_de_cote ", "Saut lateral de la tortue, par defaut n=10", "#pas_de_cote 30", 0, CAT_CATEGORY_LOGO},
  {"plot(expr,x)", 0, "Graphe de fonction. Par exemple plot(sin(x)), plot(ln(x),x.0,5)", "ln(x),x=0..5", "1/x,x=1..5,xstep=1", CAT_CATEGORY_PLOT},
#ifdef RELEASE
  {"plotarea(expr,x=a..b,[n,meth])", 0, "Aire sous la courbe selon une methode d'integration.", "1/x,x=1..3,2,trapeze", 0, CAT_CATEGORY_PLOT},
#endif
  {"plotparam([x,y],t)", 0, "Graphe en parametriques. Par exemple plotparam([sin(3t),cos(2t)],t,0,pi) ou plotparam(exp(i*t),t,0,pi)", "[sin(3t),cos(2t)],t,0,pi", "[t^2,t^3],t=-1..1,tstep=0.1", CAT_CATEGORY_PLOT},
  {"plotpolar(r,theta)", 0, "Graphe en polaire.","cos(3*x),x,0,pi", "1/(1+cos(x)),x=0..pi,xstep=0.05", CAT_CATEGORY_PLOT},
  {"plotcontour(expr,[x=xm..xM,y=ym..yM],niveaux)", 0, "Lignes de niveau de expr.", "x^2+2y^2, [x=-2..2,y=-2..2],[1,2]", 0, CAT_CATEGORY_PLOT},
  {"plotfield(f(t,y), [t=tmin..tmax,y=ymin..ymax])", 0, "Champ des tangentes de y'=f(t,y), optionnellement graphe avec plotode=[t0,y0]", "sin(t*y), [t=-3..3,y=-3..3],plotode=[0,1]", "5*[-y,x], [x=-1..1,y=-1..1]", CAT_CATEGORY_PLOT},
  {"plotode(f(t,y), [t=tmin..tmax,y],[t0,y0])", 0, "Graphe de solution d'equation differentielle y'=f(t,y), y(t0)=y0.", "sin(t*y),[t=-3..3,y],[0,1]", 0, CAT_CATEGORY_PLOT},
  {"plotlist(list)", 0, "Graphe d'une liste", "[3/2,2,1,1/2,3,2,3/2]", "[1,13],[2,10],[3,15],[4,16]", CAT_CATEGORY_PLOT},
  {"plotseq(f(x),x=[u0,m,M],n)", 0, "Trace f(x) sur [m,M] et n termes de la suite recurrente u_{n+1}=f(u_n) de 1er terme u0.","sqrt(2+x),x=[6,0,7],5", 0, CAT_CATEGORY_PLOT},
  {"plus_point", "plus_point", "Option d'affichage", "#display=blue+plus_point", 0, CAT_CATEGORY_PROGCMD },
  {"point(x,y)", 0, "Point", "1,2", 0, CAT_CATEGORY_PROGCMD},
  {"polygon(list)", 0, "Polygone ferme.", "1-i,2+i,3", 0, CAT_CATEGORY_PROGCMD},
  {"polygonscatterplot(Xlist,Ylist)", 0, "Nuage de points relies.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"polynomial_regression(Xlist,Ylist,n)", 0, "Regression polynomiale de degre <= n.", "[1,2,3,4,5],[0,1,3,4,4],2", 0, CAT_CATEGORY_STATS},
  {"pour (boucle Xcas)", "pour  de  jusque  faire  fpour;", "Boucle definie.","#pour j de 1 jusque 10 faire print(j,j^2); fpour;", 0, CAT_CATEGORY_PROG},
  {"power_regression(Xlist,Ylist,n)", 0, "Regression puissance.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"powmod(a,n,p[,P,x])", 0, "Renvoie a^n mod p, ou a^n mod un entier p et un polynome P en x.","123,456,789", "x+1,452,19,x^4+x+1,x", CAT_CATEGORY_ARIT},
  {"print(expr)", 0, "Afficher dans la console", 0, 0, CAT_CATEGORY_PROG},
  {"proot(p)", 0, "Racines reelles et complexes approchees d'un polynome. Exemple proot([1,2.1,3,4.2]) ou proot(x^3+2.1*x^2+3x+4.2)", "x^3+2.1*x^2+3x+4.2", 0, CAT_CATEGORY_POLYNOMIAL},
  {"purge(x)", 0, "Purge le contenu de la variable x. Raccourci SHIFT-FORMAT", 0, 0, CAT_CATEGORY_PROGCMD | (CAT_CATEGORY_SOFUS<<8)},
  {"python(f)", 0, "Affiche la fonction f en syntaxe Python.", 0, 0, CAT_CATEGORY_PROGCMD},
  {"python_compat(0|1|2)", 0, "python_compat(0) syntaxe Xcas, python_compat(1) syntaxe Python avec ^ interprete comme puissance, python_compat(2) ^ interprete comme ou exclusif bit a bit", "0", "1", CAT_CATEGORY_PROG},
  {"q2a(expr,[vars])", 0, "Matrix of a quadratic form", "x^2+3*x*y","x^2+3*x*y,[x,y]", CAT_CATEGORY_LINALG},
  {"qr(A)", 0, "Factorisation A=Q*R avec Q orthogonale et R triangulaire superieure", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
  {"quartile1(l)", 0, "1er quartile", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS},
  {"quartile3(l)", 0, "3eme quartile", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS},
  {"quo(p,q,x)", 0, "Quotient de division euclidienne polynomiale en x.", 0, 0, CAT_CATEGORY_POLYNOMIAL},
  {"quote(x)", 0, "Renvoie l'expression x non evaluee.", 0, 0, CAT_CATEGORY_ALGEBRA},
  {"rand()", "rand()", "Reel aleatoire entre 0 et 1", 0, 0, CAT_CATEGORY_PROBA},
  {"randint(n)", 0, "Entier aleatoire entre 1 et n ou entre a et b si on fournit 2 arguments", "6", "5,20", CAT_CATEGORY_PROBA},
  {"ranm(n,m,[loi,parametres])", 0, "Matrice aleatoire a coefficients entiers ou selon une loi de probabilites (ranv pour un vecteur). Exemples ranm(2,3), ranm(3,2,binomial,20,.3), ranm(4,2,normald,0,1)", "4,2,normald,0,1", "3,3,10", CAT_CATEGORY_MATRIX},
  {"ranv(n,[loi,parametres])", 0, "Vecteur aleatoire", "4,normald,0,1", "10,30", CAT_CATEGORY_LINALG},
  {"ratnormal(x)", 0, "Ecrit sous forme d'une fraction irreductible.", "(x+1)/(x^2-1)^2", 0, CAT_CATEGORY_ALGEBRA},
  {"re(z)", 0, "Partie reelle.", "1+i", 0, CAT_CATEGORY_COMPLEXNUM},
  {"read(\"filename\")", "read(\"", "Lire un fichier. Voir aussi write", 0, 0, CAT_CATEGORY_PROGCMD},
  {"rectangle_plein a,b", "rectangle_plein ", "Rectangle direct rempli depuis la tortue de cotes a et b (si b est omis, la tortue remplit un carre)", "#rectangle_plein 30", "#rectangle_plein(20,40)", CAT_CATEGORY_LOGO},
  {"recule n", "recule ", "La tortue recule de n pas, par defaut n=10", "#recule 30", 0, CAT_CATEGORY_LOGO},
  {"red", "red", "Option d'affichage", "#display=red", 0, CAT_CATEGORY_PROGCMD},
  {"rem(p,q,x)", 0, "Reste de division euclidienne polynomiale en x", 0, 0, CAT_CATEGORY_POLYNOMIAL},
  {"repete(n,...)", "repete( ", "Repete plusieurs fois les instructions", "#repete(4,avance,tourne_gauche)", 0, CAT_CATEGORY_LOGO},
#ifdef RELEASE
  {"residue(f(z),z,z0)", 0, "Residu de l'expression en z0.", "1/(x^2+1),x,i", 0, CAT_CATEGORY_COMPLEXNUM},
#endif
  {"resultant(p,q,x)", 0, "Resultant en x des polynomes p et q.", "#P:=x^3+p*x+q;resultant(P,P',x);", 0, CAT_CATEGORY_POLYNOMIAL},
  {"revert(p[,x])", 0, "Developpement de Taylor reciproque, p doit etre nul en 0","x+x^2+x^4", 0, CAT_CATEGORY_CALCULUS},
  {"rgb(r,g,b)", 0, "couleur definie par niveau de rouge, vert, bleu entre 0 et 255", "255,0,255", 0, CAT_CATEGORY_PROGCMD},
  {"rhombus_point", "rhombus_point", "Option d'affichage", "#display=magenta+rhombus_point", 0, CAT_CATEGORY_PROGCMD },
  {"rond n", "rond ", "Cercle tangent a la tortue de rayon n. Utiliser rond n,theta pour un arc de cercle.", "#rond 30", "#rond(30,90)", CAT_CATEGORY_LOGO},
  {"rref(A)", 0, "Pivot de Gauss", "[[1,2,3],[4,5,6]]", 0, CAT_CATEGORY_MATRIX|  (CAT_CATEGORY_LINALG<<8)},
  {"rsolve(equation,u(n),[init])", 0, "Expression d'une suite donnee par une recurrence.","u(n+1)=2*u(n)+3,u(n),u(0)=1", "([u(n+1)=3*v(n)+u(n),v(n+1)=v(n)+u(n)],[u(n),v(n)],[u(0)=1,v(0)=2]", CAT_CATEGORY_SOLVE},
  {"saute n", "saute ", "La tortue fait un saut de n pas, par defaut n=10", "#saute 30", 0, CAT_CATEGORY_LOGO},
  {"scatterplot(Xlist,Ylist)", 0, "Nuage de points.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"segment(A,B)", 0, "Segment", "1,2+i", 0, CAT_CATEGORY_PROGCMD},
  {"seq(expr,var,a,b[,pas])", 0, "Liste de terme general donne.","j^2,j,1,10", "j^2,j,1,10,2", CAT_CATEGORY_LIST},
  {"si (test Xcas)", "si  alors  sinon  fsi;", "Test.", "#f(x):=si x>0 alors x; sinon -x; fsi;// valeur absolue", 0, CAT_CATEGORY_PROG},
  {"sign(x)", 0, "Renvoie -1 si x est negatif, 0 si x est nul et 1 si x est positif.", 0, 0, CAT_CATEGORY_REAL},
  {"simplify(expr)", 0, "Renvoie en general expr sous forme simplifiee. Raccourci expr=>/", "sin(3x)/sin(x)", "ln(4)-ln(2)", CAT_CATEGORY_ALGEBRA},
  {"solve(equation,x)", 0, "Resolution exacte d'une equation en x (ou d'un systeme polynomial). Utiliser csolve pour les solutions complexes, linsolve pour un systeme lineaire. Raccourci SHIFT XthetaT", "x^2-x-1=0,x", "[x^2-y^2=0,x^2-z^2=0],[x,y,z]", CAT_CATEGORY_SOLVE},
  {"sort(l)", 0, "Trie une liste.","[3/2,2,1,1/2,3,2,3/2]", "[[1,2],[2,3],[4,3]],(x,y)->when(x[1]==y[1],x[0]>y[0],x[1]>y[1]", CAT_CATEGORY_LIST},
  {"square_point", "square_point", "Option d'affichage", "#display=cyan+square_point", 0, CAT_CATEGORY_PROGCMD },
  {"star_point", "star_point", "Option d'affichage", "#display=magenta+star_point", 0, CAT_CATEGORY_PROGCMD },
  {"stddev(l)", 0, "Ecart-type d'une liste l", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS},
  {"subst(a,b=c)", 0, "Remplace b par c dans a. Raccourci a(b=c). Pour faire plusieurs remplacements, saisir subst(expr,[b1,b2...],[c1,c2...])", "x^2,x=3", "x+y^2,[x,y],[1,2]", CAT_CATEGORY_ALGEBRA},
  {"sum(f,k,m,M)", 0, "Somme de l'expression f dependant de k pour k variant de m a M. Exemple sum(k^2,k,1,n)=>*. Raccourci ALPHA F3", "k,k,1,n", "k^2,k", CAT_CATEGORY_CALCULUS},
  {"svd(A)", 0, "Singular Value Decomposition, renvoie U orthogonale, S vecteur des valeurs singulières, Q orthogonale tels que A=U*diag(S)*tran(Q).", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
  {"tabvar(f,[x=a..b])", 0, "Tableau de variations de l'expression f, avec arguments optionnels la variable x dans l'intervalle a..b.", "sqrt(x^2+x+1)", "[cos(t),sin(3t)],t", CAT_CATEGORY_CALCULUS},
  {"tantque (boucle Xcas)", "tantque  faire  ftantque;", "Boucle indefinie.", "#j:=13; tantque j!=1 faire j:=when(even(j),j/2,3j+1); print(j); ftantque;", 0, CAT_CATEGORY_PROG},
  {"taylor(f,x=a,n,[polynom])", 0, "Developpement de Taylor de l'expression f en x=a a l'ordre n, ajouter le parametre polynom pour enlever le terme de reste.","sin(x),x=0,5", "sin(x),x=0,5,polynom", CAT_CATEGORY_CALCULUS},
  {"tchebyshev1(n)", 0, "Polynome de Tchebyshev de 1ere espece: cos(n*x)=T_n(cos(x))", "10", 0, CAT_CATEGORY_POLYNOMIAL},
  {"tchebyshev2(n)", 0, "Polynome de Tchebyshev de 2eme espece: sin((n+1)*x)=sin(x)*U_n(cos(x))", "10", 0, CAT_CATEGORY_POLYNOMIAL},
  {"tcollect(expr)", 0, "Linearisation trigonometrique et regroupement.","sin(x)+cos(x)", 0, CAT_CATEGORY_TRIG},
  {"texpand(expr)", 0, "Developpe les fonctions trigonometriques, exp et ln.","sin(3x)", "ln(x*y)", CAT_CATEGORY_TRIG},
  {"time(cmd)", 0, "Temps pour effectuer une commande ou mise a l'heure de horloge","int(1/(x^4+1),x)","8,0", CAT_CATEGORY_PROG},
  {"tlin(expr)", 0, "Linearisation trigonometrique de l'expression.","sin(x)^3", 0, CAT_CATEGORY_TRIG},
  {"tourne_droite n", "tourne_droite ", "La tortue tourne de n degres, par defaut n=90", "#tourne_droite 45", 0, CAT_CATEGORY_LOGO},
  {"tourne_gauche n", "tourne_gauche ", "La tortue tourne de n degres, par defaut n=90", "#tourne_gauche 45", 0, CAT_CATEGORY_LOGO},
  {"trace(A)", 0, "Trace de la matrice A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
  {"tran(A)", 0, "Transposee de la matrice A. Pour la transconjuguee utiliser trn(A) ou A^*.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX |  (CAT_CATEGORY_LINALG<<8)},
  {"triangle_point", "triangle_point", "Option d'affichage", "#display=yellow+triangle_point", 0, CAT_CATEGORY_PROGCMD},
  {"trig2exp(expr)", 0, "Convertit les fonctions trigonometriques en exponentielles.","cos(x)^3", 0, CAT_CATEGORY_TRIG},
  {"trigcos(expr)", 0, "Exprime sin^2 et tan^2 avec cos^2.","sin(x)^4", 0, CAT_CATEGORY_TRIG},
  {"trigsin(expr)", 0, "Exprime cos^2 et tan^2 avec sin^2.","cos(x)^4", 0, CAT_CATEGORY_TRIG},
  {"trigtan(expr)", 0, "Exprime cos^2 et sin^2 avec tan^2.","cos(x)^4", 0, CAT_CATEGORY_TRIG},
  {"uniformd(a,b,x)", 0, "loi uniforme sur [a,b] de densite 1/(b-a)", 0, 0, CAT_CATEGORY_PROBA},
  {"v augmente_de n", " augmente_de ", "La variable v augmente de n, ou de n %", "#v:=3; v augmente_de 1", 0, CAT_CATEGORY_SOFUS},
  {"v diminue_de n", " diminue_de ", "La variable v diminue de n, ou de n %", "#v:=3; v diminue_de 1", 0, CAT_CATEGORY_SOFUS},
  {"v est_divise_par n", " est_divise_par ", "La variable v est divisee par n", "#v:=3; v est_divise_par 2", 0, CAT_CATEGORY_SOFUS},
  {"v est_eleve_puissance n", " est_eleve_puissance ", "La variable v est eleveee a la puissance n", "#v:=3; v est_eleve_puissance 2", 0, CAT_CATEGORY_SOFUS},
  {"v est_multiplie_par n", " est_multiplie_par ", "La variable v est multipliee par n", "#v:=3; v est_multiplie_par 2", 0, CAT_CATEGORY_SOFUS},
  //{"version", "version()", "Khicas 1.5.0, (c) B. Parisse et al. www-fourier.ujf-grenoble.fr/~parisse. License GPL version 2. Interface adaptee d'Eigenmath pour Casio, G. Maia, http://gbl08ma.com", 0, 0, CAT_CATEGORY_PROGCMD},
  {"write(\"filename\",var)", "write(\"", "Sauvegarde une ou plusieurs variables dans un fichier. Par exemple f(x):=x^2; write(\"func_f\",f).",  0, 0, CAT_CATEGORY_PROGCMD},
  {"yellow", "yellow", "Option d'affichage", "#display=yellow", 0, CAT_CATEGORY_PROGCMD},
  {"|", "|", "Ou logique", "#1|2", 0, CAT_CATEGORY_PROGCMD},
  {"~", "~", "Complement", "#~7", 0, CAT_CATEGORY_PROGCMD},
};

const char aide_khicas_string[]="Aide Khicas";
const char shortcuts_string[]="Pour mettre a l'heure l'horloge, tapez heure,minute puis touche STO puis , par exemple 13,10=>,\n\nRaccourcis clavier (shell et editeur)\nF1-F3: selon legendes\nF4: catalogue\nF5: blocage en minuscule ou bascule minuscule/majuscule\nF6: menu fichier, configuration\nshift-PRGM: caracteres pour programmer et commande debug\nshift-FRAC: graphiques plot...\n\nOPTN: options\nshift-QUIT: tortue\nshift-Lst: listes\nshift-Mtr: matrices\nVARS: liste des variables (shell) ou dessin tortue (editeur)\n=>+: partfrac\n=>*: factor\n=>sin/cos/tan\n=>=>: solve\nHistorique calculs:\nF3 Editeur 2d ou graphique ou texte selon objet\nshift-F3: editeur texte\n+ ou - modifie un parametre\n\nEditeur d'expressions\npave directionnel: deplace la selection dans l'arborescence de l'expression\nshift-droit/gauche echange selection avec argument a droite ou a gauche\nALPHA-droit/gauche dans une somme ou un produit: augmente la selection avec argument droit ou gauche\nF3: Editer selection, shift-F3: taille police + grande, ALPHA-F3: taille plus petite\nF4: catalogue\nF5: minuscule/majuscule\nF6: Evaluer la selection, shift-F6: valeur approchee, ALPHA-F6: commande regroup\nDEL: supprime l'operateur racine de la selection\n\nEditeur de scripts\nshift-CLIP: marque le debut de la selection, deplacer le curseur vers la fin puis DEL pour effacer ou shift-CLIP pour copier sans effacer. shift-PASTE pour coller.\nF6-6 recherche seule: entrer un mot puis EXE puis EXIT. Taper EXE pour l'occurence suivante, AC pour annuler.\nF6-6 remplacer: entrer un mot puis EXE puis le remplacement et EXE. Taper EXE ou EXIT pour remplacer ou non et passer a l'occurence suivante, AC pour annuler\nshift-Ans: tester syntaxe\n\nRaccourcis Graphes:\n+ - zoom\n(-): zoomout selon y\n*: autoscale\n/: orthonormalisation\nOPTN: axes on/off";
const char apropos_string[]="Khicas 1.5.0, (c) 2019 B. Parisse et al. www-fourier.univ-grenoble-alpes.fr/~parisse.\nLicense GPL version 2.\nInterface adaptee d'Eigenmath pour Casio, G. Maia (http://gbl08ma.com), Mike Smith, Nemhardy, LePhenixNoir, ...";

int CAT_COMPLETE_COUNT=sizeof(completeCat)/sizeof(catalogFunc);

ustl::string insert_string(int index){
  ustl::string s;
  if (completeCat[index].insert)
    s=completeCat[index].insert;
  else {
    s=completeCat[index].name;
    int pos=s.find('(');
    if (pos>=0 && pos<s.size())
      s=s.substr(0,pos+1);
  }
  return s;//s+' ';
}

int showCatalog(char* insertText,int preselect,int menupos) {
  // returns 0 on failure (user exit) and 1 on success (user chose a option)
  MenuItem menuitems[CAT_CATEGORY_LOGO+1];
  menuitems[CAT_CATEGORY_ALL].text = (char*)"Tout";
  menuitems[CAT_CATEGORY_ALGEBRA].text = (char*)"Algebre";
  menuitems[CAT_CATEGORY_LINALG].text = (char*)"Alg. (bi)lineaire";
  menuitems[CAT_CATEGORY_CALCULUS].text = (char*)"Analyse";
  menuitems[CAT_CATEGORY_ARIT].text = (char*)"Arithmetic, crypto";
  menuitems[CAT_CATEGORY_COMPLEXNUM].text = (char*)"Complexes";
  menuitems[CAT_CATEGORY_PLOT].text = (char*)"Graphes";
  menuitems[CAT_CATEGORY_POLYNOMIAL].text = (char*)"Polynomes";
  menuitems[CAT_CATEGORY_PROBA].text = (char*)"Probabilites";
  menuitems[CAT_CATEGORY_PROG].text = (char*)"Programmes PRGM";
  menuitems[CAT_CATEGORY_PROGCMD].text = (char*)"Programmes cmds";
  menuitems[CAT_CATEGORY_REAL].text = (char*)"Reels";
  menuitems[CAT_CATEGORY_SOLVE].text = (char*)"Resoudre";
  menuitems[CAT_CATEGORY_STATS].text = (char*)"Statistiques";
  menuitems[CAT_CATEGORY_TRIG].text = (char*)"Trigonometrie";
  menuitems[CAT_CATEGORY_OPTIONS].text = (char*)"Options";
  menuitems[CAT_CATEGORY_LIST].text = (char*)"Listes";
  menuitems[CAT_CATEGORY_MATRIX].text = (char*)"Matrices";
  menuitems[CAT_CATEGORY_SOFUS].text = (char*)"Modifier des variables";
  menuitems[CAT_CATEGORY_LOGO].text = (char*)"Tortue shift QUIT";
  
  Menu menu;
  menu.items=menuitems;
  menu.numitems=sizeof(menuitems)/sizeof(MenuItem);
  menu.scrollout=1;
  menu.title = (char*)"Liste de commandes";
  //puts("catalog 1");
  while(1) {
    if (preselect)
      menu.selection=preselect;
    else {
      if (menupos>0)
	menu.selection=menupos;
      int sres = doMenu(&menu);
      if (sres != MENU_RETURN_SELECTION)
	return 0;
    }
    // puts("catalog 3");
    if(doCatalogMenu(insertText, menuitems[menu.selection-1].text, menu.selection-1)) {
      const char * ptr=0;
      if (strcmp("matrix ",insertText)==0 && (ptr=input_matrix(false)) )
	return 0;
      if (strcmp("list ",insertText)==0 && (ptr=input_matrix(true)) )
	return 0;
      return 1;
    }
    if (preselect)
      return 0;
  }
  return 0;
}

using namespace giac;
// 0 on exit, 1 on success
int doCatalogMenu(char* insertText, const char* title, int category,const char * cmdname) {
  for (;;){
    int allcmds=builtin_lexer_functions_end()-builtin_lexer_functions_begin();
    int allopts=lexer_tab_int_values_end-lexer_tab_int_values_begin;
    bool isall=category==CAT_CATEGORY_ALL;
    bool isopt=category==CAT_CATEGORY_OPTIONS;
    int nitems = isall? allcmds:(isopt?allopts:CAT_COMPLETE_COUNT);
    MenuItem menuitems[nitems];
    int cur = 0,curmi = 0,i=0,menusel=-1,cmdl=cmdname?strlen(cmdname):0;
    gen g;
    while(cur<nitems) {
      menuitems[curmi].type = MENUITEM_NORMAL;
      menuitems[curmi].color = TEXT_COLOR_BLACK;    
      if (isall || isopt) {
	const char * text=isall?(builtin_lexer_functions_begin()+curmi)->first:(lexer_tab_int_values_begin+curmi)->keyword;
	if (menusel<0 && cmdname && !strncmp(cmdname,text,cmdl))
	  menusel=curmi;
	menuitems[curmi].text = (char*) text;
	menuitems[curmi].isfolder = allcmds; // assumes allcmds>allopts
	menuitems[curmi].token=isall?((builtin_lexer_functions_begin()+curmi)->second.subtype+256):((lexer_tab_int_values_begin+curmi)->subtype+(lexer_tab_int_values_begin+curmi)->return_value*256);
	// menuitems[curmi].token=isall?find_or_make_symbol(text,g,0,false,contextptr):((lexer_tab_int_values_begin+curmi)->subtype+(lexer_tab_int_values_begin+curmi)->return_value*256);
	for (;i<CAT_COMPLETE_COUNT;++i){
	  const char * catname=completeCat[i].name;
	  int tmp=strcmp(catname,text);
	  if (tmp>=0){
	    size_t st=strlen(text),j=tmp?0:st;
	    for (;j<st;++j){
	      if (catname[j]!=text[j])
		break;
	    }
	    if (j==st && (!isalphanum(catname[j]))){
	      menuitems[curmi].isfolder = i;
	      ++i;
	    }
	    break;
	  }
	}
	// compare text with completeCat
	++curmi;
      }
      else {
	int cat=completeCat[cur].category;
	if ( (cat & 0xff) == category ||
	     (cat & 0xff00) == (category<<8) ||
	     (cat & 0xff0000) == (category <<16)
	     ){
	  menuitems[curmi].isfolder = cur; // little hack: store index of the command in the full list in the isfolder property (unused by the menu system in this case)
	  menuitems[curmi].text = (char *) completeCat[cur].name;
	  curmi++;
	}
      }
      cur++;
    }
  
    Menu menu;
    if (menusel>=0)
      menu.selection=menusel+1;
    menu.items=menuitems;
    menu.numitems=curmi;
    if (isopt){ menu.selection=5; menu.scroll=4; }
    if (curmi>=100)
      SetSetupSetting( (unsigned int)0x14, 0x88);	
    // DisplayStatusArea();
    menu.scrollout=1;
    menu.title = (char *) title;
    menu.type = MENUTYPE_FKEYS;
    menu.height = 13;
    while(1) {
      drawRectangle(0,56,LCD_WIDTH_PX,8,COLOR_BLACK);
      PrintMini(0,57,(const unsigned char *)(category==CAT_CATEGORY_ALL?"input| ex1 | ex2 |     |    | help":"input| ex1 | ex2 |cmds |    |help"),MINI_REV);
      int sres = doMenu(&menu);
      if (sres==KEY_CTRL_F4 && category!=CAT_CATEGORY_ALL){
	break;
      }
      if(sres == MENU_RETURN_EXIT){
	reset_alpha();
	return sres;
      }
      int index=menuitems[menu.selection-1].isfolder;
      if(sres == KEY_CTRL_F6) {
	const char * example=index<allcmds?completeCat[index].example:0;
	const char * example2=index<allcmds?completeCat[index].example2:0;
	textArea text;
	text.editable=false;
	text.clipline=-1;
	text.title = (char*)"Aide sur la commande";
	text.allowF1=true;
	text.python=python_compat(contextptr);
	ustl::vector<textElement> & elem=text.elements;
	elem = ustl::vector<textElement> (example2?4:3);
	elem[0].s = index<allcmds?completeCat[index].name:menuitems[menu.selection-1].text;
	elem[0].newLine = 0;
	//elem[0].color = COLOR_BLUE;
	elem[1].newLine = 1;
	elem[1].lineSpacing = 1;
	elem[1].minimini=1;
	ustl::string autoexample;
	if (index<allcmds)
	  elem[1].s = completeCat[index].desc;
	else {
	  int token=menuitems[menu.selection-1].token;
	  elem[1].s="Desole, pas d'aide disponible...";
	  // *logptr(contextptr) << token << endl;
	  if (isopt){
	    if (token==_INT_PLOT+T_NUMBER*256){
	      autoexample="display="+elem[0].s;
	      elem[1].s ="Option d'affichage: "+ autoexample;
	    }
	    if (token==_INT_COLOR+T_NUMBER*256){
	      autoexample="display="+elem[0].s;
	      elem[1].s="Option de couleur: "+ autoexample;
	    }
	    if (token==_INT_SOLVER+T_NUMBER*256){
	      autoexample=elem[0].s;
	      elem[1].s="Option de fsolve: " + autoexample;
	    }
	    if (token==_INT_TYPE+T_TYPE_ID*256){
	      autoexample=elem[0].s;
	      elem[1].s="Type d'objet: " + autoexample;
	    }
	  }
	  if (isall){
	    if (token==T_UNARY_OP || token==T_UNARY_OP_38)
	      elem[1].s=elem[0].s+"(args)";
	  }
	}
	ustl::string ex("F2: ");
	elem[2].newLine = 1;
	elem[2].lineSpacing = 0;
	//elem[2].minimini=1;
	if (example){
	  if (example[0]=='#')
	    ex += example+1;
	  else {
	    ex += insert_string(index);
	    ex += example;
	    ex += ")";
	  }
	  elem[2].s = ex;
	  if (example2){
	    string ex2="F3: ";
	    if (example2[0]=='#')
	      ex2 += example2+1;
	    else {
	      ex2 += insert_string(index);
	      ex2 += example2;
	      ex2 += ")";
	    }
	    elem[3].newLine = 1;
	    // elem[3].lineSpacing = 0;
	    //elem[3].minimini=1;
	    elem[3].s=ex2;
	  }
	}
	else {
	  if (autoexample.size())
	    elem[2].s=ex+autoexample;
	  else
	    elem.pop_back();
	}
	sres=doTextArea(&text);
      }
      if (sres == KEY_CTRL_F2 || sres==KEY_CTRL_F3) {
	reset_alpha();
	if (index<allcmds && completeCat[index].example){
	  ustl::string s(insert_string(index));
	  const char * example=0;
	  if (sres==KEY_CTRL_F2)
	    example=completeCat[index].example;
	  else
	    example=completeCat[index].example2;
	  if (example){
	    if (example[0]=='#')
	      s=example+1;
	    else {
	      s += example;
	      s += ")";
	    }
	  }
	  strcpy(insertText, s.c_str());
	  return 1;
	}
	if (isopt){
	  int token=menuitems[menu.selection-1].token;
	  if (token==_INT_PLOT+T_NUMBER*256 || token==_INT_COLOR+T_NUMBER*256)
	    strcpy(insertText,"display=");
	  else
	    *insertText=0;
	  strcat(insertText,menuitems[menu.selection-1].text);
	  return 1;
	}
	sres=KEY_CTRL_F1;
      }
      if(sres == MENU_RETURN_SELECTION || sres == KEY_CTRL_F1) {
	reset_alpha();
	strcpy(insertText,index<allcmds?insert_string(index).c_str():menuitems[menu.selection-1].text);
	return 1;
      }
    }
    title="CATALOG";
    category=0;
  } // end endless for
}
