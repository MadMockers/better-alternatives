
#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>

#include "class.h"
#include "map.h"
#include "debug.h"

LIST_HEAD(classList);

ClassDefinition get_class(char *name)
{
    char buf[128];
    snprintf(buf, sizeof(buf), "__init_class_%s", name);
    void (*init)(void) = (void(*)(void))dlsym(RTLD_DEFAULT, buf);
    if(init)
    {
        init();
    }
    else
    {
        DPRINTF("Failed to find symbol '%s': %s\n", buf, dlerror());
    }
    struct class_register *i;
    list_for_each_entry(i, &classList, list)
    {
        if(strcmp(i->name, name) == 0)
            return i->definition;
    }
    return NULL;
}

#define CLASS_NAME(a,b) a## Object ##b
static Object METHOD_IMPL(construct)
{
    if(this->pure_virtual)
    {
        DPRINTF("Attempted to initialize purely virtual class %s\n", this->class_name);
        CONSTRUCT_FAILED();
    }
    return this;
}

static void METHOD_IMPL(deconstruct){}

static struct class_register __Object_register;
static Object_t __Object_alloc;
ClassDefinition __Object;

void __init_class_Object()
{
    static int initted = 0;
    if(initted)
        return;
    initted = 1;
    Object this = &__Object_alloc;
    __Object = this;
    this->__super__ = NULL;
    this->class_size = sizeof(Object_t);
    this->malloced = 1;
    this->subclassed = 0;
    this->pure_virtual = 0;
    this->class_name = "Object";
    list_add(&__Object_register.list, &classList);
    __Object_register.name = this->class_name;
    __Object_register.definition = __Object;

    VMETHOD(construct);
    VMETHOD(deconstruct);
}
#undef CLASS_NAME
