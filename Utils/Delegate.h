#ifndef __incl_Utils_Delegate_h
#define __incl_Utils_Delegate_h

template <typename SENDER, typename METHODT>
class BaseDelegate
{
public:
    typedef METHODT METHOD;

protected:
    SENDER *Sender;
    void   *Receiver;
    METHOD  Method;

public:
    BaseDelegate()
        : Sender(NULL), Receiver(NULL), Method(NULL)
    {}
    BaseDelegate(SENDER &sender, void *receiver, METHOD method)
        : Sender(&sender), Receiver(receiver), Method(method)
    {}

    void Set(void *receiver, METHOD method)
    {
        Receiver = receiver;
        Method   = method;
    }
    void Set(METHOD method)
    {
        Receiver = NULL;
        Method   = method;
    }

    BaseDelegate<SENDER, METHODT>
    &operator = (const BaseDelegate<SENDER, METHODT> &source) {
        Sender   = source.Sender;
        Receiver = source.Receiver;
        Method   = source.Method;
        return *this;
    }
};

#define DELEGATE_CONSTRUCTORS                                               \
    Delegate() : BaseDelegate() {}                                          \
    Delegate(SENDER &sender, void *receiver = NULL, METHOD method = NULL)   \
        : BaseDelegate(sender, receiver, method) {}

template <typename SENDER, typename DATA1 = void, typename DATA2 = void>
class Delegate : public BaseDelegate<SENDER, void (*) (SENDER &sender, void *receiver, DATA1 &data1, DATA2 &data2)>
{
public:
    DELEGATE_CONSTRUCTORS

    void Raise(DATA1 &data1, DATA2 &data2)
    { if (Method) Method(*Sender, Receiver, data1, data2); }

    void operator() (DATA1 &data1, DATA2 &data2)
    { Raise(data1, data2); }
};

template <typename SENDER, typename DATA>
class Delegate<SENDER, DATA, void> : public BaseDelegate<SENDER, void (*) (SENDER &sender, void *receiver, DATA &data)>
{
public:
    DELEGATE_CONSTRUCTORS

    void Raise(DATA &data)       { if (Method) Method(*Sender, Receiver, data); }
    void operator() (DATA &data) { Raise(data); }
};

template <typename SENDER>
class Delegate<SENDER, void, void> : public BaseDelegate<SENDER, void (*) (SENDER &sender, void *receiver)>
{
public:
    DELEGATE_CONSTRUCTORS

    void Raise()       { if (Method) Method(*Sender, Receiver); }
    void operator() () { Raise(); }
};

/* Usage:
// Final, uniwersal class
class Window {
public:
  // Event declaration
  typedef Delegate<Window, unsigned int, //Width
                           unsigned int  //Height
                  > WindowResizeEvent;
  WindowResizeEvent OnResize;

  Window() {
    // Empty event initialization,
    // parameter is the calling class
    OnResize = WindowResizeEvent(*this);
  }

  Resize(Width, Height) {
    // ...
    // Event call
    OnResize(Width, Height);
  }
};

// New class
void AnyClass_OnResize(Window &window, void* receiver,
                unsigned int &width, unsigned int &height);

class AnyClass {
public:
  Window MainWindow;

  AnyClass() {
    // Binding of function to the event,
    // parameters are destination class and intermediary function
    MainWindow.OnResize.Set(*this, ::AnyClass_OnResize);
  }

  // Final event handler code
  void OnResize( Window &window,
                 unsigned int &width, unsigned int &height );
};

// Intermediary function
void AnyClass_OnResize(Window &window, void* receiver,
                unsigned int &width, unsigned int &height)
{
  // Simple trick with pointer casting
  ((AnyClass*)receiver)->OnResize(window, width, height);
}
*/

#endif
