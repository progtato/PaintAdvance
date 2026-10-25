//Main file libs
#include "tonc_core.h"
#include "tonc_input.h"
#include "tonc_irq.h"
#include "tonc_memdef.h"
#include "tonc_memmap.h"
#include "tonc_oam.h"
#include "tonc_types.h"
#include "tonc_video.h"
#include <stdarg.h>
#include <strings.h>
#include <tonc.h>

//Enums
typedef enum {
	DRAWING,
	TOOL_SELECT,
	COLOUR_SELECT
} ScreenState;

typedef enum {
	BRUSH,
	LINE,
	SQUARE,
	FILLSQUARE,
	COLOURPICK
} ToolType;

typedef enum {
	SRED,
	SGREEN,
	SBLUE
} SelectedColour;

typedef struct {
	ToolType tool;
	int size;
	int y;
	int x;
	int r;
	int g;
	int b;
} TOOL;

typedef struct {
	int strX;
	int strY;
	int prs;
	int r;
	int g;
	int b;
	int prevs;
} TOOLINFO;

typedef struct {
	int clr1r;
	int clr1g;
	int clr1b;
	int clr2r;
	int clr2g;
	int clr2b;
	int clr3r;
	int clr3g;
	int clr3b;
	int clr4r;
	int clr4g;
	int clr4b;
	int sel;
} COLOURBUFFER;

//Sprites
#include "BrushSize16.h"
#include "BrushSize8.h"
#include "BrushSize4.h"
#include "BrushSize2.h"
#include "DrawMode.h"
#include "ColourMode.h"
#include "Settings.h"
#include "ArrowTool.h"
#include "DrawTool.h"
#include "SquareFillTool.h"
#include "SquareTool.h"
#include "Select.h"
#include "ColourPick.h"
//Function definitions



//CAN'T WRITE TO VRAM IN SINGLE BYTES


//Var definitions
OBJ_ATTR obj_buffer[128];
OBJ_AFFINE *obj_aff_buffer= (OBJ_AFFINE*)obj_buffer;
TOOL cursor = {BRUSH, 16, 10, 10, 12, 7, 10};
ScreenState SCREEN_STATUS = DRAWING;
SelectedColour clrS = SRED;
TOOLINFO info = {0,0,0,0,0,0,0};
COLOURBUFFER clrbuff = {0,0,0,0,0,0,0,0,0,0,0,0,1};

void drawCursor(){
	OBJ_ATTR* brush;
	if( cursor.size == 16){
		brush = &obj_buffer[0];
		obj_set_attr(brush,
		ATTR0_SQUARE,
		ATTR1_SIZE_16x16,
		ATTR2_PALBANK(0) | 512);
		//obj_set_pos(brush,cursor.x, cursor.y);

	} else if( cursor.size == 8){
		brush = &obj_buffer[0];
		obj_set_attr(brush,
		ATTR0_SQUARE,
		ATTR1_SIZE_8x8,
		ATTR2_PALBANK(0) | 516 );
		//obj_set_pos(brush, cursor.x, cursor.y);
	} else if(cursor.size == 4) {

		brush = &obj_buffer[0];
		obj_set_attr(brush,
		ATTR0_SQUARE,
		ATTR1_SIZE_8x8,
		ATTR2_PALBANK(0) | 517 );
		//obj_set_pos(brush, 10, 10);

	} else {
		brush = &obj_buffer[0];
		obj_set_attr(brush,
		ATTR0_SQUARE,
		ATTR1_SIZE_8x8,
		ATTR2_PALBANK(0) | 518 );
		//obj_set_pos(brush, 20, 20);
	}
	cursor.x += 1*key_tri_horz();

	if (cursor.x < 0) {
		cursor.x = 0;
	}
	if (cursor.x > (220-cursor.size)) {
		cursor.x = (220-cursor.size);
	}
	cursor.y += 1*key_tri_vert();
	
	if (cursor.y < 0) {
		cursor.y = 0;
	}
	if (cursor.y > (160-cursor.size)) {
		cursor.y = (160-cursor.size);
	}
	//BrushSize16->attr2 = ATTR2_BUILD(tid, pb, 10);
//	cursor.x = x;
//	cursor.y = y;
	obj_set_pos(brush, cursor.x, cursor.y);
	
}

void cursorSizeChange(){
	if(key_hit(KEY_SELECT)){
		cursor.size *= 2;
	}
	if(cursor.size > 16){
		cursor.size = 2;
	}
}
//Main loop

