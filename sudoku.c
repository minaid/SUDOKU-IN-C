
#include "sudoku.h"
#include <errno.h>
#include <string.h>
#include <ctype.h>

/*DECLARATION OF STATIC FUNCTIONS*/
static int check_row_errors(Grid_T g);
static int check_col_errors(Grid_T g);
static int check_subgrid_errors(Grid_T g);
static int findBlock(int i, int j);
static int isComplete(Grid_T g);
static int emptyCells(Grid_T g);
static void sudoku_init_choices(Grid_T *g);
static int sudoku_try_next(Grid_T g, int *row, int *col);
static int sudoku_update_choice(Grid_T *g, int i, int j, int n);
static void sudoku_eliminate_choice(Grid_T *g, int r, int c, int n);
static Grid_T sudoku_generate_complete(void);
/*END OF DECLARATION*/

/* Read a sudoku grid from stdin and return an object Grid_T
   initialized to these values. The input has the format:

1 2 3 4 5 6 7 8 9
4 5 6 7 8 9 1 2 3
7 8 9 1 2 3 4 5 6
2 3 4 5 6 7 8 9 1
5 6 7 8 9 1 2 3 4
8 9 1 2 3 4 5 6 7
3 4 5 6 7 8 9 1 2
6 7 8 9 1 2 3 4 5
9 1 2 3 4 5 6 7 8

   Each number is followed by a space. Each line is terminated with
   \n. Values of 0 indicate empty grid cells.
*/
Grid_T sudoku_read(void){
  Grid_T g;
  char c;
  int num, count=0, i=0, j=0;

  while((c = getchar()) != EOF){    /*read until you reach end of file*/
    if(isdigit(c)){                 /*if the character is a digit number*/
      num = atoi(c);
      grid_update_value(&g,i,j,num);
      j++;
      count++;
    }
    else if(c==' ') continue;       /*if a space is encountered continue 
				      to next character*/
    else if(c=='\n'){               /*if a new line char is encountered*/
      i++;                          /*continue to next line*/
      j=0;                          /*start counting columns from zero*/
    }
    else{                           /*if any non valid char is found*/
      perror("Wrong input for sudoku.");
      exit(EXIT_FAILURE);
    }
  }

  if(count != 81){                  /*a normal 9x9 sudoku consists of 81 chars*/
    perror("Input must be 81 numbers.");
    exit(EXIT_FAILURE);
  }

  return g;
}

/* Print the sudoku puzzle defined by g to stream s in the same format
   as expected by sudoku_read(). */
void sudoku_print(FILE *s, Grid_T g){
  int i, j;

  for(i=0; i<9; i++){
    for(j=0; j<9; j++) fprintf(s, "%d ", grid_read_value(g,i,j));
    fprintf(s,"\n");
  }
}

/* Print all row, col, sub-grid errors/conflicts found in puzzle g;
   some errors may be reported more than once. */
/*Not used*/
void sudoku_print_errors(Grid_T g){
  int row=0, col=0, grid=0;

  row = check_row_errors(g);
  col = check_col_errors(g);
  grid = check_subgrid_errors(g);

  if(row==0 && col==0 && grid==0){
    fprintf(stderr,"\nThe sudoku puzzle has no errors!");
  }
}

/* Return true iff puzzle g is correct. Returns true even if there are 
   still empty cells.*/
int sudoku_is_correct(Grid_T g){
  int row=0, col=0, grid=0;

  row = check_row_errors(g);
  col = check_col_errors(g);
  grid = check_subgrid_errors(g);

  if(row==0 && col==0 && grid==0){
    fprintf(stderr,"\nThe sudoku puzzle is correct!");
    return 1;
  }
  return 0;
}

/* Solve puzzle g and return the solved puzzle; if the puzzle has
   multiple solutions, return one of the possible solutions. */
Grid_T sudoku_solve(Grid_T g){
  int i, j, choice, count;
  Grid_T tmp;

  /*Initialize the choices of the cells*/
  sudoku_init_choices(&g);

  while((choice=sudoku_try_next(g,&i,&j)) !=0){ 
    /*if every time there is a cell with only one choice then the puzzle has
      a unique solution*/
    if((count=sudoku_update_choice(&g,i,j,choice)) ==1){
      grid_update_value(&g,i,j,choice);
      sudoku_eliminate_choice(&g,i,j,choice);
    }else{                        /*else, there are multiple solutions*/
      grid_clear_unique(&g);
      tmp = g;
      grid_update_value(&tmp,i,j,choice);
      tmp = sudoku_solve(tmp);
      if(sudoku_is_correct(tmp)) return tmp;
    }
  }
  /*gets to this point if try_next returns 0, then the grid is complete*/
  return g;
}

/* Returns true if solution g, as returned by sudoku_solve, has a
   unique choice for each step (no backtracking required). (Note, g
   must have been computed with the use of sudoku_solve.) */
