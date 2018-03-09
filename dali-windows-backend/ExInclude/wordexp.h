#ifndef _WORD_EXP_INCLUDE_
#define _WORD_EXP_INCLUDE_

/* Structure describing a word-expansion run.  */
typedef struct
{
  size_t we_wordc;		/* Count of words matched.  */
  char **we_wordv;		/* List of expanded words.  */
  size_t we_offs;		/* Slots to reserve in `we_wordv'.  */
} wordexp_t;

/* Do word expansion of WORDS into PWORDEXP.  */
int wordexp( const char *__restrict __words, wordexp_t *__restrict __pwordexp, int __flags )
{
  return 0;
}

/* Free the storage allocated by a `wordexp' call.  */
void wordfree( wordexp_t *__wordexp )
{

}

size_t readlink( const char *__restrict __path, char *__restrict __buf, size_t __len )
{
  return 0;
}

#endif