/*
 * Binární vyhledávací strom — iterativní varianta
 *
 * S využitím datových typů ze souboru btree.h, zásobníku ze souboru stack.h
 * a připravených koster funkcí implementujte binární vyhledávací
 * strom bez použití rekurze.
 * Funkcie doplnil: xnovakf00
 * Datum poslednej upravy: 30/10/2024
 */

#include "../btree.h"
#include "stack.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Inicializace stromu.
 *
 * Uživatel musí zajistit, že inicializace se nebude opakovaně volat nad
 * inicializovaným stromem. V opačném případě může dojít k úniku paměti (memory
 * leak). Protože neinicializovaný ukazatel má nedefinovanou hodnotu, není
 * možné toto detekovat ve funkci.
 */
void bst_init(bst_node_t **tree)
{
  *tree = NULL;
}

/*
 * Vyhledání uzlu v stromu.
 *
 * V případě úspěchu vrátí funkce hodnotu true a do proměnné value zapíše
 * ukazatel na obsah daného uzlu. V opačném případě funkce vrátí hodnotu false a proměnná
 * value zůstává nezměněná.
 *
 * Funkci implementujte iterativně bez použité vlastních pomocných funkcí.
 */
bool bst_search(bst_node_t *tree, char key, bst_node_content_t **value)
{
  bst_node_t *root = tree;
  bool search      = false;
  bool done        = (root == NULL);

  while(!done)
  {
    if(root->key == key)
    {
      search = true;
      done   = true;
      *value  = &root->content;
    }
    else if(root->key > key)
    {
      root = root->left;
    }
    else
    {
      root = root->right;
    }
    if(root == NULL)
    {
      done = true;
    }
  }
  return search;
}

/*
 * Vložení uzlu do stromu.
 *
 * Pokud uzel se zadaným klíče už ve stromu existuje, nahraďte jeho hodnotu.
 * Jinak vložte nový listový uzel.
 *
 * Výsledný strom musí splňovat podmínku vyhledávacího stromu — levý podstrom
 * uzlu obsahuje jenom menší klíče, pravý větší.
 *
 * Funkci implementujte iterativně bez použití vlastních pomocných funkcí.
 */
void bst_insert(bst_node_t **tree, char key, bst_node_content_t value)
{
  if(tree == NULL){return;}
  bst_node_t *root = *tree;
  bool found       = false;
  bst_node_t *pos  = NULL;

  if(root != NULL)
  {
    do
    {
      pos = root;
      if(root->key > key) // going left
      {
        root = root->left;
      }
      else if(root->key < key) // going right
      {
        root = root->right;
      }
      else // found
      {
        found = true;
      }
    } while (!found && root!=NULL); // repeat until end of tree or found

    if(found)
    {
      free(pos->content.value); // free previous value to prevent mem leak
      pos->content = value;     // rewrite value
    }
    else // creating new node
    {
      bst_node_t *newNode = (bst_node_t *)malloc(sizeof(bst_node_t));
      if(newNode == NULL){return;}
      newNode->key     = key;
      newNode->content = value;
      newNode->left    = NULL;
      newNode->right   = NULL;

      if(pos->key > key) // key of new added is smaller as its parent, set it to left child
      {
        pos->left = newNode;
      }
      else // key of new added is larger as its parent, set it to right child
      {
        pos->right = newNode;
      }

    }
  }
  else // there was no root, adding root
  {
    bst_node_t *newNode = (bst_node_t *)malloc(sizeof(bst_node_t));
    if(newNode == NULL){return;}
    newNode->key     = key;
    newNode->content = value;
    newNode->left    = NULL;
    newNode->right   = NULL;
    *tree            = newNode;
  }
}

/*
 * Pomocná funkce která nahradí uzel nejpravějším potomkem.
 *
 * Klíč a hodnota uzlu target budou nahrazené klíčem a hodnotou nejpravějšího
 * uzlu podstromu tree. Nejpravější potomek bude odstraněný. Funkce korektně
 * uvolní všechny alokované zdroje odstraněného uzlu.
 *
 * Funkce předpokládá, že hodnota tree není NULL.
 *
 * Tato pomocná funkce bude využita při implementaci funkce bst_delete.
 *
 * Funkci implementujte iterativně bez použití vlastních pomocných funkcí.
 */
