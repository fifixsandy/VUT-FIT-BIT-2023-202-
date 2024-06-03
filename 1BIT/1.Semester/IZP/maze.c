
// AUTHOR: Filip Novak
// XLOGIN: xnovakf00
// THIS PROJECT HAS BEEN STORED ON GITHUB FOR PERSONAL USE, IF THIS COMMENT IS HERE, THE PERSON THAT HAS SUBMITTED IT HAS STOLEN IT AND DIDNT EVEN OPEN IT

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MIN_ARGS 2
#define MAX_ARGS 5

#define UP -1        // move directions
#define DOWN 1      //

#define L 1         // bit border, rule, move direction
#define R 2         //
#define UD 4        // bit border (UP OR DOWN)

#define PATH 1
#define WRONG 2
#define HELP 3
#define TEST 4


typedef struct 
{
    int rows;                     
    int cols;                    
    unsigned char *cells;       
}Map;

typedef struct
{
    int RLpath,
        startR,
        startC;
    char *fileName;   
}Args;


enum err 
{
    eALCFAIL = 1,
    eFILEOPEN,
    eDIM,
    eMAPDIM,
    eBORDCOMP,
    eCOOR,
    eENTRANCE,
    eSYNTAX,
    eINVCELL,
};


/// @brief Prints syntax of correctly using this programm
void syntaxPrint()
{
    fprintf(stdout, "\nUSAGE: ./maze [OPTIONS]\n\n"
                    "A program to print the path through the maze using right-hand rule or left-hand rule. It also test if maze provided is valid or not.\n\n"
                    "OPTIONS:\n"
                    "  --help                       Show this message.\n"
                    "  --test file_name.txt         Test the validity of a maze stored in file_name.txt.\n"
                    "  --rpath R C file_name.txt    Print the path through the maze starting from position (R, C) following right-hand rule.\n"
                    "  --lpath R C file_name.t      Print the path through the maze starting from position (R, C) following left-hand rule.\n\n"
                    "VALIDITY OF THE MAZE:\n"
                    "  The maze in file is made up of cells that are triangles. \n"
                    "  - The first two numbers are the dimensions of the maze. First number=number of rows, second number=number of columns.\n"
                    "  - Following numbers are cells, each represented by a number from 0 to 7.\n"
                    "  - Numbers are divided by any white-space character.\n"
                    "    - Each number means if border is present using binary representation:\n"
                    "      - 001: Left border\n"
                    "      - 010: Right border\n"
                    "      - 100: Bottom or top border\n"
                    "  - The number of cells should match the specified dimensions.\n"
                    "  - Cells next to eachother should have compatible borders."
                    "  - The maze is considered valid if it meets these criteria.\n\n"
                    "PATH PRINTING:\n"
                    "  Provide starting position (R, C) to print the path through the maze.\n"
                    "  R - Starting row\n"
                    "  C - Starting column\n"
                    "  The program will check the validity of the maze file before printing the path.\n"
                    "  If the file is invalid, it will display a message.\n");
                    
}

/// @brief Prints error message
/// @param errID Discriminator of errors
void errorPrint(int errID)
{
    switch (errID)
    {
        case eDIM:
            fprintf(stderr, "ERROR: INVALID DIMENSIONS DESCRIPTION IN .TXT FILE\n");
            break;
        case eMAPDIM:
            fprintf(stderr, "ERROR: DIMENSIONS DESCRIPTION AND NUMBER OF CELLS DO NOT MATCH IN .TXT FILE\n");
            break;
        case eBORDCOMP:
            fprintf(stderr, "ERROR: BORDERS NOT COMPATIBLE IN .TXT FILE\n");
            break;
        case eENTRANCE:
            fprintf(stderr, "ERROR: NO ENTRANCE ON GIVEN CELL\n");
            break;
        case eFILEOPEN:
            fprintf(stderr, "ERROR: UNABLE TO OPEN FILE\n");
            break;
        case eINVCELL:
            fprintf(stderr, "ERROR: INVALID NUMBER IN CELL DESCRIPTION\n");
            break;
        default:
            fprintf(stderr, "ERROR: UNKNOWN ERROR ID %d\n",errID);
            break;
    }
}
// THIS PROJECT HAS BEEN STORED ON GITHUB FOR PERSONAL USE, IF THIS COMMENT IS HERE, THE PERSON THAT HAS SUBMITTED IT HAS STOLEN IT AND DIDNT EVEN OPEN IT
/// @brief Loads arguments, evaluates their validity and stores needed information
/// @param argc Character count
/// @param argv Array of arguments
/// @param args Pointer to a struct Args, where needed information will be stored
/// @return Flag on which action should be executed
int argLoader(int argc, char **argv, Args *args)                                  
{
    if(argc < MIN_ARGS || argc > MAX_ARGS)
    {
        fprintf(stderr,"INVALID NUMBER OF ARGUMENTS\n");
        return WRONG;
    }

    if(strcmp(argv[1], "--help") == 0)                      //strcmp returns 0 when it matches
        return HELP;

    else if(strcmp(argv[1], "--test") == 0)
    {
        if(argc != 3)
            return WRONG;

        args->fileName = argv[argc - 1];
        return TEST;
    }
    else if(strcmp(argv[1], "--rpath") == 0 || strcmp(argv[1], "--lpath") == 0)
    {
        if(argc != 5)
            return WRONG;

        args->startR = atoi(argv[2]);
        args->startC = atoi(argv[3]);
        args->fileName = argv[argc - 1];

        if(strcmp(argv[1], "--rpath") == 0) 
            args->RLpath = R;
        else
            args->RLpath = L;
        
        return PATH;
    }

    return WRONG;
}

