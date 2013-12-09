/*************************************************
                                                                                                               
 ZEM 200
                                                                                                               
 lcdmenu.c menu fucntions
                                                                                                               
 Copyright (C) 2003-2004, ZKSoftware Inc.
 
 $Log: lcdmenu.c,v $
 Revision 5.14  2006/03/04 17:30:09  david
 Add multi-language function

 Revision 5.13  2005/11/06 02:41:34  david
 Fixed RTC Bug(Synchronize time per hour)

 Revision 5.12  2005/08/07 08:13:15  david
 Modfiy Red&Green LED and Beep

 Revision 5.11  2005/08/02 16:07:51  david
 Add Mifare function&Duress function

 Revision 5.10  2005/07/14 16:59:53  david
 Add update firmware by SDK and U-disk

 Revision 5.9  2005/07/07 08:09:02  david
 Fixed AuthServer&Add remote register

 Revision 5.8  2005/06/16 23:27:51  david
 Add AuthServer function

 Revision 5.7  2005/06/10 17:11:01  david
 support tcp connection

 Revision 5.6  2005/05/13 23:19:32  david
 Fixed some minor bugs

 Revision 5.5  2005/04/27 00:15:37  david
 Fixed Some Bugs

 Revision 5.4  2005/04/24 11:11:26  david
 Add advanced access control function
                                                                                                       
*************************************************/                                                                                                               

#include <stdlib.h>
#include <string.h>
#include "exfun.h"
#include "msg.h"
#include "utils.h"
#include "options.h"
#include "lcdmenu.h"
#include "lcm.h"
#include "kb.h"

int InputTimeOut=0;

#define MENU_ITEM_CHARWIDTH 32


int CalcMenuItemOffset(PMenu menu, int index)
{
	int j=0, cc;
	int ShowTitle=(menu->Title && ((menu->TopIndex<0) || (RowCount>2)));
	if(menu->Title && ShowTitle)
		j++;
	cc=menu->TopIndex;
	if(cc<0)cc=0;
	return index-cc+j;
}

#define LeftStart (MenuIndicatorWidth*gLangDriver->CharWidth/2)

int CheckMenuStyle(PMenu menu)
{
        if(MenuStyle_ICON==menu->Style)
        {
                if(gLCDHeight<64)
                        menu->Style=MenuStyle_OLD;
                else
                {
                        int i;
                        for(i=0;i<menu->Count;i++)
                        if(menu->Items[i].Icon==NULL)
                        {
                                menu->Style=MenuStyle_OLD; break;
                        }
                }
        }
        return 0;
}

char *FormatTime(char *line, int v)
{
	v=v & 0xFFFF;
	if(((v & 0xFF)>59) || ((v>>8)>23))
	{
		sprintf(line,"%s", LoadStrByID(HID_NO));
	}
	else
	{
		sprintf(line,"%02d:%02d", v>>8, v & 0xFF);
	}
	return line;
}

