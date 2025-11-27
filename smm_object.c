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
#define MAX_GRADE       9   // grade array

static char smmObj_nodeName[MAX_NODETYPE][MAX_CHARNAME] = {
       "lecture",
       "restaurant",
       "laboratory",
       "home",
       "gotoLab",
       "foodChance",
       "festival"
};

static int smm_nodeNr = 0; // saved node number

typedef struct {
       char name[MAX_CHARNAME];
       int type;
       int credit;
       int energy;
} smmObj_board_t; // _t: type (not instance)

static smmObj_board_t smmObj_board[MAX_NODENR];

//object generation
int smmObj_genNode(char* name, int type, int credit, int energy)
{
    strcpy(smmObj_board[smm_nodeNr].name, name);
    smmObj_board[smm_nodeNr].type = type;
    smmObj_board[smm_nodeNr].credit = credit;
    smmObj_board[smm_nodeNr].energy = energy;
    
    #if 0
    strcpy(smm_name[smm_nodeNr], name);
    smm_type[smm_nodeNr] = type;
    smm_credit[smm_nodeNr] = credit;
    smm_energy[smm_nodeNr] = energy;
    #endif
     
    smm_nodeNr++;
     
    return (smm_nodeNr);
}

//member retrieving
char* smmObj_getNodeName(int node_nr) // print class name
{
      return (smmObj_board[node_nr].name);
}

int smmObj_getNodeType(int node_nr)
{
      return (smmObj_board[node_nr].type);
}

int smmObj_getNodeEnergy(int node_nr)
{
      return (smmObj_board[node_nr].energy);
}

char* smmObj_getTypeName(int node_type)
{
      return (smmObj_board[node_type].name);
}

int smmObj_getNodeCredit(int node_nr)
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
