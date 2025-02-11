/*
 *  Předmět: Algoritmy (IAL) - FIT VUT v Brně
 *  Rozšíření pro příklad c206.c (Dvousměrně vázaný lineární seznam)
 *  Vytvořil: Daniel Dolejška, září 2024
 * 
 *  xnovakf00	
 * 
 */

#include "c206-ext.h"

bool error_flag;
bool solved;



/**
 * Táto funkcia vymaže každý druhý prvok v liste dodanom parametrom.
 * 
 * @param list Ukazateľ na list paketov na premazanie.
 */
void deletePackets(DLList *list){
	DLL_First(list);
	while(DLL_IsActive(list))
	{
		DLL_DeleteAfter(list);
		DLL_Next(list);
	}
	return;
}

/** 
 * Táto funkcia slúži na vytvorenie nového listu paketov s prioritou podľa paketu.
 * Volá sa v prípade, že list paketov s pridelenou prioritou neexistuje.
 * 
 * @param packet Paket určujúci prioritu.
 * 
 * @return Ukazateľ na nový PacketList obsahujúci prioritu a list paketov.
*/
QosPacketListPtr createNewPacketList( PacketPtr packet ) {

	QosPacketListPtr new = malloc(sizeof(QosPacketList)); // allocate new element with list and priority
	DLList      *newList = malloc(sizeof(DLList));        // allocate new list for the element
	if(new == NULL || newList == NULL){return NULL;}
	DLL_Init(newList); // init the list
	new->list            = newList; // set the list to new element
	new->priority        = packet->priority; // set the priority to priority of a packet
	return new;
}

/**
 * Tato metoda simuluje příjem síťových paketů s určenou úrovní priority.
 * Přijaté pakety jsou zařazeny do odpovídajících front dle jejich priorit.
 * "Fronty" jsou v tomto cvičení reprezentovány dvousměrně vázanými seznamy
 * - ty totiž umožňují snazší úpravy pro již zařazené položky.
 * 
 * Parametr `packetLists` obsahuje jednotlivé seznamy paketů (`QosPacketListPtr`).
 * Pokud fronta s odpovídající prioritou neexistuje, tato metoda ji alokuje
 * a inicializuje. Za jejich korektní uvolnení odpovídá volající.
 * 
 * V případě, že by po zařazení paketu do seznamu počet prvků v cílovém seznamu
 * překročil stanovený MAX_PACKET_COUNT, dojde nejdříve k promazání položek seznamu.
 * V takovémto případě bude každá druhá položka ze seznamu zahozena nehledě
 * na její vlastní prioritu ovšem v pořadí přijetí.
 * 
 * @param packetLists Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param packet Ukazatel na strukturu přijatého paketu
 */
void receive_packet( DLList *packetLists, PacketPtr packet ) {
	if(MAX_PACKET_COUNT == 0){return;}
	DLL_First(packetLists);

	if(packetLists->activeElement == NULL) // there is no priority list
	{
		QosPacketListPtr new = createNewPacketList(packet); // create it
		if(new==NULL){return;}
		DLL_InsertLast(packetLists, (long)new); // add the new element to the whole packetLists
		DLL_InsertLast(new->list, (long)packet); // insert the packet to the list
		return;
	}
	else // there is at least one element
	{
		while(DLL_IsActive(packetLists))
		{
			QosPacketListPtr currentPriority = (QosPacketListPtr)packetLists->activeElement->data;

			if(currentPriority->priority == packet->priority) // priority matches
			{
				if(currentPriority->list->currentLength == MAX_PACKET_COUNT)
				{
					if(MAX_PACKET_COUNT == 1) // info from forum, not possible to add any new packet
					{
						return;
					}
					deletePackets(currentPriority->list);
				}
				
				DLL_InsertLast(currentPriority->list, (long)packet);
				return;
			}
			else if(currentPriority->priority > packet->priority) // current priority is greater, we need to continue to find other
			{
				DLL_Next(packetLists);
				continue;
			}
			else if(currentPriority->priority < packet->priority) // there is no element with the priority, but it should not be last
			{
				QosPacketListPtr new = createNewPacketList(packet);
				if(new==NULL){return;}
				DLL_InsertBefore(packetLists, (long)new); // add the new element to the whole packetLists before the current one
				DLL_InsertLast(new->list, (long)packet);

				return;
			}
		}
	}

	// if we come to the end of the list without any of the priorities matching
	QosPacketListPtr new = createNewPacketList(packet);
	if(new==NULL){return;}
	DLL_InsertLast(packetLists, (long)new); // add the new element to the whole packetLists
	DLL_InsertLast(new->list, (long)packet); // insert the packet to the list	
}

/**
 * Tato metoda simuluje výběr síťových paketů k odeslání. Výběr respektuje
 * relativní priority paketů mezi sebou, kde pakety s nejvyšší prioritou
 * jsou vždy odeslány nejdříve. Odesílání dále respektuje pořadí, ve kterém
 * byly pakety přijaty metodou `receive_packet`.
 * 
 * Odeslané pakety jsou ze zdrojového seznamu při odeslání odstraněny.
 * 
 * Parametr `packetLists` obsahuje ukazatele na jednotlivé seznamy paketů (`QosPacketListPtr`).
 * Parametr `outputPacketList` obsahuje ukazatele na odeslané pakety (`PacketPtr`).
 * 
 * @param packetLists Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param outputPacketList Ukazatel na seznam paketů k odeslání
 * @param maxPacketCount Maximální počet paketů k odeslání
 */
void send_packets( DLList *packetLists, DLList *outputPacketList, int maxPacketCount ) {
	int sent = 0;
	DLL_First(packetLists);

	while(sent < maxPacketCount && DLL_IsActive(packetLists))
	{
		QosPacketListPtr indPacketList = (QosPacketListPtr)packetLists->activeElement->data;
		DLList *list = indPacketList->list;				
		while(list->firstElement != NULL && sent < maxPacketCount)
		{

			DLL_InsertLast(outputPacketList, (long)list->firstElement->data);
			sent++;
			DLL_DeleteFirst(list);
		}
		DLL_Next(packetLists);
	}
}