void WriteMenuValue(TMenuItem *mi, int Row, int Modify)
{
	char line[MAX_CHAR_WIDTH];
	int len;
	line[0]=0;
	if(mi->TitleItem==NULL)
	{
		if(mi->ValueItem!=NULL)	//时间
		{
			FormatTime(line, *mi->ValueField);
		}
		else	//普通数值
			sprintf(line,"%d",*mi->ValueField);
	}
	else
	{
		int Index=*mi->ValueField;
		if(mi->ValueItem)
		{
			int j;
			for(j=0;j<mi->MaxValue;j++)
				if(mi->ValueItem[j]==Index)
				{
					Index=j; break;
				}
		}
		if(Index<mi->MaxValue)
		{
			char buffer[MAX_CHAR_WIDTH];
			if(SCopyStrFrom(buffer, mi->TitleItem, Index)>0)
				sprintf(line,"%s",buffer);
		}
	}
	len=gLangDriver->GetTextWidthFun(gLangDriver, line)/gLangDriver->CharWidth;
	if(mi->ValueWidth<len) len=mi->ValueWidth;
	LCDWriteStr(Row,gLCDCharWidth-len,line,Modify);
}
void ShowMenu(PMenu menu)
{
	int i,j,c, ShowTitle=0, cc;
	char line[MAX_CHAR_WIDTH];

	if(MenuStyle_ICON==menu->Style)
                c=gLCDWidth/32;
	else
	{
		ShowTitle=(menu->Title && ((menu->TopIndex<0) || (RowCount>2)));
		if(ShowTitle) c=RowCount-1; else c=RowCount;	//LCD上显示的菜单项数量
		//校正第一个显示的菜单项序号
		if(menu->Count<=menu->TopIndex) menu->TopIndex=menu->Count-c;  
		if(menu->ItemIndex<menu->TopIndex) menu->TopIndex=menu->ItemIndex;
	}

	LCDBufferStart(LCD_BUFFER_ON);
	LCD_Clear();
	j=0;
        if(MenuStyle_ICON==menu->Style)
        {
                cc=menu->TopIndex;
                if(cc<0)cc=0;

                for(i=cc;i<menu->Count && i<cc+c; i++,j++)
                {
                        if(i==menu->ItemIndex)
                        {
                                LCD_Bar(j*32, 3, j*32+31, 6+32+3);
                                LCD_OutBMP1Bit(j*32, 6, menu->Items[i].Icon, 0,0,32,32,1);
                        }
                        else
                                LCD_OutBMP1Bit(j*32, 6, menu->Items[i].Icon, 0,0,32,32,0);
                        LCD_Line(0, gLCDHeight-gRowHeight-3, gLCDWidth, gLCDHeight-gRowHeight-3);
                        LCD_Line(0, gLCDHeight-gRowHeight-1, gLCDWidth, gLCDHeight-gRowHeight-1);
                }
                LCDWriteStr(gLCDRowCount-1, 0, "                     ", LCD_HIGH_LIGHT);
                LCDWriteStr(gLCDRowCount-1, 0, menu->Items[menu->ItemIndex].Caption, LCD_HIGH_LIGHT);
        }
	else if(MenuStyle_STD==menu->Style)
	{
		if(menu->Title && ShowTitle)
		{
/*			for(i=1;i<gLCDCharWidth/2;i++)
			line[i*2]=(char)CHLB0, line[i*2+1]=(char)CHTL;	//horized line
			line[0]=(char)CHLB0;	
			if(menu->TopIndex>0)	//不是从菜单第一项开始显示的
				line[1]=(char)CHLT1;			//arrowed lefttop corner 
			else
				line[1]=(char)CHLT0;			//lefttop corner 
			LCDWriteStrLng(gSymbolDriver, 0, 0, line,0);
*/
			LCD_Line(LeftStart,gRowHeight/2,gLCDWidth-1,gRowHeight/2);
			LCD_Line(LeftStart,gRowHeight/2,LeftStart, gRowHeight);
			if(menu->TopIndex>0)
				LCD_Triangle(LeftStart,gRowHeight/2+1, 4, TriDir_Bottom);

			LCDWriteCenter(j++,menu->Title);
		}
		cc=menu->TopIndex;
		if(cc<0)cc=0;
		for(i=cc;i<menu->Count && i<cc+c; i++,j++)
		{
			int len=strlen(menu->Items[i].Caption);
			if(len>32) len=32;
			memset(line,' ',32);			
			memcpy(line,menu->Items[i].Caption,len);
			
			if(i==menu->ItemIndex)
				LCDWriteStr(j,MenuIndicatorWidth,line,LCD_HIGH_LIGHT);
			else if(i==menu->Count-1) //菜单的最后一项
				LCDWriteStr(j,1,line,LCD_BOTTOM_LINE);
			else
				LCDWriteStr(j,MenuIndicatorWidth,line,0);
			line[1]=' ';
			line[MenuIndicatorWidth]=0;
			
			LCD_Line(LeftStart, j*gRowHeight, LeftStart, j*gRowHeight+gRowHeight);
			if(i==menu->Count-1) //菜单的最后一项
			{
				LCD_Line(LeftStart, j*gRowHeight+gRowHeight-1, gLCDWidth, j*gRowHeight+gRowHeight-1);
			}
			else if(i==menu->TopIndex+c-1) //还有菜单项没有显示出来
			{
				LCD_Triangle(LeftStart,gLCDHeight-1, 4, TriDir_Top);
			}
			else 
			{
				LCD_Line(LeftStart, j*gRowHeight, LeftStart, j*gRowHeight+gRowHeight);
			}
		}
	}
	else if(MenuStyle_OLD==menu->Style)
	{
		if(menu->Title && ShowTitle)
		{
			memset(line,' ',32);
			memcpy(line,menu->Title,strlen(menu->Title));
			LCDWriteStr(j++,0,line,0);
		}
		cc=menu->TopIndex;
		if(cc<0)cc++;
		for(i=cc;i<menu->Count && i<cc+c; i++,j++)
		{
			memset(line,' ',32);
//			printf("caption %s,address of item=%p\n",menu->Items[i].Caption,menu->Items[i].Caption);//treckle
			memcpy(line+MenuIndicatorWidth,menu->Items[i].Caption,strlen(menu->Items[i].Caption));
			LCDWriteStr(j,0,line,0);
			if(menu->Items[i].ValueWidth)
			{
				WriteMenuValue(&menu->Items[i], j, 0);
			}
			if(i==menu->ItemIndex)
			{
				LCD_Triangle(7, j*gRowHeight+gRowHeight/2, 6, TriDir_Left);
			}
		}
		
		if(menu->TopIndex>0 || c+menu->TopIndex<menu->Count)
		{
			LCDWriteStr(0, gLCDCharWidth-1, " ", 0);
			LCD_Line(gLCDWidth-5, 2, gLCDWidth-5, gRowHeight-2);
			if(menu->TopIndex>0 && c+menu->TopIndex<menu->Count)
			{//Up and Down Arrow
				LCD_Triangle(gLCDWidth-5, 2, 4, TriDir_Bottom);
				LCD_Triangle(gLCDWidth-5, gRowHeight-2, 4, TriDir_Top);
			}
			else if(menu->TopIndex>0)
			{//Up Arrow
				LCD_Triangle(gLCDWidth-5, 2, 4, TriDir_Bottom);
			}
			else if(c+menu->TopIndex<menu->Count)
			{//Down Arrow
				LCD_Triangle(gLCDWidth-5, gRowHeight-2, 4, TriDir_Top);
			}
		}
	}
	//填充空行
        if(MenuStyle_ICON!=menu->Style)
		for(;j<RowCount;j++)
		{
			memset(line,' ',gLCDCharWidth);
			LCDWriteStr(j,0,line,0);
		}
	LCDBufferStart(LCD_BUFFER_OFF);
}

int RunMenuMsg(PMsg msg)
{
	int oldkey,i;
	PMenu menu;
	menu=(PMenu)msg->Object;
	if(MSG_TYPE_TIMER==msg->Message && InputTimeOut>=0)
	{
		msg->Message=0;
		msg->Param1=0;
		if(++InputTimeOut>=InputTimeOutSec)
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_TimeOut);
		}
		return 1;
	}
	else if(!(MSG_TYPE_BUTTON==msg->Message)) 
		return 0;
	oldkey=msg->Param1;
	msg->Param1=0;
	if(InputTimeOut>=0) InputTimeOut=0;
	
	if(IKeyOK==oldkey)
	{
		if(menu->Items[menu->ItemIndex].Action)
		{
			//如果菜单项反显（MenuStyle_STD风格），则不要反显，重新绘制菜单项
			//——如果菜单具有现场编辑的功能，菜单项反显将无法突出显示输入项
			if(menu->Style==MenuStyle_STD)
			{
                                int row=CalcMenuItemOffset(menu, menu->ItemIndex);
                                LCDWriteStr(row, MenuIndicatorWidth, "     ", 0);
                                LCDWriteStr(row, MenuIndicatorWidth, menu->Items[menu->ItemIndex].Caption, 0);

			}
			//执行菜单命令
			if(News_TimeOut==menu->Items[menu->ItemIndex].Action(msg))
			{
				ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_TimeOut);
				return 1;
			}
		}
		if(menu->Items[menu->ItemIndex].SubMenu)
		{
			menu=menu->Items[menu->ItemIndex].SubMenu;
			msg->Object=menu;
                        CheckMenuStyle(menu);
		}
		ShowMenu(menu);
	}
	else if(IKeyUp==oldkey)
	{
		if(menu->ItemIndex>0)
		{
			menu->ItemIndex--;
			if(menu->ItemIndex<menu->TopIndex) menu->TopIndex=menu->ItemIndex;
			ShowMenu(menu);
		}
		else if(menu->ItemIndex==0 && menu->Title && (RowCount<=2))
		{
			menu->TopIndex=-1;
			ShowMenu(menu);
		}
	}
	else if(IKeyDown==oldkey)
	{ 
		if(menu->ItemIndex<menu->Count-1)
		{
			menu->ItemIndex++;
			//if(menu->Title && RowCount>2) i=RowCount-1; else i=RowCount;
                        if(menu->Title && RowCount>2 && (MenuStyle_ICON!=menu->Style)) i=RowCount-1; else i=RowCount;
			if(menu->ItemIndex>=menu->TopIndex+i) menu->TopIndex++;
			ShowMenu(menu);
		}
	}
	else if(IKeyESC==oldkey)
	{
		if(menu->Parents)
		{
			menu=menu->Parents;
			msg->Object=menu;
			ShowMenu(menu);
		}
		else
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, 0);
			return 1;
		}
		
	}
