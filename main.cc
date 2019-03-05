#include <cstdio>
#include <cmath>
#include <cstring>
#include <map>
#include <vector>
#include <string>

#ifndef WIN
#include <dlfcn.h>
#else
#include <windows.h>
#endif

class simlab {
public:
    simlab() : type(0), length(0) {}
    simlab(int v) : type(v), length(0) {}
    simlab(int v, int l) : type(v), length(l) {}
    int type;
    int length;
};

template<class T> class virtualbuffer : public simlab {
public:
    virtualbuffer() : simlab(0) {}
    virtualbuffer(int type) : simlab(type) {}
    virtualbuffer(int type, int size) : simlab(type,size) {}
    virtual T operator[](int i) const {
        return 0;
    }
    /*virtual T& operator[](int i) {
        return 0;
    }*/
    virtual void operator()(int i,T v) {}
};

template<class T> class buffer : public virtualbuffer<T> {
public:
    buffer() : virtualbuffer<T>(0), buf(0) {}
    buffer(int size) : virtualbuffer<T>(0,size), buf(0) {}
    virtual T operator[](int i) const {
        return buf[i];
    }
    virtual T& operator[](int i) {
        return buf[i];
    }
    virtual void operator()(int i,T v) {
        buf[i] = v;
    }
    T* buf;
};

template<> virtualbuffer<unsigned char>::virtualbuffer() : simlab(8) {}
template<> virtualbuffer<char>::virtualbuffer() : simlab(9) {}
template<> virtualbuffer<unsigned short>::virtualbuffer() : simlab(16) {}
template<> virtualbuffer<short>::virtualbuffer() : simlab(17) {}
template<> virtualbuffer<unsigned int>::virtualbuffer() : simlab(32) {}
template<> virtualbuffer<int>::virtualbuffer() : simlab(33) {}
template<> virtualbuffer<float>::virtualbuffer() : simlab(34) {}
template<> virtualbuffer<unsigned long long>::virtualbuffer() : simlab(64) {}
template<> virtualbuffer<long long>::virtualbuffer() : simlab(65) {}
template<> virtualbuffer<double>::virtualbuffer() : simlab(66) {}

template<> buffer<unsigned char>::buffer(int size) : virtualbuffer(8,size), buf((unsigned char*)realloc(NULL, size*sizeof(unsigned char))) {}
template<> buffer<char>::buffer(int size) : virtualbuffer(9,size), buf((char*)realloc(NULL, size*sizeof(char))) {}
template<> buffer<unsigned short>::buffer(int size) : virtualbuffer(16,size), buf((unsigned short*)realloc(NULL, size*sizeof(unsigned short))) {}
template<> buffer<short>::buffer(int size) : virtualbuffer(17,size), buf((short*)realloc(NULL, size*sizeof(short))) {}
template<> buffer<unsigned int>::buffer(int size) : virtualbuffer(32,size), buf((unsigned int*)realloc(NULL, size*sizeof(unsigned int))) {}
template<> buffer<int>::buffer(int size) : virtualbuffer(33,size), buf((int*)realloc(NULL, size*sizeof(int))) {}
template<> buffer<float>::buffer(int size) : virtualbuffer(34,size), buf((float*)realloc(NULL, size*sizeof(float))) {}
template<> buffer<unsigned long long>::buffer(int size) : virtualbuffer(64,size), buf((unsigned long long*)realloc(NULL, size*sizeof(unsigned long long))) {}
template<> buffer<long long>::buffer(int size) : virtualbuffer(65,size), buf((long long*)realloc(NULL, size*sizeof(long long))) {}
template<> buffer<double>::buffer(int size) : virtualbuffer(66,size), buf((double*)realloc(NULL, size*sizeof(double))) {}

/*template<> class virtualbuffer<float> {
public:
    virtualbuffer() : type(34) {}
    virtual float operator[](int i) {
        return 0;
    }
    int type;
};*/

template<class T> class cnst : public virtualbuffer<T> {
public:
    cnst(T co) : c(co) {}
    virtual T operator[](int i) const {
        return c;
    }
    T c;
};

class idx : public virtualbuffer<int> {
public:
    virtual int operator[](int i) const {
        return i;
    }
};

class triangular : public virtualbuffer<int> {
public:
    virtual int operator[](int i) const {
        return i*(i+1)/2;
    }
};

class fibonacci : public virtualbuffer<int> {
public:
    fibonacci() {
        sqrt5 = sqrt(5);
        Phi = (1+sqrt5)/2;
        phi = (1-sqrt5)/2;
    }
    virtual int operator[](int i) const {
        return (pow(Phi,i) - pow(phi,i))/sqrt5;
    }
    double sqrt5;
    double Phi;
    double phi;
};

class nidx : public virtualbuffer<int> {
public:
    virtual int operator[](int i) const {
        return -i;
    }
};

template<class T,class K> class map : public virtualbuffer<T> {
public:
    map(virtualbuffer<K> & m) : a(m) {}
    virtual T operator[](int i) const {return a[i];}
    virtualbuffer<K> & a;
};

template<class T> class mapping : public map<T,int> {
public:
    mapping(virtualbuffer<T> & m, virtualbuffer<int> & n) : map<T,int>(n), b(m) {}
    virtual T operator[](int i) const { return b[map<T,int>::a[i]]; }
    virtualbuffer<T> & b;
};

template<class T> class merge : public map<T,T> {
public:
    merge(virtualbuffer<T> & m, virtualbuffer<T> & n) : map<T,T>(m), b(n) {}
    virtual T operator[](int i) const { return map<T,T>::a[b[i]]; }
    virtualbuffer<T> & b;
};

