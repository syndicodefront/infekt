#ifndef _SAUCE_H
#define _SAUCE_H

#define SAUCE_EOF '\x1A'
#define SAUCE_RECORD_SIZE 128
#define SAUCE_HEADER_ID_SIZE 5
#define SAUCE_COMMENT_LINE_SIZE 64
#define SAUCE_MAX_COMMENTS 255

// this is straight from ACiD's sauce-00.5 document:
// ("Standard Architecture for Universal Comment Extensions")

#pragma pack(push)
#pragma pack(1)

typedef struct _sauce_t
{
	char           ID[SAUCE_HEADER_ID_SIZE];
	char           Version[2];
	char           Title[35];
	char           Author[20];
	char           Group[20];
	char           Date[8];
	uint32_t       FileSize;
	unsigned char  DataType;
	unsigned char  FileType;
	uint16_t       TInfo1;
	uint16_t       TInfo2;
	uint16_t       TInfo3;
	uint16_t       TInfo4;
	unsigned char  Comments;
	unsigned char  TFlags;
	char           TInfoS[22];
} SAUCE;

static_assert(sizeof(SAUCE) == SAUCE_RECORD_SIZE, "SAUCE structure must be aligned to have 128 bytes");

#pragma pack(pop)

typedef struct _sauce_comment_header_t
{
	char           ID[SAUCE_HEADER_ID_SIZE];
} SAUCE_COMMENT_HEADER;

enum SAUCE_DATATYPES {
	SAUCEDT_UNDEFINED = 0,
	SAUCEDT_CHARACTER,
	SAUCEDT_BITMAP,
	SAUCEDT_VECTOR,
	SAUCEDT_AUDIO,
	SAUCEDT_BINARYTEXT,
	SAUCEDT_XBIN,
	SAUCEDT_ARCHIVE,
	SAUCEDT_EXECUTABLE,
};

enum SAUCE_FILETYPES_UNDEFINED {
	SAUCEFT_UNDEFINED = 0,
};

enum SAUCE_FILETYPES_CHARACTER {
	SAUCEFT_CHAR_ASCII = 0,
	SAUCEFT_CHAR_ANSI,
	SAUCEFT_CHAR_ANSIMATION,
	SAUCEFT_CHAR_RIP,
	SAUCEFT_CHAR_PCBOARD,
	SAUCEFT_CHAR_AVATAR,
	SAUCEFT_CHAR_HTML,
	SAUCEFT_CHAR_SOURCE,
	SAUCEFT_CHAR_TUNDRADRAW,
};

#endif /* !_SAUCE_H */
