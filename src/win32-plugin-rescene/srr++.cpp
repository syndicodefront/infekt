/**
 * Copyright (C) 2014 cxxjoe
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

#include "stdafx.h"
#include "srr++.h"
#include "util.h"

using namespace SRR;


CContainer::CContainer()
{
}


bool CContainer::ReadFile(const std::wstring& a_filePath)
{
	FILE *l_file = NULL;
	size_t l_fileBytes;

	if(_wfopen_s(&l_file, a_filePath.c_str(), L"rb") != 0 || !l_file)
	{
		return false;
	}

	m_appName.clear();
	m_storedFiles.clear();

	l_fileBytes = _filelength(_fileno(l_file));

	if(l_fileBytes < sizeof(srr_file_header_t))
		goto READFAIL;

	srr_file_header_t file_header;

	if(fread_s(&file_header, sizeof(file_header), sizeof(file_header), 1, l_file) != 1)
		goto READFAIL;

	if(file_header.bhdr.crc != 0x6969 || file_header.bhdr.type != 0x69)
		goto READFAIL;

	// read app name if present:
	if((file_header.bhdr.flags & 0x01) != 0 && file_header.app_name_size > 0)
	{
		// sanity check:
		if(file_header.bhdr.head_size != file_header.app_name_size + sizeof(file_header))
			goto READFAIL;

		CAutoFreeBuffer<char> buf(file_header.app_name_size); // it's uint16_t, so nothing exceptionally bad can happen

		if(fread_s(buf.get(), file_header.app_name_size, 1, file_header.app_name_size, l_file) != file_header.app_name_size)
			goto READFAIL;

		m_appName = std::string(buf.get(), file_header.app_name_size);
	}

	do
	{
		srr_block_header_t block_header;

		if(fread_s(&block_header, sizeof(block_header), sizeof(block_header), 1, l_file) != 1)
		{
			if(feof(l_file))
				break;
			else
				goto READFAIL;
		}

		if(block_header.crc == 0x6A6A && block_header.type == 0x6A)
		{
			srr_stored_file_block_t stored_block;

			stored_block.bhdr = block_header;

			if((block_header.flags & 0x8000) == 0) // name is required
				goto READFAIL;

			if(fread_s(&stored_block.file_size, sizeof(stored_block) - sizeof(block_header), sizeof(stored_block) - sizeof(block_header), 1, l_file) != 1)
				goto READFAIL;

			CAutoFreeBuffer<char> namebuf(stored_block.name_size);

			if(fread_s(namebuf.get(), stored_block.name_size, 1, stored_block.name_size, l_file) != stored_block.name_size)
				goto READFAIL;

			// hard 10 MB limit:
			if(stored_block.file_size > 1024 * 1024 * 10)
				goto READFAIL;

			char* filebuf = new char[stored_block.file_size];

			if(fread_s(filebuf, stored_block.file_size, 1, stored_block.file_size, l_file) != stored_block.file_size)
			{
				delete[] filebuf;
				goto READFAIL;
			}

			const std::string l_utf8Filename(namebuf.get(), stored_block.name_size);
			const std::wstring l_wideFilename = CUtil::ToWideStr(l_utf8Filename, CP_UTF8);

			if(!l_wideFilename.empty())
			{
				PStoredFile l_instance(new CStoredFile(l_wideFilename, true, filebuf, stored_block.file_size));
				m_storedFiles.push_back(l_instance);
			}
			else
			{
				delete[] filebuf;
				goto READFAIL;
			}
		}
		else // other block, try to skip it
		{
			if(fseek(l_file, block_header.head_size, SEEK_CUR) != 0)
				goto READFAIL;

			if((block_header.flags & 0x8000) != 0)
			{
				uint32_t add_size = 0;

				if(fread_s(&add_size, sizeof(uint32_t), sizeof(uint32_t), 1, l_file) != 1)
					goto READFAIL;

				if(static_cast<long>(add_size) < 0 || fseek(l_file, add_size, SEEK_CUR) != 0)
					goto READFAIL;
			}
		}
	} while(feof(l_file) == 0 && ferror(l_file) == 0);

	fclose(l_file);

	return true;

READFAIL:
	fclose(l_file);
	return false;
}


bool CContainer::FindStoredFiles(const std::wstring& a_suffix, bool a_withoutSlashes, std::vector<const PStoredFile>& ar_found)
{
	size_t l_oldSize = ar_found.size();

	for(const auto& pf : m_storedFiles)
	{
		if(a_withoutSlashes && pf->GetFileName().find_first_of(L"\\/") != std::wstring::npos)
		{
			continue;
		}

		const std::wstring l_suffix = (pf->GetFileName().length() >= a_suffix.length() ? pf->GetFileName().substr(pf->GetFileName().length() - a_suffix.length()) : L"");

		if(_wcsicmp(l_suffix.c_str(), a_suffix.c_str()) == 0)
		{
			ar_found.push_back(pf);
		}
	}

	return ar_found.size() > l_oldSize;
}


CStoredFile::CStoredFile(const std::wstring& a_filename, bool a_ownsMemory, const char* a_data, size_t a_dataLength)
	: m_filename(a_filename), m_ownsMemory(a_ownsMemory), m_data(a_data), m_dataLength(a_dataLength)
{
}


CStoredFile::~CStoredFile()
{
	if(m_ownsMemory)
	{
		delete[] m_data;
		m_data = NULL;
	}
}
