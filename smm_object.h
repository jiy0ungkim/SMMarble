//
//  smm_object.h
//  SMMarble object
//
//  Created by Juyeop Kim on 2023/11/05.
//

#ifndef smm_object_h
#define smm_object_h

/* node type :
    lecture,
    restaurant,
    laboratory,
    home,
    experiment,
    foodChance,
    festival
*/

#define SMMNODE_TYPE_LECTURE         0
#define SMMNODE_TYPE_RESTAURANT      1
#define SMMNODE_TYPE_LABORATORY      2
#define SMMNODE_TYPE_HOME            3
#define SMMNODE_TYPE_GOTOLAB         4
#define SMMNODE_TYPE_FOODCHANCE      5
#define SMMNODE_TYPE_FESTIVAL        6

#define SMMNODE_OBJTYPE_BOARD   0
#define SMMNODE_OBJTYPE_GRADE   1
#define SMMNODE_OBJTYPE_FOOD    2
#define SMMNODE_OBJTYPE_FEST    3

#define SMMNODE_MAX_GRADE       9  // grade array

/* grade :
    A+,
    A0,
    A-,
    B+,
    B0,
    B-,
    C+,
    C0,
    C-
*/

//object generation
void* smmObj_genObject(char* name, int objType, int type, int credit, int energy, int grade);
char* smmObj_getObjectName(void *ptr);
int smmObj_getObjectType(void *ptr);
char* smmObj_getObjectTypeName(void *ptr);
int smmObj_getObjectEnergy(void *ptr);
int smmObj_getObjectCredit(void *ptr);

// object generation (grade)
char* smmObj_getObjectGradeName(void *ptr);

// 
void* smmObj_genObjectFood(char* foodName, int foodEnergy);
void* smmObj_genObjectFest(char* fest);


//member retrieving




#endif /* smm_object_h */