void bst_replace_by_rightmost(bst_node_t *target, bst_node_t **tree)
{
  while((*tree)->right != NULL) // looping until we find the rightmost
  {
    tree = &(*tree)->right;
  }
  bst_node_t *toDelete = *tree;
  free(target->content.value);
  target->content = toDelete->content;
  target->key     = toDelete->key;
  *tree           = toDelete->left;
  free(toDelete);
}

/*
 * Odstranění uzlu ze stromu.
 *
 * Pokud uzel se zadaným klíčem neexistuje, funkce nic nedělá.
 * Pokud má odstraněný uzel jeden podstrom, zdědí ho rodič odstraněného uzlu.
 * Pokud má odstraněný uzel oba podstromy, je nahrazený nejpravějším uzlem
 * levého podstromu. Nejpravější uzel nemusí být listem.
 *
 * Funkce korektně uvolní všechny alokované zdroje odstraněného uzlu.
 *
 * Funkci implementujte iterativně pomocí bst_replace_by_rightmost a bez
 * použití vlastních pomocných funkcí.
 */
void bst_delete(bst_node_t **tree, char key)
{
  if(tree == NULL){return;}
  bool found = false;

  /* below is the logic of finding the node to delete 
   (if search returned a pointer to the node, this could be replaced with it) */
  while(*tree != NULL && !found)
  {
    if((*tree)->key == key)
    {
      found = true;
    }
    else if((*tree)->key > key)
    {
      tree = &(*tree)->left;
    }
    else
    {
      tree = &(*tree)->right;
    }
  }
  
  if(!found){return;} // nothing to delete, node with this key does not exist

  // below is the deletion logic
  if((*tree)->left == NULL && (*tree)->right == NULL) // no subtree
  {
    free((*tree)->content.value);
    free((*tree));
    *tree = NULL;
  }
  else if((*tree)->left != NULL && (*tree)->right != NULL) // both subtrees
  {
    bst_replace_by_rightmost(*tree, &(*tree)->left);
  }
  else
  {
    bst_node_t *toDelete = *tree;
    if((*tree)->right != NULL) // right child present
    {
      *tree = (*tree)->right;
    }
    else // left child present
    {
      *tree = (*tree)->left;
    }
    free(toDelete->content.value);
    free(toDelete);
  }
}

/*
 * Zrušení celého stromu.
 *
 * Po zrušení se celý strom bude nacházet ve stejném stavu jako po
 * inicializaci. Funkce korektně uvolní všechny alokované zdroje rušených
 * uzlů.
 *
 * Funkci implementujte iterativně s pomocí zásobníku a bez použití
 * vlastních pomocných funkcí.
 */
void bst_dispose(bst_node_t **tree)
{
  stack_bst_t stack;
  stack_bst_init(&stack);

  do
  {
    if((*tree) == NULL)
    {
      if(!stack_bst_empty(&stack))
      {
        *tree = stack_bst_top(&stack);
        stack_bst_pop(&stack);
      }
    }
    else
    {
      if((*tree)->right != NULL)
      {
        stack_bst_push(&stack, (*tree)->right);
      }
      bst_node_t *toDelete = *tree;
      *tree = (*tree)->left;
      free(toDelete->content.value);
      free(toDelete);
    }
  }while((*tree) != NULL || !stack_bst_empty(&stack));
}

/*
 * Pomocná funkce pro iterativní preorder.
 *
 * Prochází po levé větvi k nejlevějšímu uzlu podstromu.
 * Nad zpracovanými uzly zavolá bst_add_node_to_items a uloží je do zásobníku uzlů.
 *
 * Funkci implementujte iterativně s pomocí zásobníku a bez použití
 * vlastních pomocných funkcí.
 */