///******************************** FUNCTIONS FOR CELLS MANAGEMENT ********************************///

/// @brief Dynamicaly allocates array of unsigned chars - cells
/// @param map Pointer to a struct map of maze
/// @param size Desired number of cells for creating the array
/// @return When alloc failed eALCFAIL
int cellsCtor(Map *map, unsigned int size)
{
    map->cells = malloc((size) * sizeof(unsigned char));
    if (map->cells == NULL)
    {
        fprintf(stderr,"ALLOC FAILED\n");
        return eALCFAIL;
    }
    return 0;
}

/// @brief Frees the allocated array
/// @param map Pointer to a struct map of maze
void cellsDtor(Map *map)
{
    free(map->cells);
    map->cells = NULL;
}

/// @brief Destroys the allocated array and closes opened file
/// @param map Pointer to a struct map of maze
/// @param file Pointer to a file
void tidy(Map *map, FILE *file)
{
    cellsDtor(map);
    fclose(file);
}
// THIS PROJECT HAS BEEN STORED ON GITHUB FOR PERSONAL USE, IF THIS COMMENT IS HERE, THE PERSON THAT HAS SUBMITTED IT HAS STOLEN IT AND DIDNT EVEN OPEN IT
///***************** FUNCTIONS FOR LOADING AND CHECKING INFORMATION FROM TEXT FILE *****************///

/// @brief Loads first two numbers in text file - dimensions of maze and checks their validity
/// @param file Pointer to a file in which the maze is read from  
/// @param map Pointer to a struct map of maze where loaded dimensions are stored
/// @return eDIM if there is a problem
bool rowsColsLoader(FILE *file, Map *map)
{
    if (fscanf(file, "%d %d", &map->rows, &map->cols) != 2)
        return eDIM;

    if(map->rows <= 0 || map->cols <= 0)
        return eDIM;

    return 0;
}

/// @brief Loads the cells from text file and checks their validity
/// @param file Pointer to a file in which the maze is read from 
/// @param map Pointer to a struct map of maze where loaded cells are stored
/// @return eMAPDIM if number of cells does not correspond to dimensions
int mapChecker(FILE *file, Map *map) 
{
    int len = 0;
    while(fscanf(file,"%hhu",&(map->cells[len])) == 1 && len != (map->cols) * (map->rows))
    {
        if(map->cells[len] <= 7)
            len++;
        else
            return eINVCELL;
    }

    if(len != (map->cols) * (map->rows))
        return eMAPDIM;

    return 0;
}

///********************** FUNCTIONS FOR LOADING AND CHECKING BORDERS IN MAZE **********************///

/// @brief Creates index for working with array from row and column numbers of cell
/// @param r Row number of cell
/// @param c Column number of cell
/// @param map Pointer to a struct map of maze where dimensions are stored
/// @return Created index
int idxCtor(int r, int c, Map *map)
{
    return (((r-1)*(map->cols) + c) - 1);
}

/// @brief Checks if border is there
/// @param map Pointer to a struct map of maze where cells are stored
/// @param r Row number of cell
/// @param c Column number of cell
/// @param border Border to check
/// @return 1 if is border, 0 if is not
bool isborder(Map *map, int r, int c, int border)
{
    if(border == L)
        return ((map->cells[idxCtor(r,c, map)]) & L);
    
    else if(border == R)
        return ((map->cells[idxCtor(r,c, map)]) & R);
    
    else
        return ((map->cells[idxCtor(r,c, map)]) & UD);
}

/// @brief Checks whether neighbouring cells have same borders
/// @param map Pointer to a struct map of maze where cells are stored
/// @return eBORDCOMP when there is a problem
bool borderChecker(Map *map)
{
    for(int row = 1; row <= map->rows; row++)
    {
        for(int col = 1; col < map->cols; col++)
        {
            if(isborder(map, row, col, R) != isborder(map, row, col+1, L))    // for cells next to eachother
                return eBORDCOMP;

            if((row+col) & 1 && row != map->rows)    // for odd sums of row and col and not for last row
            {
                if(isborder(map, row, col, UD) != isborder(map, row+1, col, UD))
                    return eBORDCOMP;
            }
        }
    }
    return 0;
}

