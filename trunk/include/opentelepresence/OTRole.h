/*
* Copyright (C) 2013 Mamadou DIOP
* Copyright (C) 2013 Doubango Telecom <http://www.doubango.org>
* License: GPLv3
* This file is part of the open source SIP TelePresence system <https://code.google.com/p/telepresence/>
*/
#ifndef OPENTELEPRESENCE_ROLE_H
#define OPENTELEPRESENCE_ROLE_H

#include "OpenTelepresenceConfig.h"

typedef enum OTRole_e
{
	OTRole_None = 0x00,
	OTRole_Attendee = (0x01<<0),
	OTRole_Participant = (0x01<<1),
	OTRole_Speaker = (0x01<<2) | OTRole_Attendee | OTRole_Participant,
	OTRole_Presenter = OTRole_Speaker,
}
OTRole_t;

static bool OTRoleIsAttendee(OTRole_t eRole)
{
	return (eRole & OTRole_Attendee) == OTRole_Attendee;
}

static bool OTRoleIsParticipant(OTRole_t eRole)
{
	return (eRole & OTRole_Participant) == OTRole_Participant;
}

static bool OTRoleIsSpeaker(OTRole_t eRole)
{
	return (eRole & OTRole_Speaker) == OTRole_Speaker;
}

static bool OTRoleIsPresenter(OTRole_t eRole)
{
	return (eRole & OTRole_Presenter) == OTRole_Presenter;
}

#endif /* OPENTELEPRESENCE_ROLE_H */
