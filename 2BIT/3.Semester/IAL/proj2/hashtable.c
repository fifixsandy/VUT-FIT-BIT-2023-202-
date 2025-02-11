/*
 * Tabulka s rozptýlenými položkami
 *
 * S využitím datových typů ze souboru hashtable.h a připravených koster
 * funkcí implementujte tabulku s rozptýlenými položkami s explicitně
 * zretězenými synonymy.
 *
 * Při implementaci uvažujte velikost tabulky HT_SIZE.
 * 
 * Funkcie doplnil: xnovakf00
 * Datum poslednej upravy: 30/10/2024
 */

#include "hashtable.h"
#include <stdlib.h>
#include <string.h>

int HT_SIZE = MAX_HT_SIZE;

/*
 * Rozptylovací funkce která přidělí zadanému klíči index z intervalu
 * <0,HT_SIZE-1>. Ideální rozptylovací funkce by měla rozprostírat klíče
 * rovnoměrně po všech indexech. Zamyslete sa nad kvalitou zvolené funkce.
 */
int get_hash(char *key) {
  int result = 1;
  int length = strlen(key);
  for (int i = 0; i < length; i++) {
    result += key[i];
  }
  return (result % HT_SIZE);
}

/*
 * Inicializace tabulky — zavolá sa před prvním použitím tabulky.
 */
void ht_init(ht_table_t *table) {
  if(table == NULL){return;}
  for(int i = 0; i < HT_SIZE; i++)
  {
    (*table)[i] = NULL; // in initial state, every element should be empty -> pointing to NULL
  }
}

/*
 * Vyhledání prvku v tabulce.
 *
 * V případě úspěchu vrací ukazatel na nalezený prvek; v opačném případě vrací
 * hodnotu NULL.
 */
ht_item_t *ht_search(ht_table_t *table, char *key) {
  if(table == NULL){return NULL;}
  int hash = get_hash(key); 
  ht_item_t *item = (*table)[hash]; // access first item with this hash 
  while(item != NULL) // while there is a item with this hash
  {
    if(strcmp(key, item->key) == 0) // if the key matches 
    {
      return item;
    }
    else // current item is a synonym of the one we are looking for
    {
      item = item->next;
    }
  }
  return NULL; // we did not find item
}

/*
 * Vložení nového prvku do tabulky.
 *
 * Pokud prvek s daným klíčem už v tabulce existuje, nahraďte jeho hodnotu.
 *
 * Při implementaci využijte funkci ht_search. Pri vkládání prvku do seznamu
 * synonym zvolte nejefektivnější možnost a vložte prvek na začátek seznamu.
 */
void ht_insert(ht_table_t *table, char *key, float value) {
  if(table == NULL){return;}
  ht_item_t *existing = ht_search(table, key);
  if(existing != NULL) // item with this key already exists
  {
    existing->value = value; // rewrite value
    return;
  }

  ht_item_t *newItem = (ht_item_t *)malloc(sizeof(ht_item_t)); // create new item
  if(newItem == NULL){return;} // malloc fail check

  // equal to strdup
  newItem->key = malloc(strlen(key) + 1); // allocate memory for the key 
  if(newItem->key == NULL){free(newItem); return;} // check malloc fail
  strcpy(newItem->key, key); // copy the key                

  newItem->value = value;
  int hash = get_hash(key);
  newItem->next  = (*table)[hash]; // set the first of the synonyms list as next of new item
  (*table)[hash] = newItem; // set newItem as first of the synonyms list

  return;
}

/*
 * Získání hodnoty z tabulky.
 *
 * V případě úspěchu vrací funkce ukazatel na hodnotu prvku, v opačném
 * případě hodnotu NULL.
 *
 * Při implementaci využijte funkci ht_search.
 */
float *ht_get(ht_table_t *table, char *key) {
  if(table == NULL){return NULL;}
  ht_item_t *toFind = ht_search(table, key);
  if(toFind == NULL)
  {
    return NULL;
  }
  
  return &(toFind->value);
}

/*
 * Smazání prvku z tabulky.
 *
 * Funkce korektně uvolní všechny alokované zdroje přiřazené k danému prvku.
 * Pokud prvek neexistuje, funkce nedělá nic.
 *
 * Při implementaci NEPOUŽÍVEJTE funkci ht_search.
 */
void ht_delete(ht_table_t *table, char *key) {
  if(table == NULL){return;}
  int hash = get_hash(key);

  ht_item_t *toDelete     = (*table)[hash];
  ht_item_t *toDeletePrev = NULL;

  while(toDelete != NULL)
  {
    if(strcmp(toDelete->key, key) == 0)
    {
      break;
    }
    else
    {
      toDeletePrev = toDelete;
      toDelete     = toDelete->next;
    }
  }

  if(toDelete == NULL){return;} // no element with this key

  if(toDeletePrev != NULL) // toDelete is not first
  {
    toDeletePrev->next = toDelete->next; // reconnect the list of synonyms
  }
  else // toDelete was first
  {
    (*table)[hash] = toDelete->next; // set new first
  }

  free(toDelete->key);
  free(toDelete);
  return;
}

/*
 * Smazání všech prvků z tabulky.
 *
 * Funkce korektně uvolní všechny alokované zdroje a uvede tabulku do stavu po 
 * inicializaci.
 */
void ht_delete_all(ht_table_t *table) {
  if(table == NULL){return;}
  for(int i = 0; i < HT_SIZE; i++) // loop through all the cells
  {
    ht_item_t *toDelete = (*table)[i]; // get first of the list of synonyms
    while(toDelete != NULL) // while there is something to delete
    {
      ht_item_t *toDeleteNext = toDelete->next; // get next so we don't lose the list of synonyms
      free(toDelete->key);
      free(toDelete);
      toDelete = toDeleteNext;
    }
    (*table)[i] = NULL;
  }
}
