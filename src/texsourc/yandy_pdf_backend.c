/* Copyright 2014 Clerk Ma

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301 USA.  */

#ifdef _WINDOWS
  #define NOCOMM
  #define NOSOUND
  #define NODRIVERS
  #define STRICT
  #pragma warning(disable:4115) // kill rpcasync.h complaint
  #include <windows.h>
  #define MYLIBAPI __declspec(dllexport)
#endif

#pragma warning(disable:4996)
#pragma warning(disable:4131) // old style declarator
#pragma warning(disable:4135) // conversion between different integral types
#pragma warning(disable:4127) // conditional expression is constant

#include <setjmp.h>

#define EXTERN extern

#include "texd.h"

HPDF_Doc  yandy_pdf;
HPDF_Page yandy_page;
HPDF_Font yandy_font;

void pdf_out(unsigned char i)
{
  char temp[2] = {i, '\0'};
  HPDF_PageAttr attr = (HPDF_PageAttr) yandy_page->attr;
  HPDF_Stream_WriteStr(attr->stream, temp);
}

void pdf_print_octal(integer n)
{
  unsigned int k = 0;

  do
    {
      dig[k] = n % 8;
      n = n / 8;
      incr(k);
    }
  while (n != 0);

  if (k == 1)
  {
    pdf_out('0');
    pdf_out('0');
  }

  if (k == 2)
  {
    pdf_out('0');
  }

  while (k > 0)
  {
    decr(k);
    pdf_out('0' + dig[k]);
  }
}

void pdf_print_int(integer n)
{
  integer k;
  integer m;

  if (n < 0)
  {
    pdf_out('-');
    if (n > -100000000)
      n = -n;
    else
    {
      m = -1 - n;
      n = m / 10;
      m = m % 10 + 1;
      k = 1;
      if (m < 10)
        dig[0] = (char) m;
      else
      {
        dig[0] = 0;
        incr(n);
      }
    }
  }
  do
    {
      dig[k] = n % 10;
      n = n / 10;
      incr(k);
    }
  while (n != 0);

  while (k > 0)
  {
    decr(k);
    pdf_out('0' + dig[k]);
  }
}

void pdf_print_char(unsigned char c)
{
  if ((c < 32) || (c == 92) || (c == 40) || (c == 41) || (c > 127))
  {
    pdf_out(92);
    pdf_print_octal(c);
  }
  else
  {
    pdf_out(c);
  }
}

void pdf_print_string(char * s)
{
  HPDF_PageAttr attr = (HPDF_PageAttr) yandy_page->attr;
  HPDF_Stream_WriteStr(attr->stream, s);
}

void pdf_end_string(void)
{
  if (pdf_doing_string)
  {
    pdf_print_string(")] TJ\012");
    pdf_doing_string = false;
  }
}

void pdf_begin_string(void)
{
  scaled s;

  s = cur_h - pdf_delta_h;

  if (!pdf_doing_string)
  {
    pdf_print_string(" [");
    if (s == 0)
      pdf_out('(');
  }

  if (s != 0)
  {
    if (pdf_doing_string)
      pdf_out(')');
    pdf_print_int(-s);
    pdf_out('(');
    pdf_delta_h = cur_h;
  }

  pdf_doing_string = true;
}