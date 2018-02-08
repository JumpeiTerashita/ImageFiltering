#include "DxLib.h"
#include "game_main.h"
#include "../support/Support.h"
#include "../support/vec3.h"
#include "../support/bitmap.h"
#include <vector>
#include <list>
#include <time.h>

#define BLOCKS 25

// 初期化フラグ
bool init = false;

FILE *fp;
Picture* p;
unsigned int *col_buff[2] = { 0 };

enum COLOR
{
	COLOR_R,
	COLOR_G,
	COLOR_B
};

void render(int px, int py, int w, int h, unsigned int* buff) {
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			DrawPixel(px + x, py + y, buff[(w * y) + x]);
		}
	}
}

int getX(int _k)
{
	int retX = _k % (p->x);
	return retX;
}

int getY(int _k)
{
	int X = getX(_k);
	int retY = (_k - X) / p->x;
	return retY;
}

unsigned char getColorBit(int _targetCOLOR, int _x, int _y, unsigned int* _colBuff)
{
	if (_x < 0 || _y < 0) return NULL;
	if (_x >= p->x || _y >= p->y) return NULL;

	int targetAddress = _y * p->x + (_x - 1);

	unsigned int maskInt = 0;

	switch (_targetCOLOR)
	{
	case 0:
		maskInt = 0x000000ff;
		break;
	case 1:
		maskInt = 0x0000ff00;
		break;
	case 2:
		maskInt = 0x00ff0000;
		break;
	}

	unsigned char retColor = ((maskInt & (_colBuff[targetAddress])) >> (8 * _targetCOLOR));

	return retColor;
}

unsigned char getColorBit(int _targetCOLOR, int _k, unsigned int* _colBuff)
{
	if (_k < 0 || _k >= (p->x * p->y)) return NULL;

	int targetAddress = _k;

	unsigned int maskInt = 0;

	switch (_targetCOLOR)
	{
	case 0:
		maskInt = 0x000000ff;
		break;
	case 1:
		maskInt = 0x0000ff00;
		break;
	case 2:
		maskInt = 0x00ff0000;
		break;
	}

	unsigned char retColor = ((maskInt & (_colBuff[targetAddress])) >> (8 * _targetCOLOR));

	return retColor;
}

int sobelFilter(int _targetCOLOR, int _k)
{
	int centerX = getX(_k);
	int centerY = getY(_k);

	 int tmpColorBit[18] = { 0 };

	tmpColorBit[0] = getColorBit(_targetCOLOR, centerX - 1	, centerY - 1	, col_buff[0]) * -1;
	tmpColorBit[1] = getColorBit(_targetCOLOR, centerX		, centerY - 1	, col_buff[0]) * 0;
	tmpColorBit[2] = getColorBit(_targetCOLOR, centerX + 1	, centerY - 1	, col_buff[0]) * 1;
	tmpColorBit[3] = getColorBit(_targetCOLOR, centerX - 1	, centerY		, col_buff[0]) * -2;
	tmpColorBit[4] = getColorBit(_targetCOLOR, centerX		, centerY		, col_buff[0]) * 0;
	tmpColorBit[5] = getColorBit(_targetCOLOR, centerX + 1	, centerY		, col_buff[0]) * 2;
	tmpColorBit[6] = getColorBit(_targetCOLOR, centerX - 1	, centerY + 1	, col_buff[0]) * -1;
	tmpColorBit[7] = getColorBit(_targetCOLOR, centerX		, centerY + 1	, col_buff[0]) * 0;
	tmpColorBit[8] = getColorBit(_targetCOLOR, centerX + 1	, centerY + 1	, col_buff[0]) * 1;

	tmpColorBit[9] = getColorBit(_targetCOLOR, centerX - 1, centerY - 1, col_buff[0]) * -1;
	tmpColorBit[10] = getColorBit(_targetCOLOR, centerX, centerY - 1, col_buff[0]) * -2;
	tmpColorBit[11] = getColorBit(_targetCOLOR, centerX + 1, centerY - 1, col_buff[0]) * -1;
	tmpColorBit[12] = getColorBit(_targetCOLOR, centerX - 1, centerY, col_buff[0]) *0;
	tmpColorBit[13] = getColorBit(_targetCOLOR, centerX, centerY, col_buff[0])*0;
	tmpColorBit[14] = getColorBit(_targetCOLOR, centerX + 1, centerY, col_buff[0])*0;
	tmpColorBit[15] = getColorBit(_targetCOLOR, centerX - 1, centerY + 1, col_buff[0])*1;
	tmpColorBit[16] = getColorBit(_targetCOLOR, centerX, centerY + 1, col_buff[0])*2;
	tmpColorBit[17] = getColorBit(_targetCOLOR, centerX + 1, centerY + 1, col_buff[0])*1;

	int sum_colorbit = 0;
	for (int i = 0; i < 18; i++)
	{
		sum_colorbit += tmpColorBit[i];
	}

	int retColorBit = sum_colorbit / 2;

	if (retColorBit > 255) retColorBit = 255;
	else if (retColorBit < 0) retColorBit = -retColorBit;
	

	return retColorBit;
}

