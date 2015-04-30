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
#include <math.h>

//---------------------------------------------------------
//	STRUCTURES
//
	//nodes of fp tree
	struct Node
	{
		int label;
		int support;
		struct Child_node *children;
		struct Node *parent;
		struct Node *cross;
	};
	typedef struct Node node;
	
	
	//node for list per node for maintaining the children
	struct Child_node
	{
		node *child;
		struct Child_node *nxt;
	};
	typedef struct Child_node child_node;
	
	//nodes for maintaining a liked list for keeping track of added nodes in new tree
	typedef struct record{
		node * old_tree_node;
		node * new_tree_node;
		struct record *next,*prev;
	} record;
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
	void print_FPtree(node * root);
	void create_FPtree();
	void print_subtree(node *location,int num_tab);
	void dfs(node *mapper,int *total_count , node *location);
	void create_cross_pointers(node **mapper, node *root);
	void trav_and_add(node *location,node * map_node);
	void process_mapper(node *mapper);
	void print_rev_branch(int label, node *mapper , node*root);
	record * add_2_ll(record * root , node * old_node , node * new_node);
	record * search_record_ll(record * root , node * old_node);
	node *copy_subtree(int label, node *root , node *mapper);
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
		create_cross_pointers(&mapper,root);
		
		//___________________________DEBUG CODES	FOLLOWS
		
		node *ntree = copy_subtree(4,root,mapper);
		print_FPtree(ntree);
		node *mapper_2 = NULL;
		create_cross_pointers(&mapper_2,ntree);
		printf("\n");
		node *otree = copy_subtree(2,ntree,mapper_2);
		print_FPtree(otree);
		//______________________________________________________________
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

	void create_cross_pointers(node **mapper_r, node *root)
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
		node *mapper = (node * )malloc(sizeof(node) * numItems);
		*mapper_r =  mapper;
		for (i=0;i<numItems;i++)
		{	
			mapper[i].label = i;
			mapper[i].children =NULL;
		}
			
		//use dfs to fill total count
		//use dfs to map address of visited nodes in mapper
		dfs( mapper , total_count , root);
		
		//use mapper to create cross nodes
		process_mapper(mapper);
		//print the total count for every item
		for (i=0;i<numItems;i++)
			printf(" [%d] = %d\n",i,total_count[i]);
	
		return;
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
		return;
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
	void process_mapper(node *mapper)
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
		
		print_FPtree(root);
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
			new_node->parent = root;
			
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
			
			//if the label is same then increase count
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
			new_node->parent = location;
			
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
//		Print branches containing given element
//
	node *copy_subtree(int label, node *root ,node *mapper )
	{
		int count = 0;
		//node for root of new subtree
		node *new_root = (node *) malloc(sizeof(node));
		new_root->children = NULL;
		new_root->label = -1;
		//ll for holding record of previously added nodes
		record *added_nodes = NULL;
		//starting point ffor going to nodes with given label
		child_node * t_child = mapper[label].children;
		
		//go to every node containing the given element
		while(t_child != NULL)
		{	
			//printf(" \t in branch %d\n",count);
			//node to traverse branch upwards
			node *trav = t_child->child;
			// last travelled to node
			node *last_node = NULL;
			node *top_node =  NULL;
			while (trav != root )
			{
				//if given node is in the added nodes list
				record *res = search_record_ll(added_nodes,trav);
				
				if (res == NULL)
				{
					//printf("res not found\n");
					//create a new node
					// copy iver the trav node
					node * new_node = (node *) malloc(sizeof(node*));
					new_node->label = trav->label;
					new_node->support = trav->support;
					new_node->parent = new_root;
					added_nodes = add_2_ll(added_nodes, trav , new_node);
					//update the travelled list
					top_node = new_node;
					
					if (last_node ==NULL)
					{
						last_node = new_node;
					}
					else
					{
					//	printf("-************|\n");
						child_node *c_node = (child_node *) malloc(sizeof(child_node));
						c_node->child =  last_node;
						c_node->nxt = NULL;
						new_node->children = c_node;
						last_node->parent = new_node;
						last_node = new_node;
					}
					
				}
				else
				{
					//given node jas been processed
					//create a child node for last node
					child_node *c_node = (child_node *) malloc(sizeof(child_node));
					c_node->child =  last_node;
					last_node->parent = res->new_tree_node;
					//no last node
					last_node = NULL;
					top_node = NULL;
					//go to corresponding new tree node to visited old tree node
					node * n_node = res->new_tree_node;
					//add to the child ll
					child_node * ch = n_node->children;
					n_node->children = c_node;
					c_node->nxt = ch;
					break;
				}
				trav = trav->parent;
			}
			
			if (top_node != NULL)
			{
				//printf("\t*********** END of branch -- top_node %d\n",top_node->label);
				//create child node for the root
				child_node *nc = (child_node*)malloc(sizeof(child_node));
				top_node->parent = new_root;
				nc->child = top_node;
				nc->nxt = NULL;
				// connect to new root
				if (new_root->children == NULL)
					new_root->children = nc;
				else
				{
					child_node *temp = new_root->children;
					new_root->children = nc;
					nc->nxt = temp;
				}
			}
			t_child = t_child->nxt;
			count ++;
		}
		return new_root;
	}
//________________________________________________________



//--------------------------------------------------------
//		Print branches containing given element
//

	void print_rev_branch(int label , node * mapper ,node *root)
	{
		int i = 0;
		child_node *t_child =  mapper[label].children;
		node * trav ;
		printf("in rev branch");
		while (t_child != NULL)
		{
			printf("\n\n\t");
			//a node in the branch with the label is reached
			trav = t_child->child;
			//to travrese back to root
			//initialiase a parent pointer
			node *par = trav->parent;
			while(par != root)
			{
				printf("-- %d -- ",par->label);
				par = par->parent;
			}
			t_child = t_child->nxt;
		}
		return;
	}
//________________________________________________________



//--------------------------------------------------------
//	PRINT FP - TREE level order equivalent
//

	void print_FPtree(node  * root)
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
	//	if (location->parent ==  NULL && location != root)
	//		printf("FAIL AT %d \n",location->label);
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
//	Record LL addition/Creation
//

	record * add_2_ll(record * root , node * old_node , node * new_node)
	{
		record *new_r = (record *)malloc(sizeof(record));
		new_r->old_tree_node = old_node;
		new_r->new_tree_node = new_node;
		
		if (root == NULL)
		{
			root = new_r;
			return root;
		}
		
		record *temp =  root;
		root = new_r;
		new_r->next = temp;
		temp->prev = new_r;
		return root;
	}
//_________________________________________________________	



//---------------------------------------------------------
//	Search in Record LL
//

	record * search_record_ll(record * root , node * old_node)
	{
		if (old_node == NULL)
		{
			old_node = (node *)malloc(sizeof(node));
			old_node->label =-2;
		}
		record * traverse = root;
		//printf("\nIn search node function-- looking for %d \n",old_node->label);
		while (traverse != NULL)
		{
			//printf(" %d \t",traverse->old_tree_node->label);
			if (traverse->old_tree_node == old_node)
			{
				//printf("  Found  \n");
				return traverse;
			}
			traverse = traverse->next;
		}
		printf("\n");
		return NULL;
	}
//_________________________________________________________



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
