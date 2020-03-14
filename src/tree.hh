// STL-like templated tree class.
//
// Copyright (C) 2001-2019 Kasper Peeters <kasper@phi-sci.com>
// Distributed under the GNU General Public License version 3.
//
// Special permission to use tree.hh under the conditions of a
// different license can be requested from the author.

/*	\mainpage tree.hh
	\author   Kasper Peeters
	\version  3.13
	\date     28-Mar-2019
	\see      http://tree.phi-sci.com/
	\see      http://github.com/kpeeters/tree.hh/

	The tree.hh library for C++ provides an STL-like container class
	for n-ary trees, templated over the data stored at the
	nodes. Various types of iterators are provided (post-order,
	pre-order, and others). Where possible the access methods are
	compatible with the STL or alternative algorithms are
	available.
*/


#ifndef tree_hh_
#define tree_hh_

#include <cassert>
#include <memory>
#include <stdexcept>
#include <iterator>
#include <set>
#include <queue>
#include <algorithm>
#include <cstddef>


#ifndef TREE_HH_EXTRA
// Defining TREE_HH_EXTRA will enable the following features:
// - enable size() return in O(1)
// - enable expanding and collapsing nodes in the tree
// - enable selecting nodes in the tree
// - enable node filtering
// - enable tree swap
#define TREE_HH_EXTRA
#endif // TREE_HH_EXTRA

/// A node in the tree, combining links to other nodes as well as the actual data.
template<class T>
class tree_node_ { // size: 5*4=20 bytes (on 32 bit arch), can be reduced by 8.
public:
	tree_node_();
	tree_node_(const T&);
	tree_node_(T&&);

	tree_node_<T>* parent;
	tree_node_<T>* first_child, * last_child;
	tree_node_<T>* prev_sibling, * next_sibling;
	T data;
#ifdef TREE_HH_EXTRA
	size_t count;
	size_t count_expanded;
	size_t count_expandable;
	bool expanded;
	bool visible;
#endif // TREE_HH_EXTRA
};

template<class T>
tree_node_<T>::tree_node_()
	: parent(0), first_child(0), last_child(0), prev_sibling(0), next_sibling(0)
#ifdef TREE_HH_EXTRA
	, count(0), count_expanded(0), count_expandable(0)
	, expanded(false), visible(false)
#endif // TREE_HH_EXTRA
{ }

template<class T>
tree_node_<T>::tree_node_(const T& val)
	: parent(0), first_child(0), last_child(0), prev_sibling(0), next_sibling(0), data(val)
#ifdef TREE_HH_EXTRA
	, count(0), count_expanded(0), count_expandable(0)
	, expanded(false), visible(false)
#endif // TREE_HH_EXTRA
{ }

template<class T>
tree_node_<T>::tree_node_(T&& val)
	: parent(0), first_child(0), last_child(0), prev_sibling(0), next_sibling(0), data(val)
#ifdef TREE_HH_EXTRA
	, count(0), count_expanded(0), count_expandable(0)
	, expanded(false), visible(false)
#endif // TREE_HH_EXTRA
{ }

template<class T, class tree_node_allocator = std::allocator<tree_node_<T>>>
class tree {
public:
	typedef tree_node_<T> tree_node;
public:
	/// Value of the data stored at a node.
	typedef T value_type;

	class iterator_base;
	class pre_order_iterator;
#ifdef TREE_HH_EXTRA
	class expanded_iterator;
	class selected_iterator;
	class expandable_iterator;
#endif // TREE_HH_EXTRA
	class post_order_iterator;
	class sibling_iterator;
	class leaf_iterator;

	tree();                                                                       // empty constructor
	tree(const T&);                                                               // constructor setting given element as head
	tree(const iterator_base&);
	tree(const tree<T, tree_node_allocator>&);                                    // copy constructor
	tree(tree<T, tree_node_allocator>&&);                                         // move constructor
	~tree();
	tree<T, tree_node_allocator>& operator=(const tree<T, tree_node_allocator>&); // copy assignment
	tree<T, tree_node_allocator>& operator=(tree<T, tree_node_allocator>&&);      // move assignment

	/// Base class for iterators, only pointers stored, no traversal logic.
#ifdef __SGI_STL_PORT
	class iterator_base : public stlport::bidirectional_iterator<T, ptrdiff_t> {
#else
	class iterator_base {
#endif
	public:
		typedef T                               value_type;
		typedef T*                              pointer;
		typedef T&                              reference;
		typedef size_t                          size_type;
		typedef ptrdiff_t                       difference_type;
		typedef std::bidirectional_iterator_tag iterator_category;

		iterator_base();
		iterator_base(tree_node*);

		T& operator*() const;
		T* operator->() const;

		/// When called, the next increment/decrement skips children of this node.
		void skip_children();
		void skip_children(bool skip);
		/// Number of children of the node pointed to by the iterator.
		size_t number_of_children() const;

		sibling_iterator begin() const;
		sibling_iterator end() const;

		tree_node* node;
	protected:
		bool skip_current_children_;
	};

	/// Depth-first iterator, first accessing the node, then its children.
	class pre_order_iterator : public iterator_base {
	public:
		pre_order_iterator();
		pre_order_iterator(tree_node*);
		pre_order_iterator(const iterator_base&);
		pre_order_iterator(const sibling_iterator&);

		bool operator==(const pre_order_iterator&) const;
		bool operator!=(const pre_order_iterator&) const;
		pre_order_iterator& operator++();
		pre_order_iterator& operator--();
		pre_order_iterator operator++(int);
		pre_order_iterator operator--(int);
		pre_order_iterator& operator+=(size_t);
		pre_order_iterator& operator-=(size_t);

		pre_order_iterator& next_skip_children();
	};

#ifdef TREE_HH_EXTRA
	// Depth-first iterator, first accessing the node, then its children.
	class expanded_iterator : public iterator_base {
	public:
		expanded_iterator();
		expanded_iterator(tree_node*);
		expanded_iterator(const iterator_base&);
		expanded_iterator(const sibling_iterator&);

		bool operator==(const iterator_base&) const;
		bool operator!=(const iterator_base&) const;
		expanded_iterator& operator++();
		expanded_iterator& operator--();
		expanded_iterator operator++(int);
		expanded_iterator operator--(int);
		expanded_iterator& operator+=(size_t);
		expanded_iterator& operator-=(size_t);

		expanded_iterator& next_skip_children();
	};

	class expandable_iterator : public iterator_base {
	public:
		expandable_iterator();
		expandable_iterator(tree_node*);
		expandable_iterator(const iterator_base&);
		expandable_iterator(const sibling_iterator&);

		bool operator==(const expandable_iterator&) const;
		bool operator!=(const expandable_iterator&) const;
		expandable_iterator& operator++();
		expandable_iterator& operator--();
		expandable_iterator operator++(int);
		expandable_iterator operator--(int);
		expandable_iterator& operator+=(size_t);
		expandable_iterator& operator-=(size_t);

		expandable_iterator& next_skip_children();
	};
#endif // TREE_HH_EXTRA

	/// Depth-first iterator, first accessing the children, then the node itself.
	class post_order_iterator : public iterator_base {
	public:
		post_order_iterator();
		post_order_iterator(tree_node*);
		post_order_iterator(const iterator_base&);
		post_order_iterator(const sibling_iterator&);

		bool operator==(const post_order_iterator&) const;
		bool operator!=(const post_order_iterator&) const;
		post_order_iterator& operator++();
		post_order_iterator& operator--();
		post_order_iterator operator++(int);
		post_order_iterator operator--(int);
		post_order_iterator& operator+=(size_t);
		post_order_iterator& operator-=(size_t);

		/// Set iterator to the first child as deep as possible down the tree.
		void descend_all();
	};

	/// Breadth-first iterator, using a queue
	class breadth_first_queued_iterator : public iterator_base {
	public:
		breadth_first_queued_iterator();
		breadth_first_queued_iterator(tree_node*);
		breadth_first_queued_iterator(const iterator_base&);

		bool operator==(const breadth_first_queued_iterator&) const;
		bool operator!=(const breadth_first_queued_iterator&) const;
		breadth_first_queued_iterator& operator++();
		breadth_first_queued_iterator operator++(int);
		breadth_first_queued_iterator& operator+=(size_t);

	private:
		std::queue<tree_node*> traversal_queue;
	};

	/// The default iterator types throughout the tree class.
	typedef pre_order_iterator iterator;
	typedef breadth_first_queued_iterator breadth_first_iterator;

	/// Iterator which traverses only the nodes at a given depth from the root.
	class fixed_depth_iterator : public iterator_base {
	public:
		fixed_depth_iterator();
		fixed_depth_iterator(tree_node*);
		fixed_depth_iterator(const iterator_base&);
		fixed_depth_iterator(const sibling_iterator&);
		fixed_depth_iterator(const fixed_depth_iterator&);

		bool operator==(const fixed_depth_iterator&) const;
		bool operator!=(const fixed_depth_iterator&) const;
		fixed_depth_iterator& operator++();
		fixed_depth_iterator& operator--();
		fixed_depth_iterator operator++(int);
		fixed_depth_iterator operator--(int);
		fixed_depth_iterator& operator+=(size_t);
		fixed_depth_iterator& operator-=(size_t);

		tree_node* top_node;
	};

	/// Iterator which traverses only the nodes which are siblings of each other.
	class sibling_iterator : public iterator_base {
	public:
		sibling_iterator();
		sibling_iterator(tree_node*);
		sibling_iterator(const sibling_iterator&);
		sibling_iterator(const iterator_base&);

		bool operator==(const sibling_iterator&) const;
		bool operator!=(const sibling_iterator&) const;
		sibling_iterator& operator++();
		sibling_iterator& operator--();
		sibling_iterator operator++(int);
		sibling_iterator operator--(int);
		sibling_iterator& operator+=(size_t);
		sibling_iterator& operator-=(size_t);

		tree_node* range_first() const;
		tree_node* range_last() const;
		tree_node* parent_;
	private:
		void set_parent_();
	};

	/// Iterator which traverses only the leaves.
	class leaf_iterator : public iterator_base {
	public:
		leaf_iterator();
		leaf_iterator(tree_node*, tree_node* top = 0);
		leaf_iterator(const sibling_iterator&);
		leaf_iterator(const iterator_base&);

		bool operator==(const leaf_iterator&) const;
		bool operator!=(const leaf_iterator&) const;
		leaf_iterator& operator++();
		leaf_iterator& operator--();
		leaf_iterator operator++(int);
		leaf_iterator operator--(int);
		leaf_iterator& operator+=(size_t);
		leaf_iterator& operator-=(size_t);
	private:
		tree_node* top_node;
	};

	/// Return iterator to the beginning of the tree.
	inline pre_order_iterator begin() const;
	/// Return iterator to the end of the tree.
	inline pre_order_iterator end() const;
#ifdef TREE_HH_EXTRA
	/// Return iterator to the beginning of the tree.
	inline expanded_iterator begin_expanded() const;
	/// Return iterator to the end of the tree.
	inline expanded_iterator end_expanded() const;
	/// Return iterator to the beginning of the tree.
	inline expandable_iterator begin_expandable() const;
	/// Return iterator to the end of the tree.
	inline expandable_iterator end_expandable() const;
#endif // TREE_HH_EXTRA
	/// Return post-order iterator to the beginning of the tree.
	post_order_iterator begin_post() const;
	/// Return post-order end iterator of the tree.
	post_order_iterator end_post() const;
	/// Return fixed-depth iterator to the first node at a given depth from the given iterator.
	/// If 'walk_back=true', a depth=0 iterator will be taken from the beginning of the sibling
	/// range, not the current node.
	fixed_depth_iterator begin_fixed(const iterator_base&, size_t, bool walk_back = true) const;
	/// Return fixed-depth end iterator.
	fixed_depth_iterator end_fixed(const iterator_base&, size_t) const;
	/// Return breadth-first iterator to the first node at a given depth.
	breadth_first_queued_iterator begin_breadth_first() const;
	/// Return breadth-first end iterator.
	breadth_first_queued_iterator end_breadth_first() const;
	/// Return sibling iterator to the first child of given node.
	static sibling_iterator begin(const iterator_base&);
	/// Return sibling end iterator for children of given node.
	static sibling_iterator end(const iterator_base&);
	/// Return leaf iterator to the first leaf of the tree.
	leaf_iterator begin_leaf() const;
	/// Return leaf end iterator for entire tree.
	leaf_iterator end_leaf() const;
	/// Return leaf iterator to the first leaf of the subtree at the given node.
	leaf_iterator begin_leaf(const iterator_base& top) const;
	/// Return leaf end iterator for the subtree at the given node.
	leaf_iterator end_leaf(const iterator_base& top) const;

	typedef std::vector<size_t> path_t;
	/// Return a path (to be taken from the 'top' node) corresponding to a node in the tree.
	/// The first integer in path_t is the number of steps you need to go 'right' in the sibling
	/// chain (so 0 if we go straight to the children).
	path_t path_from_iterator(const iterator_base& iter, const iterator_base& top) const;
	/// Return an iterator given a path from the 'top' node.
	iterator iterator_from_path(const path_t&, const iterator_base& top) const;
#ifdef TREE_HH_EXTRA
	path_t path_from_iterator(const expanded_iterator& iter) const;
#endif // TREE_HH_EXTRA

	/// Return iterator to the parent of a node.
	template<typename iter> static iter parent(iter);
	/// Return iterator to the previous sibling of a node.
	template<typename iter> static iter previous_sibling(iter);
	/// Return iterator to the next sibling of a node.
	template<typename iter> static iter next_sibling(iter);
	/// Return iterator to the next node at a given depth.
	template<typename iter> iter next_at_same_depth(iter) const;

	/// Erase all nodes of the tree.
	void clear();
	/// Erase element at position pointed to by iterator, return incremented iterator.
	template<typename iter> iter erase(iter);
	/// Erase all children of the node pointed to by iterator.
	void erase_children(const iterator_base&);
	/// Erase all siblings to the right of the iterator.
	void erase_right_siblings(const iterator_base&);
	/// Erase all siblings to the left of the iterator.
	void erase_left_siblings(const iterator_base&);

	/// Insert empty node as last/first child of node pointed to by position.
	template<typename iter> iter append_child(iter position);
	template<typename iter> iter prepend_child(iter position);
	/// Insert node as last/first child of node pointed to by position.
	template<typename iter> iter append_child(iter position, const T& x);
	template<typename iter> iter append_child(iter position, T&& x);
	template<typename iter> iter prepend_child(iter position, const T& x);
	template<typename iter> iter prepend_child(iter position, T&& x);
	/// Append the node (plus its children) at other_position as last/first child of position.
	template<typename iter> iter append_child(iter position, iter other_position);
	template<typename iter> iter prepend_child(iter position, iter other_position);
	/// Append the nodes in the from-to range (plus their children) as last/first children of position.
	template<typename iter> iter append_children(iter position, sibling_iterator from, sibling_iterator to);
	template<typename iter> iter prepend_children(iter position, sibling_iterator from, sibling_iterator to);

	/// Short-hand to insert topmost node in otherwise empty tree.
	pre_order_iterator set_head(const T& x);
	pre_order_iterator set_head(T&& x);
	/// Insert node as previous sibling of node pointed to by position.
	template<typename iter> iter insert(iter position, const T& x);
	template<typename iter> iter insert(iter position, T&& x);
	/// Specialisation of previous member.
	sibling_iterator insert(sibling_iterator position, const T& x);
	/// Insert node (with children) pointed to by subtree as previous sibling of node pointed to by position.
	/// Does not change the subtree itself (use move_in or move_in_below for that).
	template<typename iter> iter insert_subtree(iter position, const iterator_base& subtree);
	/// Insert node as next sibling of node pointed to by position.
	template<typename iter> iter insert_after(iter position, const T& x);
	template<typename iter> iter insert_after(iter position, T&& x);
	/// Insert node (with children) pointed to by subtree as next sibling of node pointed to by position.
	template<typename iter> iter insert_subtree_after(iter position, const iterator_base& subtree);