void changeMode(){
	if(key_hit(KEY_R)){
		if(SCREEN_STATUS == TOOL_SELECT){
			SCREEN_STATUS = DRAWING;
		} else {
			SCREEN_STATUS = TOOL_SELECT;
		}
	}
	if(key_hit(KEY_L)){
		if(SCREEN_STATUS == COLOUR_SELECT){
			SCREEN_STATUS = DRAWING;
		} else {
			SCREEN_STATUS = COLOUR_SELECT;
		}
	}
}

void draw(){
	COLOR clr;
	if(key_held(KEY_A)){
		clr = RGB15(cursor.r, cursor.g, cursor.b);
	} else {
		clr = RGB15(0,0,0);
	}
	if(key_held(KEY_B)){
		clr = RGB15(31,31,31);
	}
	if(key_held(KEY_A) || key_held(KEY_B)){
	int s = cursor.size;
	if(s == 2){
	vid_mem[(cursor.y+3)*M3_WIDTH+cursor.x+3] = clr;
	vid_mem[(cursor.y+3)*M3_WIDTH+cursor.x+4] = clr;
	vid_mem[(cursor.y+4)*M3_WIDTH+cursor.x+3] = clr;
	vid_mem[(cursor.y+4)*M3_WIDTH+cursor.x+4] = clr;
	}
	if(s == 4) {
		for (int i = 0; i<4;i++){
			for (int j = 0; j<4; j++){
				vid_mem[(cursor.y+2+j)*M3_WIDTH+cursor.x+2+i] = clr;
			}
		}
	}
	if(s == 8) {
		for (int i = 0; i<6;i++){
			for (int j = 0; j<6; j++){
				vid_mem[(cursor.y+1+j)*M3_WIDTH+cursor.x+1+i] = clr;
			}
		}
		vid_mem[(cursor.y)*M3_WIDTH+cursor.x+3] = clr;
		vid_mem[(cursor.y)*M3_WIDTH+cursor.x+4] = clr;
		vid_mem[(cursor.y+7)*M3_WIDTH+cursor.x+3] = clr;
		vid_mem[(cursor.y+7)*M3_WIDTH+cursor.x+4] = clr;
		vid_mem[(cursor.y+3)*M3_WIDTH+cursor.x+0] = clr;
		vid_mem[(cursor.y+4)*M3_WIDTH+cursor.x+0] = clr;
		vid_mem[(cursor.y+3)*M3_WIDTH+cursor.x+7] = clr;
		vid_mem[(cursor.y+4)*M3_WIDTH+cursor.x+7] = clr;
	}
	if(s == 16){
		for (int i = 0; i<12;i++){
			for (int j = 0; j<12; j++){
				vid_mem[(cursor.y+2+j)*M3_WIDTH+cursor.x+2+i] = clr;
			}
		}
		for (int i = 0; i<8; i++){
			vid_mem[(cursor.y+1)*M3_WIDTH+cursor.x+4+i] = clr;
			vid_mem[(cursor.y+14)*M3_WIDTH+cursor.x+4+i] = clr;
		}
		for (int i = 0; i<4; i++){
			vid_mem[(cursor.y+0)*M3_WIDTH+cursor.x+6+i] = clr;
			vid_mem[(cursor.y+15)*M3_WIDTH+cursor.x+6+i] = clr;
		}
		for (int i = 0; i<4; i++){
			vid_mem[(cursor.y+6+i)*M3_WIDTH+cursor.x] = clr;
			vid_mem[(cursor.y+6+i)*M3_WIDTH+cursor.x+15] = clr;
		}
		for (int i = 0; i<8; i++){
			vid_mem[(cursor.y+5+i)*M3_WIDTH+cursor.x+1] = clr;
			vid_mem[(cursor.y+5+i)*M3_WIDTH+cursor.x+14] = clr;
		}
	}
	}
	if(key_held(KEY_START)){
		m3_fill(CLR_WHITE);
	}
	if(key_held(KEY_START)&& key_held(KEY_A)){
		m3_fill(clr);
	}

}

