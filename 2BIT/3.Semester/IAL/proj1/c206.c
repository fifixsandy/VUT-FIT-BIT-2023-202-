/* ******************************* c206.c *********************************** */
/*  Předmět: Algoritmy (IAL) - FIT VUT v Brně                                 */
/*  Úkol: c206 - Dvousměrně vázaný lineární seznam                            */
/*  Návrh a referenční implementace: Bohuslav Křena, říjen 2001               */
/*  Vytvořil: Martin Tuček, říjen 2004                                        */
/*  Upravil: Kamil Jeřábek, září 2020                                         */
/*           Daniel Dolejška, září 2021                                       */
/*           Daniel Dolejška, září 2022                                       */
/* ************************************************************************** */
/*
** Implementujte abstraktní datový typ dvousměrně vázaný lineární seznam.
** Užitečným obsahem prvku seznamu je hodnota typu int. Seznam bude jako datová
** abstrakce reprezentován proměnnou typu DLList (DL znamená Doubly-Linked
** a slouží pro odlišení jmen konstant, typů a funkcí od jmen u jednosměrně
** vázaného lineárního seznamu). Definici konstant a typů naleznete
** v hlavičkovém souboru c206.h.
**
** Vaším úkolem je implementovat následující operace, které spolu s výše
** uvedenou datovou částí abstrakce tvoří abstraktní datový typ obousměrně
** vázaný lineární seznam:
**
**      DLL_Init ........... inicializace seznamu před prvním použitím,
**      DLL_Dispose ........ zrušení všech prvků seznamu,
**      DLL_InsertFirst .... vložení prvku na začátek seznamu,
**      DLL_InsertLast ..... vložení prvku na konec seznamu,
**      DLL_First .......... nastavení aktivity na první prvek,
**      DLL_Last ........... nastavení aktivity na poslední prvek,
**      DLL_GetFirst ....... vrací hodnotu prvního prvku,
**      DLL_GetLast ........ vrací hodnotu posledního prvku,
**      DLL_DeleteFirst .... zruší první prvek seznamu,
**      DLL_DeleteLast ..... zruší poslední prvek seznamu,
**      DLL_DeleteAfter .... ruší prvek za aktivním prvkem,
**      DLL_DeleteBefore ... ruší prvek před aktivním prvkem,
**      DLL_InsertAfter .... vloží nový prvek za aktivní prvek seznamu,
**      DLL_InsertBefore ... vloží nový prvek před aktivní prvek seznamu,
**      DLL_GetValue ....... vrací hodnotu aktivního prvku,
**      DLL_SetValue ....... přepíše obsah aktivního prvku novou hodnotou,
**      DLL_Previous ....... posune aktivitu na předchozí prvek seznamu,
**      DLL_Next ........... posune aktivitu na další prvek seznamu,
**      DLL_IsActive ....... zjišťuje aktivitu seznamu.
**
** Při implementaci jednotlivých funkcí nevolejte žádnou z funkcí
** implementovaných v rámci tohoto příkladu, není-li u funkce explicitně
** uvedeno něco jiného.
**
** Nemusíte ošetřovat situaci, kdy místo legálního ukazatele na seznam
** předá někdo jako parametr hodnotu NULL.
**
** Svou implementaci vhodně komentujte!
**
** Terminologická poznámka: Jazyk C nepoužívá pojem procedura.
** Proto zde používáme pojem funkce i pro operace, které by byly
** v algoritmickém jazyce Pascalovského typu implemenovány jako procedury
** (v jazyce C procedurám odpovídají funkce vracející typ void).
**
** xnovakf00
**
**/

#include "c206.h"

bool error_flag;
bool solved;

/**
 * Vytiskne upozornění na to, že došlo k chybě.
 * Tato funkce bude volána z některých dále implementovaných operací.
 */
void DLL_Error(void) {
	printf("*ERROR* The program has performed an illegal operation.\n");
	error_flag = true;
}

