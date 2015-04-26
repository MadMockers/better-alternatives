
/**
 * HOW TO:
 *
 * == Header file:
 * 
 *  #include "class.h"
 *  #define CLASS_NAME(a,b) a## NameOfClass ##b
 *
 *  // Classes with no super-class should have 'Object' as their super class
 *  CLASS(SuperClass)
 *      int METHOD(MethodName, int arg1, int arg2);
 *
 *      int fieldName;
 *  END_CLASS
 *  #undef CLASS_NAME // NameOfClass
 *
 *  == C file:
 *
 *  #include "hdr_file.h"
 *
 *  #define CLASS_NAME(a,b) a## NameOfClass ##b
 *  // constructor implementation (if it needs to be over-ridden)
 *  NameOfClass METHOD_IMPL(construct)
 *  {
 *      // the base object is the object where the method being
 *      // called was declared - The constructor is declared in Object
 *      SUPER_CALL(Object, this, construct);
 *      // class initiation code..
 *      return this;
 *  }
 *
 *  int METHOD_IMPL(MethodName, int arg1, int arg2)
 *  {
 *      return this->fieldName + arg1 + arg2;
 *  }
 *
 *  VIRTUAL(SuperClass)
 *  {
 *      // over-rides the default constructor
 *      VMETHOD_BASE(Object, construct);
 *      VMETHOD(MethodName);
 *
 *      // sets the initial value of a field when a new object is created
 *      VFIELD(fieldName) = 5;
 *  }
 *  #undef CLASS_NAME // NameOfClass
 *
 *  == Using a class:
 *
 *  int main()
 *  {
 *      NameOfClass obj = NEW(NameOfClass);
 *
 *      printf("%d\n", CALL(obj, MethodName, 5, 7));
 *      obj->fieldName = 1;
 *      printf("%d\n", CALL(obj, MethodName, 1, 3));
 *
 *      DELETE(obj);
 *
 *      return 0;
 *  }
 *
 */

#ifndef CLASS_H
#define CLASS_H

#include <string.h>

#include "util.h"
#include "list.h"

typedef const struct Object *ClassDefinition;

/* merely decares the pointer type such that it can be used
 * where there's a dependency cycle */
#define DECLARE_CLASS(name) typedef struct name *name

/* declares the required typedefs, and starts defining the struct.
 * Additional members / methods are declared in the object within
 * the CLASS and END_CLASS macros */
