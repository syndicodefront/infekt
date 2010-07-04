/* gutf8.c - Operations on UTF-8 strings.
 *
 * Copyright (C) 1999 Tom Tromey
 * Copyright (C) 2000 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <assert.h>

#define gchar char
#define guchar unsigned char
#define gunichar unsigned int
#define G_UNLIKELY(expr) expr
#define G_STMT_START do
#define G_STMT_END while(0)
#define gssize size_t
#define gboolean int
#define glong long
#define TRUE 1
#define FALSE 0
#define g_assert assert
#define g_return_val_if_fail(COND, RET) if(!(COND)) { return RET; }

#define g_utf8_next_char(p) (char *)((p) + g_utf8_skip[*(const guchar *)(p)])

#define CONTINUATION_CHAR                           \
 G_STMT_START {                                     \
  if ((*(guchar *)p & 0xc0) != 0x80) /* 10xxxxxx */ \
    goto error;                                     \
  val <<= 6;                                        \
  val |= (*(guchar *)p) & 0x3f;                     \
 } G_STMT_END

#define UNICODE_VALID(Char)                   \
	((Char) < 0x110000 &&                     \
	(((Char) & 0xFFFFF800) != 0xD800) &&     \
	((Char) < 0xFDD0 || (Char) > 0xFDEF) &&  \
	((Char) & 0xFFFE) != 0xFFFE)

static const gchar utf8_skip_data[256] = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1
};

const gchar * const g_utf8_skip = utf8_skip_data;

static const gchar *
fast_validate (const char *str)

{
  gunichar val = 0;
  gunichar min = 0;
  const gchar *p;

  for (p = str; *p; p++)
    {
      if (*(guchar *)p < 128)
	/* done */;
      else 
	{
	  const gchar *last;
	  
	  last = p;
	  if ((*(guchar *)p & 0xe0) == 0xc0) /* 110xxxxx */
	    {
	      if (G_UNLIKELY ((*(guchar *)p & 0x1e) == 0))
		goto error;
	      p++;
	      if (G_UNLIKELY ((*(guchar *)p & 0xc0) != 0x80)) /* 10xxxxxx */
		goto error;
	    }
	  else
	    {
	      if ((*(guchar *)p & 0xf0) == 0xe0) /* 1110xxxx */
		{
		  min = (1 << 11);
		  val = *(guchar *)p & 0x0f;
		  goto TWO_REMAINING;
		}
	      else if ((*(guchar *)p & 0xf8) == 0xf0) /* 11110xxx */
		{
		  min = (1 << 16);
		  val = *(guchar *)p & 0x07;
		}
	      else
		goto error;
	      
	      p++;
	      CONTINUATION_CHAR;
	    TWO_REMAINING:
	      p++;
	      CONTINUATION_CHAR;
	      p++;
	      CONTINUATION_CHAR;
	      
	      if (G_UNLIKELY (val < min))
		goto error;

	      if (G_UNLIKELY (!UNICODE_VALID(val)))
		goto error;
	    } 
	  
	  continue;
	  
	error:
	  return last;
	}
    }

  return p;
}

static const gchar *
fast_validate_len (const char *str,
		   gssize      max_len)

{
  gunichar val = 0;
  gunichar min = 0;
  const gchar *p;

  g_assert (max_len >= 0);

  for (p = str; ((size_t)(p - str) < max_len) && *p; p++)
    {
      if (*(guchar *)p < 128)
	/* done */;
      else 
	{
	  const gchar *last;
	  
	  last = p;
	  if ((*(guchar *)p & 0xe0) == 0xc0) /* 110xxxxx */
	    {
	      if (G_UNLIKELY (max_len - (p - str) < 2))
		goto error;
	      
	      if (G_UNLIKELY ((*(guchar *)p & 0x1e) == 0))
		goto error;
	      p++;
	      if (G_UNLIKELY ((*(guchar *)p & 0xc0) != 0x80)) /* 10xxxxxx */
		goto error;
	    }
	  else
	    {
	      if ((*(guchar *)p & 0xf0) == 0xe0) /* 1110xxxx */
		{
		  if (G_UNLIKELY (max_len - (p - str) < 3))
		    goto error;
		  
		  min = (1 << 11);
		  val = *(guchar *)p & 0x0f;
		  goto TWO_REMAINING;
		}
 	      else if ((*(guchar *)p & 0xf8) == 0xf0) /* 11110xxx */
		{
		  if (G_UNLIKELY (max_len - (p - str) < 4))
		    goto error;
		  
		  min = (1 << 16);
		  val = *(guchar *)p & 0x07;
		}
	      else
		goto error;
	      
	      p++;
	      CONTINUATION_CHAR;
	    TWO_REMAINING:
	      p++;
	      CONTINUATION_CHAR;
	      p++;
	      CONTINUATION_CHAR;
	      
	      if (G_UNLIKELY (val < min))
		goto error;
	      if (G_UNLIKELY (!UNICODE_VALID(val)))
		goto error;
	    } 
	  
	  continue;
	  
	error:
	  return last;
	}
    }

  return p;
}

