#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 4
#define NxN (N*N)
#define TRUE 1
#define FALSE 0

struct node 
{
	int tiles[N][N];
	int f, g, h;
	short zero_row, zero_column;	/* location (row and colum) of blank tile 0 */
	struct node *next;
	struct node *parent;			/* used to trace back the solution */
};

int goal_rows[NxN];
int goal_columns[NxN];
struct node *start,*goal;
struct node *open = NULL, *closed = NULL;
struct node *succ_nodes[4];

void print_a_node(struct node *pnode) 
{
	int i,j;
	for (i=0; i<N; i++) 
    {
		for (j=0; j<N; j++)
        {
			printf("%2d ", pnode->tiles[i][j]);
        }
		printf("\n");
	}
	printf("\n");
}

struct node *initialize(char **argv)
{
	int i,j,k,index, tile;
	struct node *pnode;

	pnode=(struct node *) malloc(sizeof(struct node));
	index = 1;
	for (j=0; j<N; j++)
    {
		for (k=0; k<N; k++) 
        {
			tile=atoi(argv[index++]);
			pnode->tiles[j][k]=tile;
			if(tile==0) 
			{
				pnode->zero_row=j;
				pnode->zero_column=k;
			}
		}
    }

	pnode->f=0;
	pnode->g=0;
	pnode->h=0;
	pnode->next=NULL;
	pnode->parent=NULL;
	start=pnode;
	printf("initial state\n");
	print_a_node(start);

	pnode=(struct node *) malloc(sizeof(struct node));
	goal_rows[0] = 3;
	goal_columns[0] = 3;

	for(index=1; index<NxN; index++)
    {
		j=(index-1)/N;
		k=(index-1)%N;
		goal_rows[index]=j;
		goal_columns[index]=k;
		pnode->tiles[j][k]=index;
	}

	pnode->tiles[N-1][N-1]=0;	      /* empty tile=0 */
	pnode->f=0;
	pnode->g=0;
	pnode->h=0;
	pnode->next=NULL;
	goal=pnode; 
	printf("goal state\n");
	print_a_node(goal);

	return start;
}

/* merge the remaining nodes pointed by succ_nodes[] into open list. 
 * Insert nodes based on their f values --- keep f values sorted. */

void merge_to_open()
{
	for(int i = 0; i < N; i++)
	{
		struct node *pnode = succ_nodes[i];
		struct node *previous = NULL; //previous tracks node before the insertion point

		if(pnode == NULL) //avoids merging nodes that are null
		{
			continue;
		}

		struct node *current = open; //current set to open to keep open unchanged(represents head of list)

		while(current != NULL && pnode->f > open->f) //finds position that needs to be inserted based on f values
		{
			previous = open; 
			current = current->next;
		}

		pnode->next = current; //inserting current (open list node) into pnode (succ_nodes[i])
		if(previous != NULL)
		{
			previous->next = pnode; //Insert pnode into previous node if only there is a node before the insertion point
		}
		else
		{
			open = pnode; //set open = pnode if inserting at the head of the open list
		}
	}
}

/*swap two tiles in a node*/
void swap(int row1,int column1,int row2,int column2, struct node * pnode) 
{
	int temp_tile = pnode->tiles[row2][column2]; //temp_tile to hold the information that will be changed
	pnode->tiles[row2][column2] = pnode->tiles[row1][column1];
	pnode->tiles[row1][column1] = temp_tile;
}

/*update the f,g,h function values for a node */

void update_fgh(int i) //based off of manhatten distance formula
{
	struct node *pnode = succ_nodes[i];
	if(pnode == NULL) //cant update values on a null value
	{
		return;
	}
	int *ptr = (int *) pnode->tiles; //ptr set to both 2d arrays of pnode and goal state
	int *ptr_goal = (int *) goal->tiles;

	int h = 0; //represents heuristic value
	int x1, x2, y1, y2; //represents coordinates of two tiles in the puzzle that will be used to get the distance between them

	for(int i = 0 ; i < NxN; i++)
	{
		if(ptr[i] != ptr_goal[i]) //avoids redundancy in getting the distance of the two tiles that are already in the same spot in the puzzle
		{
			int num = ptr[i]; 
			x1 = i / N; //divide the index by N (4) in order get the x value (rows) of the tile
			y1 = i % N; //remainder division to get the y value (columns) of the tile

			for(int j = 0; j < NxN; j++)
			{
				if(ptr_goal[j] == num) //do the same thing for the goal state tiles
				{
					x2 = j / N;
					y2 = j % N;
					break;
				}
			}
			int cost = (abs(x1-x2)) + (abs(y1-y2)); //add the absolute values of x1-x2 and y1-y2
            h += cost; //setting heuristic value to cost
		}
	}
	pnode->g = pnode->parent->g + 1;
	pnode->h = h;
	pnode->f = pnode->g + pnode->h;
}

/* 0 goes down by a row */
void move_down(struct node * pnode)
{
	swap(pnode->zero_row, pnode->zero_column, pnode->zero_row + 1, pnode->zero_column, pnode); //swapping the tile values
	pnode->zero_row += 1; //actually moving the 0 tile
}

/* 0 goes right by a column */
void move_right(struct node * pnode)
{
	swap(pnode->zero_row, pnode->zero_column, pnode->zero_row, pnode->zero_column + 1, pnode);
	pnode->zero_column += 1;
}

/* 0 goes up by a row */
void move_up(struct node * pnode)
{
	swap(pnode->zero_row, pnode->zero_column, pnode->zero_row - 1, pnode->zero_column, pnode);
	pnode->zero_row -= 1;
}

/* 0 goes left by a column */
void move_left(struct node * pnode)
{
	swap(pnode->zero_row, pnode->zero_column, pnode->zero_row, pnode->zero_column - 1, pnode);
	pnode->zero_column -= 1;
}

