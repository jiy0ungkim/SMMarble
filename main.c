//
//  main.c
//  SMMarble
//
//  Created by Juyeop Kim on 2023/11/05.
//

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "smm_object.h"
#include "smm_database.h"
#include "smm_common.h"

#define BOARDFILEPATH "marbleBoardConfig.txt"
#define FOODFILEPATH "marbleFoodConfig.txt"
#define FESTFILEPATH "marbleFestivalConfig.txt"

//board configuration parameters
static int smm_board_nr;
static int smm_food_nr;
static int smm_festival_nr;
static int smm_player_nr; 

typedef struct{
        int pos; // player's position
        int credit; // player's credit
        char name[MAX_CHARNAME]; // player's name
        int energy; // player's energy
        int flag_graduated;
} smm_player_t;

smm_player_t *smm_players; // dynamic memory allocation

void goForward(int player, int step); //make player go "step" steps on the board (check if player is graduated)
void printPlayerStatus(void); //print all player status at the beginning of each turn
void generatePlayers(int n, int initEnergy); //generate a new player
void* findGrade(int player, char *lectureName); //find the grade from the player's grade history
int isGraduated(void); //check if any player is graduated
void printGrades(int player); //print all the grade history of the player

//function prototypes
#if 0
float calcAverageGrade(int player); //calculate average grade of the player
smmGrade_e takeLecture(int player, char *lectureName, int credit); //take the lecture (insert a grade of the player)
#endif

void* findGrade(int player, char *lectureName)
{
      int size = smmdb_len(LISTNO_OFFSET_GRADE + player); // 수강 과목 개수
      int i;
      
      for (i=0; i<size; i++)
      {
          void *ptr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
          if (strcmp(smmObj_getObjectName(ptr), lectureName) == 0) // 수강한 과목 출력 
             return ptr; 
      } 
      return NULL;
}
      
void printGrades(int player)
{
     int i;
     int size = smmdb_len(LISTNO_OFFSET_GRADE + player);
     
     printf("\n===== %s's Grade =====\n", smm_players[player].name);
     
     if (size == 0)
     {
        printf("No lectures taken yet.\n");
        printf("========================\n\n");
        return;
     }
     
     for (i=0; i<size; i++)
     {
         void *gradePtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
         
         printf("Lecture: %s | Grade: %s | Credit: %i\n",
               smmObj_getObjectName(gradePtr),
               smmObj_getObjectGradeName(gradePtr),
               smmObj_getObjectCredit(gradePtr));
     }
     printf("========================\n\n");
}
        
int isGraduated(void)
{
    int i;
    for (i=0; i<smm_player_nr; i++)
    {
        if (smm_players[i].flag_graduated == 1)
           return 1; // game end
    }
    return 0; // game continue
}

void goForward(int player, int step)
{
     int i;
     int tmpPos = smm_players[player].pos;
     
     void *nodePtr = smmdb_getData(LISTNO_NODE, tmpPos);

     //player_pos[player] = player_pos[player] + step;
     printf("Start from %i (%s) (%i)\n", smm_players[player].pos, 
                                         smmObj_getObjectName(nodePtr), 
                                         step);

     for (i=0; i<step; i++)
     {   
         tmpPos = (tmpPos + 1) % smm_board_nr;
         nodePtr = smmdb_getData(LISTNO_NODE, tmpPos);
         //smm_players[player].pos = (smm_players[player].pos + 1) % smm_board_nr;
         printf(" => moved to %i (%s)\n", tmpPos, 
                                          smmObj_getObjectName(nodePtr));
     }
     smm_players[player].pos = tmpPos;
}

void printPlayerStatus(void)
{    
     int i;
     for (i=0; i<smm_player_nr; i++)
     {
         void *nodePtr = smmdb_getData(LISTNO_NODE, smm_players[i].pos);
         printf("%s - position : %i (%s), credit : %i, energy : %i\n", 
                    smm_players[i].name, 
                    smm_players[i].pos, 
                    smmObj_getObjectTypeName(nodePtr), 
                    smm_players[i].credit, 
                    smm_players[i].energy); 
     }
}