/**
 * g_utf8_validate:
 * @str: a pointer to character data
 * @max_len: max bytes to validate, or -1 to go until NUL
 * @end: return location for end of valid data
 * 
 * Validates UTF-8 encoded text. @str is the text to validate;
 * if @str is nul-terminated, then @max_len can be -1, otherwise
 * @max_len should be the number of bytes to validate.
 * If @end is non-%NULL, then the end of the valid range
 * will be stored there (i.e. the start of the first invalid 
 * character if some bytes were invalid, or the end of the text 
 * being validated otherwise).
 *
 * Note that g_utf8_validate() returns %FALSE if @max_len is 
 * positive and NUL is met before @max_len bytes have been read.
 *
 * Returns %TRUE if all of @str was valid. Many GLib and GTK+
 * routines <emphasis>require</emphasis> valid UTF-8 as input;
 * so data read from a file or the network should be checked
 * with g_utf8_validate() before doing anything else with it.
 * 
 * Return value: %TRUE if the text was valid UTF-8
 **/
gboolean
g_utf8_validate (const char   *str,
		 gssize        max_len,    
		 const gchar **end)

{
  const gchar *p;

  if (max_len < 0)
    p = fast_validate (str);
  else
    p = fast_validate_len (str, max_len);

  if (end)
    *end = p;

  if ((max_len >= 0 && p != str + max_len) ||
      (max_len < 0 && *p != '\0'))
    return FALSE;
  else
    return TRUE;
}

/**
* g_utf8_find_next_char:
* @p: a pointer to a position within a UTF-8 encoded string
* @end: a pointer to the byte following the end of the string,
* or %NULL to indicate that the string is nul-terminated.
*
* Finds the start of the next UTF-8 character in the string after @p.
*
* @p does not have to be at the beginning of a UTF-8 character. No check
* is made to see if the character found is actually valid other than
* it starts with an appropriate byte.
* 
* Return value: a pointer to the found character or %NULL
**/
gchar *
g_utf8_find_next_char (const gchar *p,
					   const gchar *end)
{
	if (*p)
	{
		if (end)
			for (++p; p < end && (*p & 0xc0) == 0x80; ++p)
				;
		else
			for (++p; (*p & 0xc0) == 0x80; ++p)
				;
	}
	return (p == end) ? NULL : (gchar *)p;
}

/**
 * g_utf8_strlen:
 * @p: pointer to the start of a UTF-8 encoded string.
 * @max: the maximum number of bytes to examine. If @max
 *       is less than 0, then the string is assumed to be
 *       nul-terminated. If @max is 0, @p will not be examined and 
 *       may be %NULL.
 * 
 * Returns the length of the string in characters.
 *
 * Return value: the length of the string in characters
 **/
gssize
g_utf8_strlen (const gchar *p,
               long       max)
{
  gssize len = 0;
  const gchar *start = p;
  g_return_val_if_fail (p != NULL || max == 0, 0);

  if (max < 0)
    {
      while (*p)
        {
          p = g_utf8_next_char (p);
          ++len;
        }
    }
  else
    {
      if (max == 0 || !*p)
        return 0;
      
      p = g_utf8_next_char (p);          

      while (p - start < (intptr_t)max && *p)
        {
          ++len;
          p = g_utf8_next_char (p);          
        }

      /* only do the last len increment if we got a complete
       * char (don't count partial chars)
       */
      if (p - start <= (intptr_t)max)
        ++len;
    }

  return len;
}