/*	else if(MenuStyle_PEFIS!=menu->Style && (oldkey>='1') && (oldkey<=('0'+menu->Count)))
	{
		menu->ItemIndex=oldkey-'1';
		if(menu->Items[menu->ItemIndex].Action) 
			menu->Items[menu->ItemIndex].Action(&menu->Items[menu->ItemIndex]);
		if(menu->Items[menu->ItemIndex].SubMenu)
		{
			menu=menu->Items[menu->ItemIndex].SubMenu;
			msg->Object=menu;
		}
		ShowMenu(menu);
	}*/
	msg->Message=0;
	msg->Param1=0;
	return 1;
}

int RunMenu(PMenu menu)
{
	int ret,i=RegMsgProc(RunMenuMsg);
	U32 mm=SelectNewMsgMask(MSG_TYPE_BUTTON|MSG_TYPE_TIMER);
	if(menu->Title && (gLCDRowCount<=2)) menu->TopIndex=-1;
        CheckMenuStyle(menu);
	ShowMenu(menu);
	InputTimeOut=0;
	ret=DoMsgProcess(menu, News_Exit_Input);	
	SelectNewMsgMask(mm);
	UnRegMsgProc(i);
	return ret;
}

void ShowInput(PInputBox box)
{
	char s[32],tmp[32];
	int i,Align, Theflag;
	Align=box->Alignment;
	if(Align==Alignment_Auto)
	{
		if(box->Style==InputStyle_Number || box->Style==InputStyle_ANumber || box->Style==InputStyle_Number2)
			Align=Alignment_Right;
		else
			Align=Alignment_Left;
	}
	memset(s,' ',32);
	if(box->SelectStart<box->TopIndex)
		box->TopIndex=box->SelectStart;
	strcpy(tmp,box->Text+box->TopIndex);
	i=box->SelectStart-box->TopIndex;
	tmp[i]=0;
	if(i>0 && box->PasswordChar) memset(tmp,box->PasswordChar,i);
	if(Align==Alignment_Left)  //gLCDCharWidth
		memcpy(s,tmp,i);
	else if(Align==Alignment_Center)
		SPadCenterStr(s, box->Width, tmp);
	else 
		SPadRightStr(s, box->Width, tmp);
	s[box->Width]=0;
	//Theflag=LCD_RIGHT_LINE|LCD_TOP_LINE|LCD_LEFT_LINE|LCD_RIGHT_LINE; 
	Theflag=LCD_HIGH_LIGHT;
	LCDWriteStr(box->Row,box->Col,s,Theflag);
	if(i<box->Width)
	{
		if(box->SelectLength)
		{
			if(box->PasswordChar)
				memset(tmp,box->PasswordChar,32);
			else
				strcpy(tmp,box->Text+box->SelectStart);
			tmp[box->Width-i]=0;
			if(Align==Alignment_Left)
				LCDWriteStr(box->Row,box->Col+i,tmp,LCD_BOTTOM_LINE|Theflag);
			else if(i==0)
			{
				if(Align==Alignment_Center)
					LCDWriteStr(box->Row,box->Col+(box->Width-box->SelectLength)/2,
						tmp,LCD_BOTTOM_LINE|Theflag);
				else
					LCDWriteStr(box->Row,box->Col+box->Width-box->SelectLength,
						tmp,LCD_BOTTOM_LINE|Theflag);
			}
		}
		else if(Align==Alignment_Left)
		{
			s[i]='_';
			LCDWriteStr(box->Row,box->Col+i,s+i,Theflag);
		}
	}

}

#define ReturnMsg(NEWS,p1) msg->Message=MSG_TYPE_CMD;\
				msg->Param1=NEWS;\
				msg->Param2=p1;\
				return 1;

