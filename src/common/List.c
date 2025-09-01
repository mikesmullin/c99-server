#pragma once

#include "../unity.h"  // IWYU pragma: keep

// ---
// Linked List

void List__reset(List* list) {
  list->len = 0;
  list->head = NULL;
}

List* List__alloc(Arena* arena) {
  List* list = (List*)Arena__push(arena, sizeof(List));
  List__reset(list);
  return list;
}

// @deprecated use ListIt__each()
// iterator V1 (for...loop)
bool List__each(List__Node** node, const List* list) {
  if (NULL == list) {
    return false;  // invalid list!
  }
  if (NULL == *node) {
    *node = list->head;  // begin()
    if (NULL == *node) {
      return false;  // empty list
    }
    return true;  // first node
  }
  if (NULL == (*node)->next) {
    return false;  // end()
  }
  *node = (List__Node*)(*node)->next;  // ++()
  return true;  // next node
}

// iterator V2 (simple while...loop)
// usage:
//   ListIt it = {list};
//   while(ListIt__each(&it)) { ... }
bool ListIt__each(ListIt* it) {
  bool r = List__each(&it->node, it->list);
  it->i = it->node == it->list->head ? 0 : it->i + 1;
  return r;
}

void* List__shift(List* list) {
  List__Node* c = list->head;
  if (0 == list->len)
    return NULL;
  if (1 == list->len) {
    list->head = list->tail = NULL;
    list->len = 0;
    return c->data;
  }
  list->head = c->next;
  list->len--;
  return c->data;
}

void List__prepend(Arena* arena, List* list, void* data) {
  List__Node* node = (List__Node*)Arena__push(arena, sizeof(List__Node));
  node->data = data;
  node->next = NULL;

  if (0 == list->len) {
    list->head = node;
    list->tail = node;
  } else {
    node->next = list->head;
    list->head = node;
  }
  list->len++;
}

void List__append(Arena* arena, List* list, void* data) {
  List__Node* node = (List__Node*)Arena__push(arena, sizeof(List__Node));
  node->data = data;
  node->next = NULL;

  if (0 == list->len) {
    list->head = node;
    list->tail = node;
  } else {
    list->tail->next = node;
    list->tail = node;
  }
  list->len++;
}

void* List__get(List* list, u32 index) {
  List__Node* c = list->head;
  if (index >= list->len) {
    return NULL;
  }
  for (u32 i = 0; i < index; i++) {
    c = c->next;
  }
  return c->data;
}

bool List__remove_item(List* list, void* data) {
  List__Node* c = list->head;

  // special case for head
  if (c->data == data) {
    list->head = c->next;
    list->len--;
    return true;
  }

  for (u32 i = 0; i < list->len; i++) {
    if (c->next->data == data) {
      c->next = c->next->next;
      list->len--;
      if (NULL == c->next) {
        list->tail = c;
      }
      return true;
    }
    c = c->next;
  }
  return false;
}

void* List__pop(List* list) {
  List__Node* c = list->head;
  List__Node* prev;
  if (0 == list->len) {
    return NULL;
  }
  if (1 == list->len) {
    list->head = list->tail = NULL;
    list->len = 0;
    return c->data;
  }
  for (u32 i = 0; i < list->len; i++) {
    if (i == list->len - 1) {
      list->tail = prev;
      list->tail->next = NULL;
      list->len--;  // update
      return c->data;  // return last
    }
    prev = c;
    c = c->next;
  }
  return NULL;
}

// insert in sorted position
void List__insort(Arena* arena, List* list, void* data, List__sorter_t sortCb) {
  List__Node* node = (List__Node*)Arena__push(arena, sizeof(List__Node));
  node->data = data;
  node->next = NULL;

  // special case for first node
  if (NULL == list->head) {
    list->head = node;
    list->tail = node;
    list->len++;
    return;
  }

  // locate the node before the point of insertion
  List__Node* c = list->head;
  // while data is deeper than head (-Z_FWD = head -Z, tail +Z)
  while (NULL != c->next && sortCb(node->data, c->next->data) == -1) {  // -1 a<b, 0 a==b, +1 a>b
    c = c->next;
  }

  // insert
  node->next = c->next;
  c->next = node;
  if (list->tail == c) {
    list->tail = node;
  }
  list->len++;
}

bool List__has_item(List* list, void* data) {
  for (List__Node* n = NULL; List__each(&n, list);) {
    if (n->data == data) {
      return true;
    }
  }
  return false;
}

bool List__replace_idx(List* list, u32 idx, void* replace) {
  u32 i = 0;
  for (List__Node* n = NULL; List__each(&n, list); i++) {
    if (i == idx) {
      n->data = replace;
      return true;
    }
  }
  return false;
}

bool List__replace_item(List* list, void* search, void* replace) {
  for (List__Node* n = NULL; List__each(&n, list);) {
    if (n->data == search) {
      n->data = replace;
      return true;
    }
  }
  return false;
}

// merge two lists into a new list
List* List__merge(Arena* arena, List* a, List* b) {
  List* dst = List__alloc(arena);
  ListIt itA = {a}, itB = {b};
  while (ListIt__each(&itA)) {
    List__append(arena, (List*)dst, itA.node->data);
  }
  while (ListIt__each(&itB)) {
    List__append(arena, (List*)dst, itB.node->data);
  }
  return dst;
}