/*
This is calculate.c

Compile using
cc calculate.c -lm -o calculate
*/

/*
The function calculate_expression(char*, double, int*) takes as its
input a string and a number, and interpretes the string as
an expression, substituting x for the number, and returning
a non-zero value for the integer if there is an error.

For example, if error is of type int, then

  calculate_expression("-3*(sqrt x + 3)",16.0,&error)

returns the number -21.0, with error set to 0.

Written by Stephen Montgomery-Smith.  The ideas are probably
not original, so there is no copywrite, but also absolutely no
warrenty.
*/

#include<stdio.h>
#include<math.h>
#include<strings.h>

/*
Here we list the allowable functions of one variable.
The name is what the program looks for in the string.
The code is an internal marker so that the program
knows how to evaluate it.  (Notice that the code
is really there to make writing the program easier.)
It is easy to add new functions - add them here, and
in apply_function immediately below, and the program
will compile it in correctly.
*/

struct FUNC_TYPE
{
  char *name;
  int code;
} func[] =
{
  "-",   '-',
  "sqrt", 'q',
  "log", 'l',
  "exp", 'e',
  "cos", 'c',
  "sin", 's',
  "tan", 't',
  "atan", 'a',
  "",    '\0'
};

/*
And here are the rules for how to evaluate the function
based upon its code.
*/

double apply_function(int func_nr, double x)
{
  double ret_value;

  switch(func[func_nr].code)
  {
    case '-':  ret_value = -x; break;
    case 'q':  ret_value = sqrt(x); break;
    case 'l':  ret_value = log(x); break;
    case 'e':  ret_value = exp(x); break;
    case 'c':  ret_value = cos(x); break;
    case 's':  ret_value = sin(x); break;
    case 't':  ret_value = tan(x); break;
    case 'a':  ret_value = atan(x); break;
  }
  return(ret_value);
}

/*
Here are the allowed binary operations.  Each binary
operation comes with a level, which tells the program
the order in which to evaluate them.  So
a+b*c is done as a+(b*c), because * has a higher level 
than +.  It also comes with a boolean left_to_right.  If
it is 0, an expression as a^b^c becomes a^(b^c) - if
it is 1, an expression as a/b/c becomes (a/b)/c.
*/

struct BIN_OP_TYPE
{
  char *name;
  int level, left_to_right, code;
} bin_op[] =
{
  "+", 10, 1, '+',
  "-", 10, 1, '-',
  "*", 20, 1, '*',
  "/", 20, 1, '/',
  "^", 30, 0, '^',
  ")", 0,  1, ')',
  "",  0,  1, '\0'
};

/*
And the rules for evaluating the binary operations.
*/

double apply_bin_op(int bin_op_nr, double x, double y)
{
  double ret_value;

  switch(bin_op[bin_op_nr].code)
  {
    case '+':  ret_value = x+y; break;
    case '-':  ret_value = x-y; break;
    case '*':  ret_value = x*y; break;
    case '/':  ret_value = x/y; break;
    case '^':  ret_value = exp(y*log(x)); break;
  }
  return(ret_value);
}

/*
Here are the allowable variables.  As well as the unknown
x which we substitute for, we have pi and e.
*/

struct VAR_TYPE
{
  char *name;
  int code;
} vars[] =
{
  "x", 'x',
  "X", 'x',
  "pi", 'p',
  "Pi", 'p',
  "PI", 'p',
  "e", 'e',
  "E", 'e',
  "", '\0'
};

double apply_variable(int var_nr, double x)
{
  double ret_value;
  
  switch(vars[var_nr].code)
  {
    case 'x':  ret_value = x; break;
    case 'p':  ret_value = 4.0*atan(1.0) ; break;
    case 'e':  ret_value = exp(1.0) ; break;
  }
  return(ret_value);
}

double apply_number(char *atom)
{
  double ret_value;
  sscanf(atom,"%lf",&ret_value);
  return(ret_value);
}


