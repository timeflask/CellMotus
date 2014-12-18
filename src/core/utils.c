#include "utils.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <wchar.h>
#include <stdio.h>
#include "logger.h"

#ifdef ANDROID
int wctomb(char *s, wchar_t wc) { return wcrtomb(s,wc,NULL); }
int mbtowc(wchar_t *pwc, const char *s, size_t n) { return mbrtowc(pwc, s, n, NULL); }
#endif
char *sen_strdup (const char *s)
{
    char *d = malloc (strlen (s) + 1);
    if (d == NULL) return NULL;
    strcpy (d,s);
    return d;
}

char *
sen_strndup (const char *s, size_t n)
{
  char *result;
  size_t len = strlen (s);

  if (n < len)
    len = n;

  result = (char *) malloc (len + 1);
  if (!result)
    return 0;

  result[len] = '\0';
  return (char *) memcpy (result, s, len);
}

wchar_t *sen_strdupW (const wchar_t *s)
{
  size_t memc = wcslen (s) * sizeof(wchar_t);
  wchar_t *d = calloc (memc + sizeof(wchar_t), 1);
  if (d == NULL) return NULL;
  memcpy(d,s,memc);
  return d;
}

static int UtfToWide(const char *Src,wchar_t *Dest,size_t DestSize)
{
  int Success=1;
  long dsize=(long)DestSize;
  dsize--;
  while (*Src!=0)
  {
    unsigned int c=(unsigned char)(*(Src++)),d;
    if (c<0x80)
      d=c;
    else
      if ((c>>5)==6)
      {
        if ((*Src&0xc0)!=0x80)
        {
          Success=0;
          break;
        }
        d=((c&0x1f)<<6)|(*Src&0x3f);
        Src++;
      }
      else
        if ((c>>4)==14)
        {
          if ((Src[0]&0xc0)!=0x80 || (Src[1]&0xc0)!=0x80)
          {
            Success=0;
            break;
          }
          d=((c&0xf)<<12)|((Src[0]&0x3f)<<6)|(Src[1]&0x3f);
          Src+=2;
        }
        else
          if ((c>>3)==30)
          {
            if ((Src[0]&0xc0)!=0x80 || (Src[1]&0xc0)!=0x80 || (Src[2]&0xc0)!=0x80)
            {
              Success=0;
              break;
            }
            d=((c&7)<<18)|((Src[0]&0x3f)<<12)|((Src[1]&0x3f)<<6)|(Src[2]&0x3f);
            Src+=3;
          }
          else
          {
            Success=0;
            break;
          }
    if (Dest!=NULL && --dsize<0)
      break;
    if (d>0xffff)
    {
      if (Dest!=NULL && --dsize<0)
        break;
      if (d>0x10ffff)
      {
        Success=0;
        continue;
      }
      if (Dest!=NULL) {
        if (sizeof(*Dest)==2)
        {
          *(Dest++)=((d-0x10000)>>10)+0xd800;
          *(Dest++)=(d&0x3ff)+0xdc00;
        }
        else
          *(Dest++)=d;
      }
    }
    else
      if (Dest!=NULL)
        *(Dest++)=d;
  }
  if (Dest!=NULL)
    *Dest=0;
  return Success;
}



wchar_t *sen_strdupToW (const char* s)
{
  int len = (strlen(s)+1) * 4 * sizeof(wchar_t);
  wchar_t * r = malloc( len );
  UtfToWide(s, r, len);
  return r;
}
