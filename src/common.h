#ifndef COMMON_H
#define COMMON_H

//12
//03
struct TTexture
{
	QString name;
	QImage img;
	int texNum;
	int size;
	float texVerts[8];

	int x,y;
	bool isPacked;
	bool markSelected;
	//QRectF rect;
	TTexture()
	{
		markSelected=false;
	}
};

#endif // COMMON_H