void generatePlayers(int n, int initEnergy) //generate a new player
{
     int i;
     
     smm_players = (smm_player_t *)malloc(n*sizeof(smm_player_t));
     
     for (i=0; i<n; i++)
     {
         smm_players[i].pos = 0; // start position = home
         smm_players[i].credit = 0; // init credit = 0
         smm_players[i].energy = initEnergy; // init energy = initEnergy
         smm_players[i].flag_graduated = 0;
         
         printf("Input %i-th player name: ", i);
         scanf("%s", &smm_players[i].name[0]);
         fflush(stdin);     
     }
}
   
int rolldie(int player)
{
    char c;
    printf("!Press any key to roll a die (press g to see grade): ");
    c = getchar();
    fflush(stdin);

    if (c == 'g')
        printGrades(player);
    
    return (rand()%MAX_DIE + 1);
}

//action code when a player stays at a node
void actionNode(int player)
{
     void *nodePtr = smmdb_getData(LISTNO_NODE, smm_players[player].pos);
     void *foodPtr = smmdb_getData(LISTNO_FOODCARD, smm_players[player].pos);
     void *gradePtr;
     
     int type = smmObj_getObjectType(nodePtr);
     char* typeName = smmObj_getObjectTypeName(nodePtr);
     int credit = smmObj_getObjectCredit(nodePtr);
     int energy = smmObj_getObjectEnergy(nodePtr);
     
     int grade;
     
     char* foodName = smmObj_getObjectFoodName(foodPtr);
     int foodEnergy = smmObj_getObjectFoodEnergy(foodPtr);
     
     switch(type)
     {
        case SMMNODE_TYPE_LECTURE:
             if(findGrade(player, smmObj_getObjectName(nodePtr)) == NULL && smm_players[player].energy >= energy) // 재수강 불가 & 에너지 충분 필요 
             {
                 int ch;
                 
                 printf(" --> [%ith Player RESULT] pose : %i | type : %s | credit : +%i | energy : -%i\n", 
                          player, smm_players[player].pos, typeName, credit, energy);
                 printf("Take this lecture? (y / n): ");
                 ch = getchar();
                 fflush(stdin);
                 
                 if (ch == 'y'){
                    smm_players[player].credit += credit;
                    smm_players[player].energy -= energy;

                    grade = rand()%SMMNODE_MAX_GRADE;
                    gradePtr = smmObj_genObject(smmObj_getObjectName(nodePtr), SMMNODE_OBJTYPE_GRADE, type, credit, energy, grade);
                    smmdb_addTail(LISTNO_OFFSET_GRADE+player, gradePtr);
                 }
                 if (ch == 'n')
                    break;
             }
             break;
        
        case SMMNODE_TYPE_RESTAURANT:
             printf(" --> [%ith Player RESULT] pose : %i | type : %s | energy : +%i\n", 
                      player, smm_players[player].pos, typeName, energy);
             smm_players[player].energy += energy;
             break;
             
        case SMMNODE_TYPE_LABORATORY:
             break;
             
        case SMMNODE_TYPE_HOME:
             printf(" --> [%ith Player RESULT] pose : %i | type : %s | energy : +%i\n", 
                      player, smm_players[player].pos, typeName, energy);
                      
             smm_players[player].energy += energy;
             
             if (smm_players[player].credit >= GRADUATE_CREDIT)
                smm_players[player].flag_graduated = 1; // graduation
             break;
             
        case SMMNODE_TYPE_GOTOLAB:
             break;
             
        case SMMNODE_TYPE_FOODCHANCE:
             printf(" --> [%ith Player RESULT] pose : %i | type : %s | food : %s | energy : %i\n", 
                      player, smm_players[player].pos, typeName, foodName, foodEnergy);
                      
             smm_players[player].energy += foodEnergy;
             break;
             
        case SMMNODE_TYPE_FESTIVAL:
             break;
             
        //case lecture:
        default:
            break;
     }
}