///************************ FUNCTIONS FOR WAY OF MOVING AROUND IN THE MAZE ************************///
// THIS PROJECT HAS BEEN STORED ON GITHUB FOR PERSONAL USE, IF THIS COMMENT IS HERE, THE PERSON THAT HAS SUBMITTED IT HAS STOLEN IT AND DIDNT EVEN OPEN IT
/// @brief Checks if coordinates are not out of bound of maze
/// @param r Row number of cell
/// @param c Column number of cell
/// @param map Pointer to a struct map of maze 
/// @return  eCOOR when there is a problem
int checkCoor(int r, int c, Map *map)
{
    if((r <= 0) || (r > map->rows) || (c<=0) || c>map->cols) 
        return eCOOR;

    return 0;  
}

/// @brief Based on sum of coordinates function evaluates position of cell's base 
/// @param r Row number of cell
/// @param c Column number of cell
/// @return UP or DOWN
int triBase(int r, int c)
{
    if((r+c)%2)
        return DOWN;
    else
        return UP;
}


/// @brief Checks all borders of an entrance cell given by user
/// @param map Pointer to a struct map of maze 
/// @param r Row number of cell
/// @param c Column number of cell
/// @return Border which should be used to enter the maze (according to specification in task), if there is none - error
int entrance(Map *map, int r, int c)
{
    if(!checkCoor(r, c, map))
    {
        if(c == 1)
        {
            if(!isborder(map, r, c, L))
            {
                return L;
            }
        }
        else if(c == map->cols)
        {
            if(!isborder(map, r, c, R))
                return R;
        }
        if(r == 1 && triBase(r,c) == UP)
        {
            if(!isborder(map, r, c, UD))
                return UD;
        }    
        if(r == map->rows && triBase(r,c) == DOWN)
        {
            if(!isborder(map, r, c, UD))
            return UD;
        }
    }
    return eENTRANCE;
}


/// @brief Evaluates which border "the hand lands on" after entering cell
/// @param r Row number of cell
/// @param c Column number of cell
/// @param leftright Right-hand or Left-hand rule
/// @param enteringFrom Border used as entrance
/// @return Border to land on or eENTRANCE
int borderAfterEnter(int r, int c, int leftright, int enteringFrom)
{
    int baseOrient = triBase(r,c);

    if(baseOrient == UP)
    {
        if(enteringFrom == L)
        {
            if(leftright == R)
                return R;
            else
                return UD;
        }
        else if(enteringFrom == R)
        {
            if(leftright == R)
                return UD;
            else
                return L;
        }
        else if(enteringFrom == UD)
        {
            if(leftright == R)
                return L;
            else
                return R;
        }
    }

    else if(baseOrient == DOWN)
    {
        if(enteringFrom == L)
        {
            if(leftright == R)
                return UD;
            else
                return R;
        }
        else if(enteringFrom == R)
        {
            if(leftright == R)
                return L;
            else
                return UD;
        }
        else if(enteringFrom == UD)
        {
            if(leftright == R)
                return R;
            else
                return L;
        }
    }

    return eENTRANCE;
}

// THIS PROJECT HAS BEEN STORED ON GITHUB FOR PERSONAL USE, IF THIS COMMENT IS HERE, THE PERSON THAT HAS SUBMITTED IT HAS STOLEN IT AND DIDNT EVEN OPEN IT
/// @brief Finds the border on which "the hand lands on first" when entering the maze
/// @param map Pointer to a struct map of maze 
/// @param r Row number of start cell
/// @param c Column number of start cell
/// @param leftright Right-hand or Left-hand rule
/// @return Border which "the hand should land on"
int start_border(Map *map, int r, int c, int leftright)
{
    int entranceBorder = entrance(map, r, c);
    return borderAfterEnter(r, c, leftright, entranceBorder);
}


/// @brief Based on parameters evaluates which border of cell we should work with next
/// @param r Row number
/// @param c Column number
/// @param leftright Right-hand or Left-hand rule
/// @param border Current border
/// @return Bit value of next border to be worked with
int nextBorder(int r, int c, int leftright, int border)
{
    int baseOrient = triBase(r,c);
    if(leftright == R)
    {
        if(baseOrient == DOWN)
        {
            if(border == UD)
                return R;
            else if(border == R)
                return L;
            else
                return UD;
        }
        if(baseOrient == UP)
        {
            if(border == UD)
                return L;
            else if(border == R)
                return UD;
            else
                return R;
        }
    }

    else
    {
        if(baseOrient == DOWN)
        {
            if(border == UD)
                return L;
            else if(border == R)
                return UD;
            else
                return R;
        }

        if(baseOrient == UP)
        {
            if(border == UD)
                return R;
            else if(border == R)
                return L;
            else
                return UD;   
        }
    }
    return -1;
}

