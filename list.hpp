#ifndef LIST_HPP
# define LIST_HPP

#include "header.hpp"

#define OFFSETOF(member, type) &(((type*)0)->member)

#define LIST_ENTRY(list_ptr, member, type) (type*) ((type*)((char*)(list_ptr) - (char*)OFFSETOF(member, type)))

#define LIST_HEAD(head, member, type) LIST_ENTRY(head.next, member, type)
#define LIST_TAIL(head, member, type) LIST_ENTRY(head.prev, member, type)

#define LIST_FOREACH(it, head) for ((it) = (head)->next; (it) != (head); (it) = (it)->next)
#define LIST_FOREACH_SAFE(it, tmp, head) for ((it) = (head)->next, (tmp) = (it)->next; (it) != (head); (it) = (tmp), (tmp) = (it)->next)

#define LIST_FOREACH_ENTRY(it, head, member) for (it = LIST_ENTRY((head)->next, member, typeof(*(it))); &it->member != (head); it = LIST_ENTRY(it->member.next,member, typeof(*(it))))


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
    int singular();
    void rotate();
// elt functions:
    void del();
};

#endif
