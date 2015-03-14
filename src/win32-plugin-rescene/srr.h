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

#ifndef _SRR_H
#define _SRR_H

#include <stdint.h>

// file format reference: https://bitbucket.org/Gfy/pyrescene/src/953c857099a8b8d30fb26bfaf8bba0c6f25740fc/dev-docs/srr_spec.txt?at=default#cl-508

#pragma pack(push)
#pragma pack(1)

struct srr_block_header_t
{
	uint16_t crc;
	uint8_t type;
	uint16_t flags;
	uint16_t head_size;
};

static_assert(sizeof(srr_block_header_t) == 7, "srr_block_header_t size/alignment");

struct srr_file_header_t
{
	srr_block_header_t bhdr;
	uint16_t app_name_size;
	/* - APP_NAME: SRR application that created the file.  APP_NAME_SIZE bytes */
};

static_assert(sizeof(srr_file_header_t) == 9, "srr_file_header_t size/alignment");

struct srr_stored_file_block_t
{
	srr_block_header_t bhdr;
	uint32_t file_size;
	uint16_t name_size;
	/* - NAME: path and name of the stored file            NAME_SIZE bytes
    [Stored File Data] */
};

static_assert(sizeof(srr_stored_file_block_t) == 13, "srr_stored_file_block_t size/alignment");


#pragma pack(pop)

#endif /* !_SRR_H */
