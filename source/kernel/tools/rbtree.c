/**
 * 
 * rbtree实现
 * 
 */

#include "tools/rbtree.h"

void rb_link_node(rb_node* node, rb_node* parent, rb_node** rb_link)
{
    // 新节点的颜色需为红色, 否则在插入时会破坏性质4
    // 若新节点的父节点也是红色, 就会破坏性质2, 此时会进行调整, 使树平衡
    node->rb_color = RB_RED;

    node->rb_parent = parent;
    node->rb_left = node->rb_right = NULL;

    // 调整parent的子节点为node, rb_link在插入时决定是左还是右
    *rb_link = node;
}

void __rb_rotate_left(rb_node* node, rb_root* root)	
{   
	/***
	 * 
	 *      A  <- node                                  C  <- right
	 *     / \                                         / \
	 *    B   C  <- right    =============>   node->  A   E
	 *       / \                                	 / \ 
	 *      D   E                           		B   D
	 * 
	 */

	// 这棵子树的新根是node->rb_right
	rb_node* right = node->rb_right;

	// right的左孩子(D)要变为node(A)的右孩子
	node->rb_right = right->rb_left;
	if(node->rb_right) {	// 如果该结点(D)不空的话, 还要调整其父节点
		node->rb_right->rb_parent = node;
	}

	// right新的左孩子应调整为node(A)
	right->rb_left = node;

	// 调整新根的父节点
	right->rb_parent = node->rb_parent;

	// 若right的父节点为空, 说明新根right是整颗树的根
	if(!right->rb_parent) {
		root->rb_node = right;	// 调整root指针
	}
	// 否则right不是整棵树的根
	else { 
		// 那么还需调整原根父节点的指针使其指向新根right

		// 若node是其原父节点的左孩子, 那么right也是左孩子
		if(node == node->rb_parent->rb_left) {
			node->rb_parent->rb_left = right;
		}
		// 否则right就是右孩子
		else {
			node->rb_parent->rb_right = right;
		}
	}

	// 调整原根的父节点
	node->rb_parent = right;
}

void __rb_rotate_right(rb_node* node, rb_root* root)
{

	/***
	 * 
	 *              C  <- node                    A  <- left
	 *             / \                           / \
	 *   left ->  A   E      =============>     B   C  <- node
	 *           / \                               / \ 
	 *          B   D                             D   E
	 * 
	 */

	// 这棵树的新根是node->rb_left
	rb_node *left = node->rb_left;

	// left的右孩子(D)要变为node(C)的左孩子
	node->rb_left = left->rb_right;
	if(node->rb_left) {		// 如果该结点(D)不空的话, 还要调整其父节点
		node->rb_left->rb_parent = node;
	}

	// left新的右孩子应调整为node(C)
	left->rb_right = node;

	// 调整新根的父节点
	left->rb_parent = node->rb_parent;
	
	// 若left的父节点为空, 说明新根left是整颗树的根
	if(!left->rb_parent) {
		root->rb_node = left;	// 调整root指针
	}
	// 否则left不是整棵树的根
	else {
		// 那么还需调整原根父节点的指针使其指向新根left

		// 若node是其原父节点的左孩子, 那么left也是左孩子
		if(node == node->rb_parent->rb_right) {
			node->rb_parent->rb_right = left;
		}
		// 否则left就是右孩子
		else {
			node->rb_parent->rb_left = left;
		}
	}

	// 调整原根的父节点
    node->rb_parent = left;
}