int sudoku_solution_is_unique(Grid_T g){
  return (grid_read_unique(g) == 1);
}

/* Generate and return a sudoku puzzle with "approximately" nelts
   elements having non-0 value. The smaller nelts the harder may be to
   generate/solve the puzzle. For instance, nelts=81 should return a
   completed and correct puzzle. */
Grid_T sudoku_generate(int nelts){
  int i, j;
  Grid_T g = sudoku_generate_complete();

  while((81 - emptyCells(g)) > nelts){
    /*choose a random cell*/
    i = rand() % 9;
    j = rand() % 9;

    grid_update_value(&g,i,j,0);   /*set value to 0*/
    grid_set_count(&g,i,j);        /*set count to 9*/
  }

  return g;
}


/*--------------------------------EXTRA FUNCTIONS, USED ONLY IN THIS FILE-----------------------------------*/


/*Check each row of the puzzle for errors. Each error is printed.
  Returns 0 if all rows have no errors, 1 otherwise.
  Checks both complete and non complete grids.*/
static int check_row_errors(Grid_T g){
  int i, j, k, correct=0;

  for(i=0; i<9; i++){
    for(j=0; j<9; j++){
      if(grid_read_value(g,i,j) == 0) continue;
      for(k=j+1; k<9; k++){
	if(grid_read_value(g,i,j) == grid_read_value(g,i,k)){
	  fprintf(stderr,"\nThe number (%d) exists more than once in row (%d)", g.elts[i][j].val, i);
	  correct = 1;
	  break;
	}
      }
    }
  }
  return correct;
}
/*Check each column of the puzzle for errors. Each error is printed.
  Returns 0 if all columns have no errors, 1 otherwise.
  Checks both complete and non complete grids.*/
static int check_col_errors(Grid_T g){
  int i, j, k, correct=0;

  for(j=0; j<9; j++){
    for(i=0; i<9; i++){
      if(grid_read_value(g,i,j) == 0) continue;
      for(k=i+1; k<9; k++){
	if(grid_read_value(g,i,j) == grid_read_value(g,k,j)){
	  fprintf(stderr,"\nThe number (%d) exists more than once in column (%d)", g.elts[i][j].val, j);
	  correct = 1;
	  break;
	}
      }
    }
  }
  return correct;
}

/*Check each subgrid of the puzzle for errors. Each error is printed.
  Returns 0 if all subgrids have no errors, 1 otherwise.
  Checks both complete and non complete grids.*/
static int check_subgrid_errors(Grid_T g){
  int i, j, b, r, c, row, col, correct=0;  

  for(i=0; i<9; i++){
    for(j=0; j<9; j++){

      /*Find the block and set the limits for checking*/
      b = findBlock(i,j);
      if(b==1 || b==2 || b==3) row=0;
      else if(b==4 || b==5 || b==6) row=3;
      else row=6;
      if(b==1 || b==4 || b==7) col=0;
      else if(b==2 || b==5 || b==8) col=3;
      else col=6;

      /*Check the block for errors*/
      for(r=row; r<row+3; r++){
	for(c=col; c<col+3; c++){
	  if(grid_read_value(g,r,c)==0) continue;
	  if((r!=i || c!=j) && (grid_read_value(g,i,j)==grid_read_value(g,r,c))){
	    fprintf(stderr, "\nThe number (%d) exists more than once in subgrid (%d)", grid_read_value(g,i,j), b);
	    correct = 1;
	    break;
	  }
	}
      }

    }
  }
  return correct;
}

/*Find in which subgrid/block the coordinates belong.*/
static int findBlock(int i, int j){
  int row = i/3;
  int col = j/3;

  if(row==0)
    if(col==0)      return 1;
    else if(col==1) return 2;
    else            return 3;
  else if(row==1)
    if(col==0)      return 4;
    else if(col==1) return 5;
    else            return 6;
  else
    if(col==0)      return 7;
    else if(col==1) return 8;
    else            return 9;
}

/*Checks if the puzzle is complete (has no empty cells)*/
static int isComplete(Grid_T g){
  int i, j;

  for(i=0; i<9; i++){
    for(j=0; j<9; j++){
      /*if at least one cell is empty*/
      if(grid_read_count(g,i,j) != 0) return 0;
    }
  }
  return 1;
}

/*Returns the number of empty cells in a grid*/
static int emptyCells(Grid_T g){
  int count=0, i, j;

  for(i=0; i<9; i++){
    for(j=0; j<9; j++){
      if(grid_read_value(g,i,j)==0) count++;
    }
  }
  return count;
}

/*Changes the grid g so that every empty cell of the puzzle contains all the
  possible choices, according to the rules.*/
