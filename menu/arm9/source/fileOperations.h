#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

extern char sdLabel[12], fatLabel[12];
extern int sdSize, fatSize;

extern bool sdFound(void);
extern bool flashcardFound(void);
extern bool sdMount(void);
extern bool flashcardMount(void);

#endif