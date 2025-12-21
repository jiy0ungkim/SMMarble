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
static int smm_fest_nr;
static int smm_player_nr; 

typedef struct{
        int pos; // player's position
        int credit; // player's credit
        char name[MAX_CHARNAME]; // player's name
        int energy; // player's energy
        
        int flag_graduated; // 0: not graduated, 1: graduated
        int flag_experimenting; // 0: not experimenting, 1: experimenting
        int exp_success_thr; // experiment success criterion(threshold) (1~MAX_DIE)
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

int getGraduatedPlayerIdx(void)
{
    int i;
    
    for (i=0; i<smm_player_nr; i++)
    {
        if (smm_players[i].flag_graduated == 1)
           return i;
    }
    return -1;
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
         if (i<(step-1) && smmObj_getObjectType(nodePtr) == SMMNODE_TYPE_HOME)
         {
            int homeEnergy = smmObj_getObjectEnergy(nodePtr);
            smm_players[player].energy += homeEnergy;
            printf("    [PASS HOME] energy +%i\n", homeEnergy);
            
            // graduation check 
            if (smm_players[player].credit >= GRADUATE_CREDIT)
            {
               smm_players[player].flag_graduated = 1;
               printf("    [PASS HOME] %s meets GRADUATE_CREDIT (%i/%i). GAME OVER! \n",
                           smm_players[player].name, smm_players[player].credit, GRADUATE_CREDIT);
               return;
            }
               
         }                  
     }
     smm_players[player].pos = tmpPos;
}

