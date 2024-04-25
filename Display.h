extern int display_init(char *bus, unsigned int address);

extern int displayRGB_init(char *bus, unsigned int address);

extern void display_write(int x, int y, char *txt);

extern void display_Clear();

extern void display_setColor(int red, int green, int blue);