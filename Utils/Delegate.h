#ifndef __incl_Utils_Delegate_h
#define __incl_Utils_Delegate_h

#define EVENT_HANDLER0(HANDLER_METHOD, SENDER, RECEIVER)                      \
    static void HANDLER_METHOD(SENDER &control, void* receiver)               \
    { reinterpret_cast<RECEIVER*>( receiver )->HANDLER_METHOD(control); }     \
    void HANDLER_METHOD(SENDER &control)

#define EVENT_HANDLER1(HANDLER_METHOD, SENDER, RECEIVER, PARAM1TYPE, PARAM1NAME)       \
    static void HANDLER_METHOD(SENDER &control, void* receiver, PARAM1TYPE PARAM1NAME) \
    { reinterpret_cast<RECEIVER*>( receiver )->HANDLER_METHOD(control, PARAM1NAME); }  \
    void HANDLER_METHOD(SENDER &control, PARAM1TYPE PARAM1NAME)

#define EVENT_HANDLER2(HANDLER_METHOD, SENDER, RECEIVER, PARAM1TYPE, PARAM1NAME, PARAM2TYPE, PARAM2NAME)      \
    static void HANDLER_METHOD(SENDER &control, void* receiver, PARAM1TYPE PARAM1NAME, PARAM2TYPE PARAM2NAME) \
    { reinterpret_cast<RECEIVER*>( receiver )->HANDLER_METHOD(control, PARAM1NAME, PARAM2NAME); }             \
    void HANDLER_METHOD(SENDER &control, PARAM1TYPE PARAM1NAME, PARAM2TYPE PARAM2NAME)

template <typename SENDER, typename DATA1 = void, typename DATA2 = void>
class Delegate
{
public:
    typedef void (*METHOD) (SENDER &sender, void *receiver, DATA1 &data1, DATA2 &data2);

protected:
    SENDER *Sender;
    void   *Receiver;
    METHOD  Method;

public:

    void Set(void *receiver, METHOD method)
    {
        Receiver = receiver;
        Method   = method;
    }
    void Set(METHOD method)
    {
        Receiver = 0;
        Method   = method;
    }

    Delegate<SENDER, DATA1, DATA2>
    &operator = (const Delegate<SENDER, DATA1, DATA2> &source) {
        Sender   = source.Sender;
        Receiver = source.Receiver;
        Method   = source.Method;
        return *this;
    }

public:
    Delegate()
        : Sender(0), Receiver(0), Method(0) {}
    Delegate(SENDER &sender, void *receiver = 0, METHOD method = 0)
        : Sender(&sender), Receiver(receiver), Method(method) {}

    void Raise(DATA1 &data1, DATA2 &data2)
    { if (Method) Method(*Sender, Receiver, data1, data2); }

    void operator() (DATA1 &data1, DATA2 &data2)
    { Raise(data1, data2); }
};

template <typename SENDER, typename DATA>
class Delegate<SENDER, DATA, void>
{
public:
    typedef void (*METHOD) (SENDER &sender, void *receiver, DATA &data);

protected:
    SENDER *Sender;
    void   *Receiver;
    METHOD  Method;

public:

    void Set(void *receiver, METHOD method)
    {
        Receiver = receiver;
        Method   = method;
    }
    void Set(METHOD method)
    {
        Receiver = 0;
        Method   = method;
    }

    Delegate<SENDER, DATA>
    &operator = (const Delegate<SENDER, DATA> &source) {
        Sender   = source.Sender;
        Receiver = source.Receiver;
        Method   = source.Method;
        return *this;
    }

public:
    Delegate()
        : Sender(0), Receiver(0), Method(0) {}
    Delegate(SENDER &sender, void *receiver = 0, METHOD method = 0)
        : Sender(&sender), Receiver(receiver), Method(method) {}

    void Raise(DATA &data)
    { if (Method) Method(*Sender, Receiver, data); }

    void operator() (DATA &data)
    { Raise(data); }
};

template <typename SENDER>
class Delegate<SENDER, void, void>
{
public:
    typedef void (*METHOD) (SENDER &sender, void *receiver);

protected:
    SENDER *Sender;
    void   *Receiver;
    METHOD  Method;

public:

    void Set(void *receiver, METHOD method)
    {
        Receiver = receiver;
        Method   = method;
    }
    void Set(METHOD method)
    {
        Receiver = 0;
        Method   = method;
    }

    Delegate<SENDER>
    &operator = (const Delegate<SENDER> &source) {
        Sender   = source.Sender;
        Receiver = source.Receiver;
        Method   = source.Method;
        return *this;
    }

public:
    Delegate()
        : Sender(0), Receiver(0), Method(0) {}
    Delegate(SENDER &sender, void *receiver = 0, METHOD method = 0)
        : Sender(&sender), Receiver(receiver), Method(method) {}

    void Raise()
    { if (Method) Method(*Sender, Receiver); }

    void operator() ()
    { Raise(); }
};

/* Usage:
// Final, universal class
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
class AnyClass {
public:
  Window MainWindow;

  AnyClass() {
    // Binding of function to the event,
    // parameters are destination class and intermediary function
    MainWindow.OnResize.Set(*this, ::OnResize);
  }

  // Final event handler code
  EVENT_HANDLER2(OnResize, Window, AnyClass,
                 unsigned int &, width, unsigned int &, height);
};

// Intermediary function
void AnyClas::OnResize(Window &sender, unsigned int &width, unsigned int &height)
{
  //do what you have to do
}
*/

#endif
