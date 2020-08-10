#include <hal/debug.h>
#include <hal/video.h>
#include <windows.h>
#include <typeinfo>

class Interface
{
public:
        virtual void GenericOp() = 0;// pure virtual function
};

class SpecificClass : public Interface
{
public:
        virtual void GenericOp();
        virtual void SpecificOp();
};

void SpecificClass::GenericOp ()
{

}

void SpecificClass::SpecificOp ()
{

}

class N
{
int c;
};

static void muh (void)
{
  int *p = nullptr;
  int a = *p;
}

#include <excpt.h>

static int excepttest (void)
{
  DbgPrint("filter function reached\n");
  return EXCEPTION_ACCESS_VIOLATION;
}

static void moo2 (void)
{
  __try {
    muh();
  } __finally {
    DbgPrint("inner __finally called\n");
    int *i = NULL;
    *i = 5;
  }
}

static void moo (void)
{
  __try {
    __try {
      __try {
        moo2();
      }  __finally {
        DbgPrint("__finally called\n");
      }
    } __except (GetExceptionCode() == excepttest() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
      DbgPrint("EXCEPTION!\n");
    }
  } __finally {
    DbgPrint("this should not be called!\n");
  }
}

static void moopoo (void)
{
  __try {
    DbgPrint("babamm\n");
  } __finally {
    DbgPrint("babamm nooo\n");
  }
}

int main(void) {
  XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);
  SpecificClass d;
  SpecificClass &a = d;
  /*
  Interface &b = dynamic_cast<Interface&>(a);
  N &n = dynamic_cast<N&>(a);
  */

 /*
  void *v = dynamic_cast<void *>(&a);

  debugPrint("%s\n", typeid(SpecificClass).raw_name());
*/

  moo();

  moopoo();


  // This prints 0 if no RTTI is availble!
  uint32_t *vftable_ptr = (uint32_t *)*(uint32_t *)&d;
  debugPrint("%x\n", vftable_ptr[-1]);
/*
  try {
    throw 3;
  } catch (int e) {
    debugPrint("e: %d\n", e);
  }
  */

while (true) Sleep(2000);
  return 0;
}