int RunInputMsg(PMsg msg)
{
	int oldkey,i;
	PInputBox box=(PInputBox)msg->Object;
	if(MSG_TYPE_TIMER==msg->Message && InputTimeOut>=0)
	{
		msg->Message=0;
		msg->Param1=0;
		if(++InputTimeOut>=InputTimeOutSec)
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_TimeOut);
		}
		return 1;
	}
	if(!(MSG_TYPE_BUTTON==msg->Message)) return 0;
	oldkey=msg->Param1;
	msg->Param1=0;
	if(InputTimeOut>=0) InputTimeOut=0;
	if(oldkey==IKeyOK)
	{
		box->Text[box->SelectStart+box->SelectLength]=0;
		if(box->ValidFun)
		{
			if(!box->ValidFun(box->Text))
			{
				ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_ErrorInput);
				return 1;
			}
		}
		ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_CommitInput);
		return 1;
	}
	else if(oldkey==IKeyESC)
	{
		ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_CancelInput);
		return 1;
	}
	else if(oldkey==IKeyDown || oldkey==IKeyUp)
	{
		int NewValue=0;
		if (box->Style==InputStyle_Number2)
			return 1;
		if(box->Items && box->ItemCount)
		{
			i=SearchIndex(box->Items,box->Text,box->ItemCount);
			if(oldkey==IKeyUp) i-=1; else i+=1;
			if(i<0) i=box->ItemCount-1; else if(i>=box->ItemCount) i=0;
			strcpy(box->Text,box->Items[i]);
			box->SelectStart=0;
			box->SelectLength=strlen(box->Text);
			NewValue=1;
		}
		else if(box->AllowNav)
		{
			if(oldkey==IKeyDown)
				ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_NextInput);
			else
				ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_PrevInput);
			return 1;				
		}
		else if(box->Style==InputStyle_Number || box->Style==InputStyle_ANumber)
		{
			int v,v1;
			char tmp[40];
			i=0;
			while(box->Text[i]) if(box->Text[i]>'0') break; else i++;
			if(!(strtou32(box->Text+i,(U32*)&v)==0)) v=0;
			if(oldkey==IKeyDown) i=-1;else i=1;
			v+=i;v1=v;
			if(box->ValidFun && (box->MaxValue-box->MinValue<0x10000)) 
			do{
				if(box->Style==InputStyle_ANumber)
					Pad0Num(tmp, box->MaxLength, v);
				else
					sprintf(tmp, "%d", v);	
				if(box->ValidFun(tmp)) break;
				v+=i;
				if(box->MaxValue && v>box->MaxValue) v=box->MinValue;
				if(box->MinValue && v<box->MinValue) v=box->MaxValue;
				if(v==v1) break;
			}
			while(1);

			if(box->MaxValue && v>box->MaxValue) v=box->MinValue;
			else if(v<box->MinValue) v=box->MaxValue;
			if(box->Style==InputStyle_ANumber)
				Pad0Num(box->Text, box->MaxLength, v);
			else
				sprintf(box->Text, "%d", v);	
			NewValue=1;
		}
		if(NewValue)
		{
			box->TopIndex=0;
			box->SelectStart=0;
			box->SelectLength=strlen(box->Text);
			ShowInput(box);
		}
	}
	else if(oldkey>0x20 && oldkey<0x7F && box->Style!=InputStyle_Select) //ascii
	{
		box->SelectLength=0;
		if(box->Style==InputStyle_Number || box->Style==InputStyle_ANumber || box->Style==InputStyle_Number2)
		{
			if(oldkey>'9' || oldkey<'0') return 1;
		}
		if(box->SelectStart==(box->MaxLength)  && oldkey>='0' && oldkey<='9' && box->Text[0]=='0')
		{
			for(i=0;i<box->SelectStart;i++)
				box->Text[i]=box->Text[i+1];
			box->SelectStart--;
		}
		//2006.10.11 增加当输人数字的长度=宽度时，不再将输人值赋给box->Text功能
		if (box->MaxLength != box->SelectStart)
			box->Text[box->SelectStart]=oldkey;
		if(!box->MaxLength || box->MaxLength>box->SelectStart)
		{
			box->SelectStart++;
			box->Text[box->SelectStart]=0;
		}
		else
		{
			box->Text[box->SelectStart-1]=oldkey;
		}
		if(box->SelectStart-box->TopIndex>box->Width)
			box->TopIndex++;
		if(box->Style==InputStyle_ANumber)
		{
			U32 v;
			i=0;
			while(box->Text[i]=='0') {i++;}
			if(!(strtou32(box->Text+i,(U32*)&v)==0)) v=0;
			Pad0Num(box->Text, box->MaxLength, v);
			box->SelectStart=box->MaxLength;
		}
		if(box->AutoCommit && box->SelectStart>=box->MaxLength)
		{
			ConstructMSG(msg, MSG_TYPE_CMD, News_Exit_Input, News_CommitInput);
			return 1;
		}
		ShowInput(box);
	}
	msg->Message=0;
	return 1;
}

int RunInput(PInputBox box)
{
	int ret,i=RegMsgProc(RunInputMsg);
	if(box->ValidFun && box->Text[0] && (box->MaxValue-box->MinValue<0x10000))
	{
		//改变初始值为第一个有效的值
		U32 v;
		while(!box->ValidFun(box->Text))
		{
			strtou32(box->Text, &v);
			v++;
			if((int)v>box->MaxValue)	v=box->MinValue;
			if(box->Style==InputStyle_ANumber)
				Pad0Num(box->Text, box->MaxLength, v);
			else
				sprintf(box->Text, "%d", v);
		}
	}
	ShowInput(box);
	InputTimeOut=0;
	ret=DoMsgProcess(box, News_Exit_Input);
	UnRegMsgProc(i);
	return ret;
}

PMenu CreateMenu(char *Title, int Style, PMenu Parents, PMenu menu)
{
        PMenu m;
/*	if(menu&&Title)	
	{
		memset(menu->Title, 0, strlen(Title)+1);
		strcpy(menu->Title, Title);
		return	menu;
	}*/
	if(Title)
	{	
		m=(PMenu)malloc(sizeof(TMenu));
		memset(m,0,sizeof(TMenu));
		m->Parents=Parents;
		m->Title=(char *)malloc(strlen(Title)+1);
		strcpy(m->Title, Title);
		m->Style=Style;
		m->MenuItmeLoaded=0; /*  AddMenuItem will malloc memory if MenuLoaded == 0 */
		m->OldCount = 0; 
	//	printf("Create Menu: %s, %p\n", Title, m);
		return m;
	}
	else
		return NULL;
}
                                                                                                               
int DestroyMenu(PMenu menu)
{
        int i;
	if(menu==NULL) 
		return 0;
//	printf("DestroyMenu %p\n", menu);//treckle
	/* 1 mean menu Itmes have been loaded, no need malloc and load again */
#if 0
	menu->MenuItmeLoaded = 1;
	menu->OldCount = menu->Count;
	menu->Count = 0;
	menu->ItemIndex = 0; 
#else
        for(i=0;i<menu->Count;i++)
            if ((menu->Items[i]).MemStyle)
		free((menu->Items[i]).Caption);	

	free(menu->Items);
	free(menu->Title);
	free(menu);
	menu=NULL;
#endif
        return 0;
}

PMenuItem AddMenuItem(char MemStyle, PMenu menu, char *Caption, ActionFunc Action, PMenu SubMenu)
{
        PMenuItem mi;	
	char *p=NULL;
	if (Caption==NULL) return NULL;
	if (strstr(Caption, "(null)")) return NULL;
	
//	printf("AddMenuItem menu=%p,Items=%s,Items=%p,Count=%d\n",menu,Caption,menu->Items,menu->Count);//treckle
	if (menu->Items == NULL)
		menu->Items=(PMenuItem)malloc(sizeof(TMenuItem)*20);
        menu->Count++;
        mi=&(menu->Items[menu->Count-1]);
		memset(mi,0,sizeof(TMenuItem));
        if(MemStyle)
        {
		if(menu->MenuItmeLoaded==0||menu->Count>menu->OldCount)
			mi->Caption=(char *)malloc(MENU_ITEM_CHARWIDTH);//treckle
	
		memset(mi->Caption, 0, MENU_ITEM_CHARWIDTH);	//treckle
            	strcpy(mi->Caption, Caption);
        }
	else
		mi->Caption=Caption;
        mi->Action=Action;
        mi->SubMenu=SubMenu;
        mi->MemStyle=MemStyle;
        mi->Icon=NULL;//treckle
        return mi;
}

