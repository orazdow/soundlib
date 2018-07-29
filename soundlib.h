#ifndef SOUNDLIB_H
#define SOUNDLIB_H

#pragma comment (lib, "User32.lib")

#include <stdint.h>
#include <map>
#include <stdio.h>
#include "soundlib_glob.h"

/************
need: cycle detection
************/

/**** Msg struct ****/
struct Note{
 unsigned short note;
 uint8_t vel;
 bool on;
 short pitch;
};

union Val{
    uint32_t _i;
    float _f; 
    Note _n;
};

struct Msg{
    Val* value;
    size_t num;
};


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
        m.value = new Val[obj.m.num];
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

    int master = 0;
    
    void call(Msg _m){ 
        // do stuff, alter m..
        if(_m.num){ 
            run(_m);
        }else{
            run(); 
        }
        callChildren(m);
    }

    void call(){ 
        run(); 
        callChildren(m);
    }

    void msg_alloc(size_t num){ 
         m.num = num;
         m.value = new Val[num];
    }    
    
    void callChildren(Msg _m){ 
        for(auto p : childs)
            p.second->call(_m);
    }

};


/**** Sig base ****/
class Sig{ 
public:
    float input, output;
    virtual float out(){ return 0;}
    virtual float out(double step){ return 0;}
};

/***** env base *****/
class Env : public Sig{
public: 
   unsigned int on = 0; 
   virtual float out(unsigned int trig){ return 0;}
   virtual void reset(){}
};

/**** Voice base ****/
class Voice : public Sig, public Ctl{
public:
    virtual float out(float freq, int trig){ return 0; } 
    virtual float out(int note, int trig){ return 0; }
    virtual float out(Note note){ return 0; }
    virtual float out(){ return 0; }
    virtual Env* getEnv(){return NULL;}
};

/***** Ctl root class *****/
class Glob_Ctl : public Ctl{
public:
    Glob_Ctl() : Ctl(1){}
    void run(){ callChildren(m); }
};


/********  init  *************/
void sl_init(){
    glob_ctl = new Glob_Ctl();
    init_globals();
}

void call_ctl(){
    glob_ctl->run();
}


#endif /* SOUNDLIB_H */