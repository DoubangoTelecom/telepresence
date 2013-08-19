/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_OBJECT_H
#define OPENTELEPRESENCE_OBJECT_H

#include "OpenTelepresenceConfig.h"

#include "tsk_debug.h"

#define OTObjectSafeRelease(pObject)	(pObject) = NULL
#define OTObjectSafeFree				OTObjectSafeRelease

class OTObject
{
public:
	OTObject():m_nRefCount(0){}
	OTObject(const OTObject &):m_nRefCount(0){}
	virtual ~OTObject(){}

public:
	virtual OT_INLINE const char* getObjectId() = 0;
#if !defined(SWIG)
	OT_INLINE int getRefCount() const{
		return m_nRefCount;
	}
	void operator=(const OTObject &){}
#endif


public:
	OT_INLINE int takeRef() /*const*/{
		ot_atomic_inc(&m_nRefCount);
		return m_nRefCount;
	}
	OT_INLINE int releaseRef() /*const*/{
		if(m_nRefCount){ // must never be equal to zero
			ot_atomic_dec(&m_nRefCount);
		}
		return m_nRefCount;
	}

private:
	volatile long m_nRefCount;
};


//
//	OTObjectWrapper declaration
//
template<class OTObjectType>
class OTObjectWrapper{

public:
	OT_INLINE OTObjectWrapper(OTObjectType obj = NULL);
	OT_INLINE OTObjectWrapper(const OTObjectWrapper<OTObjectType> &obj);
	OT_INLINE virtual ~OTObjectWrapper();

#if !defined(SWIG)
public:
	OT_INLINE OTObjectWrapper<OTObjectType>& operator=(const OTObjectType other);
	OT_INLINE OTObjectWrapper<OTObjectType>& operator=(const OTObjectWrapper<OTObjectType> &other);
	OT_INLINE bool operator ==(const OTObjectWrapper<OTObjectType> other) const;
	OT_INLINE bool operator!=(const OTObjectWrapper<OTObjectType> &other) const;
	OT_INLINE bool operator <(const OTObjectWrapper<OTObjectType> other) const;
	OT_INLINE OTObjectType operator->() const;
	OT_INLINE OTObjectType operator*() const;
	OT_INLINE operator bool() const;
#endif

protected:
	OT_INLINE int takeRef();
	OT_INLINE int releaseRef();

	OT_INLINE OTObjectType getWrappedObject() const;
	OT_INLINE void wrapObject(OTObjectType obj);

private:
	OTObjectType m_WrappedObject;
};

//
//	OTObjectWrapper implementation
//
template<class OTObjectType>
OTObjectWrapper<OTObjectType>::OTObjectWrapper(OTObjectType obj) { 
	wrapObject(obj), takeRef();
}

template<class OTObjectType>
OTObjectWrapper<OTObjectType>::OTObjectWrapper(const OTObjectWrapper<OTObjectType> &obj) {
	wrapObject(obj.getWrappedObject()),
	takeRef();
}

template<class OTObjectType>
OTObjectWrapper<OTObjectType>::~OTObjectWrapper(){
	releaseRef(),
	wrapObject(NULL);
}


template<class OTObjectType>
int OTObjectWrapper<OTObjectType>::takeRef(){
	if(m_WrappedObject /*&& m_WrappedObject->getRefCount() At startup*/){
		return m_WrappedObject->takeRef();
	}
	return 0;
}

template<class OTObjectType>
int OTObjectWrapper<OTObjectType>::releaseRef() {
	if(m_WrappedObject && m_WrappedObject->getRefCount()){
		if(m_WrappedObject->releaseRef() == 0){
			delete m_WrappedObject, m_WrappedObject = NULL;
		}
		else{
			return m_WrappedObject->getRefCount();
		}
	}
	return 0;
}

template<class OTObjectType>
OTObjectType OTObjectWrapper<OTObjectType>::getWrappedObject() const{
	return m_WrappedObject;
}

template<class OTObjectType>
void OTObjectWrapper<OTObjectType>::wrapObject(const OTObjectType obj){
	if(obj){
		if(!(m_WrappedObject = dynamic_cast<OTObjectType>(obj))){
			TSK_DEBUG_ERROR("Trying to wrap an object with an invalid type");
		}
	}
	else{
		m_WrappedObject = NULL;
	}
}

template<class OTObjectType>
OTObjectWrapper<OTObjectType>& OTObjectWrapper<OTObjectType>::operator=(const OTObjectType obj){
	releaseRef();
	wrapObject(obj), takeRef();
	return *this;
}

template<class OTObjectType>
OTObjectWrapper<OTObjectType>& OTObjectWrapper<OTObjectType>::operator=(const OTObjectWrapper<OTObjectType> &obj){
	releaseRef();
	wrapObject(obj.getWrappedObject()), takeRef();
	return *this;
}

template<class OTObjectType>
bool OTObjectWrapper<OTObjectType>::operator ==(const OTObjectWrapper<OTObjectType> other) const {
	return getWrappedObject() == other.getWrappedObject();
}

template<class OTObjectType>
bool OTObjectWrapper<OTObjectType>::operator!=(const OTObjectWrapper<OTObjectType> &other) const {
	return getWrappedObject() != other.getWrappedObject();
}

template<class OTObjectType>
bool OTObjectWrapper<OTObjectType>::operator <(const OTObjectWrapper<OTObjectType> other) const {
	return getWrappedObject() < other.getWrappedObject();
}

template<class OTObjectType>
OTObjectWrapper<OTObjectType>::operator bool() const {
	return (getWrappedObject() != NULL);
}

template<class OTObjectType>
OTObjectType OTObjectWrapper<OTObjectType>::operator->() const {
	return getWrappedObject();
}

template<class OTObjectType>
OTObjectType OTObjectWrapper<OTObjectType>::operator*() const{
	return getWrappedObject();
}

#endif /* OPENTELEPRESENCE_OBJECT_H */
