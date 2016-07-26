#include <bdn/init.h>
#include <bdn/java/JObject.h>

#include <bdn/java/Env.h>


namespace bdn
{
namespace java
{


JClass& JObject::getStaticClass_()
{
    static JClass cls( "java/lang/Object" );

    return cls;
}

String JObject::getCanonicalClassName_ ()
{
    jclass clazz = Env::get().getJniEnv()->GetObjectClass( getJObject_() );

    JClass cls( LocalReference((jobject)clazz) );

    return cls.getCanonicalName();
}

bool JObject::isInstanceOf_ (JClass& cls)
{
    return Env::get().getJniEnv()->IsInstanceOf( getJObject_(), (jclass)cls.getJObject_() );
}

}
}