int InputLine(int row, int col, int width, char *text)
{
	int ret,i;
//	PInputBox box;

        unsigned char box_t[sizeof(TInputBox)];
        PInputBox box = (PInputBox)box_t;

//	box=(PInputBox)malloc(sizeof(TInputBox));	//treckle
	memset(box,0,sizeof(TInputBox));
	if(text)
	{
		box->SelectLength=strlen(text);
		strcpy(box->Text,text);
	}
	else
		box->SelectLength=0;
	box->MaxLength=width;
	box->Row=row;
	box->Col=col;
	box->Width=width;
	ret=RunInput(box);
	if(News_CommitInput==ret)
	{
		i=box->SelectStart;
		if(text) 
		{
			nstrcpy(text,box->Text,i);
			text[i]=0;
		}
	}
//	free(box);
	return ret;
}

int InputNumber(int row, int col, int width, int *value, int minv, int maxv, int nav)
{
	int ret,i;
//	PInputBox box;
        unsigned char box_t[sizeof(TInputBox)];
        PInputBox box = (PInputBox)box_t;

//	box=(PInputBox)malloc(sizeof(TInputBox));	//treckle
	memset(box,0,sizeof(TInputBox));
	box->MaxLength=width;
	box->Row=row;
	box->Col=col;
	box->MinValue=minv;
	box->MaxValue=maxv;
	box->Alignment=Alignment_Right;
	if(value) 
		sprintf(box->Text,"%d",*value);
	else
		sprintf(box->Text,"%d",minv);
	box->SelectLength=strlen(box->Text);
	box->Width=width;
	box->Style=InputStyle_Number;
	box->AllowNav=nav;
	ret=RunInput(box);
	if(News_CommitInput==ret || News_NextInput==ret || News_PrevInput==ret)
	{
		i=box->SelectStart+box->SelectLength;
		box->Text[i]=0;
		if(i>0 && strtou32(box->Text,(U32*)&i)==0)
		{
			if(i<minv || i>maxv) ret=News_ErrorInput;
			if(value) *value=i;
		}
		else
			ret=News_ErrorInput;
	}
//	free(box);
	return ret;
}

int InputTextNumber(int row, int col, int width, int *value, int minv, int maxv, U8 style)
{
	int ret,i,v;
        unsigned char box_t[sizeof(TInputBox)];
        PInputBox box = (PInputBox)box_t;

//	PInputBox box;
//	box=(PInputBox)malloc(sizeof(TInputBox));	//treckle
	memset(box,0,sizeof(TInputBox));
	box->MaxLength=width;
	box->Row=row;
	box->Col=col;
	box->MinValue=minv;
	box->MaxValue=maxv;
	box->Alignment=Alignment_Right;
	if(value) 
		v=*value;
	else
		v=minv;	
	if(style==InputStyle_ANumber)	
		Pad0Num(box->Text, box->MaxLength, v);
	else
		sprintf(box->Text,"%d",v);	
	box->SelectLength=strlen(box->Text);
	box->Width=width;
	box->Style=style;
	box->AllowNav=FALSE;
	ret=RunInput(box);
	if(News_CommitInput==ret || News_NextInput==ret || News_PrevInput==ret)
	{
		i=box->SelectStart+box->SelectLength;
		box->Text[i]=0;
		if(i>0 && strtou32(box->Text,(U32*)&i)==0)
		{
			if(i<minv || i>maxv) ret=News_ErrorInput;
			if(value) *value=i;
		}
		else
			ret=News_ErrorInput;
	}
//	free(box);
	return ret;
}

static BYTE *sCard=0;

void ShowERROR(int row, int ErrorCode)
{
	ExBeep(2);
	LCDWriteCenterStrID(row, ErrorCode);
	DelayMS(1000);
	LCDWriteCenterStr(row, "");
}

void ShowHIDOK(int row, BYTE *card)
{
	char buf[40];
	if(sCard)
	{
		ExBeep(1);
		memcpy((char*)sCard, card, 5);
		sprintf(buf,"%s     ", LoadStrByID(HID_CARD_NO));
		sprintf((BYTE*)buf+gLCDCharWidth-10, "%010u", *((U32*)card));
		LCDWriteCenterStr(row, buf);
		DelayMS(50);
	}
}

int RunUNumHIDMsg(PMsg msg)
{
	PInputBox box=(PInputBox)msg->Object;
	if(MSG_TYPE_HID==msg->Message)
	{
		BYTE card[5];
		TUser user;
		int Enrolled;
		
		((U32*)card)[0]=msg->Param2;
		card[4]=msg->Param1&0xFF;
		Enrolled=(0!=FDB_GetUserByCard(card, &user));
		if((box->ValidFun==IsUsedPIN) || (box->ValidFun==IsUsedPIN2))
		{
			if(!Enrolled)
			{
				ShowERROR(box->Row-1, HID_CARD_NOTENROLLED);
			}
			else
			{
				char buf[30];
				ShowHIDOK(box->Row-1, card);
				if(box->ValidFun==IsUsedPIN2)
				{
					sprintf(buf, "%%0%dd", box->MaxLength);
					sprintf(box->Text, buf, user.PIN2);
				}
				else
					sprintf(box->Text, "%05d", user.PIN);
				box->SelectStart=0;
				box->SelectLength=box->MaxLength;
				box->TopIndex=0;
				ShowInput(box);
			}
		}
		else if(Enrolled)
		{
			ShowERROR(box->Row-1, HID_CARD_ENROLLED);
			if(gOptions.VoiceOn&&gOptions.IsOnlyRFMachine) 
				ExPlayVoice(VOICE_REPEAT_FP);
			else
				ExBeep(1);			
		}
		else
		{
			ShowHIDOK(box->Row-1, card);
		}
		msg->Message=0;
		msg->Param1=0;
		return 1;
	}
	return 0;
}

