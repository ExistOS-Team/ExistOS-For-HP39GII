#include "TeX.h"

int TeX_isTeX(char* expr)
{
  while(*expr)
  {
    if(*expr == '\\')
    {
      expr++;
      if(!strncmp(expr,"frac",4)) return 1;
      else if(!strncmp(expr,"sum", 3)) return 1;
      else if(!strncmp(expr,"sqrt",4)) return 1;
      else if(!strncmp(expr,"vec", 3)) return 1;
    }
    expr++;
  }
  return 0;
}

void TeX_sizeComplex(char *expr, int *width, int *height, int *line)
{
  /*
  **  This functions returns the size in pixels of any complex expression.
  **  Calls TeX_sizeSimple() and can be called by it.
  **  Does not handle maximum call level. Therefore, the call stack is supposed unlimited. Any call to TeX_sizeComplex()
  **  needs at least one level of subsequent calls to TeX_sizeSimple().
  */

  int w=0, h=0, l=0;            // These variables will contain the final properties of the expression.
  int tw, th, tl;               // These variables store temporary data needed for comparison between elements.
  char *next;                   // This pointer will parse the expression with expr.
  char *element;                // This pointer will contain dynamically-allocated temporary data -- the text of an element.

  while(*expr)
  {
    // Getting copied data of the next element in the pointer element.
    next = TeX_getElement(expr);
    element = TeX_extract(expr,next);

    // Getting the properties of the found element and adapting existant properties for the expression.
    TeX_sizeSimple(element,&tw,&th,&tl);
    w += tw+1;
    if(th>h) h=th;
    if(tl>l) l=tl;

    // Freeing the allocated memory and preparing the next iteration.
    free(element);
    expr = next;
  }

  // A pixel of spacing was added for each element, but the last one didn't need one.
  w -= 1;

  if(width) *width = w;
  if(height) *height = h;
  if(line) *line = l;
}

void TeX_sizeSimple(char *element, int *width, int *height, int *line)
{
  /*
  **  This function returns the size of any simple TeX element, based on its parameters. Detects if the parameter is plain
  **  text, command or any complex expression.
  **  Any call to TeX_sizeSimple() needs at least one level of subsequent calls to TeX_sizeComplex() if the element is a
  **  command that has at least one parameter.
  **  Otherwise, TeX_sizeSimple() will return a fixed size without calling any subroutine.
  */

  int w=0, h=0, l=0;            // These variables will contain the properties of the element.
  int *tw, *th, *tl;            // These variables store temporary data needed for comparison between parameters.
  char *name;                   // This pointer will save the value of element to know which command was used.
  char *next;                   // This pointer will parse the element to find the parameters.
  char *parameter;              // This pointer will contain dynamically-allocated temporary data -- a parameter.
  int param;                    // Contains the number of parameters the command use. Not used if element is plain text.
  int i, x;                     // An incremental loop counter, and just a stupid variable.

  // Testing if the element is only plain text.
  if(*element != '\\')
  {
    if(width) *width = (Txt_Width(TEX_FONT)+1)*strlen(element)-1;
    if(height) *height = Txt_Height(TEX_FONT);
    if(line) *line = Txt_Height(TEX_FONT)>>1;
    return;
  }

  // Otherwise, the element is obviously a command.
  name = element+1;
  param = TeX_getParamNumber(element+1);
  while(*element != '{') element++;

  // Allocating memory to store the properties of parameters.
  tw = calloc(TEX_MAXP,4);
  th = calloc(TEX_MAXP,4);
  tl = calloc(TEX_MAXP,4);

  // Processing each parameter.
  for(i=0;i<param;i++)
  {
    // Getting the next parameter of the detected command.
    next = TeX_getParameter(element);
    parameter = TeX_extract(element+1,next);

    // Getting the size of the parameter.
    TeX_sizeComplex(parameter,tw+i,th+i,tl+i);

    // Freeing the temporary data and preparing the next iteration.
    free(parameter);
    element = next+1;
  }

  // Debugging data.
  // debug("\n**  data for simple expression \"%s\":\n",name-1);
  // for(i=0;i<param;i++) debug("**  tw[%d]=%d, th[%d]=%d, tl[%d]=%d\n",i,tw[i],i,th[i],i,tl[i]);

  // Determining the properties of the command using the properties of the parameters.
  if(!strncmp(name,"frac",4)) w=(tw[0]>tw[1]?tw[0]:tw[1])+2, h=th[0]+th[1]+3, l=th[0]+1;
  else if(!strncmp(name,"sum", 3)) x=(tw[0]>tw[1]?tw[0]:tw[1]), w=(x>8?x:8), h=th[0]+th[1]+11, l=th[0]+4;
  else if(!strncmp(name,"sqrt",4)) w=tw[0]+6, h=th[0]+3, l=tl[0]+2;
  else if(!strncmp(name,"vec", 3)) w=tw[0], h=th[0]+4, l=tl[0]+4;
  else w=-1, h=-1, l=-1;

  if(width) *width = w;
  if(height) *height = h;
  if(line) *line = l;

  // Freeing the allocated data.
  free(tw);
  free(th);
  free(tl);
}