void drawLine(){
	COLOR clr = RGB15(cursor.r, cursor.g, cursor.b);
	cursor.size = 2;
	
	if(key_hit(KEY_A) && info.prs == 0){
		info.prs = 1;
		info.strX = cursor.x;
		info.strY = cursor.y;

	} else if(key_hit(KEY_A) && info.prs == 1){
		m3_line(info.strX, info.strY, cursor.x, cursor.y,clr);
		info.prs = 0;

	}else if(key_hit(KEY_B) && info.prs == 1){
		m3_line(info.strX, info.strY, cursor.x, cursor.y,clr);

	}else if(key_hit(KEY_START) && info.prs == 1){
		m3_line(info.strX, info.strY, cursor.x, cursor.y, CLR_WHITE);

	}else if(key_hit(KEY_START) && key_hit(KEY_A) && info.prs == 1){
		m3_line(info.strX, info.strY, cursor.x, cursor.y, CLR_WHITE);
		info.prs = 0;
	}
}

void drawSquare(){
	COLOR clr = RGB15(cursor.r, cursor.g, cursor.b);
	cursor.size = 2;
	
	if(key_hit(KEY_A) && info.prs == 0){
		info.prs = 1;
		info.strX = cursor.x;
		info.strY = cursor.y;

	} else if(key_hit(KEY_A) && info.prs == 1){
		m3_rect(info.strX, info.strY, cursor.x, cursor.y,clr);
		info.prs = 0;

	}else if(key_hit(KEY_B) && info.prs == 1){
		m3_rect(info.strX, info.strY, cursor.x, cursor.y,clr);

	}else if(key_hit(KEY_START) && info.prs == 1){
		m3_rect(info.strX, info.strY, cursor.x, cursor.y, CLR_WHITE);

	}else if(key_hit(KEY_START) && key_hit(KEY_A) && info.prs == 1){
		m3_rect(info.strX, info.strY, cursor.x, cursor.y, CLR_WHITE);
		info.prs = 0;
	}
}

void drawFrame(){
	COLOR clr = RGB15(cursor.r, cursor.g, cursor.b);
	cursor.size = 2;
	
	if(key_hit(KEY_A) && info.prs == 0){
		info.prs = 1;
		info.strX = cursor.x;
		info.strY = cursor.y;

	} else if(key_hit(KEY_A) && info.prs == 1){
		m3_frame(info.strX, info.strY, cursor.x, cursor.y,clr);
		info.prs = 0;

	}else if(key_hit(KEY_B) && info.prs == 1){
		m3_frame(info.strX, info.strY, cursor.x, cursor.y,clr);

	}else if(key_hit(KEY_START) && info.prs == 1){
		m3_frame(info.strX, info.strY, cursor.x, cursor.y, CLR_WHITE);

	}else if(key_hit(KEY_START) && key_hit(KEY_A) && info.prs == 1){
		m3_frame(info.strX, info.strY, cursor.x, cursor.y, CLR_WHITE);
		info.prs = 0;
	}
}

void toolSelect(){
		
	OBJ_ATTR* brush = &obj_buffer[2];
	obj_set_attr(brush,
	ATTR0_SQUARE,
	ATTR1_SIZE_16x16,
	ATTR2_PALBANK(3) | 531);
	obj_set_pos(brush, 60, 20);

	OBJ_ATTR* line = &obj_buffer[3];
	obj_set_attr(line,
	ATTR0_SQUARE,
	ATTR1_SIZE_16x16,
	ATTR2_PALBANK(3) | 535);
	obj_set_pos(line, 90, 20);
	
	OBJ_ATTR* sqr = &obj_buffer[4];
	obj_set_attr(sqr,
	ATTR0_SQUARE,
	ATTR1_SIZE_16x16,
	ATTR2_PALBANK(3) | 539);
	obj_set_pos(sqr, 60, 70);

	OBJ_ATTR* sqrf = &obj_buffer[5];
	obj_set_attr(sqrf,
	ATTR0_SQUARE,
	ATTR1_SIZE_16x16,
	ATTR2_PALBANK(3) | 543);
	obj_set_pos(sqrf, 90, 70);


	OBJ_ATTR* sel = &obj_buffer[6];
	obj_set_attr(sel,
	ATTR0_SQUARE,
	ATTR1_SIZE_16x16,
	ATTR2_PALBANK(3) | 547);

	OBJ_ATTR* clrpick = &obj_buffer[7];
	obj_set_attr(clrpick,
	ATTR0_SQUARE,
	ATTR1_SIZE_16x16,
	ATTR2_PALBANK(3) | 551);
	obj_set_pos(clrpick,120,20);

	if(cursor.tool == BRUSH){
		obj_set_pos(sel, 60, 20);
	}
	if(cursor.tool == SQUARE){
		obj_set_pos(sel, 60, 70);
	}
	if(cursor.tool == FILLSQUARE){
		obj_set_pos(sel, 90, 70);
	}
	if(cursor.tool == LINE){
		obj_set_pos(sel, 90, 20);
	}
	if(cursor.tool == COLOURPICK){
		obj_set_pos(sel, 120, 20);
	}


	if(key_hit(KEY_UP)){
			if(cursor.tool == SQUARE){
				cursor.tool = BRUSH;
			}
			if(cursor.tool == FILLSQUARE){
				cursor.tool = LINE;
			}
	}
	if(key_hit(KEY_DOWN)){
			if(cursor.tool == BRUSH){
				cursor.tool = SQUARE;
			}
			if(cursor.tool == LINE){
				cursor.tool = FILLSQUARE;
			}
	}
	if(key_hit(KEY_LEFT)){
			if(cursor.tool == LINE){
				cursor.tool = BRUSH;

			} else if(cursor.tool == FILLSQUARE){
				cursor.tool = SQUARE;

			} else if(cursor.tool == COLOURPICK){
				cursor.tool = LINE;
			}

	}
	if(key_hit(KEY_RIGHT)){
			if(cursor.tool == BRUSH){
				cursor.tool = LINE;

			} else if(cursor.tool == SQUARE){
				cursor.tool = FILLSQUARE;

			} else if(cursor.tool == LINE){
				cursor.tool = COLOURPICK;
			}

	}
	
	
}