template<class K,class T> class cast : public map<K,T> {
public:
    cast(virtualbuffer<T> & m) : map<K,T>(m) {}
    virtual K operator[](int i) const {
        return (K)map<K,T>::a[i];
    }
};

template<class K,class T> class pcast : public map<K,T> {
public:
    pcast(virtualbuffer<T> & m) : map<K,T>(m), d(sizeof(T)/sizeof(K)) {}
    virtual K operator[](int i) const {
        int k = (i*sizeof(K))/sizeof(T);
        T t = map<K,T>::a[k];
        return ((K*)&t)[i%d];
    }
    int d;
};

template<typename K,template<typename M,typename N> class T> simlab* subcast(int val, virtualbuffer<K> & vb) {
    if( val == 8 ) {
        return new T<unsigned char,K>(vb);
    } else if( val == 9 ) {
        return new T<char,K>(vb);
    } else if( val == 16 ) {
        return new T<unsigned short,K>(vb);
    } else if( val == 17 ) {
        return new T<short,K>(vb);
    } else if( val == 32 ) {
        return new T<unsigned int,K>(vb);
    } else if( val == 33 ) {
        return new T<int,K>(vb);
    } else if( val == 34 ) {
        return new T<float,K>(vb);
    } else if( val == 66 ) {
        return new T<double,K>(vb);
    }
    return NULL;
}

template<template<class M,class K> class T> simlab* scast(int val, simlab* sl) {
    if( sl->type == 16 ) {
        virtualbuffer<unsigned short> & usvb = *(virtualbuffer<unsigned short>*)sl;
        return subcast<unsigned short,T>(val,usvb);
    } else if( sl->type == 17 ) {
        virtualbuffer<short> & svb = *(virtualbuffer<short>*)sl;
        return subcast<short,T>(val,svb);
    } else if( sl->type == 32 ) {
        virtualbuffer<unsigned int> & uivb = *(virtualbuffer<unsigned int>*)sl;
        return subcast<unsigned int,T>(val,uivb);
    } else if( sl->type == 33 ) {
        virtualbuffer<int> & ivb = *(virtualbuffer<int>*)sl;
        return subcast<int,T>(val,ivb);
    } else if( sl->type == 34 ) {
        virtualbuffer<float> & fvb = *(virtualbuffer<float>*)sl;
        return subcast<float,T>(val,fvb);
    } else if( sl->type == 64 ) {
        virtualbuffer<unsigned long long> & ullvb = *(virtualbuffer<unsigned long long>*)sl;
        return subcast<unsigned long long,T>(val,ullvb);
    } else if( sl->type == 65 ) {
        virtualbuffer<long long> & llvb = *(virtualbuffer<long long>*)sl;
        return subcast<long long,T>(val,llvb);
    } else if( sl->type == 66 ) {
        virtualbuffer<double> & dvb = *(virtualbuffer<double>*)sl;
        return subcast<double,T>(val,dvb);
    }
    return NULL;
}

template<class K,class T> class arith : public map<K,T> {
public:
    arith(virtualbuffer<T> & m, K (*func)(K)) : map<K,T>(m), f(func)/*, ifc(invertfuncmap[func])*/ {}
    virtual K operator[](int i) const {
        return f(map<K,T>::a[i]);
    }
    /*virtual K& operator[](int i) {
        return map<K,T>::a[i];
    }*/
    K (*f)(K);
    //K (*ifc)(K);
};

template<class T> class slc_floor : public arith<double,T> {
public:
    slc_floor(virtualbuffer<T> & m) : arith<double,T>(m,floor) {}
};

template<class T> class slc_floorf : public arith<float,T> {
public:
    slc_floorf(virtualbuffer<T> & m) : arith<float,T>(m,floorf) {}
};

template<class T> class slc_sin : public arith<double,T> {
public:
    slc_sin(virtualbuffer<T> & m) : arith<double,T>(m,sin) {}
};

template<class T> class slc_sinf : public arith<float,T> {
public:
    slc_sinf(virtualbuffer<T> & m) : arith<float,T>(m,sinf) {}
};

template<class T> class slc_logf : public arith<float,T> {
public:
    slc_logf(virtualbuffer<T> & m) : arith<float,T>(m,logf) {}
};

template<class T> class slc_log : public arith<double,T> {
public:
    slc_log(virtualbuffer<T> & m) : arith<double,T>(m,log) {}
};

template<class T> class slc_cos : public arith<double,T> {
public:
    //cast<double,T>(m)
    slc_cos(virtualbuffer<T> & m) : arith<double,T>(m,cos) {}
};

/*template<> class slc_cos<double> : public arith<double,double> {
public:
    slc_cos(virtualbuffer<double> & m) : arith<double,double>(m,cos) { printf("meme2\n"); }
};*/

template<class T> class slc_cosf : public arith<float,T> {
public:
    slc_cosf(virtualbuffer<T> & m) : arith<float,T>(m,cosf) {}
};

template<class T> class diff : public map<T,T> {
public:
    diff(virtualbuffer<T> & m) : map<T,T>(m) {}
    virtual T operator[](int i) const {
        return map<T,T>::a[i+1]-map<T,T>::a[i];
    }
};

template<class T> class sq : public map<T,T> {
public:
    sq(virtualbuffer<T> & m) : map<T,T>(m) {}
    virtual T operator[](int i) const {
        return map<T,T>::a[i]*map<T,T>::a[i];
    }
};

template<class T> class sl_sqrt : public arith<double,T> {
public:
    sl_sqrt(virtualbuffer<T> & m) : arith<double,T>(m,sqrt) {}
};

template<class T> class sl_sqrtf : public arith<float,T> {
public:
    sl_sqrtf(virtualbuffer<T> & m) : arith<float,T>(m,sqrtf) {}
};