static void sudoku_init_choices(Grid_T *g){
  int i, j, r, val;

  /*first initialize ALL cells to have all the choices available*/
  grid_set_unique(g);
  for(i=0; i<9; i++){
    for(j=0; j<9; j++){
      grid_set_count(g,i,j);
      grid_clear_choice(g,i,j,0);  /*Clear choice of 0*/
      /*Set all other choices*/
      for(r=1; r<=9; r++)
	grid_set_choice(g,i,j,r);
    }
  }

  /*exclude the choices that are invalid, according to the existing values 
    in the puzzle*/
  for(i=0; i<9; i++){
    for(j=0; j<9; j++){
      val = grid_read_value(*g,i,j);
      if(val!=0){                           /*if cell is already filled*/
	grid_clear_count(g,i,j);            /*number of choices are 0*/
	/*all choices are cleared*/
	for(r=1; r<=9; r++) grid_clear_choice(g,i,j,r);
	/*eliminate the choice of val from all the other cells in the same row,
	  column or block with this cell*/
	sudoku_eliminate_choice(g,i,j,val); 
      }
    }
  }

}

/*If 1 or more unique choices, returns one of them. 
  Else returns some other choice.*/
static int sudoku_try_next(Grid_T g, int *row, int *col){
  int i, j, r, c, num, count=9;

  /*Moves past this point only when there are empty cells left*/
  if(isComplete(g)) return 0;

  /*find a cell with the least available choices*/
  for(i=0; i<9; i++){
    for(j=0; j<9; j++){
      if(grid_read_count(g,i,j) <= count) 
	count = grid_read_count(g,i,j);
    }
  }

  /*choose a random cell with the above number of choices*/
  while(1){
    r = rand() % 9;
    c = rand() % 9;
    if(grid_read_count(g,r,c) == count){
      num = (rand() %9) +1;
      if(grid_choice_is_valid(g,r,c,num)){
	*row = r;
	*col = c;
	return num;
      }
    }
  }
  return 0;
}

/*Changes the grid g so that the i,j element does not contain anymore the
  choice of n and returns the number of choices the grid had.*/
static int sudoku_update_choice(Grid_T *g, int i, int j, int n){
  int before = grid_read_count(*g, i, j);
  grid_remove_choice(g, i, j, n);
  return before;
}

/*Removes from row r, column c and the subgrid that contains the r,c element
  the choice of n*/
static void sudoku_eliminate_choice(Grid_T *g, int r, int c, int n){
  int i, j, b, row, col;

  for(i=0; i<9; i++){
    if(i!=c) grid_remove_choice(g, r, i, n);  /*remove from row*/
    if(i!=r) grid_remove_choice(g, i, c, n);  /*remove from column*/
  }

  /*Find the block and set the limits*/
  b = findBlock(r,c);                    
  if(b==1 || b==2 || b==3) row=0;
  else if(b==4 || b==5 || b==6) row=3;
  else row=6;
  if(b==1 || b==4 || b==7) col=0;
  else if(b==2 || b==5 || b==8) col=3;
  else col=6;
  
  /*Remove from block*/
  for(i=row; i<row+3; i++){
    for(j=col; j<col+3; j++){
      if(i!=r || j!=c)
	grid_remove_choice(g,i,j,n);
    }
  }

}

/*Returns a new puzzle that has been solved*/
static Grid_T sudoku_generate_complete(){
  int i, j;

  /*Create an empty grid*/
  Grid_T g;
  g.unique = 0;
  /*Set all values to zero*/
  for(i=0; i<9; i++){
    for(j=0; j<9; j++){
      grid_update_value(&g,i,j,0);
    }
  }
  /*End of create*/
  sudoku_init_choices(&g);    /*Initialize the choices*/
  g = sudoku_solve(g);        /*Solve the puzzle*/

  return g;
}


/*---------------------------------MAIN---------------------------------------*/


int main(int argc, char **argv){
  Grid_T g;
  int nelts, correct;

  /*Give feed to rand*/
  srand(getpid());

  switch(argc){
  case 1:
    g = sudoku_read();
    sudoku_print(stderr,g);
    g = sudoku_solve(g);
    correct = sudoku_is_correct(g);
    if(correct){
      if(sudoku_solution_is_unique(g))
	fprintf(stderr, "\nSudoku has unique solution!");
      else fprintf(stderr, "\nSudoku has at least one solution!");
    }
    sudoku_print(stdout,g);
    break;
  case 2:
    if(strcmp(argv[1],"-c")==0){
      g = sudoku_read();
      sudoku_print(stderr,g);
      sudoku_is_correct(g);
    }else fprintf(stderr,"\nWrong argument!");
    break;
  case 3:
    if(strcmp(argv[1],"-g")==0){
      if(isdigit(argv[2])){
	nelts = (int)argv[2];
	g = sudoku_generate(nelts);
	sudoku_print(stdout,g);
      }else fprintf(stderr,"\nSecond argument must be a number!");
    }else fprintf(stderr,"\nWrong argument!");
    break;
  default:
    fprintf(stderr,"\nNumber of arguments is wrong!");
    break;
  }

  fprintf(stderr,"\n\n");
  return 0;
}