void colourSelect(){
	
	if(key_hit(KEY_UP)){
		if(clrS == SBLUE){
			clrS = SGREEN;
		} else if(clrS == SGREEN){
			clrS = SRED;
		} else {
			clrS = SBLUE;
		}
	}
	if(key_hit(KEY_DOWN)){
		if(clrS == SRED){
			clrS = SGREEN;
		} else if(clrS == SGREEN){
			clrS = SBLUE;
		} else{
			clrS = SRED;
		}
	}
	if(clrS == SRED){
		m3_frame(225,64,240,96, RGB15(31,31,0));
	}
	if(clrS == SGREEN){
		m3_frame(225,96,240,128, RGB15(31,31,0));	
	}
	if(clrS == SBLUE){
		m3_frame(225,128,240,160, RGB15(31,31,0));
	}
	if(key_hit(KEY_LEFT)){
		if(clrS == SRED){
			cursor.r += 1;
			if(cursor.r > 31){
				cursor.r = 31;
			}
			if (cursor.r < 0){
				cursor.r = 0;
			}
		}
		if(clrS == SGREEN){
			cursor.g += 1;
			if(cursor.g > 31){
				cursor.g = 31;
			}
			if (cursor.g < 0){
				cursor.g = 0;
			}
		}
		if(clrS == SBLUE){
			cursor.b += 1;
			if(cursor.b > 31){
				cursor.b = 31;
			}
			if (cursor.b < 0){
				cursor.b = 0;
			}
		}
	}
	if(key_hit(KEY_RIGHT)){
		if(clrS == SRED){
			cursor.r -= 1;
			if(cursor.r > 31){
				cursor.r = 31;
			}
			if (cursor.r < 0){
				cursor.r = 0;
			}
		}
		if(clrS == SGREEN){
			cursor.g -= 1;
			if(cursor.g > 31){
				cursor.g = 31;
			}
			if (cursor.g < 0){
				cursor.g = 0;
			}
		}
		if(clrS == SBLUE){
			cursor.b -= 1;
			if(cursor.b > 31){
				cursor.b = 31;
			}
			if (cursor.b < 0){
				cursor.b = 0;
			}
		}
	}
	if(key_hit(KEY_A)){
		if(clrbuff.sel == 1){
			cursor.r = clrbuff.clr1r;
			cursor.g = clrbuff.clr1g;
			cursor.b = clrbuff.clr1b;
		}
		if(clrbuff.sel == 2){
			cursor.r = clrbuff.clr2r;
			cursor.g = clrbuff.clr2g;
			cursor.b = clrbuff.clr2b;
		}
		if(clrbuff.sel == 3){
			cursor.r = clrbuff.clr3r;
			cursor.g = clrbuff.clr3g;
			cursor.b = clrbuff.clr3b;
		}
		if(clrbuff.sel == 4){
			cursor.r = clrbuff.clr4r;
			cursor.g = clrbuff.clr4g;
			cursor.b = clrbuff.clr4b;
		}

	}

	if(key_hit(KEY_B)){
		clrbuff.sel += 1;
		if(clrbuff.sel > 4){
			clrbuff.sel = 1;
		}
	}
	
	if(clrbuff.sel == 1){
		m3_frame(220,32,225,64,RGB15(31,31,00));
	}
	if(clrbuff.sel == 2){
		m3_frame(220,64,225,96,RGB15(31,31,00));
	}
	if(clrbuff.sel == 3){
		m3_frame(220,96,225,128,RGB15(31,31,00));
	}
	if(clrbuff.sel == 4){
		m3_frame(220,128,225,160,RGB15(31,31,00));
	}
	
	if(key_hit(KEY_START)){
		cursor.r = 0;
		cursor.g = 0;
		cursor.b = 0;
	}

	if(key_hit(KEY_SELECT)){
		if(clrbuff.sel == 1){
			clrbuff.clr1r = cursor.r;
			clrbuff.clr1g = cursor.g;
			clrbuff.clr1b = cursor.b;
		}
		if(clrbuff.sel == 2){
			clrbuff.clr2r = cursor.r;
			clrbuff.clr2g = cursor.g;
			clrbuff.clr2b = cursor.b;
		}
		if(clrbuff.sel == 3){
			clrbuff.clr3r = cursor.r;
			clrbuff.clr3g = cursor.g;
			clrbuff.clr3b = cursor.b;
		}
		if(clrbuff.sel == 4){
			clrbuff.clr4r = cursor.r;
			clrbuff.clr4g = cursor.g;
			clrbuff.clr4b = cursor.b;
		}
	}
}

