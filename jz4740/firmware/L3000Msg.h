#ifndef	_L3000MSG
#define	_L3000MSG

#define		KEY_AVOID_NOR_UNLOCK    '4'
#define		KEY_NOR_UNLOCK          '4'
extern	int	gInputNumKey;
extern	int	L3000GetKeyChar(char key);
extern	int	L3000MsgCheck(char *buf);
#endif
