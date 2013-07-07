
template<typename Type> 
Delegate<Type>::Delegate(Type* object, Function function) :
    object_(object),
    function_(function)
{
}

template<typename Type> 
Delegate<Type>::~Delegate(void)
{
}

template<typename Type> void 
Delegate<Type>::Invoke(ByteBuffer* ptr)
{
    (object_->*function_)(ptr);
}