template<class T> class sum : public merge<T> {
public:
    sum(virtualbuffer<T> & m, virtualbuffer<T> & n) : merge<T>(m,n) {}
    virtual T operator[](int i) const {
        return merge<T>::a[i]+merge<T>::b[i];
    }
};

template<class T> class sub : public merge<T> {
public:
    sub(virtualbuffer<T> & m, virtualbuffer<T> & n) : merge<T>(m,n) {}
    virtual T operator[](int i) const {
        return merge<T>::a[i]-merge<T>::b[i];
    }
};

template<class T> class mul : public merge<T> {
public:
    mul(virtualbuffer<T> & m, virtualbuffer<T> & n) : merge<T>(m,n) {}
    virtual T operator[](int i) const {
        return merge<T>::a[i]*merge<T>::b[i];
    }
};

template<class T> class neg : public map<T,T> {
public:
    neg(virtualbuffer<T> & m) : map<T,T>(m) {}
    virtual T operator[](int i) const {
        return -map<T,T>::a[i];
    }
};

template<> class pcast<double,unsigned char> : public map<double,unsigned char> {
public:
    pcast(virtualbuffer<unsigned char> & m) : map<double,unsigned char>(m), d(sizeof(double)/sizeof(unsigned char)) {}
    virtual double operator[](int i) const {
        long long l = 0;
        for( int k = 0; k < d; k++ ) {
            l <<= 8;
            l |= a[d*i+k];
        }
        return *((double*)&l);
    }
    int d;
};

template<> class pcast<double,int> : public map<double,int> {
public:
    pcast(virtualbuffer<int> & m) : map<double,int>(m), d(sizeof(double)/sizeof(int)) {}
    virtual double operator[](int i) const {
        unsigned long long l = 0;
        for( int k = 0; k < d; k++ ) {
            l <<= (sizeof(int)*8);
            unsigned long long v = a[d*i+k];
            l |= v;
        }
        return *((double*)&l);
    }
    int d;
};

template<class T> class divd : public merge<T> {
public:
    divd(virtualbuffer<T> & m, virtualbuffer<T> & n) : merge<T>(m,n) {}
    virtual T operator[](int i) const {
        return merge<T>::a[i] / merge<T>::b[i];
    }
};

template<class T> class mod : public merge<T> {
public:
    mod(virtualbuffer<T> & m, virtualbuffer<T> & n) : merge<T>(m,n) {}
    virtual T operator[](int i) const {
        return merge<T>::a[i]%merge<T>::b[i];
    }
};

template <> class mod<float> : public merge<float> {
public:
    mod(virtualbuffer<float> & m, virtualbuffer<float> & n) : merge<float>(m,n) {}
    virtual float operator[](int i) const {
        return fmodf(a[i],b[i]);
    }
};

template <> class mod<double> : public merge<double> {
public:
    mod(virtualbuffer<double> & m, virtualbuffer<double> & n) : merge<double>(m,n) {}
    virtual double operator[](int i) const {
        return fmod(a[i],b[i]);
    }
};

template<class T> class quad : public virtualbuffer<T> {
public:
    quad(virtualbuffer<T> & a, virtualbuffer<T> & b, virtualbuffer<T> & c) : nb(b), ac(a,c), fr(4), frac(fr,ac), b2(b), b2frac(b2,frac), t(b2frac), two(2), denom(two,a), nom(nb,t), res(nom,denom) {}
    virtual T operator[](int i) const {
        return res[i];
    }
    neg<T> nb;
    mul<T> ac;
    cnst<T> fr;
    mul<T> frac;
    sq<T> b2;
    sub<T> b2frac;
    sl_sqrtf<T> t;
    cnst<T> two;
    mul<T> denom;
    sum<T> nom;
    divd<T> res;
};

class flip : public virtualbuffer<int> {
public:
    flip(virtualbuffer<int> & v) : sm(v,ni) {}
    virtual int operator[](int i) const {
        return sm[i];
    }
    nidx ni;
    sum<int> sm;
};

class shift : public virtualbuffer<int> {
public:
    shift(virtualbuffer<int> & v, virtualbuffer<int> & m) : sm(ix,v), md(sm,m) {}
    virtual int operator[](int i) const {
        return md[i];
    }
    idx ix;
    sum<int> sm;
    mod<int> md;
};

class shift2d : public shift {
public:
    shift2d(virtualbuffer<int> & v, virtualbuffer<int> & m) : shift(v,m) {}
    virtual int operator[](int i) const {
        return md[i];
    }
};

class flip2d : public flip {
public:
    flip2d(int r) : one(1), c(r), cm(r-1), ct(r*2), dd(id,c), ml(dd,ct), on(cm,ml), flip(on) {}
    virtual int operator[](int i) const {
        return sm[i];
    }
    idx id;
    cnst<int> one;
    cnst<int> c;
    cnst<int> cm;
    cnst<int> ct;
    divd<int> dd;
    mul<int> ml;
    sum<int> on;
};

class trans : public virtualbuffer<int> {
public:
    trans(int c,int r) : rc(r*c), cls(c), rws(r), ml(id,cls), md(ml,rc), dv(id,rws), sm(md,dv) {}
    virtual int operator[](int i) const {
        return sm[i];
    }
    idx id;
    cnst<int> rc;
    cnst<int> cls;
    cnst<int> rws;
    mul<int> ml;
    mod<int> md;
    divd<int> dv;
    sum<int> sm;
};

class order : public virtualbuffer<int> {
public:
    order(virtualbuffer<int> & t) : a(t) {}
    virtual int operator[](int i) const {
        int k = 0;
        while( a[k] != i ) k++;
        return k;
    }
    virtualbuffer<int> & a;
};