void rb_insert_color(rb_node* node, rb_root* root)
{
	/**
	 * 在图中, 小写字母为红色, 大写字母为黑色
	 * 判定节点(关注节点)一开始为插入的节点, node指的是关注节点
	 * 
	 * 1. node是根节点: 将node调整为黑色
	 * 2. node的父节点是黑色: 忽略
	 * 3. node的父节点是红色:
	 * 		1)uncle是红色: 将p与u调整为黑色, g调整为红色, 关注节点变为g
	 * 		2)uncle是黑色:
	 * 			a. LR或者RL: 对p进行左旋或者右旋, 关注节点变为p
	 * 			b. LL或者RR: 对g进行右旋或者左旋, 调整p和gp的颜色, 关注节点变为p
	 */
	
	rb_node *parent, *gparent, *tmp;

	while((parent = node->rb_parent) && rb_is_red(parent))
	{
		gparent = parent->rb_parent;
		
		// 情况3: node的父节点为红色, 根据parent的位置获取uncle的位置
		if(parent == gparent->rb_left)
		{
			register rb_node* uncle = gparent->rb_right;

			/**
			 * Case 1: uncle为红色
			 *
			 *       G            g
			 *      / \          / \
			 *     p   u  -->   P   U
			 *    /            /
			 *   n            n
			 * 
			 */
			if(uncle && rb_is_red(uncle)) // 若uncle不存在说明其为黑色
			{
				// 调整颜色
				rb_set_black(parent);
				rb_set_black(uncle);
				rb_set_red(gparent);
				
				// 关注节点变为g
				node = gparent;
				continue;
			}
			

			/**
			 * Case 2: uncle是黑色, 并且是LR, 对p进行左旋
			 *
			 *      G             G
			 *     / \           / \
			 *    p   U  -->    n   U
			 *     \           /
			 *      n         p
			 * 
			 */
			if(node == parent->rb_right)
			{
				// 对p进行左旋
				__rb_rotate_left(parent, root);

				// 旋转后node变为父节点, parent变为node的子节点
				// 调整关注节点node为这个子节点parent
				tmp = parent;
				parent = node;
				node = tmp;
			}

			/**
			 * Case 3: 此时为LL, 调整p与g的颜色, 对g右旋
			 *
			 *        G           P
			 *       / \         / \
			 *      p   U  -->  n   g
			 *     /                 \
			 *    n                   U
			 */
			rb_set_red(gparent);
			rb_set_black(parent);
			__rb_rotate_right(gparent, root);
		}
		else // parent == gparent->right
		{
			register rb_node* uncle = gparent->rb_left;

			// Case 1: uncle为红色
			if(uncle && rb_is_red(uncle)) // uncle不存在则说明其为黑色
			{
				rb_set_black(parent);
				rb_set_black(uncle);
				rb_set_red(gparent);
				
				// 关注节点变为g
				node = gparent;
				continue;
			}


			// Case 2: RL
			if(node == parent->rb_left)
			{
				__rb_rotate_right(parent, root);

				// 旋转后node变为父节点, parent变为node的子节点
				// 调整关注节点node为这个子节点parent
				tmp = parent;
				parent = node;
				node = tmp;
			}
			
			// Case 3 - RR
			rb_set_red(gparent);
			rb_set_black(parent);
			__rb_rotate_left(gparent, root);
		}
	}

	rb_set_black(root->rb_node);
}