	/// Replace node at 'position' with other node (keeping same children); 'position' becomes invalid.
	template<typename iter> iter replace(iter position, const T& x);
	/// Replace node at 'position' with subtree starting at 'from' (do not erase subtree at 'from'); see above.
	template<typename iter> iter replace(iter position, const iterator_base& from);
	/// Replace string of siblings (plus their children) with copy of a new string (with children); see above
	sibling_iterator replace(sibling_iterator orig_begin, sibling_iterator orig_end, sibling_iterator new_begin, sibling_iterator new_end);

	/// Move all children of node at 'position' to be siblings, returns position.
	template<typename iter> iter flatten(iter position);
	/// Move nodes in range to be children of 'position'.
	template<typename iter> iter reparent(iter position, sibling_iterator begin, sibling_iterator end);
	/// Move all child nodes of 'from' to be children of 'position'.
	template<typename iter> iter reparent(iter position, iter from);

	/// Replace node with a new node, making the old node (plus subtree) a child of the new node.
	template<typename iter> iter wrap(iter position, const T& x);
	/// Replace the range of sibling nodes (plus subtrees), making these children of the new node.
	template<typename iter> iter wrap(iter from, iter to, const T& x);

	/// Move 'source' node (plus its children) to become the next sibling of 'target'.
	template<typename iter> iter move_after(iter target, iter source);
	/// Move 'source' node (plus its children) to become the previous sibling of 'target'.
	template<typename iter> iter move_before(iter target, iter source);
	sibling_iterator move_before(sibling_iterator target, sibling_iterator source);
	/// Move 'source' node (plus its children) to become the node at 'target' (erasing the node at 'target').
	template<typename iter> iter move_ontop(iter target, iter source);

	/// Extract the subtree starting at the indicated node, removing it from the original tree.
	tree move_out(iterator);
	/// Inverse of take_out: inserts the given tree as previous sibling of indicated node by a
	/// move operation, that is, the given tree becomes empty. Returns iterator to the top node.
	template<typename iter> iter move_in(iter, tree&);
	/// As above, but now make the tree the last child of the indicated node.
	template<typename iter> iter move_in_below(iter, tree&);
	/// As above, but now make the tree the nth child of the indicated node (if possible).
	template<typename iter> iter move_in_as_nth_child(iter, size_t, tree&);

	/// Merge with other tree, creating new branches and leaves only if they are not already present.
	void merge(sibling_iterator, sibling_iterator, sibling_iterator, sibling_iterator, bool duplicate_leaves = false);
	/// As above, but using two trees with a single top node at the 'to' and 'from' positions.
	void merge(iterator to, iterator from, bool duplicate_leaves);
	/// Sort (std::sort only moves values of nodes, this one moves children as well).
	void sort(sibling_iterator from, sibling_iterator to, bool deep = false);
	template<class StrictWeakOrdering>
	void sort(sibling_iterator from, sibling_iterator to, StrictWeakOrdering comp, bool deep = false);
#ifdef TREE_HH_EXTRA
	/// Filter
	size_t filter(bool value);
	template<class StrictWeakFiltering>
	size_t filter(StrictWeakFiltering comp);
#endif // TREE_HH_EXTRA
	/// Compare two ranges of nodes (compares nodes as well as tree structure).
	template<typename iter>
	bool equal(const iter& one, const iter& two, const iter& three) const;
	template<typename iter, class BinaryPredicate>
	bool equal(const iter& one, const iter& two, const iter& three, BinaryPredicate) const;
	template<typename iter>
	bool equal_subtree(const iter& one, const iter& two) const;
	template<typename iter, class BinaryPredicate>
	bool equal_subtree(const iter& one, const iter& two, BinaryPredicate) const;
	/// Extract a new tree formed by the range of siblings plus all their children.
	tree subtree(sibling_iterator from, sibling_iterator to) const;
	void subtree(tree&, sibling_iterator from, sibling_iterator to) const;
	/// Exchange the node (plus subtree) with its sibling node (do nothing if no sibling present).
	void swap(sibling_iterator it);
	/// Exchange two nodes (plus subtrees). The iterators will remain valid and keep
	/// pointing to the same nodes, which now sit at different locations in the tree.
	void swap(iterator, iterator);

	/// Count the total number of nodes.
	size_t size() const;
	/// Count the total number of nodes below the indicated node (plus one).
	size_t size(const iterator_base&) const;
#ifdef TREE_HH_EXTRA
	/// Count the total number of nodes.
	size_t count() const;
	/// Count the total number of nodes below the indicated node (plus one).
	size_t count(const iterator_base&) const;
	/// Count the total number of expanded nodes.
	size_t count_expanded() const;
	/// Count the total number of expanded nodes below the indicated node (plus one).
	size_t count_expanded(const iterator_base&) const;
	/// Count the total number of expandable nodes.
	size_t count_expandable() const;
	/// Count the total number of expandable nodes below the indicated node (plus one).
	size_t count_expandable(const iterator_base&) const;
	/// Exchange tree
	void swap(tree<T, tree_node_allocator>& other);
#endif // TREE_HH_EXTRA
	/// Check if tree is empty.
	bool empty() const;
	/// Compute the depth to the root or to a fixed other iterator.
	static size_t depth(const iterator_base&);
	static size_t depth(const iterator_base&, const iterator_base&);
	/// Determine the maximal depth of the tree. An empty tree has max_depth=-1.
	size_t max_depth() const;
	/// Determine the maximal depth of the tree with top node at the given position.
	size_t max_depth(const iterator_base&) const;
	/// Count the number of children of node at position.
	static size_t number_of_children(const iterator_base&);
	/// Count the number of siblings (left and right) of node at iterator. Total nodes at this level is +1.
	size_t number_of_siblings(const iterator_base&) const;
	/// Determine whether node at position is in the subtrees with indicated top node.
	bool is_in_subtree(const iterator_base& position, const iterator_base& top) const;
	/// Determine whether node at position is in the subtrees with root in the range.
	bool is_in_subtree(const iterator_base& position, const iterator_base& begin, const iterator_base& end) const;
	/// Determine whether the iterator is an 'end' iterator and thus not actually pointing to a node.
	bool is_valid(const iterator_base&) const;
	/// Determine whether the iterator is one of the 'head' nodes at the top level, i.e. has no parent.
	static bool is_head(const iterator_base&);
	/// Find the lowest common ancestor of two nodes, that is, the deepest node such that
	/// both nodes are descendants of it.
	iterator lowest_common_ancestor(const iterator_base&, const iterator_base&) const;

	/// Determine the index of a node in the range of siblings to which it belongs.
	size_t index(sibling_iterator it) const;
#ifdef TREE_HH_EXTRA
	// Determine the index of a node in the range of siblings to which it belongs.
	size_t index(pre_order_iterator it) const;
	size_t index(expanded_iterator it) const;
#endif // TREE_HH_EXTRA
	/// Inverse of 'index': return the n-th child of the node at position.
	static sibling_iterator child(const iterator_base& position, size_t);
	/// Return iterator to the sibling indicated by index
	sibling_iterator sibling(const iterator_base& position, size_t) const;

	/// For debugging only: verify internal consistency by inspecting all pointers in the tree
	/// (which will also trigger a valgrind error in case something got corrupted).
	void debug_verify_consistency() const;

	/// Comparator class for iterators (compares pointer values; why doesn't this work automatically?)
	class iterator_base_less {
	public:
		bool operator()(const typename tree<T, tree_node_allocator>::iterator_base& one, const typename tree<T, tree_node_allocator>::iterator_base& two) const {
			return one.node < two.node;
		}
	};
	tree_node* head, * feet; // head/feet are always dummy; if an iterator points to them it is invalid
#ifdef TREE_HH_EXTRA
	size_t collapse();

	size_t collapse(const iterator_base& it);

	size_t expand();

	size_t expand(const iterator_base& it);

	bool is_expanded(const iterator_base& it);

	class tree_selection {
		bool operator<(const tree_selection& other) const {
			return (begin < other.begin);
		}
		size_t begin;
		size_t end;
	};

	std::set<tree_selection> selection;
	bool multiple_selection;

	void select();

	void toggle(size_t index, size_t origin, uint32_t mode);

	void unselect();

	bool is_selected(size_t index);

	void set_multiple_selection(bool multiple);

	void set_expand_new(bool expand);
#endif // TREE_HH_EXTRA
private:
	tree_node_allocator alloc_;
	void head_initialise_();
	void copy_(const tree<T, tree_node_allocator>& other);
#ifdef TREE_HH_EXTRA
	void erase_children_(const iterator_base&);
#endif // TREE_HH_EXTRA

	/// Comparator class for two nodes of a tree (used for sorting and searching).
	template<class StrictWeakOrdering>
	class compare_nodes {
	public:
		compare_nodes(StrictWeakOrdering comp) : comp_(comp) { };

		bool operator()(const tree_node* a, const tree_node* b) {
			return comp_(a->data, b->data);
		}
	private:
		StrictWeakOrdering comp_;
	};

#ifdef TREE_HH_EXTRA
	/// Comparator class for a node of a tree (used for filtering).
	template<class StrictWeakFiltering>
	class filter_nodes {
	public:
		filter_nodes(StrictWeakFiltering comp) : comp_(comp) { };

