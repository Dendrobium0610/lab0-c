#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head =
        (struct list_head *) malloc(sizeof(struct list_head));

    if (head == NULL)
        return NULL;

    INIT_LIST_HEAD(head);

    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (l == NULL)
        return;

    element_t *node, *safe;
    list_for_each_entry_safe (node, safe, l, list) {
        q_release_element(node);
    }

    free(l);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (head == NULL)
        return false;

    element_t *new_node = NULL;

    while ((new_node = (element_t *) malloc(sizeof(element_t))) == NULL)
        ;

    while ((new_node->value = (char *) malloc(strlen(s) + 1)) == NULL)
        ;

    memcpy(new_node->value, s, strlen(s));
    new_node->value[strlen(s)] = '\0';

    list_add(&new_node->list, head);

    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (head == NULL)
        return false;

    element_t *new_node = NULL;

    while ((new_node = (element_t *) malloc(sizeof(element_t))) == NULL)
        ;

    while ((new_node->value = (char *) malloc(strlen(s) + 1)) == NULL)
        ;

    memcpy(new_node->value, s, strlen(s));
    new_node->value[strlen(s)] = '\0';

    list_add_tail(&new_node->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (head == NULL || list_empty(head))
        return NULL;

    element_t *head_node = list_entry(head->next, element_t, list);
    list_del(head->next);

    memcpy(sp, head_node->value, bufsize - 1);
    sp[bufsize - 1] = '\0';

    return head_node;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (head == NULL || list_empty(head))
        return NULL;

    element_t *tail_node = list_entry(head->prev, element_t, list);
    list_del(head->prev);

    memcpy(sp, tail_node->value, bufsize - 1);
    sp[bufsize - 1] = '\0';

    return tail_node;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (head == NULL || list_empty(head))
        return 0;

    int size = 0;

    struct list_head *node = NULL;
    list_for_each (node, head)
        size++;

    return size;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (head == NULL || list_empty(head))
        return false;

    struct list_head *front = head->next, *end = head->prev;
    while (front != end && (front->next != end && end->prev != front)) {
        front = front->next;
        end = end->prev;
    }
    struct list_head *target = front;
    list_del(target);
    element_t *target_node = list_entry(target, element_t, list);
    q_release_element(target_node);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (head == NULL || list_empty(head) || list_is_singular(head))
        return false;

    q_sort(head, false);

    struct list_head *node = head->next;

    while (node->next != head) {
        struct list_head *target = node, *cmp = target->next;
        bool marked = false;

        element_t *target_node = list_entry(target, element_t, list);
        element_t *cmp_node = list_entry(cmp, element_t, list);

        while (strcmp(target_node->value, cmp_node->value) == 0) {
            list_del(cmp);
            q_release_element(cmp_node);

            marked = true;
            cmp = target->next;
            cmp_node = list_entry(cmp, element_t, list);
        }

        node = target->next;
        if (marked) {
            list_del(target);
            q_release_element(target_node);
        }
    }

    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (head == NULL)
        return;

    struct list_head *node = head->next;

    while (node != head && node->next != head) {
        struct list_head *next = node->next->next;
        list_del(node);
        list_add_tail(node, next);
        node = next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (head == NULL)
        return;

    struct list_head *end = head->prev, *node = head->next;

    while (node != end) {
        struct list_head *next = node->next;
        list_del(node);
        list_add_tail(node, end->next);
        node = next;
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (k <= 0)
        return;

    int q_sz = q_size(head);
    struct list_head *node = head->next, *last = head->next;

    if (k > q_sz)
        return;

    while (q_sz >= k) {
        for (int index = 0; index < k - 1; index++) {
            last = last->next;
        }
        struct list_head *next_start = last->next;

        while (node != last) {
            struct list_head *next = node->next;
            list_move_tail(node, last->next);
            node = next;
        }
        last = next_start;
        node = next_start;
        q_sz -= k;
    }
}

static void merge(struct list_head *left,
                  struct list_head *right,
                  struct list_head *head,
                  bool descend)
{
    struct list_head *l = left->next;
    struct list_head *r = right->next;

    while (l != left && r != right) {
        element_t *left_node = list_entry(l, element_t, list);
        element_t *right_node = list_entry(r, element_t, list);
        struct list_head *target;

        if (descend)
            target = (strcmp(left_node->value, right_node->value) > 0) ? r : l;
        else
            target = (strcmp(left_node->value, right_node->value) < 0) ? r : l;
        struct list_head *next = target->next;
        list_del(target);
        list_add_tail(target, head);

        if (target == l)
            l = next;

        if (target == r)
            r = next;
    }

    struct list_head *list = (l != left) ? left : right;
    list_splice_tail(list, head);
}

static void mergeSort(struct list_head *head, bool descend)
{
    if (list_is_singular(head)) {
        return;
    }

    struct list_head *front = head->next, *end = head->prev;

    while (front != end && (front->next != end && end->prev != front)) {
        front = front->next;
        end = end->prev;
    }

    struct list_head *mid = front;
    LIST_HEAD(left);
    LIST_HEAD(right);
    list_cut_position(&left, head, mid);
    list_cut_position(&right, head, head->prev);
    INIT_LIST_HEAD(head);

    mergeSort(&left, descend);
    mergeSort(&right, descend);
    merge(&left, &right, head, descend);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (head == NULL || list_is_singular(head) || list_empty(head))
        return;

    mergeSort(head, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (head == NULL || list_empty(head))
        return 0;

    if (list_is_singular(head))
        return 1;

    struct list_head *node = head->next;

    while (node != head) {
        struct list_head *cmp = node->next, *bigger = NULL;
        while (cmp != head) {
            element_t *target_node = list_entry(node, element_t, list);
            element_t *cmp_node = list_entry(cmp, element_t, list);

            if (strcmp(target_node->value, cmp_node->value) < 0) {
                bigger = cmp;
                break;
            }
            cmp = cmp->next;
        }

        struct list_head *del = node;
        while (bigger != NULL && del != bigger) {
            element_t *del_node = list_entry(del, element_t, list);
            struct list_head *next = del->next;
            list_del(del);
            q_release_element(del_node);
            del = next;
        }

        node = (bigger != NULL) ? bigger : node->next;
    }

    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
