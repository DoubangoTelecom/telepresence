/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_CFG_H
#define OPENTELEPRESENCE_CFG_H

#include "OpenTelepresenceConfig.h"

#include "opentelepresence/OTObject.h"

#include <list>
#include <map>
#include <string>

class OTCfgParam;

typedef std::map<std::string, OTObjectWrapper<OTCfgParam*> > OTMapOfCfgParams;

typedef enum OTCfgType_e
{
	OTCfgType_None,
	OTCfgType_Section,
	OTCfgType_Param,
	OTCfgType_EoF,
}
OTCfgType_t;

class OTCfg : public OTObject
{
protected:
	OTCfg(OTCfgType_t eType);
public:
	virtual ~OTCfg();
	virtual OT_INLINE const char* getObjectId() { return "OTCfg"; }
	virtual OT_INLINE OTCfgType_t getType() { return m_eType; }

protected:
	OTCfgType_t m_eType;
};

class OTCfgEoF : public OTCfg
{
public:
	OTCfgEoF() : OTCfg(OTCfgType_EoF){}
	virtual ~OTCfgEoF(){  }
	OT_INLINE const char* getObjectId() { return "OTCfgEoF"; }
};

class OTCfgSection : public OTCfg
{
public:
	OTCfgSection(const char* pcName);
	virtual ~OTCfgSection();
	OT_INLINE const char* getObjectId() { return "OTCfgSection"; }
	OT_INLINE const char* getName() { return m_pName; }
	OT_INLINE const std::list<OTObjectWrapper<OTCfgParam *> >* getParams(){ return &m_oParams; }
	bool addParam(OTObjectWrapper<OTCfgParam *> oParam);
	bool addParam(const char* pcName, const char* pcValue);
private:
	char* m_pName;
	std::list<OTObjectWrapper<OTCfgParam *> > m_oParams;
};

class OTCfgParam : public OTCfg
{
public:
	OTCfgParam(const char* pcName, const char* pcValue);
	virtual ~OTCfgParam();
	OT_INLINE const char* getObjectId() { return "OTCfgParam"; }
	OT_INLINE const char* getName() { return m_pName; }
	OT_INLINE const char* getValue() { return m_pValue; }
private:
	char* m_pName;
	char* m_pValue;
};

#endif /* OPENTELEPRESENCE_CFG_H */
