#include "list.hpp"

list::list() : next(0), prev(0) { }

void list::init_head()
{
    this->next = this;
    this->prev = this;
}

void list::push(list *elt)
{
    elt->next = this->next;
    elt->prev = this;
    this->next->prev = elt;
    this->next = elt;
}

void list::push_back(list *elt)
{
    elt->next = this;
    elt->prev = this->prev;
    this->prev->next = elt;
    this->prev = elt;
}

void list::pop()
{
    list *tmp;

    tmp = this->next;
    this->next = tmp->next;
    this->next->prev = this;
    tmp->next = 0;
    tmp->prev = 0;
}

void list::pop_back()
{
    list *tmp;

    tmp = this->prev;
    this->prev = tmp->prev;
    this->prev->next = this;
    tmp->next = 0;
    tmp->prev = 0;
}

void list::del()
{
    this->prev->next = this->next;
    this->next->prev = this->prev;
}

size_t list::size()
{
    size_t size = 0;
    list *tmp;

    tmp = this->next;
    do
    {
        tmp = tmp->next;
        ++size;
    } while (tmp != this);
    return size;
}

int list::empty()
{
    return this == this->next;
}

int list::singular()
{
    return !this->empty() && this->next == this->prev;
}

void list::rotate()
{
    if (empty() || singular())
        return ;
    list *first;

    first = this->next;
    first->next->prev = this;
    this->prev->next = first;
    this->next = first->next;
    first->next = this;
    first->prev = this->prev;
    this->prev = first;
}
