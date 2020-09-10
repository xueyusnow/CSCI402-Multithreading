#include <stdio.h>
#include <stdlib.h>
#include "my402list.h"
#include "cs402.h"


int My402ListLength(My402List* list)
{
	return list->num_members;
}
int My402ListEmpty(My402List* list)
{
	if(list->num_members <= 0){
		return TRUE;
	}
	else
		return FALSE;
}
int My402ListAppend(My402List* list, void *obj)
{
	int something_happened = FALSE;
	My402ListElem *newelem = (My402ListElem*)malloc(sizeof(My402ListElem));
	newelem->obj = obj;
	if(My402ListEmpty(list)){
		list->anchor.next = newelem;
		newelem->prev = &list->anchor;
		newelem->next = &list->anchor;
		list->anchor.prev = newelem;
		list->num_members++;
		something_happened = TRUE;
	}
	else{
		My402ListElem *elemprev = My402ListLast(list);
		elemprev->next = newelem;
		newelem->prev = elemprev;
		newelem->next = &list->anchor;
		list->anchor.prev = newelem;
		list->num_members++;
		something_happened = TRUE;
	}
	return something_happened;
}
int My402ListPrepend(My402List* list, void *obj)
{
	int something_happened = FALSE;
	My402ListElem *newelem = (My402ListElem*)malloc(sizeof(My402ListElem));
	newelem->obj = obj;
	if(My402ListEmpty(list)){
		list->anchor.next = newelem;
		newelem->prev = &list->anchor;
		newelem->next = &list->anchor;
		list->anchor.prev = newelem;
		list->num_members++;
		something_happened = TRUE;
	}
	else{
		My402ListElem *elemnext = My402ListFirst(list);
		list->anchor.next = newelem;
		newelem->prev = &list->anchor;
		newelem->next = elemnext;
		elemnext->prev = newelem;
		list->num_members++;
		something_happened = TRUE;
	}
	return something_happened;
}
void My402ListUnlink(My402List* list, My402ListElem *elem)
{
	elem->prev->next = elem->next;
	elem->next->prev = elem->prev;
	free(elem);
	elem = NULL;
	list->num_members--;
}
void My402ListUnlinkAll(My402List* list)
{
	My402ListElem *elem;
	elem = My402ListFirst(list);
	while(elem!=NULL){
		My402ListUnlink(list, elem);
		elem = My402ListFirst(list);
	}
	list->num_members=0;
}
int My402ListInsertBefore(My402List* list, void *obj, My402ListElem *elem)
{
	int something_happened = FALSE;
	My402ListElem *newelem = (My402ListElem*)malloc(sizeof(My402ListElem));
	newelem->obj = obj;
	elem->prev->next = newelem;
	newelem->next = elem;
	newelem->prev = elem->prev;
	elem->prev = newelem;
	list->num_members++;
	something_happened = TRUE;
	return something_happened;
}
int My402ListInsertAfter(My402List* list, void *obj, My402ListElem *elem)
{
	int something_happened = FALSE;
	My402ListElem *newelem = (My402ListElem*)malloc(sizeof(My402ListElem));
	newelem->obj = obj;
	elem->next->prev = newelem;
	newelem->prev = elem;
	newelem->next = elem->next;
	elem->next = newelem;
	list->num_members++;
	something_happened = TRUE;
	return something_happened;
}
My402ListElem *My402ListFirst(My402List* list)
{
	if(My402ListEmpty(list))
		return NULL;
	else
		return list->anchor.next;
}
My402ListElem *My402ListLast(My402List* list)
{
	if(list->anchor.prev != &list->anchor)
		return list->anchor.prev;
	else
		return NULL;
}
My402ListElem *My402ListNext(My402List* list, My402ListElem *elem)
{
	if(elem == My402ListLast(list))
		return NULL;
	else
		return elem->next;
}
My402ListElem *My402ListPrev(My402List* list, My402ListElem *elem)
{
	if(elem == My402ListFirst(list))
		return NULL;
	else
		return elem->prev;
}
My402ListElem *My402ListFind(My402List* list, void *obj)
{
	My402ListElem *elem;
	for(elem = My402ListFirst(list); elem !=NULL; elem = My402ListNext(list, elem)){
		if(elem->obj == obj)
			return elem;
	}
	return NULL;
}
int My402ListInit(My402List* list)
{
	int something_happened = FALSE;
	list->anchor.next = &list->anchor;
	list->anchor.prev = &list->anchor;
	something_happened = TRUE;
	list->num_members=0;
	return something_happened;
}