void rb_erase_next(rb_node* node, rb_root* root)
{
	rb_node *replace, *parent;
	int color;

	/* 获取node的子节点 */

	if(!node->rb_left) {
		replace = node->rb_right;
	}
	else if(!node->rb_right) {
		replace = node->rb_left;
	}
	// 若node有两个子节点, 那么就用node右子树中的最小值来代替掉node
	else {

		// node最终变成old的后继(前驱也可以)
		rb_node *old = node, *left;
		
		// 若old的后继就是其右孩子, 那么在后面有特判
		node = node->rb_right;
		while((left = node->rb_left)) {
			node = left;
		}

		replace = node->rb_right;		// node已经是最左边了, 所以它只能有右孩子
		parent = node->rb_parent;
		color = node->rb_color;

		// 修改replace
		if(replace) {
			replace->rb_parent = parent;
		}

		// 修改node父节点的孩子, 让其指向replace
		if(!parent) {
			root->rb_node = replace;
		}
		else {
			if(node == parent->rb_left) {
				parent->rb_left = replace;
			}
			else {
				parent->rb_right = replace;
			}		
		}
		
		///////////////////////////////////////////////////
		// 接下来要用node替换掉old

		// 若old的后继就是其右孩子, 那么replace的parent最终还应该是node
		if(node->rb_parent == old) {
			parent = node;
		}

		node->rb_parent = old->rb_parent;
		node->rb_color = old->rb_color;
		node->rb_right = old->rb_right;
		node->rb_left = old->rb_left;
		
		// 让old的父节点指向node
		if(!old->rb_parent) {
			root->rb_node = node;
		}
		else {
			if(old == old->rb_parent->rb_left) {
				old->rb_parent->rb_left = node;
			}
			else {
				old->rb_parent->rb_right = node;
			}
		}

		// 调整old的左右孩子的父节点为node
		old->rb_left->rb_parent = node;

		// 若old的后继就是其右孩子, 那么replace会在node的位置(即old->rb_right), 且replace有可能为空
		if(old->rb_right) {
			old->rb_right->rb_parent = node;
		}
		goto color;
	}

	parent = node->rb_parent;
	color = node->rb_color;

	// 调整replace的父节点
	if(replace) {
		replace->rb_parent = parent;
	}

	// 调整parent的子节点
	if(!parent) {
		root->rb_node = replace;
	}
	// 若parent为空, 那么replace就是新的树根
	else {
		if(node == parent->rb_left) {
			parent->rb_left = replace;
		}
		else {
			parent->rb_right = replace;
		}
	}


 color:		// 修复树的性质
	// 若实际被删节点的颜色为黑色, 则会影响黑高, 需调整
	if(color == RB_BLACK) {
		// 失衡的位置是真正被删除节点原本的位置, 该结点最多只能有一个孩子
		__rb_erase_color(replace, parent, root);
	}
}

void rb_erase_prev(rb_node* node, rb_root* root)
{
	rb_node *replace, *parent;
	int color;

	/* 获取node的子节点 */

	if(!node->rb_left) {
		replace = node->rb_right;
	}
	else if(!node->rb_right) {
		replace = node->rb_left;
	}
	// 若node有两个子节点, 那么就用node右子树中的最小值来代替掉node
	else {

		// node最终变成old的前驱
		rb_node *old = node, *right;
		
		// 若old的后继就是其右孩子, 那么在后面有特判
		node = node->rb_left;
		while((right = node->rb_right)) {
			node = right;
		}

		replace = node->rb_left;		// node已经是最右边了, 所以它只能有左孩子
		parent = node->rb_parent;
		color = node->rb_color;

		// 修改replace
		if(replace) {
			replace->rb_parent = parent;
		}

		// 修改node父节点的孩子, 让其指向replace
		if(!parent) {
			root->rb_node = replace;
		}
		else {
			if(node == parent->rb_left) {
				parent->rb_left = replace;
			}
			else {
				parent->rb_right = replace;
			}		
		}
		
		///////////////////////////////////////////////////
		// 接下来要用node替换掉old

		// 若old的前驱就是其左孩子, 那么replace的parent最终还应该是node
		if(node->rb_parent == old) {
			parent = node;
		}

		node->rb_parent = old->rb_parent;
		node->rb_color = old->rb_color;
		node->rb_right = old->rb_right;
		node->rb_left = old->rb_left;
		
		// 让old的父节点指向node
		if(!old->rb_parent) {
			root->rb_node = node;
		}
		else {
			if(old == old->rb_parent->rb_left) {
				old->rb_parent->rb_left = node;
			}
			else {
				old->rb_parent->rb_right = node;
			}
		}

		// 调整old的左右孩子的父节点为node
		// 若old的前驱就是其左孩子, 那么replace会在node的位置(即old->rb_left), 且replace有可能为空
		if(old->rb_left) {
			old->rb_left->rb_parent = node;
		}

		old->rb_right->rb_parent = node;
		
		goto color;
	}

	parent = node->rb_parent;
	color = node->rb_color;

	// 调整replace的父节点
	if(replace) {
		replace->rb_parent = parent;
	}

	// 调整parent的子节点
	if(!parent) {
		root->rb_node = replace;
	}
	// 若parent为空, 那么replace就是新的树根
	else {
		if(node == parent->rb_left) {
			parent->rb_left = replace;
		}
		else {
			parent->rb_right = replace;
		}
	}


 color:		// 修复树的性质
	// 若实际被删节点的颜色为黑色, 则会影响黑高, 需调整
	if(color == RB_BLACK) {
		// 失衡的位置是真正被删除节点原本的位置, 该结点最多只能有一个孩子
		__rb_erase_color(replace, parent, root);
	}
}