template<typename T> virtualbuffer<T> & quadratic_func(virtualbuffer<T> & a, virtualbuffer<T> & b, virtualbuffer<T> & c) {
    neg<T> nb(b);
    
    /*mul<T> ac(a,c);
    cnst<T> four(4);
    mul<T> four_ac(four,ac);
    sq<T> b2(b);
    sub<T> b2_4ac(b2,four_ac);
    sl_sqrt<T> t(b2_4ac);
    
    cnst<T> two(2);
    mul<T> denom(two,a);

    sum<T> nom(nb,t);
    divd<T> res(nom,denom);*/

    return nb;
}

template<class T> int write(virtualbuffer<T> & s, int l, FILE* file) {
    for( int i = 0; i < l; i++ ) {
        T shrt = s[i];
        fwrite(&shrt, sizeof(T), 1, file);
    }
    return 0;
}

extern "C" int sl_open(char* file) {
    printf("meme %s\n", file);

    char open[] = "open ";
    int len = std::strlen(file);
    char res[len+6];
    res[0] = 0;
    std::strcat(res, open);
    std::strcat(res, file);
    return system(res);
}

simlab* current;
simlab* prev;

extern "C" int sl_idx() {
    current = new idx();
    return 0;
}

extern "C" int sl_cast(simlab* sl) {
    int val = (*(virtualbuffer<int>*)sl)[0];
    current = scast<cast>(val, current);
    return 0;
}

extern "C" int sl_pcast(simlab* sl) {
    int val = (*(virtualbuffer<int>*)sl)[0];
    current = scast<pcast>(val, current);
    return 0;
}

extern "C" int sl_type() {
    printf("%d %d\n", current->type, current->length);
    return 0;
}

extern "C" int sl_buffer(int size, int type) {
    if( type == 32 ) {
        current = new virtualbuffer<unsigned int>(size);
    } else if( type == 33 ) {
        current = new virtualbuffer<int>(size);
    } else if( type == 34 ) {
        current = new virtualbuffer<float>(size);
    } else if( type == 64 ) {
        current = new virtualbuffer<unsigned long long>(size);
    } else if( type == 65 ) {
        current = new virtualbuffer<long long>(size);
    } else if( type == 66 ) {
        current = new virtualbuffer<double>(size);
    }

    return 0;
}

template<template<class M> class T> simlab* sarith(simlab* sl) {
    if( sl->type == 32 ) {
        return new T<unsigned int>(*(virtualbuffer<unsigned int>*)sl);
    } else if( sl->type == 33 ) {
        return new T<int>(*(virtualbuffer<int>*)sl);
    } else if( sl->type == 34 ) {
        return new T<float>(*(virtualbuffer<float>*)sl);
    } else if( sl->type == 64 ) {
        return new T<unsigned long long>(*(virtualbuffer<unsigned long long>*)sl);
    } else if( sl->type == 65 ) {
        return new T<long long>(*(virtualbuffer<long long>*)sl);
    } else if( sl->type == 66 ) {
        return new T<double>(*(virtualbuffer<double>*)sl);
    }
    return NULL;
}

extern "C" int sl_cosf() {
    current = sarith<slc_cosf>(current);
    return 0;
}

extern "C" int sl_cos() {
    current = sarith<slc_cos>(current);
    return 0;
}

extern "C" int sl_sinf() {
    current = sarith<slc_sinf>(current);
    return 0;
}

extern "C" int sl_sin() {
    current = sarith<slc_sin>(current);
    return 0;
}

extern "C" int sl_logf() {
    current = sarith<slc_logf>(current);
    return 0;
}

extern "C" int sl_log() {
    current = sarith<slc_log>(current);
    return 0;
}

extern "C" int sl_floorf() {
    current = sarith<slc_floorf>(current);
    return 0;
}

extern "C" int sl_floor() {
    current = sarith<slc_floor>(current);
    return 0;
}

/*template<class T> int tsl_add(virtualbuffer<T>* a, simlab* b) {
    if( b->type == 32 ) {
        current = new sum<T,unsigned int>(*a, *b);
    } else if( b->type == 33 ) {
        current = new sum<T,unsigned int>(*a, *b);
    }

    return 0;
}*/

template<typename K,template<class M> class T> simlab* submarith(virtualbuffer<K> & sl, simlab* b) {
    if( sl.type == b->type ) return new T<K>(sl, *(virtualbuffer<K>*)b);
    else {
        virtualbuffer<K> & vk = *(virtualbuffer<K>*)scast<cast>(sl.type,b);
        return new T<K>(sl, vk);
    }
    /*if( b->type == 32 ) {
        return new T<K>(sl, *(virtualbuffer<unsigned int>*)b);
    } else if( b->type == 66 ) {
        return new T<K>(sl, *(virtualbuffer<double>*)b);
    }*/
    return NULL;
}

template<template<class M> class T> simlab* marith(simlab* sl, simlab* b) {
    if( sl->type == 32 ) {
        return submarith<unsigned int,T>(*(virtualbuffer<unsigned int>*)sl, b);
    } else if( sl->type == 33 ) {
        return submarith<int,T>(*(virtualbuffer<int>*)sl, b);
    } else if( sl->type == 34 ) {
        return submarith<float,T>(*(virtualbuffer<float>*)sl, b);
    }
    return NULL;
}

extern "C" int sl_neg() {
    if( current->type == 32 ) {
        current = new neg<unsigned int>(*(virtualbuffer<unsigned int>*)current);
    } else if( current->type == 33 ) {
        current = new neg<int>(*(virtualbuffer<int>*)current);
    } else if( current->type == 34 ) {
        current = new neg<float>(*(virtualbuffer<float>*)current);
    }
    return 0;
}

extern "C" int sl_div(simlab* b) {
    current = marith<divd>(current, b);
    return 0;
}