void copyArray(struct node *child, struct node *parent) { // Copies tiles from parent to child
    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            child->tiles[row][col] = parent->tiles[row][col];
        }
    }
}

struct node *initial(struct node *parent){ //Creates and instanstiates a new state
		struct node *child = (struct node *) malloc(sizeof(struct node));

		child->f=0; // setting f,g,h to 0
		child->g=0;
		child->h=0;

		child->next=NULL;
		child->parent=parent; //link new child nodes (possible moves) to parent node

		copyArray(child,parent); // call helper function that copies the tiles to it new state

		child->zero_row = parent->zero_row;
		child->zero_column = parent->zero_column;
		return child;
}

/* expand a node, get its children nodes, and organize the children nodes using
 * array succ_nodes.
 */
void expand(struct node *selected) {

	struct node *down_child=NULL, *right_child=NULL, *up_child=NULL, *left_child=NULL; // creating every possible state and setting it to null
	int index = 0; //index of succ_nodes

	/* index n = 0 | move 0 tile down */
	if((selected->zero_row) + 1 < N)
    { // if moving the 0 tile down is possible
		struct node *down_child = initial(selected);
		move_down(down_child);
		succ_nodes[index] = down_child;
	}
	else
    {
		succ_nodes[index] = NULL;
	}

	index++;
	/* index n = 1 | move 0 tile right */
	if((selected->zero_column) + 1 < N)
    { // if moving the 0 tile right is possible
		struct node *right_child = initial(selected);
		move_right(right_child);
		succ_nodes[index] = right_child;
	}
	else
    {
		succ_nodes[index] = NULL;
	}

	index++;
	/* index n = 2 | move 0 tile up */
	if((selected->zero_row) - 1 >= 0)
    { // if moving the 0 tile up is possible
		struct node *up_child = initial(selected);
		move_up(up_child);
		succ_nodes[index] = up_child;
	}
	else
    {
		succ_nodes[index] = NULL;
	}

	index++;
	/* index n = 3 | move 0 tile left */
	if((selected->zero_column) - 1 >= 0)
    { // if moving the 0 tile left is possible
		struct node *left_child = initial(selected);
		move_left(left_child);
		succ_nodes[index] = left_child;
	}
	else
    {
		succ_nodes[index] = NULL;
	}

	for(int i = 0; i < 4; i++)
    { // Iterates through succ_nodes and checks for the possible states that were done and updates f,g,h
		update_fgh(i);
	}

}

int nodes_same(struct node *a,struct node *b) 
{
	int flg=FALSE;
	if (memcmp(a->tiles, b->tiles, sizeof(int)*NxN) == 0)
    {
		flg=TRUE;
    }

	return flg;
}

/* Filtering. Some states in the nodes pointed by succ_nodes may already be included in 
 * either open or closed list. There is no need to check these states. Release the 
 * corresponding nodes and set the corresponding pointers in succ_nodes to NULL. This is 
 * important to reduce execution time.
 * This function checks the (i)th node pointed by succ_nodes array. You must call this
 * function in a loop to check all the nodes in succ_nodes. Free the successor node and 
 * set the corresponding pointer to NULL in succ_node[] if a matching node is found in 
 * the list.
 */ 
void filter(int i, struct node *pnode_list)
{ 
	struct node *pnode = succ_nodes[i];
	
	while(pnode_list != NULL && pnode != NULL) //pnode_list and pnode must not be null in order to do comparisons of the states
	{
		if(nodes_same(pnode, pnode_list)) //returns 0 if true
		{
			free(pnode); //these three statements reflect of whats asked in the instructions above
			succ_nodes[i] = NULL;
			return;
		}
		pnode_list = pnode_list->next;
	}
}

int main(int argc,char **argv) 
{
	int iter,cnt;
	struct node *copen, *cp, *solution_path;
	int ret, i, pathlen=0, index[N-1];

	solution_path=NULL;
	start=initialize(argv);	/* init initial and goal states */
	open=start; 

	iter=0; 

	while (open!=NULL) 
    {	/* Termination cond with a solution is tested in expand. */
		copen=open;
		open=open->next;  /* get the first node from open to expand */
		
		/* DEBUG: print the node (layout) in *copen 
		 * Fix segmentation faults first. If the program cannot produce correct results,
		 * select an initial layout close to the goal layout, and debug the program with it.
		 * gdb is recommended. You may also print data on the screen. But this is not as
		 * convenient as using gdb.
		 * */

		if(nodes_same(copen,goal))
        { /* goal is found */
			do{ /* trace back and add the nodes on the path to a list */
				copen->next=solution_path;
				solution_path=copen;
				copen=copen->parent;
				pathlen++;
			} while(copen!=NULL);

			printf("Path (lengh=%d):\n", pathlen); 
			copen=solution_path;

			/* print out the nodes on the list */
			while(copen != NULL)
			{
				print_a_node(copen);
				copen = copen->next;
			}
			 
			break;
		}
		expand(copen);       /* Find new successors */

		/* DEBUG: print the layouts/nodes organized by succ_nodes[] */

		for(i=0; i<4; i++)
        {
			filter(i,open);
			filter(i,closed);
			update_fgh(i);
		}

		/* DEBUG: print the layouts/modes remaining in succ_nodes[] */

		merge_to_open(); /* New open list */

		/* DEBUG: print the layouts/nodes on the open list */

		copen->next=closed;
		closed=copen;		/* New closed */
		/* print out something so that you know your 
		 * program is still making progress 
		 */
		iter++;
		if(iter %1000 == 0)
        {
			printf("iter %d\n", iter);
        }
	}
	return 0;
} /* end of main */