int gaussFilter(int _targetCOLOR, int _k)
{


	int centerX = getX(_k);
	int centerY = getY(_k);

	int blockLimit = BLOCKS / 2;

	int sum_colorbit = 0;
	for (int i = 0; i < BLOCKS; i++)
	{
		for (int j = 0; j < BLOCKS; j++)
		{
			unsigned char tmpColorBit = 0;
			tmpColorBit = getColorBit(_targetCOLOR, -blockLimit + j + centerX, -blockLimit + i + centerY, col_buff[0]);
			sum_colorbit += tmpColorBit;
		}
	}


	int retColorBit = sum_colorbit / (BLOCKS*BLOCKS);


	return retColorBit;
}

//=============================================================================
// name... game_main
// work... ゲームのメインループ
// arg.... none
// ret.... [ 正常終了 : 0 ]
//=============================================================================
int game_main()
{

	//--------------------------------------------------------------------
	// 初期化
	if (!init) {
		init = true;

		fp = fopen("graphics/test.bmp", "rb");
		p = getBmp(fp);

		col_buff[0] = new unsigned int[p->x * p->y];
		col_buff[1] = new unsigned int[p->x * p->y];
		memset(col_buff[0], 0, sizeof(col_buff[0]));
		memset(col_buff[1], 0, sizeof(col_buff[1]));

		for (int k = 0; k < (p->x * p->y); ++k) {
			col_buff[0][k] = (p->b[k] << 16) | (p->g[k] << 8) | (p->r[k] << 0);
		}

		for (int k = 0; k < (p->x * p->y); ++k) {

			unsigned int retcolorBit = 0;

			int colorBit[3] = { 0 };

			for (int color = 0; color < 3; color++)
			{
				/*
								unsigned char tmpColorBit = 0;
								tmpColorBit = getColorBit(j, k, col_buff[0]);
								retcolorBit |= (tmpColorBit << (8 * j));*/

								//colorBit[color] = gaussFilter(color,k);
				colorBit[color] = sobelFilter(color,k);
				retcolorBit |= (colorBit[color] << (8 * color));

			}

			col_buff[1][k] = retcolorBit;
			//col_buff[1][k] = (p->b[k] << 16) | (p->g[k] << 8) | (p->r[k] << 0);
		}

		const int W = 25;
		const int H = 25;

		// 元画像の周囲
		int r[W * H], g[W * H], b[W * H];

		int adv[W][H][2] = { 0 };

		for (int i = 0; i < W; ++i) {
			for (int k = 0; k < H; ++k) {
				adv[i][k][0] = -((W - 1) / 2) + i;
				adv[i][k][1] = -((H - 1) / 2) + k;
			}
		}


		fclose(fp);

	}

	render(100 + 10, 20, p->x, p->y, col_buff[0]);

	render(600 + 10, 20, p->x, p->y, col_buff[1]);


	return 0;
}