/**
 * Provede inicializaci seznamu list před jeho prvním použitím (tzn. žádná
 * z následujících funkcí nebude volána nad neinicializovaným seznamem).
 * Tato inicializace se nikdy nebude provádět nad již inicializovaným seznamem,
 * a proto tuto možnost neošetřujte.
 * Vždy předpokládejte, že neinicializované proměnné mají nedefinovanou hodnotu.
 *
 * @param list Ukazatel na strukturu dvousměrně vázaného seznamu
 */
void DLL_Init( DLList *list ) {
	list->activeElement = NULL;
	list->firstElement	= NULL;
	list->lastElement	= NULL;
	list->currentLength	= 0;
}

/**
 * Zruší všechny prvky seznamu list a uvede seznam do stavu, v jakém se nacházel
 * po inicializaci.
 * Rušené prvky seznamu budou korektně uvolněny voláním operace free.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Dispose( DLList *list ) {

	if(list->firstElement == NULL) // empty list, nothing to delete
	{
		return;
	}

	DLLElementPtr curr = list->firstElement; // element to delete
	while(curr != NULL) // while the list is nonempty
	{
		DLLElementPtr next = curr->nextElement; // set the next to delete
		free(curr); 
		curr = next; 
		list->currentLength--; // in the end, it should be 0, if not, some error occured
	}
	
	// set all pointers to NULL
	list->activeElement = NULL;
	list->firstElement  = NULL;
	list->lastElement	= NULL;
}

/**
 * Vloží nový prvek na začátek seznamu list.
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení na začátek seznamu
 */
void DLL_InsertFirst( DLList *list, long data ) {
	DLLElementPtr newElement = malloc(sizeof(struct DLLElement));
	if(newElement == NULL)
	{
		DLL_Error();
		return;
	}

	newElement->data 			= data;
	newElement->previousElement = NULL;
	newElement->nextElement 	= list->firstElement; // set the previous first as next, if the list was empty, it will be NULL

	if(list->firstElement != NULL)	// for nonempty list
	{
		list->firstElement->previousElement = newElement;
	}
	else
	{
		list->lastElement = newElement; // in one-element list first == last
	}

	list->firstElement = newElement;
	list->currentLength++;

	return;
}

/**
 * Vloží nový prvek na konec seznamu list (symetrická operace k DLL_InsertFirst).
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení na konec seznamu
 */
void DLL_InsertLast( DLList *list, long data ) {
	DLLElementPtr newElement = malloc(sizeof(struct DLLElement));
	if(newElement == NULL)
	{
		DLL_Error();
		return;
	}

	newElement->data 			= data;
	newElement->previousElement = list->lastElement;
	newElement->nextElement 	= NULL;

	if(list->firstElement != NULL)	
	{
		list->lastElement->nextElement = newElement;
	}
	else
	{
		list->firstElement = newElement; 
	}

	list->lastElement = newElement;
	list->currentLength++;

	return;
}

/**
 * Nastaví první prvek seznamu list jako aktivní.
 * Funkci implementujte jako jediný příkaz, aniž byste testovali,
 * zda je seznam list prázdný.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_First( DLList *list ) {
	list->activeElement = list->firstElement;
}

/**
 * Nastaví poslední prvek seznamu list jako aktivní.
 * Funkci implementujte jako jediný příkaz, aniž byste testovali,
 * zda je seznam list prázdný.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Last( DLList *list ) {
	list->activeElement = list->lastElement;
}

/**
 * Prostřednictvím parametru dataPtr vrátí hodnotu prvního prvku seznamu list.
 * Pokud je seznam list prázdný, volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param dataPtr Ukazatel na cílovou proměnnou
 */
void DLL_GetFirst( DLList *list, long *dataPtr ) {
	if(list->firstElement == NULL)
	{
		DLL_Error();
		return;
	}

	*dataPtr = list->firstElement->data;
}

