/*
 * Copyright (C) 2019  Iván Ávalos
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <graphviz/gvc.h>
#include <graphviz/cgraph.h>

const char* help_message =
  "usage: %s output\n"
  "or:    %s --help\n";
const char* prompt = "CMD:\\> ";

typedef struct Node
{
  Agnode_t* cnode;
  int data;
  struct Node* left;
  struct Node* right;
} Node;

GVC_t* gvc;
Agraph_t* g;
Node* root;

void sigint_handler (int s);

Node* node_create (int data);
Node* node_create_from_input (void);
void node_free (Node* node);
void tree_insert (Node* root, Node* node);
void tree_connect (Node* root);
void tree_insert_from_input ();
void tree_traverse_preorder (Node* root);
void tree_traverse_inorder (Node* root);
void tree_traverse_postorder (Node* root);
void tree_free (Node* root);
void graph_init (void);
void graph_render (char* output);
void graph_free (void);

int
main (int argc,
	  char* argv[])
{
  if (argc < 2)
	{
	  fprintf (stderr, help_message, argv[0], argv[0]);
	  exit (-1);
	}
  if (strncmp("--help", argv[1], 6) == 0)
	{
	  fprintf (stderr, help_message, argv[0], argv[0]);
	  exit (0);
	}
  
  char *output, cmd[12];
  output = argv[1];
  
  graph_init ();
  printf ("Initialisation\n");
  printf ("Root value: ");
  root = node_create_from_input ();

  /* Intercept SIGINT signal */
  struct sigaction sigint;
  sigint.sa_handler = sigint_handler;
  sigemptyset(&sigint.sa_mask);
  sigint.sa_flags = 0;
  sigaction(SIGINT, &sigint, NULL);

  do
	{
	  printf ("%s", prompt);
	  scanf ("%12s", &(cmd[0]));
	  if (strncmp("insert", cmd, 6) == 0)
		tree_insert_from_input ();
	  else if (strncmp("preorder", cmd, 8) == 0)
		tree_traverse_preorder (root);
	  else if (strncmp("inorder", cmd, 7) == 0)
		tree_traverse_inorder (root);
	  else if (strncmp("postorder", cmd, 9) == 0)
		tree_traverse_postorder (root);
	  else if (strncmp("render", cmd, 6) == 0)
		graph_render (output);
	  else if (strncmp("exit", cmd, 4) == 0)
		break;
	} while (1);

  tree_free(root);
  graph_free ();
}

void
sigint_handler (int s)
{
  tree_free (root);
  graph_free ();
  printf ("\n");
  exit (0);
}

Node*
node_create (int data)
{
  Node* node;
  node = malloc (sizeof (node_t));
  node->data = data;
  node->left = NULL;
  node->right = NULL;
  
  return (node);
}

Node*
node_create_from_input ()
{
  char str_input [12];
  int num_input;
  scanf("%12s", &(str_input[0]));
  num_input = atoi (str_input);
  return (node_create(num_input));
}

void
node_free (Node* node)
{
  free (node);
}

void
tree_insert (Node* root,
			 Node* node)
{
  if (node->data < root->data)
	{
	  if (root->left == NULL)
		{
		  root->left = node;
		  return;
		}
	  tree_insert (root->left, node);
	}
  if (node->data > root->data)
	{
	  if (root->right == NULL)
		{
		  root->right = node;
		  return;
		}
	  tree_insert (root->right, node);
	}
  if (node->data == root->data)
	{
	  // there cannot be repeated elements
	  fprintf (stderr, "ERROR: cannot add duplicate element\n");
	}
}

void
tree_connect (Node* root)
{
  if (!root->cnode)
	{
	  char data [12];
	  sprintf(data, "%i", root->data);
	  root->cnode = agnode(g, data, TRUE);
	}
  
  if (root->left)
	{
	  char ldata [12];
	  sprintf(ldata, "%i", root->left->data);
	  root->left->cnode = agnode(g, ldata, TRUE);

	  if (!agedge(g, root->cnode, root->left->cnode, 0, FALSE))
		agedge (g, root->cnode, root->left->cnode, 0, TRUE);
	  
	  tree_connect (root->left);
	}
  if (root->right)
	{
	  char rdata [12];
	  sprintf(rdata, "%i", root->right->data);
	  root->right->cnode = agnode(g, rdata, TRUE);

	  if (!agedge(g, root->cnode, root->right->cnode, 0, FALSE))
		agedge (g, root->cnode, root->right->cnode, 0, TRUE);
	  
	  tree_connect (root->right);
	}
}

void
tree_insert_from_input ()
{
  Node* node;
  node = node_create_from_input ();
  tree_insert(root, node);
}

void
tree_traverse_preorder (Node* root)
{
  if (root == NULL) return;
  printf ("%s ", agnameof(root->cnode));
  tree_traverse_preorder (root->left);
  tree_traverse_preorder (root->right);
}

void
tree_traverse_inorder (Node* root)
{
  if (root == NULL) return;
  tree_traverse_preorder (root->left);
  printf ("%s ", agnameof(root->cnode));
  tree_traverse_preorder (root->right);
}

void
tree_traverse_postorder (Node* root)
{
  if (root == NULL) return;
  tree_traverse_preorder (root->left);
  tree_traverse_preorder (root->right);
  printf ("%s ", agnameof(root->cnode));
}

void
tree_free (Node* root)
{
  if (root == NULL) return;
  tree_free (root->left);
  tree_free (root->right);
  node_free(root);
}

void
graph_init ()
{
  gvc = gvContext ();
  g = agopen("g", Agdirected, 0);
}

void
graph_render (char* output)
{
  tree_connect (root);
  gvLayout (gvc, g, "dot");
  gvRenderFilename(gvc, g, "png", output);
  gvFreeLayout (gvc, g);
}

void
graph_free ()
{
  gvFreeLayout (gvc, g);
  agclose (g);
}