int main(int argc, const char * argv[]) 
{
    FILE* fp;
    char name[MAX_CHARNAME]; 
    int type; // node type
    int credit; 
    int energy;
    
    char foodName[MAX_CHARNAME];
    int foodEnergy;
    
    char festival[MAX_CHARNAME];
    
    int turn;
    
    smm_board_nr = 0;
    smm_food_nr = 0;
    smm_festival_nr = 0;
    
    srand((unsigned)time(NULL));
    
    //1. import parameters ---------------------------------------------------------------------------------
    //1-1. boardConfig 
    if ((fp = fopen(BOARDFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", BOARDFILEPATH);
        getchar();
        return -1;
    }
    
    printf("Reading board component......\n");
    
    while (fscanf(fp, "%s %i %i %i", name, &type, &credit, &energy) == 4) //read a node parameter set
    {
        //store the parameter set
        void *opjPtr = smmObj_genObject(name, SMMNODE_OBJTYPE_BOARD, type, credit, energy, 0);
        //printf("%s %i %i %i\n", name, type, credit, energy); 
        if (smmdb_addTail(LISTNO_NODE, opjPtr) < 0) {
           printf("[ERROR] failed to add node \n");
           return -1;
        }
        smm_board_nr++;
        //smm_board_nr = smmdb_addTail(LISTNO_NODE, opjPtr);  
    }
    fclose(fp);
    
    printf("Total number of board nodes : %i\n", smm_board_nr);
    

    //2. food card config 
    if ((fp = fopen(FOODFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FOODFILEPATH);
        return -1;
    }
    
    printf("\n\nReading food card component......\n");
    while (fscanf(fp, "%s %i", foodName, &foodEnergy) == 2) //read a food parameter set
    {
          void *foodPtr = smmObj_genFood(foodName, foodEnergy);
          if (smmdb_addTail(LISTNO_FOODCARD, foodPtr) < 0) {
             printf("[ERROR] failed to add food \n");
             return -1;
          }
          smm_food_nr++;
        //store the parameter set
    }
    fclose(fp);
    printf("Total number of food cards : %i\n", smm_food_nr);
    
    # if 0
    //3. festival card config 
    if ((fp = fopen(FESTFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FESTFILEPATH);
        return -1;
    }
    
    printf("\n\nReading festival card component......\n");
    while () //read a festival card string
    {
        //store the parameter set
    }
    fclose(fp);
    printf("Total number of festival cards : %i\n", festival_nr);
    #endif
    
    //2. Player configuration ---------------------------------------------------------------------------------
    do
    {
        //input player number to player_nr
        printf("Input player number: ");
        scanf("%i", &smm_player_nr); 
        fflush(stdin); // remove buffer
        
        if (smm_player_nr <=0 || smm_player_nr > MAX_PLAYER)
           printf("Invalid player number!\n");
    }
    while (smm_player_nr <= 0 || smm_player_nr > MAX_PLAYER);
    
    generatePlayers(smm_player_nr, smmObj_getObjectEnergy(smmdb_getData(SMMNODE_OBJTYPE_BOARD, 0))); // init Energy = home energy
    
    turn = 0; // player's turn
    
    //3. SM Marble game starts ---------------------------------------------------------------------------------
    while (isGraduated() == 0) //is anybody graduated?
    {
        int die_result;
        
        //4-1. initial printing
        printPlayerStatus();
        
        //4-2. die rolling (if not in experiment)
        die_result = rolldie(turn);
        
        //4-3. go forward
        goForward(turn, die_result);
        
        void *posPtr = smmdb_getData(LISTNO_NODE, smm_players[turn].pos);
        
        printf("node : %s, type : %i (%s)\n", 
                     smmObj_getObjectName(posPtr), 
                     smmObj_getObjectType(posPtr), 
                     smmObj_getObjectTypeName(posPtr));
        
		//4-4. take action at the destination node of the board
        actionNode(turn);
        
        //4-5. next turn
        turn = (turn + 1)%smm_player_nr;
    }
    
    free(smm_players);
    
    system("PAUSE");
    return 0;
}
