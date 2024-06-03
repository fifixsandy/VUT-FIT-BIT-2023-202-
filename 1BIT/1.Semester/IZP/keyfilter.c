#include <stdio.h>
#include <string.h>

// Author: Filip Novak xnovakf00
// THIS PROJECT HAS BEEN STORED ON GITHUB FOR PERSONAL USE, IF THIS COMMENT IS HERE, THE PERSON THAT HAS SUBMITTED IT HAS STOLEN IT AND DIDNT EVEN OPEN IT


#define MAX_LINE_LEN 102                    // 100 chars + \n and \0 
#define MAX_ASCII 128              

// capitalizes all lowercase letters in given string 
void capitalize(char *toCapitalize)        
{
    int stringLen = strlen(toCapitalize);
    for(int iLetter = 0; iLetter < stringLen; iLetter++)
    {
        if(toCapitalize[iLetter] >= 'a' && toCapitalize[iLetter]<='z')
        {
            toCapitalize[iLetter] = toCapitalize[iLetter]-('a'-'A');
        }
    }
}

// selection sort - characters in string in ascending order based on ASCII values   
void asciiSort(char *toSort)         
{
    int toSortLen = strlen(toSort);
    int iMin;
    for(int iNow = 0; iNow < toSortLen-1; iNow++)
    {
        iMin = iNow;
        for(int iNext = iNow+1; iNext < toSortLen; iNext++)
        {
            if(toSort[iMin] > toSort[iNext])
            {
                iMin = iNext;
            }
        }
        if(iMin != iNow)
        {
            char temp = toSort[iNow];
            toSort[iNow] = toSort[iMin];
            toSort[iMin] = temp;
        }
    }
}

// checks whether given character appears in given string
// THIS PROJECT HAS BEEN STORED ON GITHUB FOR PERSONAL USE, IF THIS COMMENT IS HERE, THE PERSON THAT HAS SUBMITTED IT HAS STOLEN IT AND DIDNT EVEN OPEN IT
int isInString(char character, char *string)        
{
    int stringLen = strlen(string);
    int flag = 0;
    for(int iString = 0; iString < stringLen; iString++)
    {
        if(string[iString] == character)
        {
            flag++;
        }
    }
    return flag;
}


int main(int argc, char **argv)
{
    char *address;

    if(argc == 1)                              // no address given - empty string
    {                                                               
        address = "";
    }
    else if(argc == 2)                         // address given as command line argument
    {
        if((int)strlen(argv[1])>MAX_LINE_LEN - 2)          //-2, strlen does not count \0 and no \n
        {
            fprintf(stderr,"ERROR in command line: ADRESS LONGER THAN MAXIMAL LENGTH\n");
            return 1;
        }
        else
        {
            address = argv[1];
            capitalize(address);
        }
    }
    else                                     
    {
        fprintf(stderr,"ERROR in command line: MORE THAN ONE ARGUMENT\n");
        return 2;       
    }                                                   

    char line[MAX_LINE_LEN + 2] = {0};               // +2 for extra \n and \0 if the line is longer than MAX_LINE_LEN
    char possible[MAX_ASCII] = {0};               
    char found[MAX_LINE_LEN] = {0};
    char nextPossibleCharacter;

    int addressLen = strlen(address);
    int iPossible = 0;
    int addressMatch = 0;
    int lineCounter = 0;
// THIS PROJECT HAS BEEN STORED ON GITHUB FOR PERSONAL USE, IF THIS COMMENT IS HERE, THE PERSON THAT HAS SUBMITTED IT HAS STOLEN IT AND DIDNT EVEN OPEN IT
    while(fgets(line, sizeof(line), stdin) != NULL)
    {   
        lineCounter++;
        
        if((int)strlen(line) > MAX_LINE_LEN - 1)       // -1, strlen does not count \0
        {
            fprintf(stderr, "ERROR on input, line %d: LINE LONGER THAN MAXIMAL LENGTH\n", lineCounter);
            return 3;
        }
        if((int)strlen(line) == 1)
        {
            fprintf(stdout, "WARNING on input, line %d: EMPTY. SKIPPING\n", lineCounter);
            continue;
        }   

        capitalize(line);

        int characterMatch = 0;
        for(int iCharacter = 0; iCharacter < addressLen; iCharacter++)      // checks how many characters match in address and line
        {
            if(address[iCharacter] == line[iCharacter])
            {
                characterMatch++;
            }
        }
       
        if(characterMatch == addressLen)                         // address is prefix to line
        {   
            nextPossibleCharacter = line[characterMatch];      
            if(!isInString(nextPossibleCharacter, possible))     // when not in possible already - add to possible
            {
                possible[iPossible] = nextPossibleCharacter;
                iPossible++;
            }
            addressMatch++;                                      // counting to how many lines the address is a prefix
            strcpy(found,line);
        }
    }
   

    asciiSort(possible); 
   
// THIS PROJECT HAS BEEN STORED ON GITHUB FOR PERSONAL USE, IF THIS COMMENT IS HERE, THE PERSON THAT HAS SUBMITTED IT HAS STOLEN IT AND DIDNT EVEN OPEN IT
    if (lineCounter == 0)                                 // no lines were read
    {
        fprintf(stderr, "ERROR on input: EMPTY\n");
        return 4;
    }

    if(addressMatch == 0)                                 // address isn't a prefix                                  
    {
        fprintf(stdout,"Not found\n");                     
    }
    else if(addressMatch == 1)                            // address is a prefix to exactly one line
    {
        fprintf(stdout,"Found: %s",found);                // \n not needed - in found already
    }
    else                                                  // address is a prefix to more than one line
    {
        fprintf(stdout,"Enable: %s\n", possible);
    }
    
	return 0;
}





















// THIS PROJECT HAS BEEN STORED ON GITHUB FOR PERSONAL USE, IF THIS COMMENT IS HERE, THE PERSON THAT HAS SUBMITTED IT HAS STOLEN IT AND DIDNT EVEN OPEN IT
