#ifndef _RB_TREE_H_
#define _RB_TREE_H_

#include <iostream.h>

//  <summary>
//  RBNode is a red-black tree node with parameterized value
//  </summary>
//
//  <synopsis>
//  RBNode stores three pointers to parent, left, and right nodes.
//  The second constructor allows the user of RBNode to specify a 
//  zero value other than NULL.
//
//  The color is assigned the value 0 for black and 1 for red.
//
//  The value is of arbitrary type.
//  </synopsis>
//

template <class T>
class RBNode
{
 public:
  
  // Create a node with NULL links and put the value "val" inside.
  // color the node black by default.
  RBNode(T val) : left(NULL), right(NULL), 
  parent(NULL), value(val), color(0) {}

  // Create a node using zero for the links, put "val" inside the node
  // and color it black.
  RBNode(T val, RBNode<T> * zero) : left(zero), right(zero), parent(zero)
    ,value(val), color(0) {}
  
  // These public variables are for use by RBTree only.
  // <group>
  class RBNode<T> * left;
  class RBNode<T> * right;
  class RBNode<T> * parent; 
  // </group>
  
  // Value of node
  T value;  
  
  // Color of incoming edge
  unsigned char color;
};

//
// <summary>
// RBTree implements a dynamic Red-Black tree.
// </summary>
//
// <etymology>
// RBTree = R (Red) + B (Black) + Tree
// </etymology>
//
// <synopsis>
//
// RBTree is a templated class that fully implements the red-black
// sorted-value tree described by Corman et al's Algorithms book.
//
// The leaves are represented by a common "nil" node for convenience.
//
// Functions insert, remove, find all take O(log n) time.
//
// The print functions take linear time.
//
// If T is a class, it must have correctly-behaving assignment and
// comparison (<,>,==) operators defined.
//
// It is probably unwise to store pointers in this container
//
// </synopsis>
//
// <todo asof="1/6/96">
// <li> add second-order traversal functions
// <li> add description function
// </todo>

template <class T>
class RBTree
{
 public:

  RBTree();

  // Insert value into the tree in its place.  Restructure the
  // tree if necessary to balance the height.
  void insert(T value)
    { insert(new RBNode<T>(value, nil_)); }

  // Print the tree as a nested parenthetical expression using
  // an inorder traversal
  // <group>
  void print() { inorderPrint(root_); }
  void inorderPrint() { inorderPrint(root_); }
  // </group>

  // Print the tree as a nested parenthetical expression using
  // a preorder traversal
  void preorderPrint() { preorderPrint(root_); }

  // Remove the first node encountered that contains the item "value"
  void remove(T value)
    {
      RBNode<T> * node = find_(value);
      if (node != nil_)
	{
	  deleteNode(node);
	  delete node;
	}
    }

  // Return non-zero if value is found in the tree
  int find(T value)
    {
      RBNode<T> * node = find_(value);
      return (node != nil_);
    }

  // Return false if value wasn't gotten
  bool get(T& value)
    {
      RBNode<T> * node = find_(value);
      if( node == nil_ )
         return false;
      //return (node != nil_);
      value = node->value;
      return true;
    }
    
  // Replace stuff
  void replace(T value)
    {
      RBNode<T> * node = find_(value);
      if( node == nil_ )
         return;
      //return (node != nil_);
      node->value = value;
    }

private:

  // Perform a single rotation at node x
  // <group>
  void rotateLeft(RBNode<T> * x);
  void rotateRight(RBNode<T> * x);
  // </group>

  // Insert the node z into the tree and rebalance
  void insert(RBNode<T> * z);

  // Delete the node z from the tree and rebalance
  void deleteNode(RBNode<T> * z);

  // Helper function for deleteNode
  void deleteFixup(RBNode<T> * x);

  // Return the next node in the inorder sequence
  RBNode<T> * successor(RBNode<T> * z);

  // Return the previous node in the inorder sequence
  RBNode<T> * predecessor(RBNode<T> * z);

  // Return the node that contains the minimum value (leftmost)
  // <group>
  RBNode<T> * minimum() { return minimum(root_); }
  RBNode<T> * minimum(RBNode<T> * x);
  // </group>