char *TeX_getElement(char *expr)
{
  /*
  **  This function returns a pointer to the end of the first element pointed to by expr. If *expr is '\\', the element is
  **  considered as a command and TeX_getElement() returns a pointer to the character just after the '}' closing the command.
  **  In any other cases, the element is supposed to be plain text and TeX_getElement() returns a pointer to the first '\\'
  **  opening a command.
  */

  char *next = expr;

  // Looking for any '\\' as first character.
  if(*expr=='\\')
  {
    // Getting the number of parameters for the detected command.
    const int param = TeX_getParamNumber(expr+1);
    int c,i;

    // Determining the end of each parameter based on sublevel elements.
    for(i=0,c=0;i<param;i++)
    {
      while(*next)
      {
        if(*next=='{') c++;
        if(*next=='}') c--;
        if(*next=='}' && !c) break;
        next++;
      }

      next++;
    }
  }

  // If element is plain text, the next element is the first command found.
  else while(*next!='\\' && *next) next++;

  return next;
}

char *TeX_getParameter(char *com)
{
  /*
  **  This function returns a pointer to the end of the first parameter foud at com. It follows a simple parameter : the
  **  parameters begins with the first '{' and ends at the '}' which is at level 0, to handle subcommands in parameters.
  */

  char *end;                    // Will be returned. Contains the address of the closing '}'.
  int c=0;

  // Positioning com on the first '{' (spaces handling).
  while(*com != '{') com++;
  end = com;

  while(*end)
  {
    if(*end=='{') c++;
    if(*end=='}') c--;
    if(*end=='}' && !c) break;
    end++;
  }

  return end;
}

char *TeX_extract(char *begin, char *end)
{
  /*
  **  This function returns a pointer to a dynamically-allocated memory block the contains the copied data found between the
  **  pointers got as arguments.
  **  Returns NULL if any error occur.
  */

  char *data;                   // Will be allocated.
  int length = end-begin;       // Length of data to be copied.
  int i;                        // Incremental loop counter.

  // Allocating data to store a copy of the memory block.
  data = calloc(length+1,1);
  if(!data) return NULL;

  // Copying the memory block and returning the resulting pointer.
  for(i=0;i<length;i++) *(data+i) = *(begin+i);
  *(data+length) = 0;
  return data;
}

int TeX_getParamNumber(char *expr)
{
  /*
  **  This function returns the number of parameters used by a defined command given as only parameter.
  **  Returns 0 if the command is unknown.
  */

  // Constant 2-parameters commands.
  if(!strncmp(expr,"frac",4)) return 2;
  if(!strncmp(expr,"sum", 3)) return 2;

  // Constant 1-parameter commands.
  if(!strncmp(expr,"sqrt",4)) return 1;
  if(!strncmp(expr,"vec", 3)) return 1;

  // Other commands that don't use any parameter.
  return 0;
}

void TeX_drawComplex(char *expr, int x, int y)
{
  /*
  **  This function draws the complex expression expr at point (x;y) on the screen. The is the mainly used function of the
  **  program.
  */

  int w;                        // These variable will contain the width of the element being drawn.
  int baseline;                 // This integer will contain the baseline of the whole expression.
  char *next;                   // This pointer will parse the expression with expr.
  char *element;                // This pointer will contain dynamically-allocated temporary data -- the text of an element.

  TeX_sizeComplex(expr,NULL,NULL,&baseline);

  while(*expr)
  {
    // Getting copied data of the next element in the pointer element.
    next = TeX_getElement(expr);
    element = TeX_extract(expr,next);

    // Getting the properties of the found element and drawing it.
    TeX_sizeSimple(element,&w,NULL,NULL);
    TeX_drawSimple(element,x,baseline+y);

    // Freeing the allocated memory and preparing the next iteration.
    free(element);
    expr = next;
    x += w+1;
  }

  // A pixel of spacing was added for each element, but the last one didn't need one.
  w -= 1;
}