void colourPick(){
	cursor.size = 2;
	if(key_hit(KEY_A)){
		info.r = cursor.r;
		info.b = cursor.b;
		info.g = cursor.g;
		u16 clr = vid_mem[(cursor.y+4)*M3_WIDTH+cursor.x+4];
		cursor.r = clr & 31;
		cursor.g = clr >> 5 & 31;
		cursor.b = clr >> 10 & 31;
	}
	if(key_hit(KEY_B)){
		cursor.r = info.r;
		cursor.g = info.g;
		cursor.b = info.b;
	}
	if(key_hit(KEY_SELECT)){
		cursor.tool = BRUSH;
	}
	//if(key_hit(KEY_START)){
	//	dma3_cpy(sram_mem, vid_mem, sizeof(u16)*10*10);
	//}
}

void displayStatus(){
	OBJ_ATTR* state;
	if(SCREEN_STATUS == DRAWING){
		state = &obj_buffer[1];
		obj_set_attr(state,
		ATTR0_SQUARE,
		ATTR1_SIZE_16x16,
		ATTR2_PALBANK(1) | 519);

	} 
	if(SCREEN_STATUS == COLOUR_SELECT){
		state = &obj_buffer[1];
		obj_set_attr(state,
		ATTR0_SQUARE,
		ATTR1_SIZE_16x16,
		ATTR2_PALBANK(2) | 523);
	} 
	if (SCREEN_STATUS == TOOL_SELECT){
		state = &obj_buffer[1];
		obj_set_attr(state,
		ATTR0_SQUARE,
		ATTR1_SIZE_16x16,
		ATTR2_PALBANK(3) | 527);
	};
	obj_set_pos(state,222,7);

	m3_frame(220,0,240,32, RGB15(20,20,20));	
	m3_rect(225,32,240,64, RGB15(cursor.r,cursor.g,cursor.b));
	//m3_frame(220,32,240,64, RGB15(20,20,20));
	m3_rect(225,64,240,96, RGB15(cursor.r,0,0));
	//m3_frame(220,64,240,96, RGB15(20,20,20));
	m3_rect(225,96,240,128, RGB15(0,cursor.g,0));	
	//m3_frame(220,96,240,128, RGB15(20,20,20));	
	m3_rect(225,128,240,160, RGB15(0,0,cursor.b));
	//m3_frame(220,128,240,160, RGB15(20,20,20));	
	m3_rect(220,32,225,64,RGB15(clrbuff.clr1r, clrbuff.clr1g, clrbuff.clr1b));
	m3_rect(220,64,225,96,RGB15(clrbuff.clr2r, clrbuff.clr2g, clrbuff.clr2b));
	m3_rect(220,96,225,128,RGB15(clrbuff.clr3r, clrbuff.clr3g, clrbuff.clr3b));
	m3_rect(220,128,225,160,RGB15(clrbuff.clr4r, clrbuff.clr4g, clrbuff.clr4b));
}