  // Return the node that contains the maximum value (rightmost)
  // <group>
  RBNode<T> * maximum() { return maximum(root_); }
  RBNode<T> * maximum(RBNode<T> * x);
  // </group>

  // return the first node that contains value or nil_
  // <group>
  RBNode<T> * find_(T value) { return find_(value, root_); }
  RBNode<T> * find_(T value, RBNode<T> * x);
  // </group>
 
  // print the nodes of the subtree rooted at x in a
  // nested parenthetical manner using inorder traversal
  void inorderPrint(RBNode<T> * x);
	    
  // print the nodes of the subtree rooted at x in a
  // nested parenthetical manner using preorder traversal
  void preorderPrint(RBNode<T> * x);

private:

  // root node of tree
  RBNode<T> * root_;

  // common nil node of tree
  RBNode<T> * nil_;
};


template <class T>
RBTree<T>::RBTree()
{
  nil_ = new RBNode<T>((T)0);

  nil_->left = nil_;
  nil_->right = nil_;
  nil_->parent = nil_;

  root_ = nil_;
}

template <class T>
void RBTree<T>::rotateLeft(RBNode<T> * x)
{
  RBNode<T> * y = x->right;
  x->right = y->left;
  if (y->left != nil_)
    y->left->parent = x;
  y->parent = x->parent;
  if (x->parent == nil_)
    {
      root_ = y;
    }
  else
    {
      if (x == x->parent->left)
	x->parent->left = y;
      else
	x->parent->right = y;
    }
  y->left = x;
  x->parent = y;
}

template <class T>
void RBTree<T>::rotateRight(RBNode<T> * x)
{
  RBNode<T> * y = x->left;
  x->left = y->right;
  if (y->right != nil_)
    y->right->parent = x;
  y->parent = x->parent;
  if (x->parent == nil_)
    root_ = y;
  else
    {
      if (x == x->parent->right)
	x->parent->right = y;
      else
	x->parent->left = y;
    }
  y->right = x;
  x->parent = y;
}

template <class T>
void RBTree<T>::insert(RBNode<T> * z)
{
  // Tree-Insert in book
  RBNode<T> * y = nil_;
  RBNode<T> * x = root_;
  while (x != nil_)
    {
      y = x;
      if (z->value < x->value)
	x = x->left;
      else
	x = x->right;
    }
  z->parent = y;
  if (y == nil_)
    root_ = z;
  else
    {
      if (z->value < y->value)
	y->left = z;
      else 
	y->right = z;
    }
  
  z->color = 1;
  while (z != root_ && z->parent->color == 1)
    {
      if (z->parent == z->parent->parent->left)
	{
	  y = z->parent->parent->right;
	  if (y->color == 1)
	    {
	      z->parent->color = 0;
	      y->color = 0;
	      z->parent->parent->color = 1;
	      z = z->parent->parent;
	    }
	  else
	    {
	      if (z == z->parent->right)
		{
		  z = z->parent;
		  rotateLeft(z);
		}
	      z->parent->color = 0;
	      z->parent->parent->color = 1;
	      rotateRight(z->parent->parent);
	    }
	}
      else
	{
	  y = z->parent->parent->left;
	  if (y->color == 1)
	    {
	      z->parent->color = 0;
	      y->color = 0;
	      z->parent->parent->color = 1;
	      z = z->parent->parent;
	    }
	  else
	    {
	      if (z == z->parent->left)
		{
		  z = z->parent;
		  rotateRight(z);
		}
	      z->parent->color = 0;
	      z->parent->parent->color = 1;
	      rotateLeft(z->parent->parent);
	    }
	}
    }
  root_->color = 0;
}

template <class T>
void RBTree<T>::deleteNode(RBNode<T> * z)
{
  RBNode<T> * y = nil_;
  RBNode<T> * x = nil_;
  
  cout << "deleteNode" << endl;

  if (z->left == nil_ || z->right == nil_)
    y = z;
  else
    y = successor(z);
  
  if (y->left != nil_)
    x = y->left;
  else
    x = y->right;
  
  x->parent = y->parent;
  
  if (y->parent == nil_)
    root_ = x;
  else
    {
      if (y == y->parent->left)
	y->parent->left = x;
      else 
	y->parent->right = x;
    }
  
  if (y != z)
    z->value = y->value;
  
  if (y->color == 0)
    deleteFixup(x);
}