extern "C" int sl_mul(simlab* b) {
    current = marith<mul>(current, b);
    return 0;
}

extern "C" int sl_add(simlab* b) {
    current = marith<sum>(current, b);
    return 0;
}

extern "C" int sl_sub(simlab* b) {
    current = marith<sub>(current, b);
    return 0;
}

extern "C" int sl_mod(simlab* b) {
    current = marith<mod>(current, b);
    return 0;
}

template<typename T> void print(virtualbuffer<T> & vb, const char* nl, const char* tl, int length, int cols) {
    for( int i = 0; i < length; i++ ) printf( i%cols==0 ? nl : tl, vb[i] );
}

extern "C" int sl_print(int cols, int rows) {
    if( current->type == 8 ) {
        virtualbuffer<unsigned char> & fvb = *(virtualbuffer<unsigned char>*)current;
        print<unsigned char>(fvb, "\n%d", "\t%d", rows*cols, cols);
    } else if( current->type == 9 ) {
        virtualbuffer<char> & fvb = *(virtualbuffer<char>*)current;
        print<char>(fvb, "\n%d", "\t%d", rows*cols, cols);
    } else if( current->type == 16 ) {
        virtualbuffer<unsigned short> & fvb = *(virtualbuffer<unsigned short>*)current;
        print<unsigned short>(fvb, "\n%d", "\t%d", rows*cols, cols);
    } else if( current->type == 17 ) {
        virtualbuffer<short> & fvb = *(virtualbuffer<short>*)current;
        print<short>(fvb, "\n%d", "\t%d", rows*cols, cols);
    } else if( current->type == 32 ) {
        virtualbuffer<unsigned int> & fvb = *(virtualbuffer<unsigned int>*)current;
        print<unsigned int>(fvb, "\n%d", "\t%d", rows*cols, cols);
    } else if( current->type == 33 ) {
        virtualbuffer<int> & fvb = *(virtualbuffer<int>*)current;
        print<int>(fvb, "\n%d", "\t%d", rows*cols, cols);
    } else if( current->type == 34 ) {
        virtualbuffer<float> & fvb = *(virtualbuffer<float>*)current;
        print<float>(fvb, "\n%f", "\t%f", rows*cols, cols);
    } else if( current->type == 64 ) {
        virtualbuffer<unsigned long long> & fvb = *(virtualbuffer<unsigned long long>*)current;
        print<unsigned long long>(fvb, "\n%lld", "\t%lld", rows*cols, cols);
    } else if( current->type == 65 ) {
        virtualbuffer<long long> & fvb = *(virtualbuffer<long long>*)current;
        print<long long>(fvb, "\n%lld", "\t%lld", rows*cols, cols);
    } else if( current->type == 66 ) {
        virtualbuffer<double> & fvb = *(virtualbuffer<double>*)current;
        print<double>(fvb, "\n%e", "\t%e", rows*cols, cols);
    }
    return 0;
}

extern "C" int sl_const(simlab* c) {
    current = c;
    return 0;
}

extern "C" int sl_convert(int cols, int rows, char* file) {
    FILE* convert;
    const char *inp = "convert -size %dx%d -depth 8 rgb:- %s";
    char cmd[256];
    int len = cols*rows;
    if( current->type == 8 ) {
        sprintf(cmd,inp,cols,rows,file);
        convert = popen(cmd, "w");
        virtualbuffer<unsigned char> & sf = *(virtualbuffer<unsigned char>*)current;
        write<unsigned char>(sf, len, convert);
    } else if( current->type == 9 ) {
        sprintf(cmd,inp,cols,rows,file);
        convert = popen(cmd, "w");
        virtualbuffer<char> & sf = *(virtualbuffer<char>*)current;
        write<char>(sf, len, convert);
    } else if( current->type == 32 ) {
        sprintf(cmd,inp,cols,rows,file);
        convert = popen(cmd, "w");
        virtualbuffer<unsigned int> & sf = *(virtualbuffer<unsigned int>*)current;
        write<unsigned int>(sf, len, convert);
    } else if( current->type == 33 ) {
        sprintf(cmd,inp,cols,rows,file);
        printf("convert -size %dx%d -depth 8 rgb:- %s\n",cols,rows,file);
        convert = popen(cmd, "w");
        virtualbuffer<int> & sf = *(virtualbuffer<int>*)current;
        write<int>(sf, len, convert);
    }
    if( convert != NULL ) pclose(convert);

    return 0;
}

extern "C" int sl_play(int len) {
    FILE* sox;
    if( current->type == 17 ) {
        sox = popen("play --bits 16 --rate 22000 --channels 1 --encoding signed-integer -t raw -", "w");
        virtualbuffer<short> & sf = *(virtualbuffer<short>*)current;
        write<short>(sf, len, sox);
    } else if( current->type == 34 ) {
        sox = popen("play --bits 32 --rate 22000 --channels 1 --encoding float -t raw -", "w");
        virtualbuffer<float> & sf = *(virtualbuffer<float>*)current;
        write<float>(sf, len, sox);
    }
    if( sox != NULL ) pclose(sox);

    return 0;
}

std::map<std::string,simlab*>    retlib;

extern "C" int sl_fetch(const char* buf) {
    std::string var(buf);
    current = retlib[var];

    return 0;
}

extern "C" int sl_store(const char* buf) {
    std::string var(buf);
    retlib[var] = current;
    
    return 0;
}

inline long dopen( char* name ) {
#ifndef WIN
	return (long)dlopen( name, RTLD_GLOBAL );
#else
	return (long)GetModuleHandle( name );
#endif
}

inline long dsym( long long handle, const char* symbol ) {
#ifndef WIN
	return (long)dlsym( (void*)handle, symbol );
#else
	return (long)GetProcAddress( (HINSTANCE)handle, symbol );
#endif
}