void __rb_erase_color(rb_node* node, rb_node* parent, rb_root* root)
{
	rb_node *other;

	/**
	 * 在图中, 小写字母为红色, 大写字母为黑色
	 * 判定节点(关注节点)node实际为被删除节点的孩子节点
	 * 
	 * 实际被删除的节点要么是叶子节点, 要么只有一个孩子
	 *   1. 若只有一个孩子: 那么孩子的颜色只能为红色(为黑色则违反了黑高), 则被删除的节点为黑色
	 * 	 2. 若是叶子节点: 既可能是黑色, 也可能是红色
	 *  可得 ===> 若被删节点是红色, 那么其肯定为叶子
	 * 
	 * 循环不变量:
	 * 	  经过node和parent的路径比其它路径的黑高少一(以经过node和sibling的路径进行判定)
	 * 
	 * 1. node是根节点: 直接跳过
	 * 2. node是红色时: 直接跳过, node原本的位置是黑色节点, 现在其为红色, 那么只要将其变为黑色就行了
	 * 3. node是黑色时:
	 * 		1) sibling为红色: 对parent左旋, 还需跳转到其它情况	-- Case 1
	 * 				sibling为红色, 对parent进行旋转, 这样红兄的一个黑子就可以落入失衡的一方
	 * 				并且经过这种情况后, node的兄弟变为了黑色(下一层判定中)
	 * 		2) sibling为黑色: 
	 * 			a. 都为黑色: 将sibling变为红色		  		-- Case 2
	 * 			b. 与node同侧为红:
	 * 			c. 与node异侧为红:
	 */

	// 若node是根节点或者红色就跳出
	while((!node || rb_is_black(node)) && node != root->rb_node)
	{
		if(node == parent->rb_left)
		{
			other = parent->rb_right;

			// 红兄
			if(rb_is_red(other))
			{
				/**
				 * Case 1 - 对P旋转
				 *
				 *     P               S
				 *    / \             / \
				 *   N   s    -->    p   Sr
				 *      / \         / \
				 *     Sl  Sr      N   Sl
				 */
				rb_set_red(parent);
				rb_set_black(other);
				__rb_rotate_left(parent, root);
				
				// 关注节点不变
				other = parent->rb_right;		// 关注节点的兄弟变为sl, 黑兄
			}

			// 黑兄二黑侄
			if((!other->rb_left || rb_is_black(other->rb_left))
				&& (!other->rb_right || rb_is_black(other->rb_right))) 
			{
				/**
				 * Case 2 - 反转sibling的颜色
				 * p可以是任意颜色
				 *
				 *    (p)           (p)
				 *    / \           / \
				 *   N   S    -->  N   s
				 *      / \           / \
				 *     Sl  Sr        Sl  Sr
				 */
				rb_set_red(other);

				// 修改关注节点为parent
				// 上移黑高
				node = parent;
				parent = node->rb_parent;
			}
			// 黑兄红侄, 需分情况哪个是红
			else
			{	
				// 同侧为红
				if(!other->rb_right || rb_is_black(other->rb_right))
				{
					/**
					 * Case 3 - 红侄, 对sibling旋转
					 *
					 *   (p)           (p)
					 *   / \           / \
					 *  N   S    -->  N   Sl
					 *     / \             \
					 *    sl  Sr            s
					 *                       \
					 *                        Sr
					 * 
					 * 以下由Case 4执行
					 * 
					 * 	 (p)             (sl)
					 *   / \             /  \
					 *  N   Sl   -->    P    S
					 *       \         /      \
					 *        s       N        sr
					 *         \
					 *          Sr
					 */

					rb_set_red(other);
					if(other->rb_left) {
						rb_set_black(other->rb_left);
					}
					__rb_rotate_right(other, root);

					// 关注节点不变
					other = parent->rb_right;		// 修改关注节点的兄弟为sl
				}
				
				// Case-3转换而来 或者 右侧为红(因为左侧无所谓, 所以不描述)

				/**
				 * Case 4 - 对p进行旋转并反转颜色
				 * 			s获得p的颜色, p变为黑色, sl不变
				 *
				 *      (p)             (s)`
				 *      / \             / \
				 *     N   S     -->   P   Sr
				 *        / \         / \
				 *      (sl) sr      N  (sl)
				 * 
				 */
				rb_set_color(other, parent);
				rb_set_black(parent);
				if(other->rb_right) {
					rb_set_black(other->rb_right);
				}
				__rb_rotate_left(parent, root);

				// 若出现这种情况, 代表整颗树已经平衡, 直接退出
				node = root->rb_node;
				break;
			}
		}
		else
		{
			other = parent->rb_left;
			
			// 红兄
			if(rb_is_red(other))
			{
				/* Case 1 - 在parent处右旋 */
				rb_set_red(parent);
				rb_set_black(other);
				__rb_rotate_right(parent, root);
				
				// 关注节点不变
				other = parent->rb_left;
			}
			
			// 黑兄红侄, 需分情况哪个是红
			if((!other->rb_left || rb_is_black(other->rb_left))
				&& (!other->rb_right || rb_is_black(other->rb_right)))
			{
				/* Case 2 - 转换sibling的颜色 */
				rb_set_red(other);
				
				// 上移黑高
				node = parent;
				parent = node->rb_parent;
			}
			// 黑兄红侄, 需判断哪一个是红色
			else
			{
				if(!other->rb_left || rb_is_black(other->rb_left))
				{
					/* Case 3 - 在sibling处右旋 */
					rb_set_red(other);
					if(other->rb_right) {
						rb_set_black(other->rb_right);
					}
					__rb_rotate_left(other, root);
					
					other = parent->rb_left;
				}

				/* Case 4 - 在parent处右旋并转换颜色 */
				rb_set_color(other, parent);
				rb_set_black(parent);
				if(other->rb_left) {
					rb_set_black(other->rb_left);
				}
				__rb_rotate_right(parent, root);

				// 若出现这种情况, 代表整颗树已经平衡, 直接退出
				node = root->rb_node;
				break;
			}
		}
	}

	// 保证 2.
	if(node) {
		rb_set_black(node);
	}
}