int InputUNumber(int row, int col, int width, int flag, void* u)
{
	int i, ret=0,mm=0;
        unsigned char box_t[sizeof(TInputBox)];
        PInputBox box = (PInputBox)box_t;

//	PInputBox box;
	PUser user=(PUser)u;

	//如果不是新用户，则必须先存在用户
	if(0==(flag & INPUT_USER_NEW))
	{
		if(FDB_CntUser()==0)
			return News_ErrorInput;
	}
//	box=(PInputBox)malloc(sizeof(TInputBox));	//treckle
	memset(box,0,sizeof(TInputBox));
	box->MaxLength=gOptions.PIN2Width;
	box->Row=row;
	box->Col=col;
	box->SelectStart=0;
	box->SelectLength=strlen(box->Text);
	box->Width=width;
	box->Style=InputStyle_ANumber;
	box->MinValue=1;
	if(flag & INPUT_USER_PIN2)
	{
		if(flag & INPUT_USER_NEW)
			box->ValidFun=IsFreePIN2;
		else
			box->ValidFun=IsUsedPIN2;
		box->MaxValue=MAX_PIN2;
	}
	else
	{
		if(flag & INPUT_USER_NEW)
			box->ValidFun=IsFreePIN;
		else
			box->ValidFun=IsUsedPIN;
		box->MaxValue=MAX_PIN;
	}
	if((user!=NULL)) 
	{
		if(flag & INPUT_USER_PIN2)
		{
			if(!(flag & INPUT_USER_NEW))
			{
				PUser u1;
				u1=FDB_GetUser(user->PIN, NULL);
				if(u1)
					user->PIN2=u1->PIN2;
			}
			Pad0Num(box->Text, box->MaxLength, user->PIN2);
		}
		else
			Pad0Num(box->Text, box->MaxLength, user->PIN);
		if(!box->ValidFun(box->Text))
			box->Text[0]=0;
		box->SelectLength=strlen(box->Text);
	}
	if(flag & INPUT_USER_CARD)
	{
		nmemset(user->IDCard,0,3);
		sCard=user->IDCard;
		if(gOptions.MifareAsIDCard)
			mm=SelectNewMsgMask(MSG_TYPE_BUTTON|MSG_TYPE_HID|MSG_TYPE_MF);
		else
			mm=SelectNewMsgMask(MSG_TYPE_BUTTON|MSG_TYPE_HID);			
	}
	i=RegMsgProc(RunUNumHIDMsg);
	InputTimeOut=0;
	while((ret=RunInput(box))==News_ErrorInput)
	{
		ExBeep(1);			
		if(flag & INPUT_USER_NEW)
			LCDWriteCenterStrID(3, HID_ERRORPIN);
		else
			LCDWriteCenterStrID(3, HID_NOTENROLLED);
		box->SelectStart=0;
		box->SelectLength=strlen(box->Text);
		DelayMS(1000);
	}
	UnRegMsgProc(i);
	if (flag & INPUT_USER_CARD) 
		SelectNewMsgMask(mm);
	
	if(News_CommitInput==ret)
	{
		i=box->SelectStart+box->SelectLength;
		box->Text[i]=0;
		if(user)
		{
			strtou32(box->Text,(U32*)&i);
			if(flag & INPUT_USER_PIN2)
			{
				if(FDB_GetUserByPIN2((U32)i, user))
				{
					if(flag & INPUT_USER_NEW)
						ret=News_ErrorInput;
				}
				else if(0==(flag & INPUT_USER_NEW))
					ret=News_ErrorInput;
				else
					user->PIN2=(U32)i;
			}
			else
				user->PIN=i;
		}
	}
//	free(box);
	return ret;
}

int RunHIDCardMsg(PMsg msg)
{
	PInputBox box=(PInputBox)msg->Object;
	
	if(MSG_TYPE_HID==msg->Message)
	{
		BYTE card[5];
		TUser user;
		int Enrolled;
		
		((U32*)card)[0]=msg->Param2;
		card[4]=msg->Param1&0xFF;
		Enrolled=(0!=FDB_GetUserByCard(card, &user));
		if(Enrolled)
		{
			ShowERROR(box->Row-1, HID_CARD_ENROLLED);
			if(gOptions.VoiceOn&&gOptions.IsOnlyRFMachine) 
				ExPlayVoice(VOICE_REPEAT_FP);
			else
				ExBeep(1);			
		}
		else
		{
			ShowHIDOK(box->Row-1, card);
		}
		msg->Message=0;
		msg->Param1=0;
		return 1;
	}
	return 0;
}

int InputHIDCardNumber(char *title, char *hint, int flag, void *u)
{
	int i, ret=0,mm=0;
        unsigned char box_t[sizeof(TInputBox)];
        PInputBox box = (PInputBox)box_t;

//	PInputBox box;
	PUser user=(PUser)u;
//	box=(PInputBox)malloc(sizeof(TInputBox));	//treckle
	
	memset(box,0,sizeof(TInputBox));
	
	if(gOptions.PIN2Width>PIN_WIDTH) flag|=INPUT_USER_PIN2;
	LCDWriteCenterStr(0, title);
	if(gLCDRowCount-1>gLCDRowCount/2) LCDWriteCenterStr(gLCDRowCount-1, hint);
	LCDWriteCenterStr(gLCDRowCount/2, "");
	if(flag & INPUT_USER_PIN2)
		box->Col=ShowUserHint(gLCDRowCount/2, TRUE, 1);
	else 
		box->Col=ShowUserHint(gLCDRowCount/2, FALSE, 1);
	
	box->MaxLength=gOptions.PIN2Width;
	box->Row=gLCDRowCount/2;
	box->SelectStart=0;
	box->SelectLength=strlen(box->Text);
	box->Width=gOptions.PIN2Width;
	box->Style=InputStyle_Select;
	box->MinValue=1;
	
	if((user!=NULL)) 
	{
		if(flag & INPUT_USER_PIN2)
			Pad0Num(box->Text, box->MaxLength, user->PIN2);
		else
			Pad0Num(box->Text, box->MaxLength, user->PIN);
		box->SelectLength=strlen(box->Text);
	}
	
	nmemset(user->IDCard,0,3);
	sCard=user->IDCard;
	if(gOptions.MifareAsIDCard)
		mm=SelectNewMsgMask(MSG_TYPE_BUTTON|MSG_TYPE_HID|MSG_TYPE_MF);
	else
		mm=SelectNewMsgMask(MSG_TYPE_BUTTON|MSG_TYPE_HID);	
	i=RegMsgProc(RunHIDCardMsg);	
	InputTimeOut=0;
	while((ret=RunInput(box))==News_ErrorInput)
	{
		ExBeep(1);			
		DelayMS(1000);
	}
	UnRegMsgProc(i);
	SelectNewMsgMask(mm);
	
//	free(box);
	return ret;
}