void printPlayerStatus(void)
{    
     int i;
     for (i=0; i<smm_player_nr; i++)
     {
         void *nodePtr = smmdb_getData(LISTNO_NODE, smm_players[i].pos);
         printf("%s - position : %i (%s), experimenting state: %i (not experimenting), credit : %i, energy : %i\n", 
                    smm_players[i].name, 
                    smm_players[i].pos, 
                    smm_players[i].flag_experimenting,
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
         smm_players[i].flag_graduated = 0; // not graduated 
         smm_players[i].flag_experimenting = 0; // not experimenting
         smm_players[i].exp_success_thr = 0;
         
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

int getRandomLabPos(void)
{
    int i;
    int labCnt = 0;
    int pick;
    
    // count laboratory node 
    for (i=0; i<smm_board_nr; i++)
    {
        void *nodePtr = smmdb_getData(LISTNO_NODE, i);
        if (smmObj_getObjectType(nodePtr) == SMMNODE_TYPE_LABORATORY)
           labCnt++;
    }
    
    if (labCnt == 0)
       return -1;
    
    pick = rand() % labCnt;
    
    // pick random laboratory node
    for (i=0; i<smm_board_nr; i++)
    {
        void *nodePtr = smmdb_getData(LISTNO_NODE, i);
        if (smmObj_getObjectType(nodePtr) == SMMNODE_TYPE_LABORATORY)
        {
            if (pick == 0) 
               return i;
            pick--;
        }
    }
    return -1;
}

//action code when a player stays at a node
void actionNode(int player)
{
     int foodIdx = rand()%smm_food_nr; // random food chance
     int festIdx = rand()%smm_fest_nr; //random festival
     
     void *nodePtr = smmdb_getData(LISTNO_NODE, smm_players[player].pos);
     void *foodPtr = smmdb_getData(LISTNO_FOODCARD, foodIdx);
     void *festPtr = smmdb_getData(LISTNO_FESTCARD, festIdx);
     void *gradePtr;
     
     int type = smmObj_getObjectType(nodePtr);
     char* typeName = smmObj_getObjectTypeName(nodePtr);
     int credit = smmObj_getObjectCredit(nodePtr);
     int energy = smmObj_getObjectEnergy(nodePtr);
     char* foodName = smmObj_getObjectFoodName(foodPtr);
     int foodEnergy = smmObj_getObjectFoodEnergy(foodPtr);
     char* fest = smmObj_getObjectFest(festPtr);
     int grade;
     int labPos = getRandomLabPos();
     int exp_result;
     
     switch(type)
     {
        case SMMNODE_TYPE_LECTURE:
             if (findGrade(player, smmObj_getObjectName(nodePtr)) != NULL)
             {
                printf(" --> [%ith PLAYER RESULT] pose : %i | type : %s\n", 
                        player, smm_players[player].pos, typeName);
                printf("     [LECTURE] You already took this lecture!\n");
                break;
             }
             
             if (smm_players[player].energy < energy)
             {
                printf(" --> [%ith PLAYER RESULT] pose : %i | type : %s\n", 
                         player, smm_players[player].pos, typeName);
                printf("     [LECTURE] Not enough energy! (%i/%i)\n", smm_players[player].energy, energy);
                break;
             }
             if(findGrade(player, smmObj_getObjectName(nodePtr)) == NULL && smm_players[player].energy >= energy) // 재수강 불가 & 에너지 충분 필요 
             {
                 int ch;
                 
                 printf(" --> [%ith Player RESULT] pose : %i | type : %s | credit : +%i | energy : -%i\n", 
                          player, smm_players[player].pos, typeName, credit, energy);
                 
                 
                 while (1)
                 {
                       printf("Take this lecture? (y / n): ");
                       ch = getchar();
                       fflush(stdin);
                       
                       if (ch == 'y')
                       {
                          smm_players[player].credit += credit;
                          smm_players[player].energy -= energy;
                          
                          grade = rand()%SMMNODE_MAX_GRADE;
                          gradePtr = smmObj_genObject(smmObj_getObjectName(nodePtr), SMMNODE_OBJTYPE_GRADE, type, credit, energy, grade);
                          smmdb_addTail(LISTNO_OFFSET_GRADE+player, gradePtr);
                          break;
                       }
                       else if (ch == 'n')
                          break;
                       else
                           printf("Invalid input!\n");
                 }
             }
             break;
        
        case SMMNODE_TYPE_RESTAURANT:
             printf(" --> [%ith Player RESULT] pose : %i | type : %s | energy : +%i\n", 
                      player, smm_players[player].pos, typeName, energy);
                      
             smm_players[player].energy += energy;
             break;
             
        case SMMNODE_TYPE_LABORATORY:
             // not experimenting -> nothing.
             if (smm_players[player].flag_experimenting == 0)
             {
                printf(" --> [%ith Player RESULT] pose : %i | type : %s (Not experimenting)\n", 
                      player, smm_players[player].pos, typeName);
                break;
             }
             
             // experimenting state -> start experiment 
             printf(" --> [%ith Player RESULT] pose : %i | type : %s (Experimenting)\n", 
                      player, smm_players[player].pos, typeName);
             
             smm_players[player].energy -= energy;
             printf("     [LAB] Experiment! energy : %i (-%i)\n", smm_players[player].energy, energy);
             
             // roll die
             exp_result = (rand() % MAX_DIE) + 1;
             printf("     [LAB] die result = %i | threshold : %i\n", exp_result, smm_players[player].exp_success_thr);
             
             if (exp_result >= smm_players[player].exp_success_thr)
             {
                printf("     [LAB] SUCESS! Experiment finished. \n");
                smm_players[player].flag_experimenting = 0;
                smm_players[player].exp_success_thr = 0;
             }
             else
                 printf("     [LAB] FAIL. \n");
             break;
             
        case SMMNODE_TYPE_HOME:
             printf(" --> [%ith Player RESULT] pose : %i | type : %s | energy : +%i\n", 
                      player, smm_players[player].pos, typeName, energy);
                      
             smm_players[player].energy += energy;
             
             if (smm_players[player].credit >= GRADUATE_CREDIT)
                smm_players[player].flag_graduated = 1; // graduation
             break;
             
        case SMMNODE_TYPE_GOTOLAB:
             printf(" --> [%ith Player RESULT] pose : %i | type : %s \n", player, smm_players[player].pos, typeName);
             
             // no laboratory node
             if (labPos < 0)
             {
                printf("     [GOTOLAB] No LABOTRATORY node exists.\n");
                break;
             }
             // set experimenting state
             smm_players[player].flag_experimenting = 1;
             
             // set experimenting success threshold randomly
             smm_players[player].exp_success_thr = (rand()%MAX_DIE +1 );
             
             // move to laboratory 
             smm_players[player].pos = labPos;
             
             printf("     [GOTOLAB] Experimenting Success Threshold = %i. Move to LABORATORY at %i\n", 
                      smm_players[player].exp_success_thr, labPos);
             
             break;
             
        case SMMNODE_TYPE_FOODCHANCE:
             printf(" --> [%ith Player RESULT] pose : %i | type : %s | food : %s | energy : %i\n", 
                      player, smm_players[player].pos, typeName, foodName, foodEnergy);
                      
             smm_players[player].energy += foodEnergy;
             break;
             
        case SMMNODE_TYPE_FESTIVAL:
             printf(" --> [%ith Player RESULT] pose : %i | type : %s | fest : %s\n", 
                      player, smm_players[player].pos, typeName, fest);
             break;
        //case lecture:
        default:
            break;
     }
}

void freeAll(void)
{
    int i;
    int last;

    // 1) board nodes
    while ((last = smmdb_len(LISTNO_NODE) - 1) >= 0)
        smmdb_deleteData(LISTNO_NODE, last);

    // 2) food cards
    while ((last = smmdb_len(LISTNO_FOODCARD) - 1) >= 0)
        smmdb_deleteData(LISTNO_FOODCARD, last);

    // 3) festival cards
    while ((last = smmdb_len(LISTNO_FESTCARD) - 1) >= 0)
        smmdb_deleteData(LISTNO_FESTCARD, last);

    // 4) grades
    for (i = 0; i < smm_player_nr; i++)
    {
        while ((last = smmdb_len(LISTNO_OFFSET_GRADE + i) - 1) >= 0)
            smmdb_deleteData(LISTNO_OFFSET_GRADE + i, last);
    }

    // 5) players
    free(smm_players);
    smm_players = NULL;
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
    
    char fest[MAX_CHARNAME]; // festival
    
    int turn;
    
    smm_board_nr = 0;
    smm_food_nr = 0;
    smm_fest_nr = 0;
    
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
          //store the parameter set
          void *foodPtr = smmObj_genFood(foodName, foodEnergy);
          if (smmdb_addTail(LISTNO_FOODCARD, foodPtr) < 0) {
             printf("[ERROR] failed to add food \n");
             return -1;
          }
          smm_food_nr++; 
    }
    fclose(fp);
    printf("Total number of food cards : %i\n", smm_food_nr);
    
    //3. festival card config 
    if ((fp = fopen(FESTFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FESTFILEPATH);
        return -1;
    }
    
    printf("\n\nReading festival card component......\n");
    while (fscanf(fp, "%s", fest) == 1) //read a festival card string
    {
          //store the parameter set
          void *festPtr = smmObj_genFest(fest);
          if (smmdb_addTail(LISTNO_FESTCARD, festPtr) < 0)
          {
             printf("[ERROR] failed to add festival \n");
             return -1;
          }
          smm_fest_nr++;
    }
    fclose(fp);
    printf("Total number of festival cards : %i\n", smm_fest_nr);

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
        
        void *posPtr = smmdb_getData(LISTNO_NODE, smm_players[turn].pos);
        int posType = smmObj_getObjectType(posPtr);
        
        //4-1. initial printing
        printPlayerStatus();
        
        // experimenting state -> no goForward
        if (smm_players[turn].flag_experimenting == 1 && posType == SMMNODE_TYPE_LABORATORY)
        {
            actionNode(turn);
                             
            turn = (turn + 1) % smm_player_nr;
            continue;
        }
        
        //4-2. die rolling (if not in experiment)
        die_result = rolldie(turn);
        
        //4-3. go forward
        goForward(turn, die_result);
        
        posPtr = smmdb_getData(LISTNO_NODE, smm_players[turn].pos); // update pose
        printf("node : %s, type : %i (%s)\n", 
                     smmObj_getObjectName(posPtr), 
                     smmObj_getObjectType(posPtr), 
                     smmObj_getObjectTypeName(posPtr));
        
		//4-4. take action at the destination node of the board
        actionNode(turn);
        
        //4-5. next turn
        turn = (turn + 1)%smm_player_nr;
    }
    
    int graduatedPlayer = getGraduatedPlayerIdx();
    
    if (graduatedPlayer >= 0)
    {
       printf("\n============= Game over ====\n");
       printf("Graduated Player: %s (credit: %i)\n", 
                         smm_players[graduatedPlayer].name, smm_players[graduatedPlayer].credit);
       printGrades(graduatedPlayer);
    }
    
    // memory free
    freeAll();
    
    system("PAUSE");
    return 0;
}
