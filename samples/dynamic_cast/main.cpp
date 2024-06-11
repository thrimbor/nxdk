#include <hal/debug.h>
#include <hal/video.h>
#include <string>
#include <vector>
#include <windows.h>
#include <typeinfo>

struct Base {
  int a = 0xBB;
  virtual void f() {};
};

struct Base2: Base {
    int b2 = 0xB2;
};


struct DerivedA : Base2 {
    int a1 = 0xa1;
    virtual void f() {debugPrint("Base2");}
};

struct DerivedB : DerivedA {
    int b1 = 0xb1;
    virtual void f() {debugPrint("DerivedA");}
};



void cast_a (Base &a) {
  DerivedA &da = dynamic_cast<DerivedA &>(a);
}

void cast_b (Base &b) {

}


void cast_ptr_a (Base2 *a) {
    debugPrint("Base address: %x\n", a);
    DerivedA *da = dynamic_cast<DerivedA *>(a);
    debugPrint("dynamic_cast<DerivedA *>(a): %x\n", da);
}

void cast_ptr_b (Base2 *a) {
    debugPrint("Base address: %x\n", a);
    DerivedB *db = dynamic_cast<DerivedB *>(a);
    debugPrint("dynamic_cast<DerivedB *>(a): %x\n", db);
}

void cast_ref_a (Base2 &a) {
    debugPrint("dynamic_cast<DerivedA &>(a):\n");
    try {
        DerivedA &da = dynamic_cast<DerivedA &>(a);
        debugPrint("Successful\n");
    } catch (std::bad_cast e) {
        debugPrint("caught std::bad_cast: %s\n", e.what());
    }
}

void cast_ref_b (Base2 &a) {
    debugPrint("dynamic_cast<DerivedB &>(a):\n");
    try {
        DerivedB &bb = dynamic_cast<DerivedB &>(a);
        debugPrint("Successful\n");
    } catch (std::bad_cast e) {
        debugPrint("caught std::bad_cast: %s\n", e.what());
    }
}

void cast_ptr_test () {
    DerivedA a;
    debugPrint("Original address: %x\n", &a);
    cast_ptr_a(&a);
    cast_ptr_b(&a);
    cast_ref_a(a);
    cast_ref_b(a);
}

///////////////////////////////////////////////
// Multiple inheritance test
///////////////////////////////////////////////
struct MiBaseA {
    virtual void f() {};
};

struct MiBaseB : MiBaseA {

};

struct MiBaseE {
    virtual void f() {};
};

struct MiBaseD : MiBaseE {

};

struct MiBaseC : MiBaseB, MiBaseD {

};

void mi_cast_b_to_e (MiBaseB *o) {
    debugPrint("o: %x\n", o);
    MiBaseE *e = dynamic_cast<MiBaseE *>(o);
    debugPrint("dynamic_cast<MiBaseE *>(o): %x\n", e);
}

void mi_cast_d_to_a (MiBaseD *o) {
    debugPrint("o: %x\n", o);
    MiBaseA *e = dynamic_cast<MiBaseA *>(o);
    debugPrint("dynamic_cast<MiBaseA *>(o): %x\n", e);
}

void mi_cast_test () {
    MiBaseC c;

    debugPrint("&c: %x\n", &c);

    //mi_cast_b_to_e(&c);
    mi_cast_d_to_a(&c);
}

void mi_cast_to_void_test() {

}



void cast_to_void_test ();

int main(void) {
  XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);

  debugPrint("Hello nxdk\n");

  mi_cast_test();

  //cast_ptr_test();

  //cast_to_void_test();

  debugPrint("done\n");

  //DerivedA a;

  //cast_a(a);
  //cast_b(a);
/*
  std::vector<std::string> words;
  words.emplace_back("Hello");
  words.emplace_back(" ");
  words.emplace_back("nxdk!");
  words.emplace_back("\n");
  */

  while (true) {
    //for (auto& word : words) {
      //debugPrint("%s", word.c_str());
    //}
    Sleep(2000);
  }

  return 0;
}

//////////////////////////////////////////////
// Cast to void * test
//////////////////////////////////////////////
struct BaseV {
    int a2;
    virtual ~BaseV() = default;
};

struct DerivedC : public Base, public BaseV {
    int x;
};

void cast_to_void_subfunc (BaseV *base) {
    debugPrint("base: %08X\n", base);

    void *d_a = dynamic_cast<void *>(base);
    debugPrint("d_a: %08X\n", d_a);
}

void cast_to_void_test () {
    DerivedC a;

    debugPrint("a: %08x\n", &a);

    cast_to_void_subfunc(&a);
}