template<char count>
struct passa {
	char dw;
    passa<count-1> big;
};
template<> struct passa<0>{};

int			bsize;
unsigned long long passcurr;
passa<31>	passnext;
char    passargs[8];
int passi;

long long module = 0;
extern "C" int sl_init() {
    module = dopen( NULL );

    retlib["zero"] = new cnst<int>(0);
    retlib["one"] = new cnst<int>(1);
    retlib["two"] = new cnst<int>(2);
    retlib["three"] = new cnst<int>(3);
    retlib["ten"] = new cnst<int>(10);
    retlib["hundred"] = new cnst<int>(100);
    retlib["thousand"] = new cnst<int>(1000);
    retlib["million"] = new cnst<int>(1000000);

    retlib["PI"] = new cnst<double>( acos( -1.0 ) );
	retlib["e"] = new cnst<double>( exp( 1.0 ) );

    retlib["ubyte"] = new cnst<int>(8);
    retlib["byte"] = new cnst<int>(9);
    retlib["uchar"] = new cnst<int>(8);
    retlib["char"] = new cnst<int>(9);
    retlib["ushort"] = new cnst<int>(16);
    retlib["short"] = new cnst<int>(17);
    retlib["uint"] = new cnst<int>(32);
    retlib["int"] = new cnst<int>(33);
    retlib["float"] = new cnst<int>(34);
    retlib["ulong"] = new cnst<int>(64);
    retlib["long"] = new cnst<int>(65);
    retlib["double"] = new cnst<int>(66);

    retlib["idx"] = new idx();

    return 0;
}

int parseParameters( int bytesize ) {
	char *result = strtok( NULL, " ,)\n" );
	if( result != NULL ) {
		if( result[0] == '"' || result[0] == '.' ) {
			std::string str = result;
			if( str[ str.length()-1 ] != '"' ) {
				char *rs = strtok( NULL, "\"" );
				str += " ";
				str += rs;
				str += "\"";
			}

			char*	c_str = new char[ str.length()-1 ];
			str.copy( c_str, str.length()-2, 1 );
			c_str[ str.length() - 2 ] = 0;

            passargs[passi] = 'p';
            passi++;

            printf("lptr %lld\n", (long long)c_str);

			char* here = (char*)&passnext;
			int size = sizeof(void*); // len-2
			memcpy( here+bytesize, &c_str, size );

			return parseParameters( bytesize+size );
		} else if( result[0] == '[' ) {
			int len = strlen(result);
			std::vector<double>		d_vec;
			float					fval;
			sscanf( result+1, "%e", &fval );
			d_vec.push_back( (double)fval );

			result = strtok( NULL, " ,)\n" );
			len = strlen(result);
			while( result[ len-1 ] != ']' ) {
				sscanf( result, "%e", &fval );
				d_vec.push_back( (double)fval );
				result = strtok( NULL, " ,)\n" );
				len = strlen(result);
			}
			sscanf( result, "%e]", &fval );
			d_vec.push_back( (double)fval );

			//t_simlab<double>		data;
			buffer<double> adata(d_vec.size());
			memcpy( (void*)(adata.buf), &d_vec[0], adata.length*sizeof(double) );

			char* here = (char*)&passnext;
			memcpy( here+bytesize, &adata, sizeof(simlab) );
			//sig += "S";
			return parseParameters( bytesize+sizeof(simlab) );
		} else if( result[0] == '-' ) {
			int value = result[1] - '0';
			int i = 2;
			int mul = 1;
			int add = 0;
herem:
			while( result[i] != 0 && result[i] != '.' && result[i] != '*' ) {
				value *= 10;
				value += result[i] - '0';
				i++;
			}
			if( result[i] == '.' ) {
				double dvalue = (double)value;
				double mnt = 1.0;
				int k = i+1;
				while( result[k] != 0 ) {
					mnt *= 10.0;
					dvalue += (result[k] - '0')/mnt;
					k++;
				}
				float fvalue = -(float)dvalue;
				//printf( "%f %d\n", fvalue, sizeof(fvalue) );
				char* here = (char*)&passnext;
				here += bytesize;
				buffer<float> fval(0);
                double dd = *((float*)&fvalue);
                unsigned long long ull = *(unsigned long long*)&dd;
				fval.buf = (float*)ull;
				memcpy( here, &fval, sizeof(simlab) );
				return parseParameters( bytesize+sizeof(simlab) );
			} else if( result[i] == '*' ) {
				mul *= value;
				i++;
				value = result[i] - '0';
				i++;
				goto herem;
			} else {
				char* here = (char*)&passnext;
				here += bytesize;
				buffer<unsigned int> val(0);
				val.buf = (unsigned int*)-(long)(value*mul+add);

				//char cc[100];
				//sprintf( cc, "%d", val.buffer );
				//prnt(cc);

				memcpy( here, &val, sizeof(virtualbuffer<unsigned int>) );
				return parseParameters( bytesize+sizeof(simlab) );
			}
			/*int value = result[1] - '0';
			int i = 2;
			while( result[i] != 0 && result[i] != '.' ) {
				value *= 10;
				value += result[i] - '0';
				i++;
			}
			if( result[i] == '.' ) {
				double dvalue = (double)value;
				double mnt = 1.0;
				int k = i+1;
				while( result[k] != 0 ) {
					mnt *= 10.0;
					dvalue += (result[k] - '0')/mnt;
					k++;
				}
				dvalue = -dvalue;
				char* here = (char*)&passnext;
				here += bytesize;
				//sig += "D";
				memcpy( here, &dvalue, sizeof(double) );
				return parseParameters( bytesize+sizeof(double) );
			} else  {
				value = -value;
				char* here = (char*)&passnext;
				here += bytesize;
				//sig += "I";
				memcpy( here, &value, sizeof(long) );
				return parseParameters( bytesize+sizeof(long) );
			}*/
		} else if( result[0] >= '0' && result[0] <= '9' ) {
			int value = result[0] - '0';
			int i = 1;
			int mul = 1;
			int add = 0;
here:
			while( result[i] != 0 && result[i] != '.' && result[i] != '*' ) {
				value *= 10;
				value += result[i] - '0';
				i++;
			}
			if( result[i] == '.' ) {
				double dvalue = (double)value;
				double mnt = 1.0;
				int k = i+1;
				while( result[k] != 0 && result[k] != 'f' ) {
					mnt *= 10.0;
					dvalue += (result[k] - '0')/mnt;
					k++;
				}
				char* here = (char*)&passnext;
				here += bytesize;
                simlab* sl;
                if( result[k] = 'f' ) sl = new cnst<float>(value);
                else sl = new cnst<double>(value);

                passargs[passi] = 'p';
                passi++;
                printf("%f\n", (*(virtualbuffer<float>*)sl)[0]);

				memcpy( here, &sl, sizeof(simlab*) );
				return parseParameters( bytesize+sizeof(simlab*) );
			} else if( result[i] == '*' ) {
				mul *= value;
				i++;
				value = result[i] - '0';
				i++;
				goto here;
			} else  {
				char* here = (char*)&passnext;
				here += bytesize;
				int val = value*mul+add;
                int* iptr = (int*)here;
                *iptr = val;
				
                passargs[passi] = 'i';
                passi++;

				return parseParameters( bytesize+sizeof(int) );
			}
		} else {
			std::map<std::string,simlab*>::iterator rit = retlib.find(result);
			if( rit != retlib.end() ) {
                simlab* fetch = retlib[result];

				char* here = (char*)&passnext;
				here += bytesize;
				memcpy( here, &fetch, sizeof(simlab*) );

                passargs[passi] = 'p';
                passi++;

				//data = tmp;
				return parseParameters( bytesize+sizeof(simlab) );
			} else {
				long fnc = dsym( module, result );
				if( strcmp( result, "prev" ) == 0 ) {
					char* here = (char*)&passnext;
					here += bytesize;
					memcpy( here, &prev, sizeof(simlab) );
					return parseParameters( bytesize+sizeof(simlab) );
				} else if( strcmp( result, "len" ) == 0 ) {
					char* here = (char*)&passnext;
					here += bytesize;
					buffer<unsigned int> len(0);
                    len.buf = (unsigned int*)(unsigned long long)current->length;
					memcpy( here, &len, sizeof(simlab) );
					return parseParameters( bytesize+sizeof(simlab) );
				} else if( fnc != 0 ) {
					char* here = (char*)&passnext;
					here += bytesize;
					buffer<unsigned int> fdata(0);
                    fdata.buf = (unsigned int*)fnc;
					memcpy( here, &fdata, sizeof(fdata) );
					return parseParameters( bytesize+sizeof(virtualbuffer<unsigned int>) );
				} else {
					sl_fetch( result );

					char* here = (char*)&passnext;
					here += bytesize;
					memcpy( here, current, sizeof(simlab) );

					//data = tmp;
					return parseParameters( bytesize+sizeof(simlab) );
				}

				/*} else {
					int val = 0;
					if( result[0] == 't' && result[1] == 'r' && result[2] == 'u' ) val = 1;
					char* here = (char*)&passnext;
					memcpy( here+bytesize, &val, sizeof(long) );
					return parseParameters( func, bytesize+sizeof(long) );
				}*/
			}
		}
	} else {
		char* here = (char*)&passnext;
		here += bytesize;
		//memcpy( here, &nulldata, sizeof(simlab) );
	}

	bsize = bytesize;
	return bytesize;
}

