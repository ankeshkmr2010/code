//-------------------------------------------------------------------------------------------------
//									@author : Ankesh Kumar
//									 date:	25/04/2015
//								BITS Pilani Hyderabad Campus
//
//			FP TREE encoding from a given file containing transaction matrix
//						File name to be given as argument at runtime
//					 cross pointers link all nodes with the same label 
//
//_________________________________________________________________________________________________

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



//---------------------------------------------------------
//	STRUCTURES
//

	struct Node
	{
		int label;
		int support;
		struct Child_node *children;
		struct Node *cross;
	};
	typedef struct Node node;
	
	struct Child_node
	{
		node *child;
		struct Child_node *nxt;
	};
	typedef struct Child_node child_node;
//_________________________________________________________


//---------------------------------------------------------
//	GLOBAL VARIABLES
//

	int numTransactions;
	int numItems;
	int **transaction_matrix;
	FILE *inp;
	node *root;
	int print_flag;
	int t_flag;
	node *mapper;
//_________________________________________________________




//---------------------------------------------------------
//	FUNCTION PROTOTYPING
//

	int get_transaction_matrix(char *inp_file);
	void print_transaction_matrix();
	node  *FP_growth_level(int label, node *location);
	void print_FPtree();
	void create_FPtree();
	void print_subtree(node *location,int num_tab);
	void dfs(node *mapper,int *total_count , node *location);
	void create_cross_pointers();
	void trav_and_add(node *location,node * map_node);
	void process_mapper();
//_________________________________________________________



//---------------------------------------------------------
//	MAIN
//

	int main(int argc ,char * argv[])
	{
		printf("\t FP TREE \n\n");
		//	 get input matrix from file
		if (argc >= 2){
			int ret = get_transaction_matrix(argv[1]);
		}
		//	create 	FP tree
		create_FPtree();
		create_cross_pointers();
		return 0;
	}
//_________________________________________________________


//---------------------------------------------------------
//	POPULATE TRANSACTION MATRIX
//

	int get_transaction_matrix(char *inp_file)
	{
		//
		// 		open file
		//
		inp = fopen(inp_file,"r");
		if (inp == NULL)
		{
			printf("ERROR IN OPENING FILE\n\n\n");
			return  0;
		}
		printf("\tFILE OPENED\n\n");
		
		//
		//	read transactions
		//
		int i,row = 0,col = 0 , temp = -1;	;
		
		//	//
		//	//		read the first two numbers (ASSUMPTION : numTransaction And numItems )
		//	//
		fscanf(inp,"%d %d",&numTransactions,&numItems);
		printf("\tTHE number of transactions are  [%d]\n",numTransactions);
		printf("\tTHE number of items are  [%d]\n",numItems);
			
		//	//
		//	//	Dynamically allocate the transaction matrix
		//	//	print the matrix (initialised to zero)
		//	//
			
		transaction_matrix = (int **) malloc(sizeof(int *)*numTransactions);
		for(i=0;i<numTransactions;i++)
		{
			transaction_matrix[i] = (int *)malloc(sizeof(int)*numItems);
		}
		print_transaction_matrix();
		
		//	//
		//	//	Read the values one by one
		//	//		for every row , then insert in matrix
		//	//
		
		while (fscanf(inp,"%d",&temp) != EOF)
		{
			// DEBUG DTATEMENT 
			//	printf ("debug %d at [%d][%d]\n",temp,row,col);
			transaction_matrix[row][col++] = temp;
			if (col == (numItems))
			{	
				row++;
				col = 0;
			}
		}
		print_transaction_matrix();
		return 1;
	}
//_________________________________________________________



//---------------------------------------------------------
//	Crosss_Linking
//

	void create_cross_pointers()
	{
		// create a 2d linked list that stores the location for encountered nodes
		//	go through every node in depth-first manner
		//  	address of every encountered node
		
		int total_count[numItems];
		int i = 0;
		for (;i<numItems;i++)
			total_count[i] = 0;
		
		//create mapper as array of nodes 
		//each node having all of child_nodes 
		// each child node points a node address
		mapper = (node * )malloc(sizeof(node) * numItems);
		for (i=0;i<numItems;i++)
			mapper[i].label = i;
			
		//use dfs to fill total count
		//use dfs to map address of visited nodes in mapper
		dfs( mapper , total_count , root);
		//use mapper to create cross nodes
		process_mapper();
		//print the total count for every item
		for (i=0;i<numItems;i++)
			printf(" [%d] = %d\n",i,total_count[i]);
		
	}
//________________________________________________________	



//--------------------------------------------------------
//	Depth First Search
//
	void dfs(node *mapper,int *total_count , node *location)
	{
		child_node *t_child = location->children;
		
		// go to every child and for each child
		while ( t_child != NULL)
		{
			//go to node pointed by the child
			node *trav = t_child->child;
			//go to its sub tree
			dfs(mapper, total_count , trav);
			//after traversing its subtree go to next child
			t_child = t_child->nxt;
		}
		
		if (location != root)
		{
			//printf(" -- %d --\n",location->label);
			total_count[location->label] += location->support;
			trav_and_add(&mapper[location->label], location);
		}
	}

//_________________________________________________________



//---------------------------------------------------------
// Add to a linkedlist
//

	void trav_and_add(node *location,node * map_node)
	{
		//create a child_node trav that has start of attached linked list
		child_node *trav = location->children;
		//create a new child_node to be added to the end of linked list
		child_node * temp_c = ( child_node * ) malloc(sizeof(child_node));
		//set the child pointer in the child_node to map_node
		temp_c->child = map_node;
		temp_c->nxt = NULL;
		
		//check if linkedlist is empty
		if (location->children == NULL)
		{
			//the first entry is the new node
			location->children = temp_c;
			return;
		}		
		//else go to the end of thelinked list
		// stop at the last child_node which points to NULL
		while(trav->nxt != NULL)
			trav = trav->nxt;
		
		trav->nxt = temp_c;
		return;
	}
