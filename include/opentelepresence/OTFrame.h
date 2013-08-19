#ifndef OPENTELEPRESENCE_FRAME_H
#define OPENTELEPRESENCE_FRAME_H

#include "OpenTelepresenceConfig.h"
#include "opentelepresence/OTObject.h"
#include "opentelepresence/OTCommon.h"
#include "opentelepresence/patterns/OTPattern.h"

#include "tsk_mutex.h"

class OTFrame : public OTObject
{
protected:
	OTFrame(OTMediaType_t eMediaType, bool bOwnBuffer, const void *pBufferPtr, uint32_t nBufferSize);
public:
	virtual ~OTFrame();
	virtual OT_INLINE const char* getObjectId() { return "OTFrame"; }

	OT_INLINE bool isValid() { return m_pBufferPtr && m_nBufferSize; }
	OT_INLINE OTMediaType_t getMediaType() { return m_eMediaType; }
	OT_INLINE OTDimension_t getDimension() { return m_eDimension; }
	OT_INLINE void* getBufferPtr() { return m_pBufferPtr; }
	OT_INLINE uint32_t getBufferSize() { return m_nBufferSize; }
	OT_INLINE uint32_t getValidDataSize() { return m_nValidDataSize; }
	OT_INLINE OTPatternType_t getPatternType() { return m_ePatternType; }
	OT_INLINE void setPatternType(OTPatternType_t ePatternType) { m_ePatternType = ePatternType; }
	OT_INLINE void setValidDataSize(uint32_t nValidDataSize) { m_nValidDataSize = TSK_CLAMP(0, nValidDataSize, m_nBufferSize); }
	bool copyBuffer(const void *pSrcBufferPtr, uint32_t nSrcBufferSize);
	bool resizeBuffer(uint32_t nNewBufferSize);
	OT_INLINE void resetValidDataSize(){ m_nValidDataSize = 0; }

	virtual bool prepareLock();
	virtual bool lock();
	virtual bool unlock();

	static OTObjectWrapper<OTFrame*> New(OTMediaType_t eMediaType, bool bOwnBuffer, const void *pBufferPtr, uint32_t nBufferSize);

protected:
	OTMediaType_t m_eMediaType;
	OTDimension_t m_eDimension;
	bool m_bOwnBuffer;
	void *m_pBufferPtr;
	uint32_t m_nBufferSize;
	uint32_t m_nValidDataSize;
	OTPatternType_t m_ePatternType;
	tsk_mutex_handle_t *m_phMutex;
};

#endif /* OPENTELEPRESENCE_FRAME_H */
