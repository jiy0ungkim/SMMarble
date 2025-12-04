//
//  smm_node.c
//  SMMarble
//
//  Created by Juyeop Kim on 2023/11/05.
//

#include "smm_common.h"
#include "smm_object.h"
#include <string.h>

#define MAX_NODENR      100 // node array
#define MAX_NODETYPE    7   // node type array

#define GRADE_A+ =      0
#define GRADE_A0 =      1
#define GRADE_A- =      2
#define GRADE_B+ =      3
#define GRADE_B0 =      4
#define GRADE_B- =      5
#define GRADE_C+ =      6
#define GRADE_C0 =      7
#define GRADE_C- =      8

static char smmObj_nodeName[MAX_NODETYPE][MAX_CHARNAME] = {
       "lecture",
       "restaurant",
       "laboratory",
       "home",
       "gotoLab",
       "foodChance",
       "festival"
};

static char smmObj_gradeName[MAX_GRADE][MAX_CHARNAME] = {
       "A+",
       "A0",
       "A-",
       "B+",
       "B0",
       "B-",
       "C+",
       "C0",
       "C-"
};

//structure type definition
typedef struct {
       char name[MAX_CHARNAME];
       int objType; 
       int type; // node type
       int credit;
       int energy;
       int grade;
} smmObj_object_t; // _t: type (not instance)


//static smmObj_board_t smmObj_board[MAX_NODENR];


//object generation
void* smmObj_genObject(char* name, int objType, int type, int credit, int energy, int grade)
{
    smmObj_object_t *ptr;
    
    ptr = (smmObj_object_t *)malloc(sizeof(smmObj_object_t));
    
    strcpy(ptr->name, name);
    ptr->type = type;
    ptr->objType = objType;
    ptr->credit = credit;
    ptr->energy = energy;
    ptr->grade = grade;
    
    return ((void*)ptr);
}

//member retrieving
char* smmObj_getObjectName(void *ptr) // print class name
{
      smmObj_object_t *objPtr = (smmObj_object_t *)ptr;
      return (objPtr->name);
}

int smmObj_getObjectType(void *ptr)
{
    smmObj_object_t *objPtr = (smmObj_object_t *)ptr;
    return (objPtr->type);
}

int smmObj_getObjectEnergy(void *ptr)
{
    smmObj_object_t *objPtr = (smmObj_object_t *)ptr;
    return (objPtr->energy);
}

char* smmObj_getObjectTypeName(void *ptr)
{
      return (smmObj_board[node_type].name);
}

int smmObj_getObjectCredit(void *ptr)
{
    return (smmObj_board[node_nr].credit);
}

#if 0
//element to string
char* smmObj_getNodeName(smmNode_e type)
{
    return smmNodeName[type];
}

char* smmObj_getGradeName(smmGrade_e grade)
{
    return smmGradeName[grade];
}
#endif
