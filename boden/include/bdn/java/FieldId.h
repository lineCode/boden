#ifndef BDN_JAVA_FieldId_H_
#define BDN_JAVA_FieldId_H_

#include <bdn/java/Env.h>
#include <bdn/java/JClass.h>

namespace bdn
{
namespace java
{

/** Represents the ID of a Java object field.
 *  These IDs are used to call access an object's java-side fields from native code code.
 *  See #FieldAccessor.*/
template<class NativeType>
class FieldId
{
public:
    FieldId(JClass& cls, const char* fieldName)
    {
        _id = Env::get().getJniEnv()->GetFieldID( (jclass)cls.getJObject_(), fieldName, TypeConversion<NativeType>::getJavaSignature().asUtf8Ptr() );
    }

    /** Returns the Id. Throws and exception if the Id has not been initialized yet.*/
    jfieldID getId() const
    {
        return _id;
    }

private:
    jfieldID _id;
};


}
}



#endif