/**
 * Prostřednictvím parametru dataPtr vrátí hodnotu posledního prvku seznamu list.
 * Pokud je seznam list prázdný, volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param dataPtr Ukazatel na cílovou proměnnou
 */
void DLL_GetLast( DLList *list, long *dataPtr ) {
	if(list->firstElement == NULL)
	{
		DLL_Error();
		return;
	}

	*dataPtr = list->lastElement->data;
}

/**
 * Zruší první prvek seznamu list.
 * Pokud byl první prvek aktivní, aktivita se ztrácí.
 * Pokud byl seznam list prázdný, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteFirst( DLList *list ) {
	if(list->firstElement == NULL) // empty list
	{
		return;
	}

	if(list->activeElement == list->firstElement) // delete activity
	{
		list->activeElement = NULL;
	}

	if(list->firstElement == list->lastElement) // one-element list where first == last
	{
		free(list->firstElement);
		list->firstElement = NULL;
		list->lastElement  = NULL;
	}
	else
	{
		DLLElementPtr tmp = list->firstElement;	// hide the first one to be freed later
		list->firstElement = list->firstElement->nextElement; // set new first element

		list->firstElement->previousElement = NULL; // delete the previous of NEW first
		free(tmp);
	}

	list->currentLength--;
}

/**
 * Zruší poslední prvek seznamu list.
 * Pokud byl poslední prvek aktivní, aktivita seznamu se ztrácí.
 * Pokud byl seznam list prázdný, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteLast( DLList *list ) {
	if(list->firstElement == NULL) // empty list
	{
		return;
	}

	if(list->activeElement == list->lastElement) // delete activity
	{
		list->activeElement = NULL;
	}

	if(list->firstElement == list->lastElement) // one-element list where first == last
	{
		free(list->lastElement);
		list->firstElement = NULL;
		list->lastElement  = NULL;
	}
	else
	{
		DLLElementPtr tmp = list->lastElement;	// hide the last one to be freed later
		list->lastElement = list->lastElement->previousElement; // set new last element

		list->lastElement->nextElement = NULL; // delete the next of NEW last
		free(tmp);
	}

	list->currentLength--;
}

/**
 * Zruší prvek seznamu list za aktivním prvkem.
 * Pokud je seznam list neaktivní nebo pokud je aktivní prvek
 * posledním prvkem seznamu, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteAfter( DLList *list ) {
	if(list->activeElement == NULL || list->activeElement == list->lastElement) // nothing to delete
	{
		return;
	}

	DLLElementPtr tmp = list->activeElement->nextElement; // store element to delete before manipulating previous and next

	if(tmp == list->lastElement)
	{
		list->lastElement = list->activeElement; // set the last element to active if the one to delete is last
	}
	else
	{
		tmp->nextElement->previousElement = list->activeElement; // connect the previous and next elements of the one to delete
	}                                                            //                           |
                                                                 //                           |
	list->activeElement->nextElement     = tmp->nextElement;     //                          -|-
	free(tmp);
	list->currentLength--;
}

/**
 * Zruší prvek před aktivním prvkem seznamu list .
 * Pokud je seznam list neaktivní nebo pokud je aktivní prvek
 * prvním prvkem seznamu, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteBefore( DLList *list ) {
	if(list->activeElement == NULL || list->activeElement == list->firstElement) // nothing to delete
	{
		return;
	}

	DLLElementPtr tmp = list->activeElement->previousElement; // store element to delete before manipulating previous and next
	if(tmp == list->firstElement)
	{
		list->firstElement = list->activeElement; // set the first element to active if the one to delete is first
	}
	else
	{
		tmp->previousElement->nextElement = list->activeElement; // connect the previous and next elements of the one to delete
	}                                                            //                           |
                                                                 //                           |
	list->activeElement->previousElement  = tmp->previousElement;//                          -|-
	free(tmp);
	list->currentLength--;
}

/**
 * Vloží prvek za aktivní prvek seznamu list.
 * Pokud nebyl seznam list aktivní, nic se neděje.
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení do seznamu za právě aktivní prvek
 */