/*
The expression is split into what we call atoms.  So
"2.5*(x+sqrt(pi))"
becomes listed as the series of atoms
"2.5", "*", "(", "x", "+", "sqrt", "(", "pi", ")", ")".
Each atom has an atom_type, which we list below.  Notice that
the FUNC atom type has 1000*l added to it, where l is where the
function atom appears in the list in func[] above.  Similarly
for VARIABLE and BIN_OP.  So for example, "*" has atom_type
equal to 6+1000*2 = 2006.
*/

#define END      0
#define FUNC     1
#define BIN_OP   6
#define NUM      2
#define VARIABLE 7
#define LBRACK   3
#define RBRACK   4
#define UNKNOWN -1

/*
get_atom gets an atom when it is expecting any atom_type other
than BIN_OP or RBRACK.  This is usually the first atom, or the
atom after a BIN_OP.

The inputs are
  expr_str - this is the string from which we are extracting the atom.
  count - this is an integer telling us where to start seraching for
          the atom in the string.
The outputs are
  atom - the string containing the atom.
  atom_type - described above.
  next_count - the position in the string right after the atom.
*/

get_atom(char *expr_str, char *atom, int count, int *next_count, int *atom_type)
{
  int i,j,func_found,var_found;
  char last_char;

  i = 0;
  while(expr_str[count] == ' ') count++;

  j = 0;
  func_found = 0;
  while(!func_found && func[j].name[0] != '\0')
  {
    if (strncmp(func[j].name, expr_str+count, strlen(func[j].name)) == 0)
      func_found = 1;
    else
      j++;
  }

  if (!func_found)
  {
    j = 0;
    var_found = 0;
    while(!var_found && vars[j].name[0] != '\0')
    {
      if (strncmp(vars[j].name, expr_str+count, strlen(vars[j].name)) == 0)
        var_found = 1;
      else
        j++;
    }
  }
 
  if (func_found)
  {
    strcpy(atom,func[j].name);
    i = strlen(func[j].name);
    count = count + i;
    *atom_type = FUNC + 1000*j;
  }
  else if (var_found)
  {
    strcpy(atom,vars[j].name);
    i = strlen(vars[j].name);
    count = count + i;
    *atom_type = VARIABLE + 1000*j;
  }
  else if ((expr_str[count] == '.') || ((expr_str[count] >='0') &&
           (expr_str[count] <= '9')))
  {
    last_char = '0';
    while ((expr_str[count] == '.') || (expr_str[count] == 'e') ||
            (expr_str[count] == 'E') ||
            ((expr_str[count] >='0') && (expr_str[count] <= '9')) ||
            ((expr_str[count] == '-') && (last_char == 'e' || last_char == 'E'))
          )
    {
      atom[i] = expr_str[count];
      last_char = expr_str[count];
      i++;
      count++;
    }
    *atom_type = NUM;
  }
  else 
  {
    atom[0] = expr_str[count];
    i = 1;
    count++;
    if (atom[0] == '(') *atom_type = LBRACK;
    else *atom_type = UNKNOWN;
  }
  atom[i] = '\0';
  *next_count = count;
}

/*
get_bin_atom is exactly like get_atom, except it is looking for an atom
of atom_type BIN_OP or RBRACK or END.  This usually comes after
an atom of the type that we extracted with get_atom.
*/

get_bin_atom(char* expr_str, char *atom, int count, int *next_count, 
             int *atom_type)
{
  int i,j,bin_found;

  i = 0;
  while(expr_str[count] == ' ') count++;


  j = 0;
  bin_found = 0;
  while(!bin_found && bin_op[j].name[0] != '\0')
  {
    if (strncmp(bin_op[j].name, expr_str+count, strlen(bin_op[j].name)) == 0)
      bin_found = 1;
    else
      j++;
  }

  if (bin_found)
  {
    strcpy(atom,bin_op[j].name);
    i = strlen(bin_op[j].name);
    count = count + i;
    if (strcmp(atom,")") == 0) *atom_type = RBRACK;
    else *atom_type = BIN_OP + 1000*j;
  }
  else if (expr_str[count] == '\0' || expr_str[count] == '\n')
  {
    atom[0] = '\0';
    i = 0;
   *atom_type = END;
  }
  else 
  {
    atom[0] = expr_str[count];
    i = 1;
    count++;
    *atom_type = UNKNOWN;
  }

  *next_count = count;
}