		bool operator()(const tree_node* a) {
			return comp_(a->data);
		}
	private:
		StrictWeakFiltering comp_;
	};
#endif // TREE_HH_EXTRA
};

//template<class T, class tree_node_allocator>
//class iterator_base_less {
//public:
//	bool operator()(const typename tree<T, tree_node_allocator>::iterator_base& one, const typename tree<T, tree_node_allocator>::iterator_base& two) const {
//		txtout << "operatorclass<" << one.node < two.node << std::endl;
//		return (one.node < two.node);
//	}
//};

//template<class T, class tree_node_allocator>
//bool operator<(const typename tree<T, tree_node_allocator>::iterator& one, const typename tree<T, tree_node_allocator>::iterator& two) {
//	txtout << "operator< " << one.node < two.node << std::endl;
//	return (one.node < two.node);
//}
//
//template<class T, class tree_node_allocator>
//bool operator==(const typename tree<T, tree_node_allocator>::iterator& one, const typename tree<T, tree_node_allocator>::iterator& two) {
//	txtout << "operator== " << one.node == two.node << std::endl;
//	return (one.node == two.node);
//}
//
//template<class T, class tree_node_allocator>
//bool operator>(const typename tree<T, tree_node_allocator>::iterator_base& one, const typename tree<T, tree_node_allocator>::iterator_base& two) {
//	txtout << "operator> " << one.node < two.node << std::endl;
//	return (one.node > two.node);
//}



// Tree

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::tree() {
	head_initialise_();
}

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::tree(const T& x) {
	head_initialise_();
	set_head(x);
}

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::tree(tree<T, tree_node_allocator>&& x) {
	head_initialise_();

#ifdef TREE_HH_EXTRA
	head->next_sibling = x.head->next_sibling;
	x.head->next_sibling->prev_sibling = head;

	head->count = x.head->count;
	head->count_expanded = x.head->count_expanded;
	head->count_expandable = x.head->count_expandable;
	head->expanded = x.head->expanded;
	head->visible = x.head->visible;

	feet->prev_sibling = x.feet->prev_sibling;
	x.feet->prev_sibling->next_sibling = feet;

	feet->count = x.feet->count;
	feet->count_expanded = x.feet->count_expanded;
	feet->count_expandable = x.feet->count_expandable;
	feet->expanded = x.feet->expanded;
	feet->visible = x.feet->visible;

	selection = std::move(x.selection);
	multiple_selection = x.multiple_selection;

	x.head->next_sibling = x.feet;
	x.feet->prev_sibling = x.head;
#else // TREE_HH_EXTRA
	if (x.head->next_sibling != x.feet) { // move tree if non-empty only
		head->next_sibling = x.head->next_sibling;
		feet->prev_sibling = x.head->prev_sibling;
		x.head->next_sibling->prev_sibling = head;
		x.feet->prev_sibling->next_sibling = feet;
		x.head->next_sibling = x.feet;
		x.feet->prev_sibling = x.head;
	}
#endif // TREE_HH_EXTRA
}

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::tree(const iterator_base& other) {
	head_initialise_();
	set_head((*other));
	replace(begin(), other);
}

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::~tree() {
	clear();
	alloc_.destroy(head);
	alloc_.destroy(feet);
	alloc_.deallocate(head, 1);
	alloc_.deallocate(feet, 1);
}

template<class T, class tree_node_allocator>
void tree<T, tree_node_allocator>::head_initialise_() {
	head = alloc_.allocate(1, 0); // MSVC does not have default second argument
	feet = alloc_.allocate(1, 0);
	alloc_.construct(head, tree_node_<T>());
	alloc_.construct(feet, tree_node_<T>());

	head->parent = 0;
	head->first_child = 0;
	head->last_child = 0;
	head->prev_sibling = 0; //head;
	head->next_sibling = feet; //head;

	feet->parent = 0;
	feet->first_child = 0;
	feet->last_child = 0;
	feet->prev_sibling = head;
	feet->next_sibling = 0;

#ifdef TREE_HH_EXTRA
	head->count = 0;
	head->count_expanded = 0;
	head->count_expandable = 0;
	head->expanded = true;
	head->visible = true;

	feet->count = 0;
	feet->count_expanded = 0;
	feet->count_expandable = 0;
	feet->expanded = false;
	feet->visible = true;

	multiple_selection = false;
#endif // TREE_HH_EXTRA
}

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>& tree<T, tree_node_allocator>::operator=(const tree<T, tree_node_allocator>& other) {
	if (this != &other) {
		copy_(other);
	}

	return *this;
}

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>& tree<T, tree_node_allocator>::operator=(tree<T, tree_node_allocator>&& x) {
	if (this != &x) {
		clear(); // clear any existing data.

		head->next_sibling = x.head->next_sibling;
		feet->prev_sibling = x.feet->prev_sibling;
		x.head->next_sibling->prev_sibling = head;
#ifdef TREE_HH_EXTRA
		head->count = x.head->count;
		head->count_expanded = x.head->count_expanded;
		head->count_expandable = x.head->count_expandable;
		head->expanded = x.head->expanded;
		head->visible = x.head->visible;
#endif // TREE_HH_EXTRA

		x.feet->prev_sibling->next_sibling = feet;
#ifdef TREE_HH_EXTRA
		feet->count = x.feet->count;
		feet->count_expanded = x.feet->count_expanded;
		feet->count_expandable = x.feet->count_expandable;
		feet->expanded = x.feet->expanded;
		feet->visible = x.feet->visible;
#endif // TREE_HH_EXTRA
#ifdef TREE_HH_EXTRA
		selection = std::move(x.selection);
		multiple_selection = x.multiple_selection;
#endif // TREE_HH_EXTRA
		x.head->next_sibling = x.feet;
		x.feet->prev_sibling = x.head;
	}

	return *this;
}

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::tree(const tree<T, tree_node_allocator>& other) {
	head_initialise_();
	copy_(other);
}

template<class T, class tree_node_allocator>
void tree<T, tree_node_allocator>::copy_(const tree<T, tree_node_allocator>& other) {
	clear();
	pre_order_iterator it = other.begin(), to = begin();

	while (it != other.end()) {
		to = insert(to, (*it));
		it.skip_children();
		++it;
	}

	to = begin();
	it = other.begin();

	while (it != other.end()) {
		to = replace(to, it);
		to.skip_children();
		it.skip_children();
		++to;
		++it;
	}
}

template<class T, class tree_node_allocator>
void tree<T, tree_node_allocator>::clear() {
	if (head) {
		while (head->next_sibling != feet) {
			erase(pre_order_iterator(head->next_sibling));
		}
	}
}

template<class T, class tree_node_allocator>
void tree<T, tree_node_allocator>::erase_children(const iterator_base& it) {
//	std::cout << "erase_children " << it.node << std::endl;
	if (it.node == 0) {
		return;
	}

#ifdef TREE_HH_EXTRA
	// iterate all parent nodes and update size.
	// in addition iterate all expanded parent nodes and update expanded size.
	tree_node* parent = it.node->parent;
	bool parent_expanded = true;

	while (parent) {
		parent_expanded &= parent->expanded;

		if (parent_expanded) {
			parent->count_expanded -= it.node->count_expanded;
		}

		parent->count -= it.node->count;
		parent->count_expandable -= it.node->count_expandable;
		parent = parent->parent;
	}

	// if all parents are expanded update the tree expanded size.
	if (parent_expanded) {
		head->count_expanded -= it.node->count_expanded;
	}

	head->count -= it.node->count;
	head->count_expandable -= it.node->count_expandable;

	erase_children_(it);
#else // TREE_HH_EXTRA
	tree_node* cur = it.node->first_child;
	tree_node* prev = 0;

	while (cur != 0) {
		prev = cur;
		cur = cur->next_sibling;
		erase_children(pre_order_iterator(prev));
//		kp::destructor(&prev->data);
		alloc_.destroy(prev);
		alloc_.deallocate(prev, 1);
	}

	it.node->first_child = 0;
	it.node->last_child = 0;
//	std::cout << "exit" << std::endl;
#endif // TREE_HH_EXTRA
}

#ifdef TREE_HH_EXTRA
template<class T, class tree_node_allocator>
void tree<T, tree_node_allocator>::erase_children_(const iterator_base& it) {
//	std::cout << "erase_children " << it.node << std::endl;
	if (it.node == 0) {
		return;
	}

	tree_node* cur = it.node->first_child;
	tree_node* prev = 0;

	while (cur) {
		prev = cur;
		cur = cur->next_sibling;
		erase_children_(pre_order_iterator(prev));
//		kp::destructor(&prev->data);
		alloc_.destroy(prev);
		alloc_.deallocate(prev, 1);
	}

	it.node->first_child = 0;
	it.node->last_child = 0;
//	std::cout << "exit" << std::endl;
}
#endif // TREE_HH_EXTRA

template<class T, class tree_node_allocator>
void tree<T, tree_node_allocator>::erase_right_siblings(const iterator_base& it) {
	if (it.node == 0) {
		return;
	}

	tree_node* cur = it.node->next_sibling;
	tree_node* prev = 0;

	while (cur != 0) {
		prev = cur;
		cur = cur->next_sibling;
		erase_children(pre_order_iterator(prev));
//		kp::destructor(&prev->data);
		alloc_.destroy(prev);
		alloc_.deallocate(prev, 1);
	}

	it.node->next_sibling = 0;

	if (it.node->parent != 0) {
		it.node->parent->last_child = it.node;
	}
}

template<class T, class tree_node_allocator>
void tree<T, tree_node_allocator>::erase_left_siblings(const iterator_base& it) {
	if (it.node == 0) {
		return;
	}

	tree_node* cur = it.node->prev_sibling;
	tree_node* prev = 0;

	while (cur != 0) {
		prev = cur;
		cur = cur->prev_sibling;
		erase_children(pre_order_iterator(prev));
//		kp::destructor(&prev->data);
		alloc_.destroy(prev);
		alloc_.deallocate(prev, 1);
	}

	it.node->prev_sibling = 0;

	if (it.node->parent != 0) {
		it.node->parent->first_child = it.node;
	}
}

template<class T, class tree_node_allocator>
template<class iter>
iter tree<T, tree_node_allocator>::erase(iter it) {
	tree_node* cur = it.node;
	assert(cur != head);
	iter ret = it;
	ret.skip_children();
	++ret;
#ifdef TREE_HH_EXTRA
	erase_children_(it);
#else // TREE_HH_EXTRA
	erase_children(it);
#endif // TREE_HH_EXTRA

	if (cur->prev_sibling == 0) {
		cur->parent->first_child = cur->next_sibling;
	} else {
		cur->prev_sibling->next_sibling = cur->next_sibling;
	}

	if (cur->next_sibling == 0) {
		cur->parent->last_child = cur->prev_sibling;
	} else {
		cur->next_sibling->prev_sibling = cur->prev_sibling;
	}

#ifdef TREE_HH_EXTRA
	tree_node* parent = it.node->parent;
	bool parent_expanded = true;

	while (parent) {
		parent_expanded &= parent->expanded;

		if (parent_expanded) {
			--parent->count_expanded -= it.node->count_expanded;
		}

		--parent->count -= it.node->count;
		parent = parent->parent;
	}

	if (parent_expanded) {
		--head->count_expanded -= it.node->count_expanded;
	}

	--head->count -= it.node->count;
#endif // TREE_HH_EXTRA

//	kp::destructor(&cur->data);
	alloc_.destroy(cur);
	alloc_.deallocate(cur, 1);
	return ret;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::pre_order_iterator tree<T, tree_node_allocator>::begin() const {
	return pre_order_iterator(head->next_sibling);
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::pre_order_iterator tree<T, tree_node_allocator>::end() const {
	return pre_order_iterator(feet);
}

#ifdef TREE_HH_EXTRA
template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::expanded_iterator tree<T, tree_node_allocator>::begin_expanded() const {
	expanded_iterator it(head->next_sibling);
	if (!it.node->visible) {
		++it;
	}
	return it;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::expanded_iterator tree<T, tree_node_allocator>::end_expanded() const {
	return expanded_iterator(feet);
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::expandable_iterator tree<T, tree_node_allocator>::begin_expandable() const {
	return expandable_iterator(head->next_sibling);
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::expandable_iterator tree<T, tree_node_allocator>::end_expandable() const {
	return expandable_iterator(feet);
}
#endif // TREE_HH_EXTRA

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::breadth_first_queued_iterator tree<T, tree_node_allocator>::begin_breadth_first() const {
	return breadth_first_queued_iterator(head->next_sibling);
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::breadth_first_queued_iterator tree<T, tree_node_allocator>::end_breadth_first() const {
	return breadth_first_queued_iterator();
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::post_order_iterator tree<T, tree_node_allocator>::begin_post() const {
	tree_node* tmp = head->next_sibling;

	if (tmp != feet) {
		while (tmp->first_child) {
			tmp = tmp->first_child;
		}
	}

	return post_order_iterator(tmp);
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::post_order_iterator tree<T, tree_node_allocator>::end_post() const {
	return post_order_iterator(feet);
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::fixed_depth_iterator tree<T, tree_node_allocator>::begin_fixed(const iterator_base& pos, size_t dp, bool walk_back) const {
	typename tree<T, tree_node_allocator>::fixed_depth_iterator ret;
	ret.top_node = pos.node;

	tree_node* tmp = pos.node;
	size_t curdepth = 0;

	while (curdepth < dp) { // go down one level
		while (tmp->first_child == 0) {
			if (tmp->next_sibling == 0) {
				// try to walk up and then right again
				do {
					if (tmp == ret.top_node) {
						throw std::range_error("tree: begin_fixed out of range");
					}

					tmp = tmp->parent;

					if (tmp == 0) {
						throw std::range_error("tree: begin_fixed out of range");
					}

					--curdepth;
				} while (tmp->next_sibling == 0);
			}

			tmp = tmp->next_sibling;
		}

		tmp = tmp->first_child;
		++curdepth;
	}

	// Now walk back to the first sibling in this range.
	if (walk_back) {
		while (tmp->prev_sibling != 0) {
			tmp = tmp->prev_sibling;
		}
	}

	ret.node = tmp;
	return ret;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::fixed_depth_iterator tree<T, tree_node_allocator>::end_fixed(const iterator_base& pos, size_t dp) const {
	assert(1 == 0); // FIXME: not correct yet: use is_valid() as a temporary workaround
	tree_node* tmp = pos.node;
	size_t curdepth = 1;

	while (curdepth < dp) { // go down one level
		while (tmp->first_child == 0) {
			tmp = tmp->next_sibling;

			if (tmp == 0) {
				throw std::range_error("tree: end_fixed out of range");
			}
		}

		tmp = tmp->first_child;
		++curdepth;
	}

	return tmp;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::sibling_iterator tree<T, tree_node_allocator>::begin(const iterator_base& pos) {
	assert(pos.node != 0);

	if (pos.node->first_child == 0) {
		return end(pos);
	}

	return pos.node->first_child;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::sibling_iterator tree<T, tree_node_allocator>::end(const iterator_base& pos) {
	sibling_iterator ret(0);
	ret.parent_ = pos.node;
	return ret;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::leaf_iterator tree<T, tree_node_allocator>::begin_leaf() const {
	tree_node* tmp = head->next_sibling;

	if (tmp != feet) {
		while (tmp->first_child) {
			tmp = tmp->first_child;
		}
	}

	return leaf_iterator(tmp);
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::leaf_iterator tree<T, tree_node_allocator>::end_leaf() const {
	return leaf_iterator(feet);
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::path_t tree<T, tree_node_allocator>::path_from_iterator(const iterator_base& iter, const iterator_base& top) const {
	path_t path;
	tree_node* walk = iter.node;

	do {
		if (path.size() > 0) {
			walk = walk->parent;
		}

		size_t num = 0;

		while (walk != top.node && walk->prev_sibling != 0 && walk->prev_sibling != head) {
			++num;
			walk = walk->prev_sibling;
		}

		path.push_back(num);
	} while (walk->parent != 0 && walk != top.node);

	std::reverse(path.begin(), path.end());
	return path;
}

#ifdef TREE_HH_EXTRA
template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::path_t tree<T, tree_node_allocator>::path_from_iterator(const expanded_iterator& iter) const {
	path_t path;
	tree_node* walk = iter.node;

	do {
		if (path.size() > 0) {
			walk = walk->parent;
		}

		size_t num = 0;

		if (!walk) {
			break;
		}

		while (walk->prev_sibling != 0 && walk->prev_sibling != head) {
			if (walk->visible) {
				++num;
			}
			walk = walk->prev_sibling;
		}

		path.push_back(num);
	} while (walk->parent != 0);

	std::reverse(path.begin(), path.end());
	return path;
}
#endif // TREE_HH_EXTRA

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::iterator tree<T, tree_node_allocator>::iterator_from_path(const path_t& path, const iterator_base& top) const {
	iterator it = top;
	tree_node* walk = it.node;

	for (size_t step = 0; step < path.size(); ++step) {
		if (step > 0) {
			walk = walk->first_child;
		}

		if (walk == 0) {
			throw std::range_error("tree::iterator_from_path: no more nodes at step " + std::to_string(step));
		}

		for (size_t i = 0; i < path[step]; ++i) {
			walk = walk->next_sibling;

			if (walk == 0) {
				throw std::range_error("tree::iterator_from_path: out of siblings at step " + std::to_string(step));
			}
		}
	}

	it.node = walk;
	return it;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::leaf_iterator tree<T, tree_node_allocator>::begin_leaf(const iterator_base& top) const {
	tree_node* tmp = top.node;

	while (tmp->first_child) {
		tmp = tmp->first_child;
	}

	return leaf_iterator(tmp, top.node);
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::leaf_iterator tree<T, tree_node_allocator>::end_leaf(const iterator_base& top) const {
	return leaf_iterator(top.node, top.node);
}

template<class T, class tree_node_allocator>
template<typename iter>
iter tree<T, tree_node_allocator>::parent(iter position) {
	assert(position.node != 0);
	return iter(position.node->parent);
}

template<class T, class tree_node_allocator>
template<typename iter>
iter tree<T, tree_node_allocator>::previous_sibling(iter position) {
	assert(position.node != 0);
	iter ret(position);
	ret.node = position.node->prev_sibling;
	return ret;
}

template<class T, class tree_node_allocator>
template<typename iter>
iter tree<T, tree_node_allocator>::next_sibling(iter position) {
	assert(position.node != 0);
	iter ret(position);
	ret.node = position.node->next_sibling;
	return ret;
}

template<class T, class tree_node_allocator>
template<typename iter>
iter tree<T, tree_node_allocator>::next_at_same_depth(iter position) const {
	// We make use of a temporary fixed_depth iterator to implement this.

	typename tree<T, tree_node_allocator>::fixed_depth_iterator tmp(position.node);

	++tmp;
	return iter(tmp);

//	assert(position.node != 0);
//	iter ret(position);
//
//	if (position.node->next_sibling) {
//		ret.node = position.node->next_sibling;
//	} else {
//		size_t relative_depth = 0;
//
//	upper:
//		do {
//			ret.node = ret.node->parent;
//			if (ret.node == 0) {
//				return ret;
//			}
//			--relative_depth;
//		} while (ret.node->next_sibling == 0);
//
//	lower:
//		ret.node = ret.node->next_sibling;
//		while (ret.node->first_child == 0) {
//			if (ret.node->next_sibling == 0) {
//				goto upper;
//			}
//			ret.node = ret.node->next_sibling;
//			if (ret.node == 0) {
//				return ret;
//			}
//		}
//
//		while (relative_depth < 0 && ret.node->first_child != 0) {
//			ret.node = ret.node->first_child;
//			++relative_depth;
//		}
//
//		if (relative_depth < 0) {
//			goto (ret.node->next_sibling == 0) ? upper : lower;
//		}
//	}
//	return ret;
}

template<class T, class tree_node_allocator>
template<typename iter>
iter tree<T, tree_node_allocator>::append_child(iter position) {
	assert(position.node != head);
	assert(position.node != feet);
	assert(position.node);

	tree_node* tmp = alloc_.allocate(1, 0);
	alloc_.construct(tmp, tree_node_<T>());
//	kp::constructor(&tmp->data);
#ifdef TREE_HH_EXTRA
	tmp->expanded = head->expanded;
	tmp->visible = head->visible;
#endif // TREE_HH_EXTRA
	tmp->first_child = 0;
	tmp->last_child = 0;

	tmp->parent = position.node;

	if (position.node->last_child != 0) {
		position.node->last_child->next_sibling = tmp;
	} else {
		position.node->first_child = tmp;
	}

	tmp->prev_sibling = position.node->last_child;
	position.node->last_child = tmp;
	tmp->next_sibling = 0;

#ifdef TREE_HH_EXTRA
	tree_node* parent = tmp->parent;
	bool parent_expanded = true;

	while (parent) {
		parent_expanded &= parent->expanded;

		if (parent_expanded) {
			++parent->count_expanded;
		}

		++parent->count;
		++parent->count_expandable;
		parent = parent->parent;
	}

	if (parent_expanded) {
		++head->count_expanded;
	}

	++head->count;
	++head->count_expandable;
#endif // TREE_HH_EXTRA

	return tmp;
}

template<class T, class tree_node_allocator>
template<typename iter>
iter tree<T, tree_node_allocator>::prepend_child(iter position) {
	assert(position.node != head);
	assert(position.node != feet);
	assert(position.node);

	tree_node* tmp = alloc_.allocate(1, 0);
	alloc_.construct(tmp, tree_node_<T>());
//	kp::constructor(&tmp->data);
#ifdef TREE_HH_EXTRA
	tmp->expanded = head->expanded;
	tmp->visible = head->visible;
#endif // TREE_HH_EXTRA
	tmp->first_child = 0;
	tmp->last_child = 0;

	tmp->parent = position.node;

	if (position.node->first_child != 0) {
		position.node->first_child->prev_sibling = tmp;
	} else {
		position.node->last_child = tmp;
	}

	tmp->next_sibling = position.node->first_child;
	position.node->prev_child = tmp;
	tmp->prev_sibling = 0;
	return tmp;
}

template<class T, class tree_node_allocator>
template<class iter>
iter tree<T, tree_node_allocator>::append_child(iter position, const T& x) {
	// If your program fails here you probably used 'append_child' to add the top
	// node to an empty tree. From version 1.45 the top element should be added
	// using 'insert'. See the documentation for further information, and sorry about
	// the API change.
	assert(position.node != head);
	assert(position.node != feet);
	assert(position.node);

	tree_node* tmp = alloc_.allocate(1, 0);
	alloc_.construct(tmp, x);
//	kp::constructor(&tmp->data, x);
#ifdef TREE_HH_EXTRA
	tmp->expanded = head->expanded;
	tmp->visible = head->visible;
#endif // TREE_HH_EXTRA
	tmp->first_child = 0;
	tmp->last_child = 0;

	tmp->parent = position.node;

	if (position.node->last_child != 0) {
		position.node->last_child->next_sibling = tmp;
	} else {
		position.node->first_child = tmp;
	}

	tmp->prev_sibling = position.node->last_child;
	position.node->last_child = tmp;
	tmp->next_sibling = 0;

#ifdef TREE_HH_EXTRA
	tree_node* parent = tmp->parent;
	bool parent_expanded = true;

	while (parent) {
		parent_expanded &= parent->expanded;

		if (parent_expanded) {
			++parent->count_expanded;
		}

		++parent->count;
		++parent->count_expandable;
		parent = parent->parent;
	}

	if (parent_expanded) {
		++head->count_expanded;
	}

	++head->count;
	++head->count_expandable;
#endif // TREE_HH_EXTRA

	return tmp;
}

template<class T, class tree_node_allocator>
template<class iter>
iter tree<T, tree_node_allocator>::append_child(iter position, T&& x) {
	assert(position.node != head);
	assert(position.node != feet);
	assert(position.node);

	tree_node* tmp = alloc_.allocate(1, 0);
	alloc_.construct(tmp); // Here is where the move semantics kick in
	std::swap(tmp->data, x);

	tmp->first_child = 0;
	tmp->last_child = 0;

	tmp->parent = position.node;

	if (position.node->last_child != 0) {
		position.node->last_child->next_sibling = tmp;
	} else {
		position.node->first_child = tmp;
	}

	tmp->prev_sibling = position.node->last_child;
	position.node->last_child = tmp;
	tmp->next_sibling = 0;
	return tmp;
}

template<class T, class tree_node_allocator>
template<class iter>
iter tree<T, tree_node_allocator>::prepend_child(iter position, const T& x) {
	assert(position.node != head);
	assert(position.node != feet);
	assert(position.node);

	tree_node* tmp = alloc_.allocate(1, 0);
	alloc_.construct(tmp, x);
//	kp::constructor(&tmp->data, x);
#ifdef TREE_HH_EXTRA
	tmp->expanded = head->expanded;
	tmp->visible = head->visible;
#endif // TREE_HH_EXTRA
	tmp->first_child = 0;
	tmp->last_child = 0;

	tmp->parent = position.node;

	if (position.node->first_child != 0) {
		position.node->first_child->prev_sibling = tmp;
	} else {
		position.node->last_child = tmp;
	}

	tmp->next_sibling = position.node->first_child;
	position.node->first_child = tmp;
	tmp->prev_sibling = 0;
	return tmp;
}

template<class T, class tree_node_allocator>
template<class iter>
iter tree<T, tree_node_allocator>::prepend_child(iter position, T&& x) {
	assert(position.node != head);
	assert(position.node != feet);
	assert(position.node);

	tree_node* tmp = alloc_.allocate(1, 0);
	alloc_.construct(tmp);
	std::swap(tmp->data, x);

	tmp->first_child = 0;
	tmp->last_child = 0;

	tmp->parent = position.node;

	if (position.node->first_child != 0) {
		position.node->first_child->prev_sibling = tmp;
	} else {
		position.node->last_child = tmp;
	}

	tmp->next_sibling = position.node->first_child;
	position.node->first_child = tmp;
	tmp->prev_sibling = 0;
	return tmp;
}

template<class T, class tree_node_allocator>
template<class iter>
iter tree<T, tree_node_allocator>::append_child(iter position, iter other) {
	assert(position.node != head);
	assert(position.node != feet);
	assert(position.node);

	sibling_iterator aargh = append_child(position, value_type());
	return replace(aargh, other);
}

template<class T, class tree_node_allocator>
template<class iter>
iter tree<T, tree_node_allocator>::prepend_child(iter position, iter other) {
	assert(position.node != head);
	assert(position.node != feet);
	assert(position.node);

	sibling_iterator aargh = prepend_child(position, value_type());
	return replace(aargh, other);
}

template<class T, class tree_node_allocator>
template<class iter>
iter tree<T, tree_node_allocator>::append_children(iter position, sibling_iterator from, sibling_iterator to) {
	assert(position.node != head);
	assert(position.node != feet);
	assert(position.node);

	iter ret = from;

	while (from != to) {
		insert_subtree(position.end(), from);
		++from;
	}

	return ret;
}

template<class T, class tree_node_allocator>
template<class iter>
iter tree<T, tree_node_allocator>::prepend_children(iter position, sibling_iterator from, sibling_iterator to) {
	assert(position.node != head);
	assert(position.node != feet);
	assert(position.node);

	if (from == to) {
		return from; // should return end of tree?
	}

	iter ret;

	do {
		--to;
		ret = insert_subtree(position.begin(), to);
	} while (to != from);

	return ret;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::pre_order_iterator tree<T, tree_node_allocator>::set_head(const T& x) {
	assert(head->next_sibling == feet);
	return insert(iterator(feet), x);
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::pre_order_iterator tree<T, tree_node_allocator>::set_head(T&& x) {
	assert(head->next_sibling == feet);
	return insert(iterator(feet), x);
}

template<class T, class tree_node_allocator>
template<class iter>
iter tree<T, tree_node_allocator>::insert(iter position, const T& x) {
	if (position.node == 0) {
		position.node = feet; // Backward compatibility: when calling insert on a null node,
		// insert before the feet.
	}

	assert(position.node != head); // Cannot insert before head.

	tree_node* tmp = alloc_.allocate(1, 0);
	alloc_.construct(tmp, x);
//	kp::constructor(&tmp->data, x);
#ifdef TREE_HH_EXTRA
	tmp->expanded = head->expanded;
	tmp->visible = head->visible;
#endif // TREE_HH_EXTRA
	tmp->first_child = 0;
	tmp->last_child = 0;

	tmp->parent = position.node->parent;
	tmp->next_sibling = position.node;
	tmp->prev_sibling = position.node->prev_sibling;
	position.node->prev_sibling = tmp;

	if (tmp->prev_sibling == 0) {
		if (tmp->parent) { // when inserting nodes at the head, there is no parent
			tmp->parent->first_child = tmp;
		}
	} else {
		tmp->prev_sibling->next_sibling = tmp;
	}

#ifdef TREE_HH_EXTRA
	tree_node* parent = tmp->parent;
	bool parent_expanded = true;

	while (parent) {
		parent_expanded &= parent->expanded;

		if (parent_expanded) {
			++parent->count_expanded;
		}

		++parent->count;
		++parent->count_expandable;
		parent = parent->parent;
	}

	if (parent_expanded) {
		++head->count_expanded;
	}

	++head->count;
	++head->count_expandable;
#endif // TREE_HH_EXTRA

	return tmp;
}

template<class T, class tree_node_allocator>
template<class iter>
iter tree<T, tree_node_allocator>::insert(iter position, T&& x) {
	if (position.node == 0) {
		position.node = feet; // Backward compatibility: when calling insert on a null node,
		// insert before the feet.
	}

	tree_node* tmp = alloc_.allocate(1, 0);
	alloc_.construct(tmp);
	std::swap(tmp->data, x); // Move semantics
	tmp->first_child = 0;
	tmp->last_child = 0;

	tmp->parent = position.node->parent;
	tmp->next_sibling = position.node;
	tmp->prev_sibling = position.node->prev_sibling;
	position.node->prev_sibling = tmp;

	if (tmp->prev_sibling == 0) {
		if (tmp->parent) { // when inserting nodes at the head, there is no parent
			tmp->parent->first_child = tmp;
		}
	} else {
		tmp->prev_sibling->next_sibling = tmp;
	}

	return tmp;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::sibling_iterator tree<T, tree_node_allocator>::insert(sibling_iterator position, const T& x) {
	tree_node* tmp = alloc_.allocate(1, 0);
	alloc_.construct(tmp, x);
//	kp::constructor(&tmp->data, x);
#ifdef TREE_HH_EXTRA
	tmp->expanded = head->expanded;
	tmp->visible = head->visible;
#endif // TREE_HH_EXTRA
	tmp->first_child = 0;
	tmp->last_child = 0;

	tmp->next_sibling = position.node;

	if (position.node == 0) { // iterator points to end of a subtree
		tmp->parent = position.parent_;
		tmp->prev_sibling = position.range_last();
		tmp->parent->last_child = tmp;
	} else {
		tmp->parent = position.node->parent;
		tmp->prev_sibling = position.node->prev_sibling;
		position.node->prev_sibling = tmp;
	}

	if (tmp->prev_sibling == 0) {
		if (tmp->parent) { // when inserting nodes at the head, there is no parent
			tmp->parent->first_child = tmp;
		}
	} else {
		tmp->prev_sibling->next_sibling = tmp;
	}

#ifdef TREE_HH_EXTRA
	tree_node* parent = tmp->parent;
	bool parent_expanded = true;

	while (parent) {
		parent_expanded &= parent->expanded;

		if (parent_expanded) {
			++parent->count_expanded;
		}

		++parent->count;
		++parent->count_expandable;
		parent = parent->parent;
	}

	if (parent_expanded) {
		++head->count_expanded;
	}

	++head->count;
	++head->count_expandable;
#endif // TREE_HH_EXTRA

	return tmp;
}

template<class T, class tree_node_allocator>
template<class iter>
iter tree<T, tree_node_allocator>::insert_after(iter position, const T& x) {
	tree_node* tmp = alloc_.allocate(1, 0);
	alloc_.construct(tmp, x);
//	kp::constructor(&tmp->data, x);
#ifdef TREE_HH_EXTRA
	tmp->expanded = head->expanded;
	tmp->visible = head->visible;
#endif // TREE_HH_EXTRA
	tmp->first_child = 0;
	tmp->last_child = 0;

	tmp->parent = position.node->parent;
	tmp->prev_sibling = position.node;
	tmp->next_sibling = position.node->next_sibling;
	position.node->next_sibling = tmp;

	if (tmp->next_sibling == 0) {
		if (tmp->parent) { // when inserting nodes at the head, there is no parent
			tmp->parent->last_child = tmp;
		}
	} else {
		tmp->next_sibling->prev_sibling = tmp;
	}

	return tmp;
}

template<class T, class tree_node_allocator>
template<class iter>
iter tree<T, tree_node_allocator>::insert_after(iter position, T&& x) {
	tree_node* tmp = alloc_.allocate(1, 0);
	alloc_.construct(tmp);
	std::swap(tmp->data, x); // move semantics
//	kp::constructor(&tmp->data, x);
	tmp->first_child = 0;
	tmp->last_child = 0;

	tmp->parent = position.node->parent;
	tmp->prev_sibling = position.node;
	tmp->next_sibling = position.node->next_sibling;
	position.node->next_sibling = tmp;

	if (tmp->next_sibling == 0) {
		if (tmp->parent) { // when inserting nodes at the head, there is no parent
			tmp->parent->last_child = tmp;
		}
	} else {
		tmp->next_sibling->prev_sibling = tmp;
	}

	return tmp;
}

template<class T, class tree_node_allocator>
template<class iter>
iter tree<T, tree_node_allocator>::insert_subtree(iter position, const iterator_base& subtree) {
	// insert dummy
	iter it = insert(position, value_type());
	// replace dummy with subtree
	return replace(it, subtree);
}

template<class T, class tree_node_allocator>
template<class iter>
iter tree<T, tree_node_allocator>::insert_subtree_after(iter position, const iterator_base& subtree) {
	// insert dummy
	iter it = insert_after(position, value_type());
	// replace dummy with subtree
	return replace(it, subtree);
}

//template<class T, class tree_node_allocator>
//template<class iter>
//iter tree<T, tree_node_allocator>::insert_subtree(sibling_iterator position, iter subtree) {
//	// insert dummy
//	iter it(insert(position, value_type()));
//	// replace dummy with subtree
//	return replace(it, subtree);
//}

template<class T, class tree_node_allocator>
template<class iter>
iter tree<T, tree_node_allocator>::replace(iter position, const T& x) {
//	kp::destructor(&position.node->data);
//	kp::constructor(&position.node->data, x);
	position.node->data = x;
//	alloc_.destroy(position.node);
//	alloc_.construct(position.node, x);
	return position;
}

template<class T, class tree_node_allocator>
template<class iter>
iter tree<T, tree_node_allocator>::replace(iter position, const iterator_base& from) {
	assert(position.node != head);
	tree_node* current_from = from.node;
	tree_node* start_from = from.node;
	tree_node* current_to = position.node;

	// replace the node at position with head of the replacement tree at from
//	std::cout << "warning!" << position.node << std::endl;
#ifdef TREE_HH_EXTRA
	erase_children_(position);
#else // TREE_HH_EXTRA
	erase_children(position);
#endif // TREE_HH_EXTRA
//	std::cout << "no warning!" << std::endl;
	tree_node* tmp = alloc_.allocate(1, 0);
	alloc_.construct(tmp, (*from));
//	kp::constructor(&tmp->data, (*from));
#ifdef TREE_HH_EXTRA
	tmp->expanded = head->expanded;
	tmp->visible = head->visible;
#endif // TREE_HH_EXTRA
	tmp->first_child = 0;
	tmp->last_child = 0;

	if (current_to->prev_sibling == 0) {
		if (current_to->parent != 0) {
			current_to->parent->first_child = tmp;
		}
	} else {
		current_to->prev_sibling->next_sibling = tmp;
	}

	tmp->prev_sibling = current_to->prev_sibling;

	if (current_to->next_sibling == 0) {
		if (current_to->parent != 0) {
			current_to->parent->last_child = tmp;
		}
	} else {
		current_to->next_sibling->prev_sibling = tmp;
	}

	tmp->next_sibling = current_to->next_sibling;
	tmp->parent = current_to->parent;
//	kp::destructor(&current_to->data);
	alloc_.destroy(current_to);
	alloc_.deallocate(current_to, 1);
	current_to = tmp;

	// only at this stage can we fix 'last'
	tree_node* last = from.node->next_sibling;

	pre_order_iterator toit = tmp;

	// copy all children
	do {
		assert(current_from != 0);

		if (current_from->first_child != 0) {
			current_from = current_from->first_child;
			toit = append_child(toit, current_from->data);
		} else {
			while (current_from->next_sibling == 0 && current_from != start_from) {
				current_from = current_from->parent;
				toit = parent(toit);
				assert(current_from != 0);
			}

			current_from = current_from->next_sibling;

			if (current_from != last) {
				toit = append_child(parent(toit), current_from->data);
			}
		}
	} while (current_from != last);

	return current_to;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::sibling_iterator tree<T, tree_node_allocator>::replace(
	sibling_iterator orig_begin,
	sibling_iterator orig_end,
	sibling_iterator new_begin,
	sibling_iterator new_end) {
	tree_node* orig_first = orig_begin.node;
	tree_node* new_first = new_begin.node;
	tree_node* orig_last = orig_first;

	while ((++orig_begin) != orig_end) {
		orig_last = orig_last->next_sibling;
	}

	tree_node* new_last = new_first;

	while ((++new_begin) != new_end) {
		new_last = new_last->next_sibling;
	}

	// insert all siblings in new_first..new_last before orig_first
	bool first = true;
	pre_order_iterator ret;

	while (1 == 1) {
		pre_order_iterator tt = insert_subtree(pre_order_iterator(orig_first), pre_order_iterator(new_first));

		if (first) {
			ret = tt;
			first = false;
		}

		if (new_first == new_last) {
			break;
		}

		new_first = new_first->next_sibling;
	}

	// erase old range of siblings
	bool last = false;
	tree_node* next = orig_first;

	while (1 == 1) {
		if (next == orig_last) {
			last = true;
		}

		next = next->next_sibling;
		erase((pre_order_iterator)orig_first);

		if (last) {
			break;
		}

		orig_first = next;
	}

	return ret;
}

template<class T, class tree_node_allocator>
template<typename iter>
iter tree<T, tree_node_allocator>::flatten(iter position) {
	if (position.node->first_child == 0) {
		return position;
	}

	tree_node* tmp = position.node->first_child;

	while (tmp) {
		tmp->parent = position.node->parent;
		tmp = tmp->next_sibling;
	}

	if (position.node->next_sibling) {
		position.node->last_child->next_sibling = position.node->next_sibling;
		position.node->next_sibling->prev_sibling = position.node->last_child;
	} else {
		position.node->parent->last_child = position.node->last_child;
	}

	position.node->next_sibling = position.node->first_child;
	position.node->next_sibling->prev_sibling = position.node;
	position.node->first_child = 0;
	position.node->last_child = 0;

	return position;
}

template<class T, class tree_node_allocator>
template<typename iter>
iter tree<T, tree_node_allocator>::reparent(iter position, sibling_iterator begin, sibling_iterator end) {
	tree_node* first = begin.node;
	tree_node* last = first;

	assert(first != position.node);

	if (begin == end) {
		return begin;
	}

	// determine last node
	while ((++begin) != end) {
		last = last->next_sibling;
	}

	// move subtree
	if (first->prev_sibling == 0) {
		first->parent->first_child = last->next_sibling;
	} else {
		first->prev_sibling->next_sibling = last->next_sibling;
	}

	if (last->next_sibling == 0) {
		last->parent->last_child = first->prev_sibling;
	} else {
		last->next_sibling->prev_sibling = first->prev_sibling;
	}

	if (position.node->first_child == 0) {
		position.node->first_child = first;
		position.node->last_child = last;
		first->prev_sibling = 0;
	} else {
		position.node->last_child->next_sibling = first;
		first->prev_sibling = position.node->last_child;
		position.node->last_child = last;
	}

	last->next_sibling = 0;

	tree_node* pos = first;

	for (;;) {
		pos->parent = position.node;

		if (pos == last) {
			break;
		}

		pos = pos->next_sibling;
	}

	return first;
}

template<class T, class tree_node_allocator>
template<typename iter> iter tree<T, tree_node_allocator>::reparent(iter position, iter from) {
	if (from.node->first_child == 0) {
		return position;
	}

	return reparent(position, from.node->first_child, end(from));
}

template<class T, class tree_node_allocator>
template<typename iter> iter tree<T, tree_node_allocator>::wrap(iter position, const T& x) {
	assert(position.node != 0);
	sibling_iterator fr = position, to = position;
	++to;
	iter ret = insert(position, x);
	reparent(ret, fr, to);
	return ret;
}

template<class T, class tree_node_allocator>
template<typename iter> iter tree<T, tree_node_allocator>::wrap(iter from, iter to, const T& x) {
	assert(from.node != 0);
	iter ret = insert(from, x);
	reparent(ret, from, to);
	return ret;
}

template<class T, class tree_node_allocator>
template<typename iter> iter tree<T, tree_node_allocator>::move_after(iter target, iter source) {
	tree_node* dst = target.node;
	tree_node* src = source.node;
	assert(dst);
	assert(src);

	if (dst == src) {
		return source;
	}

	if (dst->next_sibling) {
		if (dst->next_sibling == src) { // already in the right spot
			return source;
		}
	}

	// take src out of the tree
	if (src->prev_sibling != 0) {
		src->prev_sibling->next_sibling = src->next_sibling;
	} else {
		src->parent->first_child = src->next_sibling;
	}

	if (src->next_sibling != 0) {
		src->next_sibling->prev_sibling = src->prev_sibling;
	} else {
		src->parent->last_child = src->prev_sibling;
	}

	// connect it to the new point
	if (dst->next_sibling != 0) {
		dst->next_sibling->prev_sibling = src;
	} else {
		dst->parent->last_child = src;
	}

	src->next_sibling = dst->next_sibling;
	dst->next_sibling = src;
	src->prev_sibling = dst;
	src->parent = dst->parent;
	return src;
}

template<class T, class tree_node_allocator>
template<typename iter> iter tree<T, tree_node_allocator>::move_before(iter target, iter source) {
	tree_node* dst = target.node;
	tree_node* src = source.node;
	assert(dst);
	assert(src);

	if (dst == src) {
		return source;
	}

	if (dst->prev_sibling) {
		if (dst->prev_sibling == src) { // already in the right spot
			return source;
		}
	}

	// take src out of the tree
	if (src->prev_sibling != 0) {
		src->prev_sibling->next_sibling = src->next_sibling;
	} else {
		src->parent->first_child = src->next_sibling;
	}

	if (src->next_sibling != 0) {
		src->next_sibling->prev_sibling = src->prev_sibling;
	} else {
		src->parent->last_child = src->prev_sibling;
	}

	// connect it to the new point
	if (dst->prev_sibling != 0) {
		dst->prev_sibling->next_sibling = src;
	} else {
		dst->parent->first_child = src;
	}

	src->prev_sibling = dst->prev_sibling;
	dst->prev_sibling = src;
	src->next_sibling = dst;
	src->parent = dst->parent;
	return src;
}

// specialisation for sibling_iterators
template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::sibling_iterator tree<T, tree_node_allocator>::move_before(sibling_iterator target,
	sibling_iterator source) {
	tree_node* dst = target.node;
	tree_node* src = source.node;
	tree_node* dst_prev_sibling;

	if (dst == 0) { // must then be an end iterator
		dst_prev_sibling = target.parent_->last_child;
		assert(dst_prev_sibling);
	} else {
		dst_prev_sibling = dst->prev_sibling;
	}

	assert(src);

	if (dst == src) {
		return source;
	}

	if (dst_prev_sibling) {
		if (dst_prev_sibling == src) { // already in the right spot
			return source;
		}
	}

	// take src out of the tree
	if (src->prev_sibling != 0) {
		src->prev_sibling->next_sibling = src->next_sibling;
	} else {
		src->parent->first_child = src->next_sibling;
	}

	if (src->next_sibling != 0) {
		src->next_sibling->prev_sibling = src->prev_sibling;
	} else {
		src->parent->last_child = src->prev_sibling;
	}

	// connect it to the new point
	if (dst_prev_sibling != 0) {
		dst_prev_sibling->next_sibling = src;
	} else {
		target.parent_->first_child = src;
	}

	src->prev_sibling = dst_prev_sibling;

	if (dst) {
		dst->prev_sibling = src;
		src->parent = dst->parent;
	}

	src->next_sibling = dst;
	return src;
}

template<class T, class tree_node_allocator>
template<typename iter> iter tree<T, tree_node_allocator>::move_ontop(iter target, iter source) {
	tree_node* dst = target.node;
	tree_node* src = source.node;
	assert(dst);
	assert(src);

	if (dst == src) {
		return source;
	}

//	if(dst==src->prev_sibling) {
//
//	}

	// remember connection points
	tree_node* b_prev_sibling = dst->prev_sibling;
	tree_node* b_next_sibling = dst->next_sibling;
	tree_node* b_parent = dst->parent;

	// remove target
	erase(target);

	// take src out of the tree
	if (src->prev_sibling != 0) {
		src->prev_sibling->next_sibling = src->next_sibling;
	} else {
		assert(src->parent != 0);
		src->parent->first_child = src->next_sibling;
	}

	if (src->next_sibling != 0) {
		src->next_sibling->prev_sibling = src->prev_sibling;
	} else {
		assert(src->parent != 0);
		src->parent->last_child = src->prev_sibling;
	}

	// connect it to the new point
	if (b_prev_sibling != 0) {
		b_prev_sibling->next_sibling = src;
	} else {
		assert(b_parent != 0);
		b_parent->first_child = src;
	}

	if (b_next_sibling != 0) {
		b_next_sibling->prev_sibling = src;
	} else {
		assert(b_parent != 0);
		b_parent->last_child = src;
	}

	src->prev_sibling = b_prev_sibling;
	src->next_sibling = b_next_sibling;
	src->parent = b_parent;
	return src;
}

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator> tree<T, tree_node_allocator>::move_out(iterator source) {
	tree ret;

	// Move source node into the 'ret' tree.
	ret.head->next_sibling = source.node;
	ret.feet->prev_sibling = source.node;
	source.node->parent = 0;

	// Close the links in the current tree.
	if (source.node->prev_sibling != 0) {
		source.node->prev_sibling->next_sibling = source.node->next_sibling;
	}

	if (source.node->next_sibling != 0) {
		source.node->next_sibling->prev_sibling = source.node->prev_sibling;
	}

	// Fix source prev/next links.
	source.node->prev_sibling = ret.head;
	source.node->next_sibling = ret.feet;

	return ret; // A good compiler will move this, not copy.
}

template<class T, class tree_node_allocator>
template<typename iter> iter tree<T, tree_node_allocator>::move_in(iter loc, tree& other) {
	if (other.head->next_sibling == other.feet) {
		return loc; // other tree is empty
	}

	tree_node* other_first_head = other.head->next_sibling;
	tree_node* other_last_head = other.feet->prev_sibling;

	sibling_iterator prev(loc);
	--prev;

	prev.node->next_sibling = other_first_head;
	loc.node->prev_sibling = other_last_head;
	other_first_head->prev_sibling = prev.node;
	other_last_head->next_sibling = loc.node;

	// Adjust parent pointers.
	tree_node* walk = other_first_head;

	while (true) {
		walk->parent = loc.node->parent;

		if (walk == other_last_head) {
			break;
		}

		walk = walk->next_sibling;
	}

	// Close other tree.
	other.head->next_sibling = other.feet;
	other.feet->prev_sibling = other.head;

	return other_first_head;
}

template<class T, class tree_node_allocator>
template<typename iter> iter tree<T, tree_node_allocator>::move_in_below(iter loc, tree& other) {
	if (other.head->next_sibling == other.feet) {
		return loc; // other tree is empty
	}

	auto n = other.number_of_children(loc);
	return move_in_as_nth_child(loc, n, other);
}

template<class T, class tree_node_allocator>
template<typename iter> iter tree<T, tree_node_allocator>::move_in_as_nth_child(iter loc, size_t n, tree& other) {
	if (other.head->next_sibling == other.feet) {
		return loc; // other tree is empty
	}

	tree_node* other_first_head = other.head->next_sibling;
	tree_node* other_last_head = other.feet->prev_sibling;

	if (n == 0) {
		if (loc.node->first_child == 0) {
			loc.node->first_child = other_first_head;
			loc.node->last_child = other_last_head;
			other_last_head->next_sibling = 0;
			other_first_head->prev_sibling = 0;
		} else {
			loc.node->first_child->prev_sibling = other_last_head;
			other_last_head->next_sibling = loc.node->first_child;
			loc.node->first_child = other_first_head;
			other_first_head->prev_sibling = 0;
		}
	} else {
		--n;
		tree_node* walk = loc.node->first_child;

		while (true) {
			if (walk == 0) {
				throw std::range_error("tree: move_in_as_nth_child position out of range");
			}

			if (n == 0) {
				break;
			}

			--n;
			walk = walk->next_sibling;
		}

		if (walk->next_sibling == 0) {
			loc.node->last_child = other_last_head;
		} else {
			walk->next_sibling->prev_sibling = other_last_head;
		}

		other_last_head->next_sibling = walk->next_sibling;
		walk->next_sibling = other_first_head;
		other_first_head->prev_sibling = walk;
	}

	// Adjust parent pointers.
	tree_node* walk = other_first_head;

	while (true) {
		walk->parent = loc.node;

		if (walk == other_last_head) {
			break;
		}

		walk = walk->next_sibling;
	}

	// Close other tree.
	other.head->next_sibling = other.feet;
	other.feet->prev_sibling = other.head;

	return other_first_head;
}

template<class T, class tree_node_allocator>
void tree<T, tree_node_allocator>::merge(sibling_iterator to1, sibling_iterator to2,
	sibling_iterator from1, sibling_iterator from2,
	bool duplicate_leaves) {
	sibling_iterator fnd;

	while (from1 != from2) {
		if ((fnd = std::find(to1, to2, (*from1))) != to2) { // element found
			if (from1.begin() == from1.end()) { // full depth reached
				if (duplicate_leaves) {
					append_child(parent(to1), (*from1));
				}
			} else { // descend further
				merge(fnd.begin(), fnd.end(), from1.begin(), from1.end(), duplicate_leaves);
			}
		} else { // element missing
			insert_subtree(to2, from1);
		}

		++from1;
	}
}

template<class T, class tree_node_allocator>
void tree<T, tree_node_allocator>::merge(iterator to, iterator from, bool duplicate_leaves) {
	sibling_iterator to1(to);
	sibling_iterator to2 = to1;
	++to2;
	sibling_iterator from1(from);
	sibling_iterator from2 = from1;
	++from2;

	merge(to1, to2, from1, from2, duplicate_leaves);
}

template<class T, class tree_node_allocator>
void tree<T, tree_node_allocator>::sort(sibling_iterator from, sibling_iterator to, bool deep) {
	std::less<T> comp;
	sort(from, to, comp, deep);
}

template<class T, class tree_node_allocator>
template<class StrictWeakOrdering>
void tree<T, tree_node_allocator>::sort(sibling_iterator from, sibling_iterator to,
	StrictWeakOrdering comp, bool deep) {
	if (from == to) {
		return;
	}

	// make list of sorted nodes
	// CHECK: if multiset stores equivalent nodes in the order in which they
	// are inserted, then this routine should be called 'stable_sort'.
	std::multiset<tree_node*, compare_nodes<StrictWeakOrdering>> nodes(comp);
	sibling_iterator it = from, it2 = to;

	while (it != to) {
		nodes.insert(it.node);
		++it;
	}

	// reassemble
	--it2;

	// prev and next are the nodes before and after the sorted range
	tree_node* prev = from.node->prev_sibling;
	tree_node* next = it2.node->next_sibling;
	typename std::multiset<tree_node*, compare_nodes<StrictWeakOrdering>>::iterator nit = nodes.begin(), eit = nodes.end();

	if (prev == 0) {
		if ((*nit)->parent != 0) { // to catch "sorting the head" situations, when there is no parent
			(*nit)->parent->first_child = (*nit);
		}
	} else {
		prev->next_sibling = (*nit);
	}

	--eit;

	while (nit != eit) {
		(*nit)->prev_sibling = prev;

		if (prev) {
			prev->next_sibling = (*nit);
		}

		prev = (*nit);
		++nit;
	}

	// prev now points to the last-but-one node in the sorted range
	if (prev) {
		prev->next_sibling = (*eit);
	}

	// eit points to the last node in the sorted range.
	(*eit)->next_sibling = next;
	(*eit)->prev_sibling = prev; // missed in the loop above

	if (next == 0) {
		if ((*eit)->parent != 0) { // to catch "sorting the head" situations, when there is no parent
			(*eit)->parent->last_child = (*eit);
		}
	} else {
		next->prev_sibling = (*eit);
	}

	if (deep) { // sort the children of each node too
		sibling_iterator bcs(*nodes.begin());
		sibling_iterator ecs(*eit);
		++ecs;

		while (bcs != ecs) {
			sort(begin(bcs), end(bcs), comp, deep);
			++bcs;
		}
	}
}

#ifdef TREE_HH_EXTRA
template<class T, class tree_node_allocator>
size_t tree<T, tree_node_allocator>::filter(bool value) {
	auto comp = [&](const T&/*node*/) {
		return value;
	};
	return filter(comp);
}

template<class T, class tree_node_allocator>
template<class StrictWeakFiltering>
size_t tree<T, tree_node_allocator>::filter(StrictWeakFiltering comp) {
	head->count_expandable = 0;
	head->count_expanded = 0;
	// iterate over all the nodes and reset expanded size and expanded flag.
	iterator it = begin();

	while (it != end()) {
		if (it.node->first_child) {
			it.node->expanded = true;
			it.node->count_expandable = 0;
			it.node->count_expanded = 0;
		}

		bool visible = comp(*it);
//		bool prev_visible = it.node->visible;
		it.node->visible = visible;
		if (visible) {
			++head->count_expandable;
			++head->count_expanded;

			auto parent = it.node->parent;
			while (parent) {
				if (!parent->visible) {
					parent->visible = true;
					++head->count_expandable;
					++head->count_expanded;
					auto parent1 = parent->parent;
					while (parent1) {
						++parent1->count_expandable;
						++parent1->count_expanded;
						parent1 = parent1->parent;
					}
				}
				++parent->count_expandable;
				++parent->count_expanded;
				parent = parent->parent;
			}
		}


		++it;
	}

	return head->count_expanded;
}
#endif // TREE_HH_EXTRA

template<class T, class tree_node_allocator>
template<typename iter>
bool tree<T, tree_node_allocator>::equal(const iter& one_, const iter& two, const iter& three_) const {
	std::equal_to<T> comp;
	return equal(one_, two, three_, comp);
}

template<class T, class tree_node_allocator>
template<typename iter>
bool tree<T, tree_node_allocator>::equal_subtree(const iter& one_, const iter& two_) const {
	std::equal_to<T> comp;
	return equal_subtree(one_, two_, comp);
}

template<class T, class tree_node_allocator>
template<typename iter, class BinaryPredicate>
bool tree<T, tree_node_allocator>::equal(const iter& one_, const iter& two, const iter& three_, BinaryPredicate fun) const {
	pre_order_iterator one(one_), three(three_);

//	if(one==two && is_valid(three) && three.number_of_children()!=0) {
//		return false;
//	}

	while (one != two && is_valid(three)) {
		if (!fun(*one, *three)) {
			return false;
		}

		if (one.number_of_children() != three.number_of_children()) {
			return false;
		}

		++one;
		++three;
	}

	return true;
}

template<class T, class tree_node_allocator>
template<typename iter, class BinaryPredicate>
bool tree<T, tree_node_allocator>::equal_subtree(const iter& one_, const iter& two_, BinaryPredicate fun) const {
	pre_order_iterator one(one_), two(two_);

	if (!fun(*one, *two)) {
		return false;
	}

	if (number_of_children(one) != number_of_children(two)) {
		return false;
	}

	return equal(begin(one), end(one), begin(two), fun);
}

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator> tree<T, tree_node_allocator>::subtree(sibling_iterator from, sibling_iterator to) const {
	assert(from != to); // if from==to, the range is empty, hence no tree to return.

	tree tmp;
	tmp.set_head(value_type());
	tmp.replace(tmp.begin(), tmp.end(), from, to);
	return tmp;
}

template<class T, class tree_node_allocator>
void tree<T, tree_node_allocator>::subtree(tree& tmp, sibling_iterator from, sibling_iterator to) const {
	assert(from != to); // if from==to, the range is empty, hence no tree to return.

	tmp.set_head(value_type());
	tmp.replace(tmp.begin(), tmp.end(), from, to);
}

template<class T, class tree_node_allocator>
size_t tree<T, tree_node_allocator>::size() const {
#ifdef TREE_HH_EXTRA
	return head->count;
#else // TREE_HH_EXTRA
	size_t i = 0;
	pre_order_iterator it = begin(), eit = end();

	while (it != eit) {
		++i;
		++it;
	}

	return i;
#endif // TREE_HH_EXTRA
}

template<class T, class tree_node_allocator>
size_t tree<T, tree_node_allocator>::size(const iterator_base& top) const {
#ifdef TREE_HH_EXTRA
	return top.node->count;
#else // TREE_HH_EXTRA
	size_t i = 0;
	pre_order_iterator it = top, eit = top;
	eit.skip_children();
	++eit;

	while (it != eit) {
		++i;
		++it;
	}

	return i;
#endif // TREE_HH_EXTRA
}

#ifdef TREE_HH_EXTRA
template<class T, class tree_node_allocator>
size_t tree<T, tree_node_allocator>::count() const {
	return head->count;
}

template<class T, class tree_node_allocator>
size_t tree<T, tree_node_allocator>::count(const iterator_base& top) const {
	return top.node->count;
}

template<class T, class tree_node_allocator>
size_t tree<T, tree_node_allocator>::count_expanded() const {
	return head->count_expanded;
}

template<class T, class tree_node_allocator>
size_t tree<T, tree_node_allocator>::count_expanded(const iterator_base& top) const {
	return top.node->count_expanded;
}

template<class T, class tree_node_allocator>
size_t tree<T, tree_node_allocator>::count_expandable() const {
	return head->count_expandable;
}

template<class T, class tree_node_allocator>
size_t tree<T, tree_node_allocator>::count_expandable(const iterator_base& top) const {
	return top.node ? top.node->count_expandable : 0;
}
#endif // TREE_HH_EXTRA

template<class T, class tree_node_allocator>
bool tree<T, tree_node_allocator>::empty() const {
	pre_order_iterator it = begin(), eit = end();
	return (it == eit);
}

template<class T, class tree_node_allocator>
size_t tree<T, tree_node_allocator>::depth(const iterator_base& it) {
	tree_node* pos = it.node;
	assert(pos != 0);
	size_t ret = 0;

	while (pos->parent != 0) {
		pos = pos->parent;
		++ret;
	}

	return ret;
}

template<class T, class tree_node_allocator>
size_t tree<T, tree_node_allocator>::depth(const iterator_base& it, const iterator_base& root) {
	tree_node* pos = it.node;
	assert(pos != 0);
	size_t ret = 0;

	while (pos->parent != 0 && pos != root.node) {
		pos = pos->parent;
		++ret;
	}

	return ret;
}

template<class T, class tree_node_allocator>
size_t tree<T, tree_node_allocator>::max_depth() const {
	size_t maxd = -1;

	for (tree_node* it = head->next_sibling; it != feet; it = it->next_sibling) {
		maxd = std::max(maxd, max_depth(it));
	}

	return maxd;
}

template<class T, class tree_node_allocator>
size_t tree<T, tree_node_allocator>::max_depth(const iterator_base& pos) const {
	tree_node* tmp = pos.node;

	if (tmp == 0 || tmp == head || tmp == feet) {
		return -1;
	}

	size_t curdepth = 0, maxdepth = 0;

	while (true) { // try to walk the bottom of the tree
		while (tmp->first_child == 0) {
			if (tmp == pos.node) {
				return maxdepth;
			}

			if (tmp->next_sibling == 0) {
				// try to walk up and then right again
				do {
					tmp = tmp->parent;

					if (tmp == 0) {
						return maxdepth;
					}

					--curdepth;
				} while (tmp->next_sibling == 0);
			}

			if (tmp == pos.node) {
				return maxdepth;
			}

			tmp = tmp->next_sibling;
		}

		tmp = tmp->first_child;
		++curdepth;
		maxdepth = std::max(curdepth, maxdepth);
	}
}

template<class T, class tree_node_allocator>
size_t tree<T, tree_node_allocator>::number_of_children(const iterator_base& it) {
	tree_node* pos = it.node->first_child;

	if (pos == 0) {
		return 0;
	}

	size_t ret = 1;

//	while(pos!=it.node->last_child) {
//		++ret;
//		pos=pos->next_sibling;
//	}

	while ((pos = pos->next_sibling)) {
		++ret;
	}

	return ret;
}

template<class T, class tree_node_allocator>
size_t tree<T, tree_node_allocator>::number_of_siblings(const iterator_base& it) const {
	tree_node* pos = it.node;
	size_t ret = 0;

	// count forward
	while (pos->next_sibling && pos->next_sibling != head && pos->next_sibling != feet) {
		++ret;
		pos = pos->next_sibling;
	}

	// count backward
	pos = it.node;

	while (pos->prev_sibling && pos->prev_sibling != head && pos->prev_sibling != feet) {
		++ret;
		pos = pos->prev_sibling;
	}

	return ret;
}

#ifdef TREE_HH_EXTRA
template<class T, class tree_node_allocator>
void tree<T, tree_node_allocator>::swap(tree<T, tree_node_allocator>& _Other) {
	if (this != &_Other) {
		std::swap(head, _Other.head);
		std::swap(feet, _Other.feet);
		std::swap(selection, _Other.selection);
		std::swap(multiple_selection, _Other.multiple_selection);
	}
}
#endif // TREE_HH_EXTRA

template<class T, class tree_node_allocator>
void tree<T, tree_node_allocator>::swap(sibling_iterator it) {
	tree_node* nxt = it.node->next_sibling;

	if (nxt) {
		if (it.node->prev_sibling) {
			it.node->prev_sibling->next_sibling = nxt;
		} else {
			it.node->parent->first_child = nxt;
		}

		nxt->prev_sibling = it.node->prev_sibling;
		tree_node* nxtnxt = nxt->next_sibling;

		if (nxtnxt) {
			nxtnxt->prev_sibling = it.node;
		} else {
			it.node->parent->last_child = it.node;
		}

		nxt->next_sibling = it.node;
		it.node->prev_sibling = nxt;
		it.node->next_sibling = nxtnxt;
	}
}

template<class T, class tree_node_allocator>
void tree<T, tree_node_allocator>::swap(iterator one, iterator two) {
	// if one and two are adjacent siblings, use the sibling swap
	if (one.node->next_sibling == two.node) {
		swap(one);
	} else if (two.node->next_sibling == one.node) {
		swap(two);
	} else {
		tree_node* nxt1 = one.node->next_sibling;
		tree_node* nxt2 = two.node->next_sibling;
		tree_node* pre1 = one.node->prev_sibling;
		tree_node* pre2 = two.node->prev_sibling;
		tree_node* par1 = one.node->parent;
		tree_node* par2 = two.node->parent;

		// reconnect
		one.node->parent = par2;
		one.node->next_sibling = nxt2;

		if (nxt2) {
			nxt2->prev_sibling = one.node;
		} else {
			par2->last_child = one.node;
		}

		one.node->prev_sibling = pre2;

		if (pre2) {
			pre2->next_sibling = one.node;
		} else {
			par2->first_child = one.node;
		}

		two.node->parent = par1;
		two.node->next_sibling = nxt1;

		if (nxt1) {
			nxt1->prev_sibling = two.node;
		} else {
			par1->last_child = two.node;
		}

		two.node->prev_sibling = pre1;

		if (pre1) {
			pre1->next_sibling = two.node;
		} else {
			par1->first_child = two.node;
		}
	}
}

//template<class BinaryPredicate>
//tree<T, tree_node_allocator>::iterator tree<T, tree_node_allocator>::find_subtree(
//	sibling_iterator subfrom, sibling_iterator subto, iterator from, iterator to,
//	BinaryPredicate fun) const {
//	assert(1 == 0); // this routine is not finished yet.
//	while (from != to) {
//		if (fun(*subfrom, *from)) {
//
//		}
//	}
//	return to;
//}

template<class T, class tree_node_allocator>
bool tree<T, tree_node_allocator>::is_in_subtree(const iterator_base& it, const iterator_base& top) const {
	sibling_iterator first = top;
	sibling_iterator last = first;
	++last;
	return is_in_subtree(it, first, last);
}

template<class T, class tree_node_allocator>
bool tree<T, tree_node_allocator>::is_in_subtree(const iterator_base& it, const iterator_base& begin,
	const iterator_base& end) const {
	// FIXME: this should be optimised.
	pre_order_iterator tmp = begin;

	while (tmp != end) {
		if (tmp == it) {
			return true;
		}

		++tmp;
	}

	return false;
}

template<class T, class tree_node_allocator>
bool tree<T, tree_node_allocator>::is_valid(const iterator_base& it) const {
	return !(it.node == 0 || it.node == feet || it.node == head);
}

template<class T, class tree_node_allocator>
bool tree<T, tree_node_allocator>::is_head(const iterator_base& it) {
	return (it.node->parent == 0);
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::iterator tree<T, tree_node_allocator>::lowest_common_ancestor(
	const iterator_base& one, const iterator_base& two) const {
	std::set<iterator, iterator_base_less> parents;

	// Walk up from 'one' storing all parents.
	iterator walk = one;

	do {
		walk = parent(walk);
		parents.insert(walk);
	} while (is_valid(parent(walk)));

	// Walk up from 'two' until we encounter a node in parents.
	walk = two;

	do {
		walk = parent(walk);

		if (parents.find(walk) != parents.end()) {
			break;
		}
	} while (is_valid(parent(walk)));

	return walk;
}

template<class T, class tree_node_allocator>
size_t tree<T, tree_node_allocator>::index(sibling_iterator it) const {
	size_t ind = 0;

	if (it.node->parent == 0) {
		while (it.node->prev_sibling != head) {
			it.node = it.node->prev_sibling;
			++ind;
		}
	} else {
		while (it.node->prev_sibling != 0) {
			it.node = it.node->prev_sibling;
			++ind;
		}
	}

	return ind;
}

#ifdef TREE_HH_EXTRA
template<class T, class tree_node_allocator>
size_t tree<T, tree_node_allocator>::index(pre_order_iterator it) const {
	size_t ind = 0;

	while (--it != head) {
		++ind;
	}

	return ind;
}

template<class T, class tree_node_allocator>
size_t tree<T, tree_node_allocator>::index(expanded_iterator it) const {
	size_t ind = 0;

	while (--it != head) {
		++ind;
	}

	return ind;
}
#endif // TREE_HH_EXTRA

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::sibling_iterator tree<T, tree_node_allocator>::sibling(const iterator_base& it, size_t num) const {
	tree_node* tmp;

	if (it.node->parent == 0) {
		tmp = head->next_sibling;

		while (num--) {
			tmp = tmp->next_sibling;
		}
	} else {
		tmp = it.node->parent->first_child;

		while (num--) {
			assert(tmp != 0);
			tmp = tmp->next_sibling;
		}
	}

	return tmp;
}

template<class T, class tree_node_allocator>
void tree<T, tree_node_allocator>::debug_verify_consistency() const {
	iterator it = begin();

	while (it != end()) {
		// std::cerr << *it << " (" << it.node << ")" << std::endl;
		if (it.node->parent != 0) {
			if (it.node->prev_sibling == 0) {
				assert(it.node->parent->first_child == it.node);
			} else {
				assert(it.node->prev_sibling->next_sibling == it.node);
			}

			if (it.node->next_sibling == 0) {
				assert(it.node->parent->last_child == it.node);
			} else {
				assert(it.node->next_sibling->prev_sibling == it.node);
			}
		}

		++it;
	}
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::sibling_iterator tree<T, tree_node_allocator>::child(const iterator_base& it, size_t num) {
	tree_node* tmp = it.node->first_child;

	while (num--) {
		assert(tmp != 0);
		tmp = tmp->next_sibling;
	}

	return tmp;
}



// Iterator base

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::iterator_base::iterator_base()
	: node(0), skip_current_children_(false) { }

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::iterator_base::iterator_base(tree_node* tn)
	: node(tn), skip_current_children_(false) { }

template<class T, class tree_node_allocator>
T& tree<T, tree_node_allocator>::iterator_base::operator*() const {
	return node->data;
}

template<class T, class tree_node_allocator>
T* tree<T, tree_node_allocator>::iterator_base::operator->() const {
	return &(node->data);
}

template<class T, class tree_node_allocator>
bool tree<T, tree_node_allocator>::post_order_iterator::operator!=(const post_order_iterator& other) const {
	return (other.node != this->node);
}

template<class T, class tree_node_allocator>
bool tree<T, tree_node_allocator>::post_order_iterator::operator==(const post_order_iterator& other) const {
	return (other.node == this->node);
}

template<class T, class tree_node_allocator>
bool tree<T, tree_node_allocator>::pre_order_iterator::operator!=(const pre_order_iterator& other) const {
	return (other.node != this->node);
}

template<class T, class tree_node_allocator>
bool tree<T, tree_node_allocator>::pre_order_iterator::operator==(const pre_order_iterator& other) const {
	return (other.node == this->node);
}

#ifdef TREE_HH_EXTRA
template<class T, class tree_node_allocator>
bool tree<T, tree_node_allocator>::expanded_iterator::operator!=(const iterator_base& other) const {
	return (other.node != this->node);
}

template<class T, class tree_node_allocator>
bool tree<T, tree_node_allocator>::expanded_iterator::operator==(const iterator_base& other) const {
	return (other.node == this->node);
}

template<class T, class tree_node_allocator>
bool tree<T, tree_node_allocator>::expandable_iterator::operator!=(const expandable_iterator& other) const {
	return (other.node != this->node);
}

template<class T, class tree_node_allocator>
bool tree<T, tree_node_allocator>::expandable_iterator::operator==(const expandable_iterator& other) const {
	return (other.node == this->node);
}
#endif // TREE_HH_EXTRA

template<class T, class tree_node_allocator>
bool tree<T, tree_node_allocator>::sibling_iterator::operator!=(const sibling_iterator& other) const {
	return (other.node != this->node);
}

template<class T, class tree_node_allocator>
bool tree<T, tree_node_allocator>::sibling_iterator::operator==(const sibling_iterator& other) const {
	return (other.node == this->node);
}

template<class T, class tree_node_allocator>
bool tree<T, tree_node_allocator>::leaf_iterator::operator!=(const leaf_iterator& other) const {
	return (other.node != this->node);
}

template<class T, class tree_node_allocator>
bool tree<T, tree_node_allocator>::leaf_iterator::operator==(const leaf_iterator& other) const {
	return (other.node == this->node && other.top_node == this->top_node);
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::sibling_iterator tree<T, tree_node_allocator>::iterator_base::begin() const {
	if (node->first_child == 0) {
		return end();
	}

	sibling_iterator ret(node->first_child);
	ret.parent_ = this->node;
	return ret;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::sibling_iterator tree<T, tree_node_allocator>::iterator_base::end() const {
	sibling_iterator ret(0);
	ret.parent_ = node;
	return ret;
}

template<class T, class tree_node_allocator>
void tree<T, tree_node_allocator>::iterator_base::skip_children() {
	skip_current_children_ = true;
}

template<class T, class tree_node_allocator>
void tree<T, tree_node_allocator>::iterator_base::skip_children(bool skip) {
	skip_current_children_ = skip;
}

template<class T, class tree_node_allocator>
size_t tree<T, tree_node_allocator>::iterator_base::number_of_children() const {
	tree_node* pos = node->first_child;

	if (pos == 0) {
		return 0;
	}

	size_t ret = 1;

	while (pos != node->last_child) {
		++ret;
		pos = pos->next_sibling;
	}

	return ret;
}



// Pre-order iterator

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::pre_order_iterator::pre_order_iterator()
	: iterator_base(0) { }

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::pre_order_iterator::pre_order_iterator(tree_node* tn)
	: iterator_base(tn) { }

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::pre_order_iterator::pre_order_iterator(const iterator_base& other)
	: iterator_base(other.node) { }

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::pre_order_iterator::pre_order_iterator(const sibling_iterator& other)
	: iterator_base(other.node) {
	if (this->node == 0) {
		if (other.range_last() != 0) {
			this->node = other.range_last();
		} else {
			this->node = other.parent_;
		}

		this->skip_children();
		++(*this);
	}
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::pre_order_iterator& tree<T, tree_node_allocator>::pre_order_iterator::operator++() {
	assert(this->node != 0);

//	if((!this->skip_current_children_ || this->node->expanded) && this->node->first_child) {
	if (!this->skip_current_children_ && this->node->first_child != 0) {
		this->node = this->node->first_child;
	} else {
		this->skip_current_children_ = false;

		while (this->node->next_sibling == 0) {
			this->node = this->node->parent;

			if (this->node == 0) {
				return *this;
			}
		}

		this->node = this->node->next_sibling;
	}

	return *this;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::pre_order_iterator& tree<T, tree_node_allocator>::pre_order_iterator::operator--() {
	assert(this->node != 0);

	if (this->node->prev_sibling) {
		this->node = this->node->prev_sibling;

//		while (this->node->last_child && this->node->expanded) { //what if parent of parent is hidden?
		while (this->node->last_child) {
			this->node = this->node->last_child;
		}
	} else {
		this->node = this->node->parent;

		if (this->node == 0) {
			return *this;
		}
	}

	return *this;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::pre_order_iterator tree<T, tree_node_allocator>::pre_order_iterator::operator++(int) {
	pre_order_iterator copy = *this;
	++(*this);
	return copy;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::pre_order_iterator& tree<T, tree_node_allocator>::pre_order_iterator::next_skip_children() {
	(*this).skip_children();
	(*this)++;
	return *this;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::pre_order_iterator tree<T, tree_node_allocator>::pre_order_iterator::operator--(int) {
	pre_order_iterator copy = *this;
	--(*this);
	return copy;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::pre_order_iterator& tree<T, tree_node_allocator>::pre_order_iterator::operator+=(size_t num) {
	while (num--) {
		++(*this);
	}

	return (*this);
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::pre_order_iterator& tree<T, tree_node_allocator>::pre_order_iterator::operator-=(size_t num) {
	while (num--) {
		--(*this);
	}

	return (*this);
}



#ifdef TREE_HH_EXTRA
// Expanded iterator

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::expanded_iterator::expanded_iterator()
	: iterator_base(0) { }

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::expanded_iterator::expanded_iterator(tree_node* tn)
	: iterator_base(tn) { }

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::expanded_iterator::expanded_iterator(const iterator_base& other)
	: iterator_base(other.node) { }

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::expanded_iterator::expanded_iterator(const sibling_iterator& other)
	: iterator_base(other.node) {
	if (this->node == 0) {
		this->node = (other.range_last()) ? other.range_last() : other.parent_;
		this->skip_children();
		++(*this);
	}
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::expanded_iterator& tree<T, tree_node_allocator>::expanded_iterator::operator++() {
//		assert(this->node);

	while (this->node != 0) {
		if (!this->skip_current_children_ && this->node->first_child && this->node->expanded) {
			this->node = this->node->first_child;
		} else {
			this->skip_current_children_ = false;

			while (this->node->next_sibling == 0) {
				this->node = this->node->parent;

				if (this->node == 0) {
					return *this;
				}
			}

			this->node = this->node->next_sibling;
		}

		if (this->node->visible) {
			break;
		}
	}

	return *this;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::expanded_iterator& tree<T, tree_node_allocator>::expanded_iterator::operator--() {
	assert(this->node);

	if (this->node->prev_sibling) {
		this->node = this->node->prev_sibling;

//		while (this->node->last_child && this->node->expanded) { //what if parent of parent is hidden?
		while (this->node->last_child) {
			this->node = this->node->last_child;
		}
	} else {
		this->node = this->node->parent;

		if (this->node == 0) {
			return *this;
		}
	}

	return *this;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::expanded_iterator tree<T, tree_node_allocator>::expanded_iterator::operator++(int) {
	expanded_iterator copy = *this;
	++(*this);
	return copy;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::expanded_iterator& tree<T, tree_node_allocator>::expanded_iterator::next_skip_children() {
	(*this).skip_children();
	(*this)++;
	return *this;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::expanded_iterator tree<T, tree_node_allocator>::expanded_iterator::operator--(int) {
	expanded_iterator copy = *this;
	--(*this);
	return copy;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::expanded_iterator& tree<T, tree_node_allocator>::expanded_iterator::operator+=(size_t num) {
	while (num--) {
		++(*this);
	}

	return (*this);
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::expanded_iterator& tree<T, tree_node_allocator>::expanded_iterator::operator-=(size_t num) {
	while (num--) {
		--(*this);
	}

	return (*this);
}
#endif // TREE_HH_EXTRA

#ifdef TREE_HH_EXTRA
template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::expandable_iterator::expandable_iterator()
	: iterator_base(0) { }

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::expandable_iterator::expandable_iterator(tree_node* tn)
	: iterator_base(tn) { }

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::expandable_iterator::expandable_iterator(const iterator_base& other)
	: iterator_base(other.node) { }

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::expandable_iterator::expandable_iterator(const sibling_iterator& other)
	: iterator_base(other.node) {
	if (this->node == 0) {
		this->node = (other.range_last()) ? other.range_last() : other.parent_;
		this->skip_children();
		++(*this);
	}
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::expandable_iterator& tree<T, tree_node_allocator>::expandable_iterator::operator++() {
	assert(this->node);

	if (!this->skip_current_children_ && this->node->first_child/* && this->node->expanded*/) {
		this->node = this->node->first_child;
	} else {
		this->skip_current_children_ = false;

		while (this->node->next_sibling == 0) {
			this->node = this->node->parent;

			if (this->node == 0) {
				return *this;
			}
		}

		this->node = this->node->next_sibling;
	}

	return *this;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::expandable_iterator& tree<T, tree_node_allocator>::expandable_iterator::operator--() {
	assert(this->node);

	if (this->node->prev_sibling) {
		this->node = this->node->prev_sibling;

//		while (this->node->last_child && this->node->expanded) { //what if parent of parent is hidden?
		while (this->node->last_child) {
			this->node = this->node->last_child;
		}
	} else {
		this->node = this->node->parent;

		if (this->node == 0) {
			return *this;
		}
	}

	return *this;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::expandable_iterator tree<T, tree_node_allocator>::expandable_iterator::operator++(int) {
	expandable_iterator copy = *this;
	++(*this);
	return copy;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::expandable_iterator& tree<T, tree_node_allocator>::expandable_iterator::next_skip_children() {
	(*this).skip_children();
	(*this)++;
	return *this;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::expandable_iterator tree<T, tree_node_allocator>::expandable_iterator::operator--(int) {
	expandable_iterator copy = *this;
	--(*this);
	return copy;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::expandable_iterator& tree<T, tree_node_allocator>::expandable_iterator::operator+=(size_t num) {
	while (num--) {
		++(*this);
	}

	return (*this);
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::expandable_iterator& tree<T, tree_node_allocator>::expandable_iterator::operator-=(size_t num) {
	while (num--) {
		--(*this);
	}

	return (*this);
}
#endif // TREE_HH_EXTRA



// Post-order iterator

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::post_order_iterator::post_order_iterator()
	: iterator_base(0) { }

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::post_order_iterator::post_order_iterator(tree_node* tn)
	: iterator_base(tn) { }

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::post_order_iterator::post_order_iterator(const iterator_base& other)
	: iterator_base(other.node) { }

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::post_order_iterator::post_order_iterator(const sibling_iterator& other)
	: iterator_base(other.node) {
	if (this->node == 0) {
		if (other.range_last() != 0) {
			this->node = other.range_last();
		} else {
			this->node = other.parent_;
		}

		this->skip_children();
		++(*this);
	}
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::post_order_iterator& tree<T, tree_node_allocator>::post_order_iterator::operator++() {
	assert(this->node != 0);

	if (this->node->next_sibling == 0) {
		this->node = this->node->parent;
		this->skip_current_children_ = false;
	} else {
		this->node = this->node->next_sibling;

		if (this->skip_current_children_) {
			this->skip_current_children_ = false;
		} else {
			while (this->node->first_child) {
				this->node = this->node->first_child;
			}
		}
	}

	return *this;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::post_order_iterator& tree<T, tree_node_allocator>::post_order_iterator::operator--() {
	assert(this->node != 0);

	if (this->skip_current_children_ || this->node->last_child == 0) {
		this->skip_current_children_ = false;

		while (this->node->prev_sibling == 0) {
			this->node = this->node->parent;
		}

		this->node = this->node->prev_sibling;
	} else {
		this->node = this->node->last_child;
	}

	return *this;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::post_order_iterator tree<T, tree_node_allocator>::post_order_iterator::operator++(int) {
	post_order_iterator copy = *this;
	++(*this);
	return copy;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::post_order_iterator tree<T, tree_node_allocator>::post_order_iterator::operator--(int) {
	post_order_iterator copy = *this;
	--(*this);
	return copy;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::post_order_iterator& tree<T, tree_node_allocator>::post_order_iterator::operator+=(size_t num) {
	while (num--) {
		++(*this);
	}

	return (*this);
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::post_order_iterator& tree<T, tree_node_allocator>::post_order_iterator::operator-=(size_t num) {
	while (num--) {
		--(*this);
	}

	return (*this);
}

template<class T, class tree_node_allocator>
void tree<T, tree_node_allocator>::post_order_iterator::descend_all() {
	assert(this->node != 0);

	while (this->node->first_child) {
		this->node = this->node->first_child;
	}
}



// Breadth-first iterator

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::breadth_first_queued_iterator::breadth_first_queued_iterator()
	: iterator_base() { }

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::breadth_first_queued_iterator::breadth_first_queued_iterator(tree_node* tn)
	: iterator_base(tn) {
	traversal_queue.push(tn);
}

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::breadth_first_queued_iterator::breadth_first_queued_iterator(const iterator_base& other)
	: iterator_base(other.node) {
	traversal_queue.push(other.node);
}

template<class T, class tree_node_allocator>
bool tree<T, tree_node_allocator>::breadth_first_queued_iterator::operator!=(const breadth_first_queued_iterator& other) const {
	return (other.node != this->node);
}

template<class T, class tree_node_allocator>
bool tree<T, tree_node_allocator>::breadth_first_queued_iterator::operator==(const breadth_first_queued_iterator& other) const {
	return (other.node == this->node);
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::breadth_first_queued_iterator& tree<T, tree_node_allocator>::breadth_first_queued_iterator::operator++() {
	assert(this->node != 0);

	// Add child nodes and pop current node
	sibling_iterator sib = this->begin();

	while (sib != this->end()) {
		traversal_queue.push(sib.node);
		++sib;
	}

	traversal_queue.pop();

	if (traversal_queue.size() > 0) {
		this->node = traversal_queue.front();
	} else {
		this->node = 0;
	}

	return (*this);
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::breadth_first_queued_iterator tree<T, tree_node_allocator>::breadth_first_queued_iterator::operator++(int) {
	breadth_first_queued_iterator copy = *this;
	++(*this);
	return copy;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::breadth_first_queued_iterator& tree<T, tree_node_allocator>::breadth_first_queued_iterator::operator+=(size_t num) {
	while (num--) {
		++(*this);
	}

	return (*this);
}



// Fixed depth iterator

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::fixed_depth_iterator::fixed_depth_iterator()
	: iterator_base() { }

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::fixed_depth_iterator::fixed_depth_iterator(tree_node* tn)
	: iterator_base(tn), top_node(0) { }

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::fixed_depth_iterator::fixed_depth_iterator(const iterator_base& other)
	: iterator_base(other.node), top_node(0) { }

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::fixed_depth_iterator::fixed_depth_iterator(const sibling_iterator& other)
	: iterator_base(other.node), top_node(0) { }

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::fixed_depth_iterator::fixed_depth_iterator(const fixed_depth_iterator& other)
	: iterator_base(other.node), top_node(other.top_node) { }

template<class T, class tree_node_allocator>
bool tree<T, tree_node_allocator>::fixed_depth_iterator::operator==(const fixed_depth_iterator& other) const {
	return (other.node == this->node && other.top_node == top_node);
}

template<class T, class tree_node_allocator>
bool tree<T, tree_node_allocator>::fixed_depth_iterator::operator!=(const fixed_depth_iterator& other) const {
	return (other.node != this->node || other.top_node != top_node);
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::fixed_depth_iterator& tree<T, tree_node_allocator>::fixed_depth_iterator::operator++() {
	assert(this->node != 0);

	if (this->node->next_sibling) {
		this->node = this->node->next_sibling;
	} else {
		size_t relative_depth = 0;

	upper:
		do {
			if (this->node == this->top_node) {
				this->node = 0; // FIXME: return a proper fixed_depth end iterator once implemented
				return *this;
			}

			this->node = this->node->parent;

			if (this->node == 0) {
				return *this;
			}

			--relative_depth;
		} while (this->node->next_sibling == 0);

	lower:
		this->node = this->node->next_sibling;

		while (this->node->first_child == 0) {
			if (this->node->next_sibling == 0) {
				goto upper;
			}

			this->node = this->node->next_sibling;

			if (this->node == 0) {
				return *this;
			}
		}

		while (relative_depth < 0 && this->node->first_child != 0) {
			this->node = this->node->first_child;
			++relative_depth;
		}

		if (relative_depth < 0) {
			goto (this->node->next_sibling == 0) ? upper : lower;
		}
	}

	return *this;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::fixed_depth_iterator& tree<T, tree_node_allocator>::fixed_depth_iterator::operator--() {
	assert(this->node != 0);

	if (this->node->prev_sibling) {
		this->node = this->node->prev_sibling;
	} else {
		size_t relative_depth = 0;

	upper:
		do {
			if (this->node == this->top_node) {
				this->node = 0;
				return *this;
			}

			this->node = this->node->parent;

			if (this->node == 0) {
				return *this;
			}

			--relative_depth;
		} while (this->node->prev_sibling == 0);

	lower:
		this->node = this->node->prev_sibling;

		while (this->node->last_child == 0) {
			if (this->node->prev_sibling == 0) {
				goto upper;
			}

			this->node = this->node->prev_sibling;

			if (this->node == 0) {
				return *this;
			}
		}

		while (relative_depth < 0 && this->node->last_child != 0) {
			this->node = this->node->last_child;
			++relative_depth;
		}

		if (relative_depth < 0) {
			goto (this->node->prev_sibling == 0) ? upper : lower;
		}
	}

	return *this;

//
//
//	assert(this->node != 0);
//	if (this->node->prev_sibling != 0) {
//		this->node = this->node->prev_sibling;
//		assert(this->node != 0);
//		if (this->node->parent == 0 && this->node->prev_sibling == 0) { // head element
//			this->node = 0;
//		}
//	} else {
//		tree_node* par = this->node->parent;
//		do {
//			par = par->prev_sibling;
//			if (par == 0) { // FIXME: need to keep track of this!
//				this->node = 0;
//				return *this;
//			}
//		} while (par->last_child == 0);
//		this->node = par->last_child;
//	}
//
//	return *this;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::fixed_depth_iterator tree<T, tree_node_allocator>::fixed_depth_iterator::operator++(int) {
	fixed_depth_iterator copy = *this;
	++(*this);
	return copy;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::fixed_depth_iterator tree<T, tree_node_allocator>::fixed_depth_iterator::operator--(int) {
	fixed_depth_iterator copy = *this;
	--(*this);
	return copy;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::fixed_depth_iterator& tree<T, tree_node_allocator>::fixed_depth_iterator::operator-=(size_t num) {
	while (num--) {
		--(*this);
	}

	return (*this);
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::fixed_depth_iterator& tree<T, tree_node_allocator>::fixed_depth_iterator::operator+=(size_t num) {
	while (num--) {
		++(*this);
	}

	return *this;
}



// Sibling iterator

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::sibling_iterator::sibling_iterator()
	: iterator_base() {
	set_parent_();
}

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::sibling_iterator::sibling_iterator(tree_node* tn)
	: iterator_base(tn) {
	set_parent_();
}

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::sibling_iterator::sibling_iterator(const iterator_base& other)
	: iterator_base(other.node) {
	set_parent_();
}

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::sibling_iterator::sibling_iterator(const sibling_iterator& other)
	: iterator_base(other), parent_(other.parent_) { }

template<class T, class tree_node_allocator>
void tree<T, tree_node_allocator>::sibling_iterator::set_parent_() {
	parent_ = 0;

	if (this->node == 0) {
		return;
	}

	if (this->node->parent != 0) {
		parent_ = this->node->parent;
	}
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::sibling_iterator& tree<T, tree_node_allocator>::sibling_iterator::operator++() {
	if (this->node) {
		this->node = this->node->next_sibling;
	}

	return *this;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::sibling_iterator& tree<T, tree_node_allocator>::sibling_iterator::operator--() {
	if (this->node) {
		this->node = this->node->prev_sibling;
	} else {
		assert(parent_);
		this->node = parent_->last_child;
	}

	return *this;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::sibling_iterator tree<T, tree_node_allocator>::sibling_iterator::operator++(int) {
	sibling_iterator copy = *this;
	++(*this);
	return copy;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::sibling_iterator tree<T, tree_node_allocator>::sibling_iterator::operator--(int) {
	sibling_iterator copy = *this;
	--(*this);
	return copy;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::sibling_iterator& tree<T, tree_node_allocator>::sibling_iterator::operator+=(size_t num) {
	while (num--) {
		++(*this);
	}

	return (*this);
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::sibling_iterator& tree<T, tree_node_allocator>::sibling_iterator::operator-=(size_t num) {
	while (num--) {
		--(*this);
	}

	return (*this);
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::tree_node* tree<T, tree_node_allocator>::sibling_iterator::range_first() const {
	tree_node* tmp = parent_->first_child;
	return tmp;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::tree_node* tree<T, tree_node_allocator>::sibling_iterator::range_last() const {
	return parent_->last_child;
}



// Leaf iterator

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::leaf_iterator::leaf_iterator()
	: iterator_base(0), top_node(0) { }

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::leaf_iterator::leaf_iterator(tree_node* tn, tree_node* top)
	: iterator_base(tn), top_node(top) { }

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::leaf_iterator::leaf_iterator(const iterator_base& other)
	: iterator_base(other.node), top_node(0) { }

template<class T, class tree_node_allocator>
tree<T, tree_node_allocator>::leaf_iterator::leaf_iterator(const sibling_iterator& other)
	: iterator_base(other.node), top_node(0) {
	if (this->node == 0) {
		if (other.range_last() != 0) {
			this->node = other.range_last();
		} else {
			this->node = other.parent_;
		}

		++(*this);
	}
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::leaf_iterator& tree<T, tree_node_allocator>::leaf_iterator::operator++() {
	assert(this->node != 0);

	if (this->node->first_child != 0) { // current node is no longer leaf (children got added)
		while (this->node->first_child) {
			this->node = this->node->first_child;
		}
	} else {
		while (this->node->next_sibling == 0) {
			if (this->node->parent == 0) {
				return *this;
			}

			this->node = this->node->parent;

			if (top_node != 0 && this->node == top_node) {
				return *this;
			}
		}

		this->node = this->node->next_sibling;

		while (this->node->first_child) {
			this->node = this->node->first_child;
		}
	}

	return *this;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::leaf_iterator& tree<T, tree_node_allocator>::leaf_iterator::operator--() {
	assert(this->node != 0);

	while (this->node->prev_sibling == 0) {
		if (this->node->parent == 0) {
			return *this;
		}

		this->node = this->node->parent;

		if (top_node != 0 && this->node == top_node) {
			return *this;
		}
	}

	this->node = this->node->prev_sibling;

	while (this->node->last_child) {
		this->node = this->node->last_child;
	}

	return *this;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::leaf_iterator tree<T, tree_node_allocator>::leaf_iterator::operator++(int) {
	leaf_iterator copy = *this;
	++(*this);
	return copy;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::leaf_iterator tree<T, tree_node_allocator>::leaf_iterator::operator--(int) {
	leaf_iterator copy = *this;
	--(*this);
	return copy;
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::leaf_iterator& tree<T, tree_node_allocator>::leaf_iterator::operator+=(size_t num) {
	while (num--) {
		++(*this);
	}

	return (*this);
}

template<class T, class tree_node_allocator>
typename tree<T, tree_node_allocator>::leaf_iterator& tree<T, tree_node_allocator>::leaf_iterator::operator-=(size_t num) {
	while (num--) {
		--(*this);
	}

	return (*this);
}

#ifdef TREE_HH_EXTRA
template<class T, class tree_node_allocator /*= std::allocator<tree_node_<T> > */>
size_t tree<T, tree_node_allocator>::collapse() {
	// tree expanded size is reset to be incemented by the head siblings.
	head->count_expanded = 0;
	// iterate over all the nodes and reset expanded size and expanded flag.
	iterator it = begin();

	while (it != end()) {
		if (it.node->first_child) {
			it.node->count_expanded = 0;
			it.node->expanded = false;
		}

		// if the node has no parent increment the expended size of the tree.
		if (!it.node->parent) {
			++head->count_expanded;
		}

		++it;
	}

	return head->count_expanded;
}

template<class T, class tree_node_allocator /*= std::allocator<tree_node_<T> > */>
size_t tree<T, tree_node_allocator>::collapse(const iterator_base& it) {
	// check if the node has children.
	if (!it.node->first_child) {
		//return;
	}

	// iterate all expanded parents and update expanded size.
	// the collapsed parents are not affected.
	tree_node* parent = it.node->parent;

	while (parent && parent->expanded) {
		parent->count_expanded -= it.node->count_expanded;
		parent = parent->parent;
	}

	// if the parent iteration has reached the head or any of its siblings
	// update the expanded size of the tree.
	if (!parent) {
		head->count_expanded -= it.node->count_expanded;
	}

	// update node expanded size and expanded flag.
	it.node->count_expanded = 0;
	it.node->expanded = false;

	return head->count_expanded;
}

template<class T, class tree_node_allocator /*= std::allocator<tree_node_<T> > */>
size_t tree<T, tree_node_allocator>::expand() {
	// iterate over all the nodes and reset expanded size and expanded flag.
	iterator it = begin();

	while (it != end()) {
		if (it.node.first_child) {
			it.node->count_expanded = it->node.count_expandable;
			it.node->expanded = true;
		}

		++it;
	}

	// update expanded size of tree.
	head->count_expanded = head->count_expandable;

	return head->count_expanded;
}

template<class T, class tree_node_allocator /*= std::allocator<tree_node_<T> > */>
size_t tree<T, tree_node_allocator>::expand(const iterator_base& it) {
	// check if the node has children.
	if (!it.node->first_child) {
		//return;
	}

	// update node expanded size and expanded flag.
	// sum the node's direct children count and their expanded size.
	tree_node* child = it.node->first_child;

	while (child) {
		it.node->count_expanded += child->count_expanded + 1;
		child = child->next_sibling;
	}

	it.node->expanded = true;

	// iterate all expanded parents and update expanded size.
	// the collapsed parents are not affected.
	tree_node* parent = it.node->parent;

	while (parent && parent->expanded) {
		parent->count_expanded += it.node->count_expanded;
		parent = parent->parent;
	}

	// if the parent iteration has reached the head or any of its siblings
	// update the expanded size of the tree.
	if (!parent) {
		head->count_expanded += it.node->count_expanded;
	}

	return head->count_expanded;

}

template<class T, class tree_node_allocator /*= std::allocator<tree_node_<T> > */>
bool tree<T, tree_node_allocator>::is_expanded(const iterator_base& it) {
	return it.node ? it.node->expanded : false;
}

template<class T, class tree_node_allocator /*= std::allocator<tree_node_<T> > */>
void tree<T, tree_node_allocator>::set_expand_new(bool expand) {
	head->expanded = expand;
}

template<class T, class tree_node_allocator /*= std::allocator<tree_node_<T> > */>
void tree<T, tree_node_allocator>::select() {
	if (multiple_selection) {
		selection.clear();
		selection.insert({ 0, head->count_expanded });
	}
}

template<class T, class tree_node_allocator /*= std::allocator<tree_node_<T> > */>
void tree<T, tree_node_allocator>::toggle(size_t index, size_t origin, uint32_t mode) {
	if (!multiple_selection || !(mode & MK_CONTROL)) {
		selection.clear();
		selection.insert({ index, index });
	} else if (mode & MK_CONTROL) {
		// Toggle selected state
		bool found = false;

		for (auto& iter = selection.begin(); iter != selection.cend(); ++iter) {
			if ((iter->begin == index) && (iter->end == index)) {
				// if single item selection range, remove selection range
				selection.erase(iter);
				found = true; break;
			} else if (iter->begin == index) {
				// if item is at the beginning of selection range, amend selection range
				++iter->begin;
				found = true; break;
			} else if (iter->end == index) {
				// if item is at the end of selection range, amend selection range
				--iter->end;
				found = true; break;
			} else if ((iter->begin <= index) && (iter->end >= index)) {
				// if item is in the middle of selection range, split selection range
				size_t end = iter->end;
				iter->end = index - 1;
				selection.insert({ index + 1, end });
				found = true; break;
			} else if ((iter->begin - 1) == index) {
				// if item is before the beginning of selection range, amend selection range
				--iter->begin;
				found = true; break;
			} else if ((iter->end + 1) == index) {
				// if item is after the end of selection range, amend selection range
				++iter->end;
				found = true; break;
			}
		}

		// if item not found in selection range then add selection range
		if (found == false) {
			selection.push_back({ index, index });
		}
	}

	if ((mode & MK_SHIFT) && (multiple_selection)) {
		size_t begin = (std::min)(origin, index);
		size_t end = (std::max)(origin, index);

		bool found = false;

		for (auto& iter = selection.begin(); iter != selection.cend(); ++iter) {
			if (iter->begin == begin) {
				// if item is before the beginning of selection range, amend selection range
				iter->end = end;
				found = true; break;
			} else if (iter->end == end) {
				// if item is after the end of selection range, amend selection range
				iter->begin = begin;
				found = true; break;
			}
		}

		// if item not found in selection then add selection
		if (found == false) {
			selection.push_back({ begin, end });
		}
	}

	// Sanitize selection range
	if ((multiple_selection) && (selection.size() > 1)) {
		for (auto& iter = selection.begin(); iter != selection.cend(); ++iter) {
			// if item in next selection range is a continuation of selection range, amend selection range and remove next selection range
			auto next = iter;
			++next;

			if ((next != selection.cend()) && ((iter->end + 1) == next->begin)) {
				iter->end = next->end;
				next = selection.erase(next);
			}

			// if item in next selection range is included in selection range, remove next selection range
			if ((next != selection.cend()) && (iter->begin <= next->begin) && (iter->end >= next->end)) {
				next = selection.erase(next);
			}
		}
	}
}

template<class T, class tree_node_allocator /*= std::allocator<tree_node_<T> > */>
void tree<T, tree_node_allocator>::unselect() {
	selection.clear();
}

template<class T, class tree_node_allocator /*= std::allocator<tree_node_<T> > */>
void tree<T, tree_node_allocator>::set_multiple_selection(bool multiple) {
	multiple_selection = multiple;
}

template<class T, class tree_node_allocator /*= std::allocator<tree_node_<T> > */>
bool tree<T, tree_node_allocator>::is_selected(size_t index) {
	for (auto& iter = selection.cbegin(); iter != selection.cend(); ++iter) {
		if (iter->begin > index) {
			return false;
		}

		if ((iter->begin <= index) && (iter->end >= index)) {
			return true;
		}
	}

	return false;
}
#endif // TREE_HH_EXTRA

#endif

// Local variables:
// tab-width: 3
// End:
