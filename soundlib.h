#ifndef SOUNDLIB_H
#define SOUNDLIB_H

#pragma comment (lib, "User32.lib")

#include <stdint.h>
#include <map>
#include <stdio.h>

/************
need: cycle detection
************/


class Ctl;
static Ctl* glob_ctl;
static bool GLOB_SET = 0;
static unsigned int g_id = 0;

typedef struct{
    uint32_t* value;
    size_t num;
}Msg;

typedef struct{
    uint16_t a;
    uint16_t b;
}Pair;


/*** Ctl base class ***/
class Ctl{

public:

    Msg m = {0,0};
    unsigned int id; 
    std::map <int, Ctl*> childs;

    Ctl(int _master = 0){
        if(_master)
        if(!GLOB_SET){ 
            master = 1;
            GLOB_SET = 1;
        }
        this->id = ++g_id;
        if(!_master){
            glob_ctl->connect(this);
        }
    }

    Ctl(const Ctl& obj){
        m.num = obj.m.num;
        m.value = new uint32_t[obj.m.num];
        for(int i = 0; i < m.num; i++){
            m.value[i] = obj.m.value[i];
        }
    }

    virtual ~Ctl(){
        delete[] m.value;
    }

    // user override
    virtual void run(){}
    virtual void run(Msg _m){}

    void connect(Ctl* child){ 
        childs[child->id] = child;
        if(!master)
            glob_ctl->disconnect(child);
    }
    
    void connect(Ctl** child, int num){
        for(int i = 0; i < num; i++){
            childs[child[i]->id] = child[i];
            if(!master)
                glob_ctl->disconnect(child[i]);
        }
    }

    void disconnect(Ctl* child){
        childs.erase(child->id);
    }
    
    void disconnect(Ctl** child, int num){
        for(int i = 0; i < num; i++){
            childs.erase(child[i]->id);
        }
    }

protected:

    int master = 1;
    
    inline void call(Msg _m){ 
        // do stuff, alter this.m..
        if(_m.num){ 
            run(_m);
        }else{
            run(); 
        }
        callChildren(m);
    }

    inline void call(){ 
        run(); 
        callChildren(m);
    }

    void msg_alloc(size_t num){ 
         m.num = num;
         m.value = new uint32_t[num];
    }    
    
    inline void callChildren(Msg _m){ 
        for(auto p : childs)
            p.second->call(_m);
    }

};

class Sig{

};

/********  init  *************/

class Glob_Ctl : public Ctl{
public:
    Glob_Ctl() : Ctl(1){}
    void run(){ callChildren(m); }
};

void sl_init(){
    glob_ctl = new Glob_Ctl();
}

void call_ctl(){
    glob_ctl->run();
}


#endif /* SOUNDLIB_H */