void TeX_drawSimple(char *element, int x, int l)
{
  /*
  **  This function draw the simple element at the given position on the screen, defined by the baseline.
  */

#define setPixel(x,y,v) Bdisp_SetPoint_VRAM(x,y,v)
  extern char *Txt_VRAM;        // Needed for using setPixel().
  char *name = element+1;       // Saving the element address before changing.
  char *next;                   // This pointer will parse the element to find the parameters.
  char **parameters;            // This pointer will contain the parameters.
  int *tw, *th, *tl;            // These variables store temporary data needed for comparison between parameters.
  int w,h,b;                    // Contain the properties of the element.
  int param;                    // Contains the number of parameters of the detected command.
  int i,j;                      // Incremental loop counters.

  TeX_sizeSimple(element,&w,&h,&b);
  debug("Baseline of \"%s\" is %d.\n",element,b);

  // Cheking if the element is plain text.
  if(*element != '\\')
  {
    Txt_Text(element,x,l-b,TEX_FONT,TEX_MODE);
    return;
  }

  // Otherwise, the element is obviously a command.
  name = element+1;
  param = TeX_getParamNumber(element+1);
  while(*element != '{') element++;

  // Allocating memory to store the properties of parameters.
  tw = calloc(TEX_MAXP,4);
  th = calloc(TEX_MAXP,4);
  tl = calloc(TEX_MAXP,4);
  parameters = calloc(TEX_MAXP,sizeof(char *));

  // Processing each parameter.
  for(i=0;i<param;i++)
  {
    // Getting the next parameter of the detected command.
    next = TeX_getParameter(element);
    parameters[i] = TeX_extract(element+1,next);

    // Getting the size of the parameter.
    TeX_sizeComplex(parameters[i],tw+i,th+i,tl+i);

    // Preparing the next iteration.
    element = next+1;
  }

  // Drawing the element depending on the detected command.

  if(!strncmp(name,"frac",4))
  {
    for(j=0;j<w;j++) setPixel(x+j,l,1);
    TeX_drawComplex(parameters[0],x+(w>>1)-(tw[0]>>1),l-th[0]-1);
    TeX_drawComplex(parameters[1],x+(w>>1)-(tw[1]>>1),l+2);
  }

  else if(!strncmp(name,"sum",3))
  {
    for(j=0;j<8;j++) setPixel(x+(w>>1)+j-4,l-4,1), setPixel(x+(w>>1)+j-4,l+4,1);
    for(j=1;j<4;j++) setPixel(x+(w>>1)-j,l-j,1), setPixel(x+(w>>1)-j,l+j,1);
    setPixel(x+(w>>1),l,1);
    setPixel(x+(w>>1)+3,l-3,1);
    setPixel(x+(w>>1)+3,l+3,1);
    TeX_drawComplex(parameters[0],x+(w>>1)-(tw[0]>>1),l+6);
    TeX_drawComplex(parameters[1],x+(w>>1)-(tw[1]>>1),l-5-th[1]);
  }

  else if(!strncmp(name,"sqrt",4))
  {
    for(j=0;j<h;j++) setPixel(x+2,l-tl[0]-2+j,1);
    setPixel(x+1,l-tl[0]+h-4,1);
    setPixel(x,l-tl[0]+h-5,1);
    for(j=2;j<w;j++) setPixel(x+j,l-tl[0]-2,1);
    setPixel(x+w-1,l-tl[0]-1,1);
    setPixel(x+w-1,l-tl[0],1);
    TeX_drawComplex(parameters[0],x+4,l-tl[0]);
  }

  else if(!strncmp(name,"vec",3))
  {
    for(j=0;j<w;j++) setPixel(x+j,l-tl[0]-3,1); j-=2;
    setPixel(x+j,l-tl[0]-4,1);
    setPixel(x+j,l-tl[0]-2,1);
    TeX_drawComplex(parameters[0],x,l-tl[0]);
  }

  else setPixel(x,l-b,1);

  // Finally freeing the allocated data.
  free(tw);
  free(th);
  free(tl);
  for(i=0;i<param;i++) free(parameters[i]);
  free(parameters);
}
