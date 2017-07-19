#ifndef LIST_HPP
# define LIST_HPP

#include "header.hpp"

#define OFFSETOF(member, type) &(((type*)0)->member)

#define LIST_ENTRY(list_ptr, member, type) (type*) ((type*)((char*)(list_ptr) - (char*)OFFSETOF(member, type)))

#define LIST_HEAD(head, member, type) LIST_ENTRY(head.next, member, type)
#define LIST_TAIL(head, member, type) LIST_ENTRY(head.prev, member, type)

#define LIST_FOREACH(elt, head) for ((elt) = (head)->next; (elt) != (head); (elt) = (elt)->next)
#define LIST_FOREACH_SAFE(elt, tmp, head) for ((elt) = (head)->next, (tmp) = (elt)->next; (elt) != (head); (elt) = (tmp), (tmp) = (elt)->next)

#define LIST_FOREACH_ENTRY(elt, head, member) for (elt = LIST_ENTRY((head)->next, member, typeof(*elt)); &elt->member != (head); elt = LIST_ENTRY(elt->member.next,member, typeof(*elt)))


class list
{
public:
    list();
    list *next, *prev;
// head functions:
    void init_head();
    void push(list *elt);
    void push_back(list *elt);
    void pop();
    void pop_back();
    size_t size();
    int empty();
// elt functions:
    void del();
};

#endif
