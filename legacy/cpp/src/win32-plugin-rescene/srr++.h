/**
 * Copyright (C) 2014 syndicode
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 **/

#ifndef _SRRPP_H
#define _SRRPP_H

#include <stdint.h>
#include <string>
#include <vector>

#include "srr.h"

namespace SRR
{

class CStoredFile
{
public:
	CStoredFile(const std::wstring& a_filename, bool a_ownsMemory, const char* a_data, size_t a_dataLength);
	virtual ~CStoredFile();

	const std::wstring& GetFileName() const { return m_filename; }
	const char* GetDataPtr() const { return m_data; }
	size_t GetDataLength() const { return m_dataLength; }

protected:
	std::wstring m_filename;
	bool m_ownsMemory;
	const char *m_data;
	size_t m_dataLength;
};

typedef std::shared_ptr<CStoredFile> PStoredFile;

class CContainer
{
public:
	CContainer();

	bool ReadFile(const std::wstring& a_filePath);

	bool FindStoredFiles(const std::wstring& a_suffix, bool a_withoutSlashes, std::vector<PStoredFile>& ar_found);

protected:
	std::string m_appName;
	std::vector<PStoredFile> m_storedFiles;
};

typedef std::shared_ptr<CContainer> PContainer;

} // namespace SRR

#endif /* !_SRRPP_H */