void bst_leftmost_preorder(bst_node_t *tree, stack_bst_t *to_visit, bst_items_t *items)
{
  bst_node_t *nodePtr = tree;
  while(nodePtr != NULL)
  {
    stack_bst_push(to_visit, nodePtr);
    bst_add_node_to_items(nodePtr, items);
    nodePtr = nodePtr->left;
  }
}

/*
 * Preorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte iterativně pomocí funkce bst_leftmost_preorder a
 * zásobníku uzlů a bez použití vlastních pomocných funkcí.
 */
void bst_preorder(bst_node_t *tree, bst_items_t *items)
{
  stack_bst_t stack;
  stack_bst_init(&stack);
  bst_leftmost_preorder(tree, &stack, items);
  bst_node_t *nodePtr = NULL;
  while(!stack_bst_empty(&stack))
  {
    nodePtr = stack_bst_top(&stack);
    stack_bst_pop(&stack);
    bst_leftmost_preorder(nodePtr->right, &stack, items);
  }
}

/*
 * Pomocná funkce pro iterativní inorder.
 *
 * Prochází po levé větvi k nejlevějšímu uzlu podstromu a ukládá uzly do
 * zásobníku uzlů.
 *
 * Funkci implementujte iterativně s pomocí zásobníku a bez použití
 * vlastních pomocných funkcí.
 */
void bst_leftmost_inorder(bst_node_t *tree, stack_bst_t *to_visit)
{
  bst_node_t *nodePtr = tree;
  while(nodePtr != NULL)
  {
    stack_bst_push(to_visit, nodePtr);
    nodePtr = nodePtr->left;
  }
}

/*
 * Inorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte iterativně pomocí funkce bst_leftmost_inorder a
 * zásobníku uzlů a bez použití vlastních pomocných funkcí.
 */
void bst_inorder(bst_node_t *tree, bst_items_t *items)
{
  stack_bst_t stack;
  stack_bst_init(&stack);
  bst_leftmost_inorder(tree, &stack);
  bst_node_t *nodePtr = NULL;
  while(!stack_bst_empty(&stack))
  {
    nodePtr = stack_bst_top(&stack);
    stack_bst_pop(&stack);
    bst_add_node_to_items(nodePtr, items);
    bst_leftmost_inorder(nodePtr->right, &stack);
  }
}

/*
 * Pomocná funkce pro iterativní postorder.
 *
 * Prochází po levé větvi k nejlevějšímu uzlu podstromu a ukládá uzly do
 * zásobníku uzlů. Do zásobníku bool hodnot ukládá informaci, že uzel
 * byl navštíven poprvé.
 *
 * Funkci implementujte iterativně pomocí zásobníku uzlů a bool hodnot a bez použití
 * vlastních pomocných funkcí.
 */
void bst_leftmost_postorder(bst_node_t *tree, stack_bst_t *to_visit,
                            stack_bool_t *first_visit)
{
  bst_node_t *nodePtr = tree;
  while(nodePtr != NULL)
  {
    stack_bst_push(to_visit, nodePtr);
    stack_bool_push(first_visit, true);
    nodePtr = nodePtr->left;
  }
}

/*
 * Postorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte iterativně pomocí funkce bst_leftmost_postorder a
 * zásobníku uzlů a bool hodnot a bez použití vlastních pomocných funkcí.
 */
void bst_postorder(bst_node_t *tree, bst_items_t *items)
{
  bool fromL;
  stack_bool_t first_visit;
  stack_bst_t  stack;
  stack_bool_init(&first_visit);
  stack_bst_init(&stack);
  bst_leftmost_postorder(tree, &stack, &first_visit);
  bst_node_t *nodePtr = NULL;
  while(!stack_bst_empty(&stack))
  {
    nodePtr = stack_bst_top(&stack);
    fromL   = stack_bool_top(&first_visit);
    stack_bool_pop(&first_visit);

    if(fromL)
    {
      stack_bool_push(&first_visit, false);
      bst_leftmost_postorder(nodePtr->right, &stack, &first_visit);
    }
    else
    {
      stack_bst_pop(&stack);
      bst_add_node_to_items(nodePtr, items);
    }
  }
}