int InputNumberAt(int row,int col, int width, int *number, int minv, int maxv)
{
	char buf[20],buf2[10];
	int ret;
	ret=InputNumber(row,col,width,number,minv,maxv,FALSE);
	sprintf(buf2, "%%%dd", width);
	sprintf(buf, buf2, *number);
	LCDWriteStr(row,col,buf,0);
	return ret;
}

//Repeat input a number until no error
int RepeatInputNumber(int row,int col, int width, int *number, int minv, int maxv)
{
	int ret;
	while((ret=InputNumber(row,col,width,number,minv,maxv,FALSE))==News_ErrorInput);
	return ret;
}

int ShowUserHint(int row, int IsPIN2, U32 pin)
{
	char buf[50], fbuf[20], *name;
	int l1, l2, w1, w2=5;

	name=LoadStrByID(HID_ENROLLNUM);
	w2=gOptions.PIN2Width;
	sprintf(buf, "%d", pin);
	if(w2<strlen(buf)) w2=strlen(buf);

	w1=gLangDriver->GetTextWidthFun(gLangDriver,name)/gLangDriver->CharWidth;
	if(w1+w2>=gLCDCharWidth-1)
	{
		l1=0;
		l2=gLCDCharWidth-w2;
	}
	else
	{
		l1=(gLCDCharWidth-(w1+w2+1)+1)/2;
		l2=l1+w1+1;
	}
	LCDClearLine(row);
	sprintf(fbuf, "%%%ds ", l1+strlen(name));
	sprintf(buf, fbuf, name);
	LCDWriteStr(row, 0, buf, 0);
	sprintf(fbuf, "%%0%dd", w2);
	sprintf(buf, fbuf, pin);
	LCDWriteStr(row, l2, buf, LCD_HIGH_LIGHT);
	return l2;	//返回输出号码的第一个位置
}

int InputPINBox(char *title, char *hint, int flag, void *u)
{
	TUser *user=(TUser*)u;
	int width, left;
	width=5;
	if(gOptions.PIN2Width>5)
	{
		width=gOptions.PIN2Width;
		flag|=INPUT_USER_PIN2;
	}
	if(gOptions.RFCardFunOn||gOptions.MifareAsIDCard)
		flag|=INPUT_USER_CARD;

	LCDWriteCenterStr(0, title);
	if(gLCDRowCount-1>gLCDRowCount/2) LCDWriteCenterStr(gLCDRowCount-1, hint);
	LCDWriteCenterStr(gLCDRowCount/2, "");
	if(flag & INPUT_USER_PIN2)
	{
		left=ShowUserHint(gLCDRowCount/2, TRUE, 1);
	}
	else 
		left=ShowUserHint(gLCDRowCount/2, FALSE, 1);
	return InputUNumber(gLCDRowCount/2,left,width, flag, (void*)user);
}

void LCDInfo(char *info, int DelaySec)
{
	LCDInfoShow(NULL, info);
	DelayMS(DelaySec*1000);
}

void LCDInfoShow(char *title, char *info)
{
	LCD_Clear();
	if(title) LCDWriteStr(0, 0, title, 0);	
	LCDWriteCenterStr(1, info);	
}

int LCDSelectOK(char *title, char *info, char *hint)
{
	LCD_Clear();
	if(title && gLCDRowCount>2) LCDWriteStr(0, 0, title, 0);	
	LCDWriteCenterStr(gLCDRowCount/4, info);	
	LCDFullALine(gLCDRowCount-1, hint);
	return InputLine(0,0,0,NULL);
}

int LCDSelectItem(int row, int col, int width, char **items, int itemcount, int *index)
{
	int ret,i=0;
        unsigned char box_t[sizeof(TInputBox)];
        PInputBox box = (PInputBox)box_t;

//	PInputBox box;
//	box=(PInputBox)malloc(sizeof(TInputBox));
	memset(box,0,sizeof(TInputBox));
	box->MaxLength=width;
	box->Row=row;
	box->Col=col;
	if(items)
	{
		box->Items=items;
		box->ItemCount=itemcount;
		strcpy(box->Text,items[*index]);
		box->Width=width;
		box->SelectLength=strlen(items[*index]);
	}
	box->Style=InputStyle_Select;
	ret=RunInput(box);
	if(News_CommitInput==ret)
	{
		if(items) i=SearchIndex(items, box->Text, itemcount);
		if(i>=0)
		{
			if(index) *index=i;
		}
		else
		{
			ret=News_ErrorInput;
		}
	}
//	free(box);
	return ret;	
}

#define MAXSELITEM	100

int LCDSelectItemValue(int row, int col, int width, char *items, int *values, int *value)
{
	char *(sitem[MAXSELITEM]), itembuf[MAXSELITEM*20];
	int i=0, count=0, j, ret;
	count=0;
	do
	{
		sitem[count]=itembuf+i;
		if(*items==0) break;
		j=SCopyStrFrom(sitem[count], items, 0);
		if(j==0)
		{
			items++;
		}
		else
		{
			count++;
			items+=j;
			i+=j+1;
		}
	}while(count<=MAXSELITEM);
	i=0;
	while(i<count)
		if(*value==values[i]) break; else i++;
	if(i>=count) i=0;
	do
	{
		ret=LCDSelectItem(row,col,width,sitem,count,&i);
		if(News_CancelInput==ret || News_TimeOut==ret) break;
		if(i<count)
		{
			*value=values[i];
			break;
		}
	}while(1);
	return ret;
}

char *FormatPin(char *buf, void* u, int FingerID, int FPEnrolled, int PwdEnrolled)
{
        char fbuf[40];
        PUser user=(PUser)u;
        int i=user->PIN2;
        int j=gOptions.PIN2Width;

        if((j<=PIN_WIDTH) || (i==0))
        {
                j=PIN_WIDTH;
                i=user->PIN;
        }

        if(FPEnrolled && PwdEnrolled)
                sprintf(fbuf,"%%0%dd-%dP",j,FingerID);
        else if(PwdEnrolled)
                sprintf(fbuf,"%%0%dd-P",j);
        else if(FPEnrolled)
                sprintf(fbuf,"%%0%dd-%d",j,FingerID);
		else if(user->IDCard[0]!=0 || user->IDCard[1]!=0 || user->IDCard[2]!=0)
                sprintf(fbuf,"%%0%dd-C",j);
        else
                return NULL;
        sprintf(buf, fbuf, i);
        return buf;
}

