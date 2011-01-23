/**
 * @file llchat.cpp
 * @brief LLChat methods
 *
 * $LicenseInfo:firstyear=2011&license=viewerlgpl$
 * Kokua Viewer Source Code
 * Copyright (C) 2011, nemurimasu (at) gmail.com
 * for the Kokua Viewer Team.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * $/LicenseInfo$
 */

#include "llchat.h"

void LLChat::assign(const LLSD &llsd)
{
	if (llsd.has("text"))
		mText = llsd["text"].asString();
	if (llsd.has("from_name"))
		mFromName = llsd["from_name"].asString();
	if (llsd.has("from_id"))
		mFromID = llsd["from_id"].asUUID();
	if (llsd.has("notification_id"))
		mNotifId = llsd["notification_id"].asUUID();
	if (llsd.has("owner_id"))
		mOwnerID = llsd["owner_id"].asUUID();
	if (llsd.has("source_type"))
		mSourceType = (EChatSourceType)llsd["source_type"].asInteger();
	if (llsd.has("chat_type"))
		mChatType = (EChatType)llsd["chat_type"].asInteger();
	if (llsd.has("audible"))
		mAudible = (EChatAudible)llsd["audible"].asInteger();
	if (llsd.has("muted"))
		mMuted = llsd["muted"].asBoolean();
	if (llsd.has("time"))
		mTime = llsd["time"].asReal();
	if (llsd.has("time_string"))
		mTimeStr = llsd["time_string"].asString();
	if (llsd.has("pos_agent"))
	{
		LLSD pos_agent = llsd["pos_agent"];
		mPosAgent = LLVector3(pos_agent[0].asReal(), pos_agent[1].asReal(), pos_agent[2].asReal());
	}
	if (llsd.has("url"))
		mURL = llsd["url"].asString();
	if (llsd.has("chat_style"))
		mChatStyle = (EChatStyle)llsd["chat_style"].asInteger();
	if (llsd.has("session_id"))
		mSessionID = llsd["session_id"].asUUID();
}

LLSD LLChat::asLLSD() const
{
	LLSD ret = LLSD::emptyMap();
	ret["text"] = mText;
	ret["from_name"] = mFromName;
	ret["from_id"] = mFromID;
	ret["notification_id"] = mNotifId;
	ret["owner_id"] = mOwnerID;
	ret["source_type"] = (LLSD::Integer)mSourceType;
	ret["chat_type"] = (LLSD::Integer)mChatType;
	ret["audible"] = mAudible;
	ret["muted"] = mMuted;
	ret["time"] = mTime;
	ret["time_string"] = mTimeStr;
	ret["pos_agent"] = LLSD::emptyArray();
	LLSD &pos_agent = ret["pos_agent"];
	pos_agent[0] = (LLSD::Real)mPosAgent[0];
	pos_agent[1] = (LLSD::Real)mPosAgent[1];
	pos_agent[2] = (LLSD::Real)mPosAgent[2];
	ret["url"] = mURL;
	ret["chat_style"] = (LLSD::Integer)mChatStyle;
	ret["session_id"] = mSessionID;
	return ret;
}