extern "C" int cmd( char* command ) {
	if( *command == '"' ) {
		command[ strlen(command)-1 ] = 0;
		buffer<char> str;
		str.buf = (char*)(command+1);
		//echo( str );
	} else { //if( *command != '\n' ) {
		char*	result = strtok( command, " (\n" );
		long long func = dsym( module, result );
		//int (*func)() = (int (*)())dsym( module, result );
		if( func != 0 ) {//&& (java == 0 || func == (long)store || func == (long)fetch || func == (long)Class || func == (long)New || func == (long)Data || func == (long)create) ) {
			//int (*func)() = (int (*)())dsym( module, "welcome" );
			//printf( "%lld\n", (long)func );
			//func();

			//simlab old = data;
			memset( &passnext, 0, sizeof(passnext) );
            memset( passargs, 0, sizeof(passargs) );
            passi = 0;
			parseParameters( 0 );
			//passcurr = (long)&passnext;
			
            //if( *(int*)&passnext != 0 ) printf( "%s\n", (char*)*(int*)&passnext );
            //((int (*)(...))func)();

            if( passargs[0] == 0 ) {
			    ((int (*)())func)();
            } else if( passargs[0] == 'p' && passargs[1] == 0 ) {
                ((int (*)(void*))func)( ((void**)&passnext)[0] );
            } /*else if( passargs[0] == 's' && passargs[1] == 0 ) {
                ((int (*)(simlab*))func)( ((void**)&passnext)[0] );
            }*/ else if( passargs[0] == 'i' && passargs[1] == 0 ) {
			    ((int (*)(int))func)( ((int*)&passnext)[0] );
            } else if( passargs[0] == 'i' && passargs[1] == 'i' && passargs[2] == 0 ) {
			    ((int (*)(int,int))func)( ((int*)&passnext)[0], ((int*)&passnext)[1] );
            } else if( passargs[0] == 'i' && passargs[1] == 'i' && passargs[2] == 'p' && passargs[3] == 0 ) {
                char* p = (char*)&passnext;
                p += 2*sizeof(int);
                char* ptr = *(char**)p;

                //printf("lptr %lld %lld\n", (long long)p, (long long)ptr);
                //printf("ok %d %d %s\n", ((int*)&passnext)[0], ((int*)&passnext)[1], ptr);
			    ((int (*)(int,int,const char*))func)( ((int*)&passnext)[0], ((int*)&passnext)[1], ptr );
            } else {
                ((int (*)(...))func)( passnext );
            }
			
            
            
            /*if( old.buffer != 0 && (data.buffer < old.buffer || data.buffer > old.buffer+bytelength(old.type,old.length)) ) {
				prev = old;
			}*/
		} else printf( "No such command %s\n", result, command );
	}

	return 0;
}