#define CLASS(spr, ...)                                                     \
typedef struct CLASS_NAME(,) *CLASS_NAME(,);                                \
typedef struct CLASS_NAME(,) CLASS_NAME(,_t);                               \
typedef CLASS_NAME(,)                                                       \
    (*CLASS_NAME(__,_constructor_func))(CLASS_NAME(,), ## __VA_ARGS__);     \
extern ClassDefinition CLASS_NAME(__,);                                     \
void CLASS_NAME(__init_class_,)(void);                                      \
struct CLASS_NAME(,) {                                                      \
    spr ## _t super;                                                        \
    spr __super__;                                                          \
    CLASS_NAME(,) __class__;                                                \

/* Ends a class definition */
#define END_CLASS   };

/* Used inside CLASS .. END_CLASS macros to declare a method
 * inside a class. Usage:
 * return_type METHOD(method_name, arg_types...) */
#define METHOD(name, ...)                                                   \
    (*name)(CLASS_NAME(,) this, ## __VA_ARGS__)

#define CONSTRUCT(type, ptr, ...)                                           \
    (__init_class_ ## type(),                                               \
     memcpy(ptr, &__ ## type, sizeof(type ## _t)),                          \
     ((Object)ptr)->malloced = 0,                                           \
     (type)((void*(*)())((Object)__ ## type)->construct)                    \
        (ptr, ## __VA_ARGS__))

/* Allocates memory and copies into it the class identity structure.
 * The objects constructor is also called */
#define NEW(type, ...)                                                      \
    (__init_class_ ## type(),                                               \
    (type)((__ ## type ## _constructor_func)((Object)__ ## type)->construct)\
        ((type)memdup((type)__ ## type, sizeof(struct type)), ## __VA_ARGS__))

/* Same as NEW, however instead of passing in a class name,
 * pass in a reference to the class definition */
#define NEW_DYNAMIC(clazz, ...)                                             \
    ((Object)((void*(*)())((Object)clazz)->construct)(                      \
        (Object)memdup(                                                     \
            (void*)clazz, ((Object)clazz)->class_size), ## __VA_ARGS__))

#define NEW_STR(str, ...)                                                   \
    NEW_DYNAMIC(get_class(str), ## __VA_ARGS__)

#if 0
#define DECONSTRUCT(obj)                                                    \
    ((Object)obj)->deconstruct((Object)obj)
#endif

/* Calls can objects deconstructor, and then frees the memory */
#define DELETE(obj)                                                         \
    (((Object)obj)->deconstruct((Object)obj),                               \
    free(((Object)obj)->malloced?obj:NULL),                                 \
    obj = NULL)

/* Helper to define a method implementation. Usage:
 * return_type METHOD_IMPL(method_name, method_args..)
 * {
 *     ... 
 * }
 */
#define METHOD_IMPL(name, ...)                                              \
    CLASS_NAME(,_ ## name)(CLASS_NAME(,) this, ## __VA_ARGS__)

/* Defines a function which sets up the 'identity' copy of this object.
 * A copy of this is taken every time an instance of the option is created.
 * VMETHOD.. and VFIELD.. helper functions fill in the methods / fields,
 * however for more complicated operations, this is merely a function. The
 * identity object is pointed to by 'this'. */
#define STRINGIFY(s) #s
#define XSTRINGIFY(s) STRINGIFY(s)
#define VIRTUAL(spr)                                                        \
static struct class_register CLASS_NAME(__,_register);                      \
static CLASS_NAME(,_t) CLASS_NAME(__,_alloc);                               \
ClassDefinition CLASS_NAME(__,);                                            \
void CLASS_NAME(__init_class_,)() {                                         \
    static int __initted = 0;                                               \
    if(__initted)                                                           \
        return;                                                             \
    __initted = 1;                                                          \
    __init_class_ ## spr();                                                 \
    CLASS_NAME(,) this = CLASS_NAME(&__,_alloc);                            \
    CLASS_NAME(__,) = (Object) this;                                        \
    memcpy(&this->super, __ ## spr, sizeof(spr ## _t));                     \
    this->__super__ = (spr) __ ## spr;                                      \
    this->__class__ = this;                                                 \
    ((Object)this)->class_size = sizeof(CLASS_NAME(,_t));                   \
    ((Object)this)->pure_virtual = 0;                                       \
    ((Object)this)->class_name = XSTRINGIFY(CLASS_NAME(,));                 \
    ((Object)this)->class_def = (ClassDefinition)this;                      \
    struct class_register *i;                                               \
    list_for_each_entry(i, &classList, list)                                \
    {                                                                       \
        if(strcmp(i->name, ((Object)this)->class_name) == 0)                \
        {                                                                   \
            list_del(&i->list);                                             \
            break;                                                          \
        }                                                                   \
    }                                                                       \
    list_add(CLASS_NAME(&__,_register).list, &classList);                   \
    CLASS_NAME(__,_register).name = ((Object)this)->class_name;             \
    CLASS_NAME(__,_register).definition = CLASS_NAME(__,);

/* Ends a class definition */
#define END_VIRTUAL }

/* For use between VIRTUAL .. END_VIRTUAL. Helper function to set
 * a method. Alternatively, a method can be set merely with:
 * this->method_name = method_implementation */
#define VMETHOD(name)                                                       \
    this->name = (void*) CLASS_NAME(,_ ## name)

/* For use between VIRTUAL .. END_VIRTUAL. Helper function to over-ride
 * a method. Alternatively, a method can be set merely with:
 * ((BaseClass)this)->method_name = method_implementation */
#define VMETHOD_BASE(base, name)                                            \
    ((base)this)->name = (void*) CLASS_NAME(,_ ## name)

/* Usage: VFIELD(name) = value; */
#define VFIELD(name)                                                        \
    this->name
#define VFIELD_BASE(base, name)                                             \
    ((base)this)->name

/* Defines a class as pure virtual - Can not be directly instantiated */
#define V_PURE  ((Object)this)->pure_virtual = 1;

/* The 'NEW' macro automatically initiates a class. If the identity
 * copy of a class needs to be set up before a new instance is created
 * via 'NEW', use this */
#define INIT_CLASS(name)                                                    \
    __init_class_ ## name()

#define GET_CLASS(name) __ ## name

#define INSTANCE_OF(obj, clazz)                                             \
    (((clazz)obj)->__class__ == (clazz)(__ ## clazz))

/* Calls a method that doesn't exist in the class declaration
 * (i.e, it's private) */
#define PRIV_CALL(me, func, ...)                                            \
    CLASS_NAME(,_ ## func)(me, ## __VA_ARGS__)
/* Calls a method in an objects class */
#define CALL(me, func, ...)                                                 \
    (me)->func(me, ## __VA_ARGS__)
/* Calls a method that is declared by a base class of the object */
#define SUPER_CALL(base, me, func, ...)                                     \
    ((base)(me)->__super__)->func((base)me, ## __VA_ARGS__)

/* If a class can't be instantiated, use this */
#define CONSTRUCT_FAILED()                                                  \
    if(((Object)this)->malloced) free(this);                                \
    return NULL;

/* Calls the super classes constructor */
#define CONSTRUCT_CALL(...)                                                 \
    this =                                                                  \
        ((CLASS_NAME(,)(*)())((Object)this->__super__)->construct)          \
            (this, ## __VA_ARGS__);                                         \
    if(!this) return NULL;

struct class_register
{
    struct list_head list;
    char *name;

    ClassDefinition definition;
};
extern struct list_head classList;
ClassDefinition get_class(char *name);

/* The 'Object' class, which all other classes will inherit from.
 * This is defined manually as it's somewhat different to a normal class */
typedef struct Object *Object;
typedef struct Object Object_t;
typedef Object(*__Object_constructor_func)(Object);
extern ClassDefinition __Object;
struct Object
{
    void *__super__;
    Object __class__;
    Object (*construct)(Object);
    void (*deconstruct)(Object);
    size_t class_size;
    char *class_name;
    ClassDefinition class_def;
    int malloced:1,
        subclassed:1,
        pure_virtual:1;
};

void __init_class_Object(void);
#endif