// 取node的下一个节点
rb_node* rb_next(rb_node* node)
{
	// node的下一个节点是其右子树的最左节点

	if(node->rb_right) {
		node = node->rb_right;
		while(node->rb_left) {
			node = node->rb_left;
		}
		return node;
	}

	// 若没有右子树, 需判断node是parent的左孩子还是左孩子
	//  若node是parent的左孩子, 那么parent就是node的下一个节点
	//  若node是parent的右孩子, 那么node的下一个节点是: 沿着父节点向上走, 碰到的第一个可以将node划分到左子树的节点
	while(node->rb_parent && node == node->rb_parent->rb_right) {
		node = node->rb_parent;
	}

	return node->rb_parent;
}

// 取node的前一个节点
rb_node* rb_prev(rb_node* node)
{
	// 左子树的最右边
	if(node->rb_left) {
		node = node->rb_left;
		while(node->rb_right) {
			node = node->rb_right;
		}
		return node;
	}

	// 若没有左子树, 需判断node是parent的左孩子还是右孩子
	//  若node是parent的右孩子, 那么parent就是node的前一个节点
	//  若node是parent的左孩子, 那么node的前一个节点是: 沿着父节点向上走, 碰到的第一个可以将node划分到右子树的节点
	while(node->rb_parent && node == node->rb_parent->rb_left) {
		node = node->rb_parent;
	}

	return node->rb_parent;
}

// 取树中第一个节点
rb_node* rb_first(rb_root* root)
{
	rb_node* node = root->rb_node;

	if(!node) {
		return NULL;
	}

	// 一直向左走
	while(node->rb_left) {
		node = node->rb_left;
	}
	return node;
}