int InputWCNumber(int row, int col, int width, int *workcode)
{
	int i, ret=0,mm=0;
        unsigned char box_t[sizeof(TInputBox)];
        PInputBox box = (PInputBox)box_t;

//	PInputBox box;

//	box=(PInputBox)malloc(sizeof(TInputBox)); //treckle
	memset(box,0,sizeof(TInputBox));
	box->MaxLength=width;
	box->Row=row;
	box->Col=col;
	box->SelectStart=0;
	box->Width=width;
	box->Style=InputStyle_Number2;
	box->MinValue=1;
	box->AllowNav=0;

	if(FDB_CntWorkCode() > 0)
		box->ValidFun=IsValidWorkCode;
	box->MaxValue=MAX_PIN2;

	if(workcode) 
		sprintf(box->Text,"%d",*workcode);
	else
		sprintf(box->Text,"%d",0);

	box->SelectLength=strlen(box->Text);

	InputTimeOut=0;
	while((ret=RunInput(box))==News_ErrorInput)
	{
		LCDWriteCenterStrID(1, HID_INVALIDWORKCODE);
                if(gOptions.VoiceOn)
                        ExPlayVoice(VOICE_WORKCODE_INVALID);
                else
                        ExBeep(1);

		box->SelectStart=0;
		box->SelectLength=strlen(box->Text);
		DelayMS(1000);
	}
	
	if(News_CommitInput==ret)
	{
		i=box->SelectStart+box->SelectLength;
		box->Text[i]=0;
		if(i>0 && strtou32(box->Text,(U32*)&i)==0)
		{
			if(i<0 || i>MAX_PIN2) ret=News_ErrorInput;
			if (workcode)
				*workcode = i;
		}
		else
			ret=News_ErrorInput;
	}
//	free(box);
	return ret;
}
int CheckResult(PMenuItem mi, int Value)
{
	if(Value!=*(mi->ValueField))
	{
		int OldValue=*mi->ValueField;
		*mi->ValueField=Value;
		if((mi->ValidFun) && (FALSE==mi->ValidFun((void*)&Value)))
		{
			*mi->ValueField=OldValue;
			return News_ErrorInput;
		}
		else if(((U32)&gOptions)<=(U32)(mi->ValueField))
			if(((U32)((&gOptions)+1))>(U32)(mi->ValueField))
				gOptions.Saved=FALSE;
	}
	return News_CommitInput;
}

int SelectValueOfMenuItem(void* p)
{
	PMenu menu=((PMenu)((PMsg)p)->Object);
	PMenuItem mi=menu->Items+menu->ItemIndex;
	int row=CalcMenuItemOffset(menu, menu->ItemIndex), ret, Value;
	int col=gLCDCharWidth - mi->ValueWidth;
	Value=*mi->ValueField;
	ret=LCDSelectItemValue(row,col,mi->ValueWidth,mi->TitleItem,mi->ValueItem,&Value);
	if(ret==News_CommitInput) ret=CheckResult(mi, Value);
	return ret;
}

PMenuItem AddSelectMenuItem(PMenu menu, char *Caption, int *ValueField, char *TitleItem, int *ValueItem, int Width, int ValueCount)
{
	PMenuItem mi=AddMenuItem(1, menu, Caption, SelectValueOfMenuItem, NULL);
	mi->ValueField=ValueField;
	mi->ValueItem=ValueItem;
	mi->MaxValue=ValueCount;
	mi->TitleItem=TitleItem;
	mi->ValueWidth=Width;
	return mi;
}

static char YesNoItems[20];
static int YesNoValues[2]={0,1};

PMenuItem AddYesNoMenu(PMenu menu, char *Caption, int *ValueField)
{
	sprintf(YesNoItems, "%s:%s", LoadStrByID(HID_NO), LoadStrByID(HID_YES));
	return AddSelectMenuItem(menu, Caption, ValueField, YesNoItems, YesNoValues, 3, 2);
}

PMenuItem AddYNOptMenu(PMenu menu, int *ValueField)
{
	char *p;
	POptionsResInt optres=QueryOptResByOffset((U32)ValueField-((U32)&gOptions));
	if(optres)
	{
		if(optres->MenuResID)
			p=LoadStrByID(optres->MenuResID);
		else
			p=optres->OptionName;
		return AddYesNoMenu(menu, p, ValueField);
	}
	else
		return NULL;
}

int InputValueOfMenuItem(void *p)
{
	PMenu menu=((PMenu)((PMsg)p)->Object);
	PMenuItem mi=menu->Items+menu->ItemIndex;
	int width=mi->ValueWidth,
		minv=mi->MinValue,
		maxv=mi->MaxValue,
		*OptionValue=mi->ValueField;
	int row=CalcMenuItemOffset(menu, menu->ItemIndex), ret, Value;
	int col=gLCDCharWidth-width;
	Value=*OptionValue;
	do
	{
		ret=InputNumber(row,col,width,&Value,minv,maxv,FALSE);
		if(News_CancelInput==ret || News_TimeOut==ret) break;
	}while(ret==News_ErrorInput);
	// News_CommitInput
	if(ret==News_CommitInput) ret=CheckResult(mi, Value);
	return ret;
}

PMenuItem AddInputMenuItem(PMenu menu, char *Caption, int *ValueField, int MinValue, int MaxValue)
{
	char buf[20];
	PMenuItem mi=AddMenuItem(1, menu, Caption, InputValueOfMenuItem, NULL);
	mi->MaxValue=MaxValue;
	mi->MinValue=MinValue;
	mi->ValueField=ValueField;
	if(MaxValue)
	{
		sprintf(buf, "%d", MaxValue);
		mi->ValueWidth=strlen(buf);
	}
	else
		mi->ValueWidth=5;
	return mi;
}

PMenuItem AddInputOptMenu(PMenu menu, int *ValueField)
{
	char *p;
	POptionsResInt optres=QueryOptResByOffset((U32)ValueField-((U32)&gOptions));
	if(optres)
	{
		if(optres->MenuResID)
			p=LoadStrByID(optres->MenuResID);
		else
			p=optres->OptionName;
		return AddInputMenuItem(menu, p, ValueField, optres->MinValue, optres->MaxValue);
	}
	else
		return NULL;
}
