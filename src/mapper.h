#ifndef touchcursor_header
#define touchcursor_header

int isMapped(int code);
int isModifier(int code);
void processKey(int code, int type, int value);

#endif