/*
calc_bin and calc_atom take the expression expr_str starting at
expr_str[count], and evaluate it, substituting variable for "x".

If you put in "2*3", calc_bin would merely calculate the "2", whereas
calc_bin would calculate the "2*3".  These procedures are recursive,
calling themselves and each other.

Well, they are a little complicated to explain - I will leave it to
you to work them out.
*/

char *error_code[]=
{
  "unknown atom",
  "unknown binary atom",
  "too many right brackets",
  "too many left brackets"
};

double calc_bin(char *expr_str, double variable, int count, int *next_count, 
                int level, int *error, int bracket_level);

double calc_atom(char *expr_str, double variable, int count, int *next_count,
                 int *error, int bracket_level)
{
  int count2, atom_type;
  double ret_value;
  char atom[100];

  *error = 0;

  get_atom(expr_str,atom,count,&count2,&atom_type);
  if (atom_type%1000 == FUNC)
    ret_value = apply_function(atom_type/1000,calc_atom(expr_str,variable,
                               count2,next_count,error,bracket_level));
  else if (atom_type == LBRACK)
    ret_value = calc_bin(expr_str,variable,count2,next_count,0,error,
                         bracket_level+1);
  else if (atom_type == NUM)
  {
    ret_value = apply_number(atom);
    *next_count = count2;
  }
  else if (atom_type%1000 == VARIABLE)
  {
    ret_value = apply_variable(atom_type/1000,variable);
    *next_count = count2;
  }
  else if (atom_type == UNKNOWN)
    *error = 1;

  return(ret_value);
}

double calc_bin(char *expr_str, double variable, int count, int *next_count, 
                int level, int *error, int bracket_level)
{
  int count2, atom_type, go_again;
  double x;
  char atom[100];

  *error = 0;

  x = calc_atom(expr_str,variable,count,next_count,error,bracket_level);

  if (*error == 0)
    do{
      get_bin_atom(expr_str,atom,*next_count,&count2,&atom_type);
      go_again = 0;

      if ((atom_type%1000 == BIN_OP) && 
          (bin_op[atom_type/1000].left_to_right && 
	   bin_op[atom_type/1000].level > level) ||
	  (!bin_op[atom_type/1000].left_to_right &&
	   bin_op[atom_type/1000].level >= level))
      {
        x = apply_bin_op(atom_type/1000,x,
            calc_bin(expr_str,variable,count2,next_count,
	             bin_op[atom_type/1000].level,error,bracket_level));
        count2 = *next_count;
        if (*error == 0)
          go_again = 1;
      }
      else if (atom_type == RBRACK && level == 0)
      {
        *next_count = count2;
	if (bracket_level<=0) 
	  *error = 3;
      }
      else if (atom_type == END && level == 0)
      {
        *next_count = count2;
	if (bracket_level>0)
	  *error = 4;
      }
      else if (atom_type == UNKNOWN)
      {
        *next_count = count2;
        *error = 2;
      }
    } while (go_again);

  return(x);
}

double calculate_expression(char *expr_str, double x, int *error)
{
  int next_count;
  return(calc_bin(expr_str,x,0,&next_count,0,error,0));
}


/*
Here is a simple program to illustrate calculate_expression.
*/


main()
{
  char expr_str[1000];
  double x, expr;
  int error;

  do {
    printf("f(x) = ");
    fgets(expr_str,999,stdin);
    error = 0;
    if (expr_str[0] != '\n')
    {
      for (x=1.0;x<=10.0 && !error;x+=1.0)
      {
        expr = calculate_expression(expr_str,x,&error);
        if (error == 0)
          printf("f(%g) = %g\n",x,expr);
        else
          printf("error in expression (%s)\n",error_code[error-1]);
      }
    }
  } while (expr_str[0] != '\n');
}

