#pragma once
#include "stdafx.h"
VOID SendTextMessage(wchar_t * wxid, wchar_t * message);
VOID sendPicMessage(wchar_t * wxid, wchar_t * picPath);
wchar_t * UTF8ToUnicode(const char* str);