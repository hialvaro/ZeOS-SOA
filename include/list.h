#ifndef _LINUX_LIST_H
#define _LINUX_LIST_H

struct list_head {
	struct list_head *next, *prev;
};

/*
 * Simple doubly linked list implementation.
 *
 * SAMPLE USE:
 *   // Declare an uninitialized list named 'list'
 *   struct list_head list;
 *
 *   // Initialize the list 'list'
 *   INIT_LIST_HEAD( &list );
 *
 *   // Define structures to be inserted in the list
 *   struct element {
 *     int a;
 *     char b;
 *     struct list_head anchor; //This is the anchor in the list
 *     ...
 *   };
 *
 *   // Declare a new element to be inserted in the list
 *   struct element el;
 *
 *   // Add the new element to the list, using the element's anchor.
 *   list_add( &(el.anchor), &list );
 *
 *   // Get the first element of the list
 *   struct list_head * e = list_first( &list );
 *
 *   // Remove the selected element from the list
 *   list_del( e ); 
 *
 *   // Get the container of this list element
 *   struct element * realelement = list_entry( e, struct element, anchor );
 *   realelement->a = 0x666;
 *
 *   // Traverse  the list
 *   list_for_each( e, &list ) {
 *      // do whatever with 'e'
 *		struct element * realelement = list_entry( e, struct element, anchor );
 *		...
 *   }
 */


/**
 * INIT_LIST_HEAD - Initializes an empty list.
 * @head: list head to initialize
 */
void INIT_LIST_HEAD(struct list_head *head);

/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
void list_add(struct list_head *new, struct list_head *head);


/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
void list_add_tail(struct list_head *new, struct list_head *head);

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
void list_del(struct list_head *entry);

/**
 * list_is_last - tests whether @list is the last entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
int list_is_last(const struct list_head *list,
				const struct list_head *head);

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
int list_empty(const struct list_head *head);

/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
            ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
 * list_for_each	-	iterate over a list
 * @pos:	the &struct list_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_safe   -       iterate over a list safe against removal of list entry
 * @pos:        the &struct list_head to use as a loop counter.
 * @n:          another &struct list_head to use as temporary storage
 * @head:       the head for your list.
 */
#define list_for_each_safe(pos, n, head) \
        for (pos = (head)->next, n = pos->next; pos != (head); \
                pos = n, n = pos->next)

/**
 * list_first   -   returns the first list item
 * @head:   the head for your list.
 */
#define list_first(head)                \
         (head)->next

#endif /* _LINUX_LIST_H */