void DLL_InsertAfter( DLList *list, long data ) {

	if(list->activeElement == NULL)
	{
		return;
	}

	DLLElementPtr newElement = malloc(sizeof(struct DLLElement));
	if(newElement == NULL)
	{
		DLL_Error();
		return;
	}

	newElement->data = data;
	newElement->nextElement = list->activeElement->nextElement; // will be NULL if the new element is last
	newElement->previousElement = list->activeElement;

	if(list->activeElement != list->lastElement) // if the new element won't be last
	{
		newElement->nextElement->previousElement = newElement; // set previous of the element that will follow the new one
	}
	else
	{
		list->lastElement = newElement; // set the last pointer to the new element
	}

	list->activeElement->nextElement = newElement;

	list->currentLength++;
}

/**
 * Vloží prvek před aktivní prvek seznamu list.
 * Pokud nebyl seznam list aktivní, nic se neděje.
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení do seznamu před právě aktivní prvek
 */
void DLL_InsertBefore( DLList *list, long data ) {
	if(list->activeElement == NULL)
	{
		return;
	}

	DLLElementPtr newElement = malloc(sizeof(struct DLLElement));
	if(newElement == NULL)
	{
		DLL_Error();
		return;
	}

	newElement->data = data;
	newElement->previousElement = list->activeElement->previousElement; // will be NULL if the new element is first
	newElement->nextElement = list->activeElement;

	if(list->activeElement != list->firstElement) // if the new element won't be first
	{
		newElement->previousElement->nextElement = newElement; // set next of the element that will precede the new one
	}
	else
	{
		list->firstElement = newElement; // set the first pointer to the new element
	}

	list->activeElement->previousElement = newElement;

	list->currentLength++;
}

/**
 * Prostřednictvím parametru dataPtr vrátí hodnotu aktivního prvku seznamu list.
 * Pokud seznam list není aktivní, volá funkci DLL_Error ().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param dataPtr Ukazatel na cílovou proměnnou
 */
void DLL_GetValue( DLList *list, long *dataPtr ) {
	if(list->activeElement == NULL)
	{
		DLL_Error();
		return;
	}

	*dataPtr = list->activeElement->data;
}

/**
 * Přepíše obsah aktivního prvku seznamu list.
 * Pokud seznam list není aktivní, nedělá nic.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Nová hodnota právě aktivního prvku
 */
void DLL_SetValue( DLList *list, long data ) {
	if(list->activeElement == NULL)
	{
		return;
	}

	list->activeElement->data = data;
}

/**
 * Posune aktivitu na následující prvek seznamu list.
 * Není-li seznam aktivní, nedělá nic.
 * Všimněte si, že při aktivitě na posledním prvku se seznam stane neaktivním.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Next( DLList *list ) {
	if(list->activeElement == NULL)
	{
		return;
	}

	list->activeElement = list->activeElement->nextElement;
}


/**
 * Posune aktivitu na předchozí prvek seznamu list.
 * Není-li seznam aktivní, nedělá nic.
 * Všimněte si, že při aktivitě na prvním prvku se seznam stane neaktivním.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Previous( DLList *list ) {
	if(list->activeElement == NULL)
	{
		return;
	}

	list->activeElement = list->activeElement->previousElement;
}

/**
 * Je-li seznam list aktivní, vrací nenulovou hodnotu, jinak vrací 0.
 * Funkci je vhodné implementovat jedním příkazem return.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 *
 * @returns Nenulovou hodnotu v případě aktivity prvku seznamu, jinak nulu
 */
bool DLL_IsActive( DLList *list ) {
	return(list->activeElement != NULL);
}

/* Konec c206.c */