int main(int argc, char** argv) {
    sl_init();

    FILE*	f = stdin;
    char	line[256];
    strcpy(line,"sl_s");
    int offset = strlen(line)-1;
    char quit[] = "quit";
	char* res = fgets( line+offset, sizeof(line), f );

	int (*fnc)( char* );
    fnc = cmd;

	while( res != NULL && strncmp( line+offset, quit, sizeof(quit)-1 ) ) {
		//s_line.length = strlen(line);
		if( *line != '\n' ) fnc( line );
		res = fgets( line+offset, sizeof(line), (FILE*)f );
	}

    return 0;
}

int main10(int argc, char** argv) {
    idx ix;
    current = &ix;
    cnst<float> ff(2);
    printf("%d %d\n", current->type, ff.type);

    return 0;
}

int main3(int argc, char** argv) {
    idx ix;
    trans tix(256,256);
    cnst<int> three(3);
    divd<int> dv(ix,three);
    sum<int> sm(ix,dv);

    pcast<char,int> pc(ix);
    mapping<char> mg(pc,sm);
    for( int i = 0; i < 30; i++ ) {
        printf("%d ", mg[i]);
    }
    printf("\n");

    FILE* img = fopen("ok.rgb","w");
    //FILE* img = popen("convert -format RAW -size 265x256 - ok.jpg", "w");
    write<char>(mg, 256*256*3, img);
    fclose(img);

    return 0;
}

int main0(int argc, char** argv) {
    idx ix;
    FILE* img = fopen("ok.raw","w");
    //FILE* img = popen("convert -format RAW -size 265x256 - ok.jpg", "w");
    write<int>(ix, 256*256, img);
    fclose(img);

    //open("ok.jpg");

    /*cast<float,int> fix(ix);
    cnst<float> ten(10.0f);
    cnst<float> tenth(10000.0f);
    divd<float> d(fix,ten);
    sl_sinf<float> sn(d);
    mul<float> ml(sn,tenth);
    cast<short,float> sf(ml);
    FILE* sox = popen("play --bits 16 --rate 22000 --channels 1 --encoding signed-integer -t raw -", "w");
    write<short>(sf, 100000, sox);
    pclose(sox);*/

    pcast<unsigned char,int> pc(ix);
    for( int i = 0; i < 30; i++ ) {
        printf("%d ", (int)pc[i]);
    }
    printf("\n");

    pcast<double,int> pc2(ix);
    for( int i = 0; i < 30; i++ ) {
        printf("%e ", (double)pc2[i]);
    }
    printf("\n");

    return 0;
}

int main2(int argc, char** argv) {
    nidx nix;
    cnst<float> two(2);
    cast<float,int> cst(nix);
    mul<float> ff(two,cst);

    cnst<float> oned(1);
    cast<float,int> fidx(nix);
    quad<float> res(oned,oned,ff);
    
    cnst<float> onef(1);
    mod<float> md(res,onef);
    //sl_floor<float> flr(res);
    /*sum<int> sum(idx, idx);
    sl_sin<int> ok(sum);*/
    //cnst<int> c(2);
    //mod<int> mod(idx, c);
    /*for( int i = 0; i < 10; i++ ) {
        printf("%f ", (float)ok[i]);
    }
    printf("\n");*/

    //fibonacci fib;
    /*triangular fib;
    diff<int> df(fib);
    sq<int> sqr(df);
    sl_sin<int> ssin(sqr);*/
    for( int i = 0; i < 30; i++ ) {
        printf("%e ", (double)md[i]);
    }
    printf("\n");

    triangular fib;
    for( int i = 0; i < 30; i++ ) {
        printf("%d ", fib[i]);
    }
    printf("\n");

    flip2d flp(5);
    for( int i = 0; i < 30; i++ ) {
        printf("%d ", flp[i]);
    }
    printf("\n");

    idx ix;
    trans trns(4,3);
    /*merge<int> mrg(trns,trns);
    merge<int> mrg1(mrg,mrg);
    merge<int> mrg2(mrg1,mrg1);
    merge<int> mrg3(mrg2,mrg2);*/
    for( int i = 0; i < 10; i++ ) {
        printf("%d\t", trns[i]);
    }
    printf("\n");

    order ord(trns);
    for( int i = 0; i < 10; i++ ) {
        printf("%d\t", ord[i]);
    }
    printf("\n");

    order ore1((virtualbuffer<int>&)ord);
    for( int i = 0; i < 10; i++ ) {
        printf("%d\t", ore1[i]);
    }
    printf("\n");

    order ord2((virtualbuffer<int>&)ore1);
    for( int i = 0; i < 10; i++ ) {
        printf("%d\t", ord2[i]);
    }
    printf("\n");

    cnst<int> fv(-5);
    cnst<int> tn(10);
    shift shft(fv,tn);
    for( int i = 0; i < 30; i++ ) {
        printf("%d ", shft[i]);
    }
    printf("\n");

    return 0;
}