//_________________________________________________________



//---------------------------------------------------------
//	PRINT MAPPER
//	Create Links
//
	void process_mapper()
	{
		int i = 0 , j = 0;
		for (i=0 ; i< numItems;i++)
		{
			//for every label assign a temp pointer to the start of associated linkedd list
			child_node *temp = mapper[i].children;
			//edge case if no linked list exists
			if (temp == NULL)
				continue;
			//until the last node is reached
			while( temp->nxt != NULL)
			{
				//present node as pointed by current linked_list node
				node *p_node = temp->child;
				//succesive node as pointed by next linked_list node
				node *n_node = temp->nxt->child;
				//present node -> cross pointer POINTS TO  next node
				p_node->cross = n_node;
				temp = temp->nxt;
			}
			
		}
		return ;
	}

//_________________________________________________________



//---------------------------------------------------------
//	Create FP tree
//

	void create_FPtree()
	{
		int i = 0,j = 0;
		//initialise a temporary node pointer 
		node * location = root;
		
		//traverse through every row
		for(i=0;i<numTransactions;i++)
		{
			// for every new transaction start at the root and then keep adding
			location = root;
			//for every item in a given ith transaction
			for(j=0;j<numItems;j++)
			{
				if( transaction_matrix[i][j] == 1)
				{
					//add to FP tree
					location = FP_growth_level(j,location);
				}
			}
		}
		
		print_FPtree();
		return;
	}
//_________________________________________________________


//---------------------------------------------------------
//	Growth Per branch
//
	
	node * FP_growth_level(int label,node *location)
	{
		if(root == NULL)
		{
			//
			//	Create the root node
			//		root node does not have any labels but only a list of children
			//		initialise root node
			//		initialise first child_node (to be used in root node list of children) ( struct Child_node )
			//		initialise child node ( struct Node)
			//
			
			//create root
			root = (node *) malloc(sizeof(node));
			root->label = -1;
			root->support = -1;
			//create child
			node *new_node = (node *) malloc(sizeof(node));
			new_node->label = label;
			new_node->support = 1;
			
			//create node in children list
			child_node *temp_c = (child_node *) malloc(sizeof(child_node));
			//assign to root->children
			root->children = temp_c;
			//assign pointer to the actual child node
			temp_c->child = new_node;
			temp_c->nxt = NULL;
			return new_node;
			
		}
		else
		{
			// initialise travelling child pointer
			//go first child node
			child_node *t_child = location->children;
			child_node *last_child = NULL;
			// initialise travelling node pointer
			node *traveller ;
			
			if(location->label == label)
			{
				location->support += 1;
				return location;
			}
			
			while (t_child != NULL)
			{
				//go to child pointed by child node
				traveller = t_child->child;
				if (label == traveller->label)
				{
					traveller->support += 1;
					return traveller;
				}
				last_child = t_child;
				t_child = t_child->nxt;
			}
			
			//
			//	NO node with given label at this level
			// 		create new node
			//
			node *new_node = (node *) malloc(sizeof(node));
			new_node->label = label;
			new_node->support = 1;
			
			//create node in children list
			child_node *temp_c = (child_node *) malloc(sizeof(child_node));
			// if no child node exits
			if (last_child == NULL)
				location->children = temp_c;
			else
			//	else nxt location to the last child node
				last_child->nxt = temp_c;
			//assign pointer to the actual child node
			temp_c->child = new_node;
			temp_c->nxt = NULL;
			return new_node;
		}
	}
//________________________________________________________



//--------------------------------------------------------
//	PRINT FP - TREE level order equivalent
//

	void print_FPtree()
	{
		//initialise flags
		print_flag = 0;
		t_flag = 0;
		printf("root----|");
		//recursively print the subtree
		print_subtree(root,0);
		printf("\n");
		return;	
	}
	
	void print_subtree(node *location,int num_tab)
	{
		child_node *t_child = location->children;
		int i = 0;
		
		while(t_child != NULL)
		{
			//print flag tells its the first element of the printed row
			// check on t_flag
			if( print_flag == 1)
			{
				for(i=0;i<num_tab;i++)
					printf("\t");
				print_flag = 0;	
			}
			//print a child
			printf("--%d[%d] ",t_child->child->label, t_child->child->support);
			//for a child print its subgraph
			print_subtree(t_child->child,num_tab+1);
			//after subgraph is printed traverse to the next child
			t_child = t_child->nxt;
		}
		//t_flag is set if consecutively two new rows have been printed
		// if 
		if( print_flag == 1)
			t_flag = 1;
		
		//if t_flag is not set only then print a new line
		if( t_flag == 0)
		{	
			printf("\n\t|\n\t|");
			print_flag = 1;
		}
		else 
			t_flag = 0;
		return;
	}
//________________________________________________________


//---------------------------------------------------------
//	PRINT TRANSACTION MATRIX
//

	void print_transaction_matrix()
	{
		int i =0 , j =0;
		printf("\n\t\t Transaction Matrix is \n\n");
		for(;i<numTransactions;i++)
		{
			printf("\t\t\t");
			for (j=0;j<numItems;j++)
			{
				printf(" %d ",transaction_matrix[i][j]);
			}
			printf("\n");
		}
		return;
	}
//_________________________________________________________