/// @brief Changes the cell coordinates (moving around the maze)
/// @param r Address of row number of cell to be changed
/// @param c Address of column number of cell to be changed
/// @param border Border used to exit the inputed cell
/// @param base Position of base of cell
void cellChange(int *r, int *c, int border,int base)
{
    if(border == R)
        (*c)++;
    else if(border == L)
        (*c)--;
    else
        (*r)+=base;
}

/// @brief From border used for exiting cell makes border used for entering the next cell
/// @param exitBorder Border of previous cell used to enter current cell 
/// @return Border used for entering
int borderSwap(int exitBorder)
{
    switch (exitBorder) 
    {
        case L:
            return R;
        case R:
            return L;
        case UD:
            return UD;
        default:
            return -1;
    }
    return -1;
}


/// @brief Prints out the cells to go through in maze 1 by 1 until row num or cell num is out of bound
/// @param map Pointer to the map of maze
/// @param startR Row number of starting cell
/// @param startC Column number of starting cell
/// @param leftright Right-hand or Left-hand rule
int path(Map *map, int startR, int startC, int leftright)
{
    int currentR = startR;
    int currentC = startC;
    int current = start_border(map, currentR, currentC, leftright);

    if(current != eENTRANCE)
    {   
        while(!checkCoor(currentR, currentC, map))
        {
            if(!isborder(map, currentR, currentC, current))    //when the border is not there, change the cell
            {
                fprintf(stdout,"%d,%d\n", currentR, currentC);                                   
                cellChange(&currentR, &currentC, current, triBase(currentR, currentC));
                current = borderAfterEnter(currentR, currentC, leftright, borderSwap(current));  
            }
            else
            {                                                 //when there is a border, find if next border is free or not
                current = nextBorder(currentR, currentC, leftright, current);
            }
        }

        return 0;
    }
    else
    {
        errorPrint(eENTRANCE);
        return eENTRANCE;
    }    
}



///********************************* FUNCTIONS FOR WANTED OUTPUT **********************************///

/// @brief Tests, if maze description is valid
/// @param file File where maze is stored
/// @param map Pointer to a struct map where info is stored 
/// @return Error discriminator or valid
int runTest(FILE *file, Map *map)
{

    if(rowsColsLoader(file, map))
        return eDIM;
    
    if(cellsCtor(map, (map->cols)*(map->rows)+1))
        return eALCFAIL;
    
    if(mapChecker(file, map) == eMAPDIM)
    {
        cellsDtor(map);
        return eMAPDIM;
    }

    if(mapChecker(file, map) == eINVCELL)
    {
        cellsDtor(map);
        return eINVCELL;
    }    

    if(borderChecker(map))
    {
        cellsDtor(map);
        return eBORDCOMP;
    }

    return 0;
}

/// @brief Prints a passage through the maze from entrance to exit
/// @param file File where maze is stored
/// @param map Pointer to a struct map where info is stored 
/// @param args Pointer to a struct args where arguments are stored
/// @return Error or no problem
int runPath(FILE *file, Map *map, Args *args)
{
    int testReturn = runTest(file, map);
    if(testReturn != 0)
    {
        errorPrint(testReturn);
        return testReturn;
    }


    path(map, args->startR, args->startC, args->RLpath);
    return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) 
{
    Map map;
    Args args;
    FILE *file = NULL;

    int argFlag = argLoader(argc, argv, &args);

    if(argFlag == WRONG)
    {
        fprintf(stdout,"ERROR: INVALID SYNTAX\n");
        syntaxPrint();
        return eSYNTAX;
    }

    if(argFlag == HELP)
    {
        syntaxPrint();
        return 0;
    }


    file = fopen(args.fileName, "r");
    if(file == NULL)
    {
        errorPrint(eFILEOPEN);
        return eFILEOPEN;
    }


    if(argFlag == TEST)
    {
        int runTestResult = runTest(file,&map);
        if(runTestResult)
            fprintf(stdout,"Invalid\n");
        else
        {
            fprintf(stdout,"Valid\n");
            cellsDtor(&map);
        }
        fclose(file);
        return 0;
    }
    
    if(argFlag == PATH)
    {
        runPath(file, &map, &args);
        tidy(&map,file);
        return 0;
    }

    tidy(&map, file);
    return 0;
}
















// THIS PROJECT HAS BEEN STORED ON GITHUB FOR PERSONAL USE, IF THIS COMMENT IS HERE, THE PERSON THAT HAS SUBMITTED IT HAS STOLEN IT AND DIDNT EVEN OPEN IT
