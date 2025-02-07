#include "VMObject.h"

#include "VMclass.h"

VMObject::VMObject(VMClass* cls) : mClass(cls), mMuty(true){

}

VMObject::~VMObject(){
}

VMObject::VMObject(VMContext* ctx, VMClass* cls) : mClass(cls), mMuty(true){
	init(ctx, cls);
}

void VMObject::init(VMContext* ctx, VMClass* cls){
	mClass = cls;
	unsigned numFields = 0;
	VMClass* clas = mClass;
	while (clas){
		numFields += clas->getNonStaticFieldOffset();
		clas = clas->getSuperclass(ctx);
	}
	mFields.resize(numFields);
}

FieldData* VMObject::getObjField(unsigned idx){
	return &mFields[idx-1];
}

VMMethod* VMObject::getObjMethod(unsigned idx){
	return mClass->getMethod(idx);
}
