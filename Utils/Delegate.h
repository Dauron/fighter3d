
template <typename SENDER, typename METHODT>
class BaseDelegate
{
public:
    typedef METHODT METHOD;

protected:
    SENDER *Sender;
    void   *Reciever;
    METHOD  Method;

public:
    BaseDelegate()
        : Sender(NULL), Reciever(NULL), Method(NULL)
    {}
    BaseDelegate(SENDER &sender, void *reciever, METHOD method)
        : Sender(&sender), Reciever(reciever), Method(method)
    {}

    void Set(void *reciever, METHOD method)
    {
        Reciever = reciever;
        Method   = method;
    }
    void Set(METHOD method)
    {
        Reciever = NULL;
        Method   = method;
    }

    BaseDelegate<SENDER, METHODT>
    &operator = (const BaseDelegate<SENDER, METHODT> &source) {
        Sender   = source.Sender;
        Reciever = source.Reciever;
        Method   = source.Method;
        return *this;
    }
};

#define DELEGATE_CONSTRUCTORS                                               \
    Delegate() : BaseDelegate() {}                                          \
    Delegate(SENDER &sender, void *reciever = NULL, METHOD method = NULL)   \
        : BaseDelegate(sender, reciever, method) {}

template <typename SENDER, typename DATA1 = void, typename DATA2 = void>
class Delegate : public BaseDelegate<SENDER, void (*) (SENDER &sender, void *reciever, DATA1 &data1, DATA2 &data2)>
{
public:
    DELEGATE_CONSTRUCTORS

    void Raise(DATA1 &data1, DATA2 &data2)
    { if (Method) Method(*Sender, Reciever, data1, data2); }

    void operator() (DATA1 &data1, DATA2 &data2)
    { Raise(data1, data2); }
};

template <typename SENDER, typename DATA>
class Delegate<SENDER, DATA, void> : public BaseDelegate<SENDER, void (*) (SENDER &sender, void *reciever, DATA &data)>
{
public:
    DELEGATE_CONSTRUCTORS

    void Raise(DATA &data)       { if (Method) Method(*Sender, Reciever, data); }
    void operator() (DATA &data) { Raise(data); }
};

template <typename SENDER>
class Delegate<SENDER, void, void> : public BaseDelegate<SENDER, void (*) (SENDER &sender, void *reciever)>
{
public:
    DELEGATE_CONSTRUCTORS

    void Raise()       { if (Method) Method(*Sender, Reciever); }
    void operator() () { Raise(); }
};