template <class T>
void RBTree<T>::deleteFixup(RBNode<T> * x)
{
  cout << "deleteFixup" << endl;
  while (x != root_ && x->color == 0)
    {
      if (x == x->parent->left)
	{
	  RBNode<T> * w = x->parent->right;
	  if (w->color == 1)
	    {
	      w->color = 0;
	      x->parent->color = 1;
	      rotateLeft(x->parent);
	      w = x->parent->right;
	    }
	  
	  if (w->left->color == 0 && w->right->color == 0)
	    {
	      w->color = 1;
	      x = x->parent;
	    }
	  else
	    {
	      if (w->right->color == 0)
		{
		  w->left->color = 0;
		  w->color = 1;
		  rotateRight(w);
		  w = x->parent->right;
		}
	      w->color = x->parent->color;
	      x->parent->color = 0;
	      w->right->color = 0;
	      rotateLeft(x->parent);
	      x = root_;
	    }
	}
      else
	{
	  RBNode<T> * w = x->parent->left;
	  if (w->color == 1)
	    {
	      w->color = 0;
	      x->parent->color = 1;
	      rotateRight(x->parent);
	      w = x->parent->left;
	    }
	  
	  if (w->right->color == 0 && w->left->color == 0)
	    {
	      w->color = 1;
	      x = x->parent;
	    }
	  else
	    {
	      if (w->left->color == 0)
		{
		  w->right->color = 0;
		  w->color = 1;
		  rotateLeft(w);
		  w = x->parent->left;
		}
	      w->color = x->parent->color;
	      x->parent->color = 0;
	      w->left->color = 0;
	      rotateRight(x->parent);
	      x = root_;
	    }
	}
    }
  x->color = 0;
}

template <class T>
RBNode<T> * RBTree<T>::successor(RBNode<T> * x)
{
  if (x->right != nil_)
    return minimum(x->right);
  RBNode<T> * retval = x->parent;
  while (retval != nil_ && retval->right == x)
    {
      x = retval;
      retval = retval->parent;
    }
  return retval;
}

template <class T>
RBNode<T> * RBTree<T>::predecessor(RBNode<T> * x)
{
  if (x->left != nil_)
    return minimum(x->left);
  RBNode<T> * retval = x->parent;
  while (retval != nil_ && retval->left == x)
    {
      x = retval;
      retval = retval->parent;
    }
  return retval;
}

template <class T>
RBNode<T> * RBTree<T>::minimum(RBNode<T> * x)
{
  while (x->left != nil_) x = x->left;
  return x;
}

template <class T>
RBNode<T> * RBTree<T>::maximum(RBNode<T> * x)
{
  while (x->right != nil_) x = x->right;
  return x;
}

template <class T>
RBNode<T> * RBTree<T>::find_(T value, RBNode<T> * x)
{
  while (x != nil_ && value != x->value)
    {
      if (value < x->value)
	x = x->left;
      else
	x = x->right;
    }
  return x;
}

template <class T>
void RBTree<T>::inorderPrint(RBNode<T> * x)
{
  if (x == nil_) return;
  
  if (x->left != nil_)
    {
      cout << "(";
      inorderPrint(x->left);
      cout << ")";
    }
  
  if (x->color == 1)
    cout << "+";
  else
    cout << "-";

  cout << x->value;
  
  if (x->right != nil_)
    {
      cout << "(";
      inorderPrint(x->right);
      cout << ")";
    }
}

template <class T>
void RBTree<T>::preorderPrint(RBNode<T> * x)
{
  if (x == nil_) return;

  if (x->color == 1)
    cout << "+";
  else
    cout << "-";

  cout << x->value;

  if (x->left != nil_)
    {
      cout << "(";
      preorderPrint(x->left);
      cout << ")";
    }

   if (x->right != nil_)
    {
      cout << "(";
      preorderPrint(x->right);
      cout << ")";
    }
}

#endif