int main(){
	irq_init(NULL);
	irq_add(II_VBLANK, NULL);
	
	//Load Tiles
	//Brush 16
	memcpy32(&tile_mem[5][0], BrushSize16Tiles, BrushSize16TilesLen / sizeof(u32));
	memcpy16(&pal_obj_mem[0], BrushSize16Pal, BrushSize16PalLen/ sizeof(u16));
	//Brush 8
	memcpy32(&tile_mem[5][4], BrushSize8Tiles, BrushSize8TilesLen / sizeof(u32));
	//memcpy16(pal_obj_mem, BrushSize8Pal, BrushSize8PalLen/ sizeof(u16));
	//Brush 4
	memcpy32(&tile_mem[5][5], BrushSize4Tiles, BrushSize4TilesLen / sizeof(u32));
	//memcpy16(pal_obj_mem, BrushSize4Pal, BrushSize4PalLen/ sizeof(u16));
	//Brush 4
	memcpy32(&tile_mem[5][6], BrushSize2Tiles, BrushSize2TilesLen / sizeof(u32));
	//memcpy16(pal_obj_mem, BrushSize2Pal, BrushSize2PalLen/ sizeof(u16));
	//Screen Modes
	//Draw
	memcpy32(&tile_mem[5][7], DrawModeTiles, DrawModeTilesLen / sizeof(u32));
	memcpy16(&pal_obj_mem[16], DrawModePal, DrawModePalLen/ sizeof(u16));
	//Colour
	memcpy32(&tile_mem[5][11], ColourModeTiles, ColourModeTilesLen / sizeof(u32));
	memcpy16(&pal_obj_mem[32], ColourModePal, ColourModePalLen/ sizeof(u16));
	//Settings
	memcpy32(&tile_mem[5][15], SettingsTiles, SettingsTilesLen / sizeof(u32));
	memcpy16(&pal_obj_mem[48], SettingsPal, SettingsPalLen/ sizeof(u16));
	//Tools 
	//Brush	
	memcpy32(&tile_mem[5][19], DrawToolTiles, DrawToolTilesLen / sizeof(u32));
	//LINE
	memcpy32(&tile_mem[5][23], ArrowToolTiles, ArrowToolTilesLen / sizeof(u32));
	//Square
	memcpy32(&tile_mem[5][27], SquareToolTiles, SquareToolTilesLen / sizeof(u32));
	//Squarefill
	memcpy32(&tile_mem[5][31], SquareFillToolTiles, SquareFillToolTilesLen / sizeof(u32));
	//Select
	memcpy32(&tile_mem[5][35], SelectTiles, SelectTilesLen / sizeof(u32));
	//Colour Pick
	memcpy32(&tile_mem[5][39], ColourPickTiles, ColourPickTilesLen / sizeof(u32));
	
	oam_init(obj_buffer,128);
	REG_DISPCNT = DCNT_MODE3 |DCNT_BG2 | DCNT_OBJ | DCNT_OBJ_1D;

	m3_fill(RGB15(31,31,31));
	
	//dma3_cpy(vid_mem, sram_mem, 2*10*10);
	
//	TOOL cursor = {BRUSH, 16, 10, 10, CLR_BLACK};
	while (1) {
		VBlankIntrWait();
		key_poll();
		changeMode();
		displayStatus();
		if(SCREEN_STATUS == DRAWING){
			obj_hide(&obj_buffer[2]);
			obj_hide(&obj_buffer[3]);
			obj_hide(&obj_buffer[4]);
			obj_hide(&obj_buffer[5]);
			obj_hide(&obj_buffer[6]);
			obj_hide(&obj_buffer[7]);
			cursorSizeChange();
			drawCursor();
			if(cursor.tool == BRUSH){
				draw();
			}
			if(cursor.tool == LINE){
				drawLine();
			}
			if(cursor.tool == SQUARE){
				drawFrame();
			}
			if(cursor.tool == FILLSQUARE){
				drawSquare();
			}
			if(cursor.tool == COLOURPICK){
				colourPick();
			}
		}
		if(SCREEN_STATUS == TOOL_SELECT){
			toolSelect();
		}
		if(SCREEN_STATUS == COLOUR_SELECT){
			obj_hide(&obj_buffer[2]);
			obj_hide(&obj_buffer[3]);
			obj_hide(&obj_buffer[4]);
			obj_hide(&obj_buffer[5]);
			obj_hide(&obj_buffer[6]);
			obj_hide(&obj_buffer[7]);
			colourSelect();
		}
		
		oam_copy(oam_mem, obj_buffer, 8);
	}
	return  0;
}
